#include "VirtualRouter.h"

struct neighbor_status* VirtualRouter::neighbor_list = NULL;
queue<struct msg_package*> VirtualRouter::sending_msg_buf = queue<struct msg_package*>();
int VirtualRouter::neighbor_count = 0;
pthread_mutex_t VirtualRouter::buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t VirtualRouter::buf_cond = PTHREAD_COND_INITIALIZER;
struct sockaddr_in VirtualRouter::client_address = {0};
char* VirtualRouter::transport_result = new char[256];

VirtualRouter::VirtualRouter() {
    neighbor_list = new neighbor_status[4];
}
VirtualRouter::~VirtualRouter() {
    delete []neighbor_list;
}
/**
* Set neighbor address (ip, port) to make prepare with neighbor.
*/
void VirtualRouter::addNeighborRouter(const char* neighbor_ip, int neighbor_port) {
    strcpy(neighbor_list[neighbor_count].neighbor_ip, neighbor_ip);
    neighbor_list[neighbor_count].neighbor_port = neighbor_port;
    neighbor_list[neighbor_count].is_connected = false;
    
    sockaddr_in neighbor_address;
    memset(&neighbor_address, 0, sizeof(neighbor_address));
    neighbor_address.sin_family = AF_INET;
    neighbor_address.sin_addr.s_addr = inet_addr(neighbor_ip);
    neighbor_address.sin_port = htons(neighbor_port);
    bzero(neighbor_address.sin_zero, sizeof(neighbor_address.sin_zero));
    neighbor_list[neighbor_count].neighbor_address = neighbor_address;
    neighbor_count++;
}


/**
* Launch router to connect with neighbor router.
*/
void VirtualRouter::launchRouter() {
    if (neighbor_list == NULL) {
        printf("Error: you need to 'setNeighborAddress(char* neighbor_ip, int neighbor_port)' first.\n");
        return;
    }
    // Initial socket for server.
    initialServerAddress();
    bindServerSocket();

    // Initial socket for sender.
    initialClientAddress();
    bindClientSocket();

    // Open connection port.
    // Then listen and wait for accepting connection request.
    if (listen(server_socket, QUEUE_SIZE) == -1) {
        perror("listen fail");
        return;
    }
    printf("Router launching");
    printResult(false);
    
    pthread_t receiving_thread;
    if (pthread_create(&receiving_thread, NULL, startListenPort, &server_socket) != 0) {
        perror("Receiving pthread create fail");
        return;
    }
    
    // Thread for detect neighbor and connect them periodically.
    pthread_t detecting_thread;
    if (pthread_create(&detecting_thread, NULL, detectNeighbor, NULL)) {
        perror("Detecting pthread create fail");
        return;
    }
    
    // Thread for sending message.
    pthread_t sending_thread;
    if (pthread_create(&sending_thread, NULL, sendData, NULL)) {
        perror("Sending pthread create fail");
        return;
    }

    // Input command to send message to other neighbor.
    struct msg_package *command;
    command = new msg_package();
    // Send message.
    printf("router@name# ");
    scanf("%s",command->msg);
    
    // Loop for user to input message for sending.
    while(strncmp(command->msg, "quit", 4) != 0) {
        pthread_mutex_lock(&buf_mutex);
        sending_msg_buf.push(command);
        pthread_cond_signal(&buf_cond);
        pthread_mutex_unlock(&buf_mutex);
        
        printResult(true);

        command = new msg_package();
        // Send message.
        printf("router@name# ");
        scanf("%s",command->msg);
    }
    printf("Router done...\n");
    pthread_cancel(sending_thread);
    pthread_cancel(receiving_thread);
}

/**
 * Thread to send data in loop.
 */
void* VirtualRouter::sendData(void *fd) {
    // Send message.
    char response[256];
    while (1) {
        pthread_mutex_lock(&buf_mutex);
        pthread_cond_wait(&buf_cond, &buf_mutex);
        while (sending_msg_buf.size() != 0) {
            struct msg_package* sending_message = sending_msg_buf.front();
            sending_msg_buf.pop();
            
            for (int i = 0; i < neighbor_count; i++) {
                if (neighbor_list[i].is_connected) {
                    send(neighbor_list[i].client_socket, sending_message->msg, strlen(sending_message->msg)+1, 0);
                    
                    if (recv(neighbor_list[i].client_socket, response, 256, 0) <= 0) {
                        // If send data error, it infer that the neighbor host is done.
                        rebuildNeighborSocket(i);
                        neighbor_list[i].is_connected = false;
                        
                        strncpy(transport_result, "Error: Neighbor is done.", 256);
                    }
                    else {
                        strncpy(transport_result, "Send message successfully.", 256);
                    }
                }
                else {
                    strncpy(transport_result, "Error: Connected refused.", 256);
                }
            }
            
            delete sending_message;
        }
        pthread_mutex_unlock(&buf_mutex);
    }
}

/**
 * Thread to wait for connection from neighbor.
 */
void* VirtualRouter::startListenPort(void *v_server_socket) {
    
    int server_socket = *(int*)v_server_socket;
    while(1) {
        // Wait to accept message from port.
        struct sockaddr_in temp_client_address;
        socklen_t addrlen = sizeof(struct sockaddr);
        int *session_socket = (int*) malloc(sizeof(int));
        (*session_socket) = accept(server_socket, (struct sockaddr*)&temp_client_address, &addrlen);
        // Get and print message from client.
        printf("Accept client message from %s:%d ....\n",
               inet_ntoa(temp_client_address.sin_addr),
               ntohs(temp_client_address.sin_port));

        
        printf("router@name# ");
        // Reflesh input to screen.
        fflush(stdout);
        
        if (*session_socket == -1) {
            perror("accept fail");
        }
        else {
            // New a thread to deal with the message from client.
            pthread_t thread;
            if (pthread_create(&thread, NULL, *receiveData, session_socket) != 0) {
                perror("pthread create fail");
                break;
            }
        }
    }
    
    free(v_server_socket);
    close(server_socket);
    pthread_exit(NULL);
}
/**
 * Thread to deal with message receive.
 */
void* VirtualRouter::receiveData(void *v_session_socket) {
    char recvbuf[256];         // Receive message buffer.
    int session_socket = *((int*)v_session_socket);
    
    while (1) {
        // Receive message and print it.
        if (recv(session_socket, recvbuf, 256, 0) <= 0) {
            perror("\nClient done...\n");
            printf("\nrouter@name# ");
            
            // Reflesh input to screen.
            fflush(stdout);
            break;
        }
        else {
            if (strncmp(recvbuf, "DETECT", 4) == 0) {
                send(session_socket, "OK", 3, 0);
                continue;
            }
            printf("\nneighbor-router:  %s\n",recvbuf);
            // TODO: Parse the data and deal with it by it style.
            printf("\nrouter@name# ");
            // Response request.
            send(session_socket, "OK", 3, 0);
            
            // Reflesh input to screen.
            fflush(stdout);
        }
    }

    
    free(v_session_socket);
    close(session_socket);
    pthread_exit(NULL);
    
}

/**
 * This function detect neighbors connectable periodically.
 */
void *VirtualRouter::detectNeighbor(void* fd){
    char response[256];
    while (1) {
        //printf("\nDetecting neighbors...\n");
        for (int i = 0; i < neighbor_count; i++) {
            if (neighbor_list[i].is_connected) {
                send(neighbor_list[i].client_socket, "DETECT", strlen("DETECT")+1, 0);
                if (recv(neighbor_list[i].client_socket, response, 256, 0) <= 0) {
                    printf("Some neighbors done\n");
                    close(neighbor_list[i].client_socket);
                    rebuildNeighborSocket(i);
                    neighbor_list[i].is_connected = false;
                    printf("router@name# ");
                    // Reflesh input to screen.
                    fflush(stdout);
                }
            }
            else {
                // Try connect.
                int client_socket = neighbor_list[i].client_socket;
                if (connect(client_socket, (struct sockaddr*)&neighbor_list[i].neighbor_address, sizeof(sockaddr_in)) == -1) {
                    // perror("connect fail");
                    neighbor_list[i].is_connected = false;
                    rebuildNeighborSocket(i);
                }
                else {
                    printf("Connect successfully\n");
                    neighbor_list[i].is_connected = true;
                    printf("router@name# ");
                    // Reflesh input to screen.
                    fflush(stdout);
                }
            }
        }
        // TODO: Tell link change.
        
        // 1 sec = 1000 ms = 1000000 us
        // 5000000 us = 5 sec
        usleep(5000000);
    }
}


/**
 * Rebuild some socket when the connect is done.
 */
void VirtualRouter::rebuildNeighborSocket(int i){
    // Create client socket that can be use to connet and send message to server.
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("create socket fail");
    }
    // Make socket can reuse the same port.
    int opt = 1;
    if(setsockopt(client_socket, SOL_SOCKET,SO_REUSEPORT, (const void *) &opt, sizeof(opt))){
        perror("setsockopt");
    }
    // bind !!!must use ::bind , for std::bind is default.
    // Just bind socket with address.
    socklen_t addrlen = sizeof(struct sockaddr);
    if (::bind(client_socket, (struct sockaddr *)&client_address, addrlen)) {
        perror("bind fail");
    }
    neighbor_list[i].client_socket = client_socket;
}

/**
 * Try to connect all neighbors.
 */
void VirtualRouter::connectAllNeighbors(){
    socklen_t addrlen = sizeof(sockaddr_in);
    for (int i = 0; i < neighbor_count; i++) {
        // Connect to neighbor.
        int client_socket = neighbor_list[i].client_socket;
        if (connect(client_socket, (struct sockaddr*)&neighbor_list[i].neighbor_address, addrlen) == -1) {
            perror("connect fail");
            neighbor_list[i].is_connected = false;
        }
        neighbor_list[i].is_connected = true;
    }
}

/**
 * Initial the address of server.
 */
void VirtualRouter::initialServerAddress() {
    // Create and initial server_address.
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    // Server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(SERVER_PORT);
    bzero(server_address.sin_zero, sizeof(server_address.sin_zero));
}
/**
 * Initial the address of client.
 */
void VirtualRouter::initialClientAddress(){
     // Create and initial server_address.
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr(CLIENT_IP);
    // Server_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(CLIENT_PORT);
    bzero(client_address.sin_zero, sizeof(client_address.sin_zero));
}   

/**
 * Create socket for receive socket.
 * In this case, server plays as the server that listens request from other client.
 * The server will use the socket to accept connection request from other client.
 * Bind the server address with server_socket
 */
void VirtualRouter::bindServerSocket() {
    // Create socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("create socket fail");
    }
    // bind !!!must use ::bind , for std::bind is default.
    // Just bind socket with address.
    socklen_t addrlen = sizeof(struct sockaddr);
    if (::bind(server_socket, (struct sockaddr *)&server_address, addrlen)) {
        perror("bind fail");
    }
}

/**
 * Create neighbor socket that used to request connection to neighbor.
 */
void VirtualRouter::bindClientSocket() {
    for (int i = 0; i < neighbor_count; i++) {
        // Create client socket that can be use to connet and send message to server.
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            perror("create socket fail");
        }
        // Make socket can reuse the same port.
        int opt = 1;
        if(setsockopt(client_socket, SOL_SOCKET,SO_REUSEPORT, (const void *) &opt, sizeof(opt))){
            perror("setsockopt");
        }
        // bind !!!must use ::bind , for std::bind is default.
        // Just bind socket with address.
        socklen_t addrlen = sizeof(struct sockaddr);
        if (::bind(client_socket, (struct sockaddr *)&client_address, addrlen)) {
            perror("bind fail");
        }
        
        neighbor_list[i].client_socket = client_socket;
    }
    
}

/**
 * Print result of transport.
 */
void VirtualRouter::printResult(bool has_result) {
    for (int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        
        // 1 sec = 1000 ms = 1000000 us
        // 600000 us = 0.6 sec
        usleep(600000);

    }
    if (has_result)
        printf("\n%s\n", transport_result);
    else printf("\n");
}

