#include "myRouter.h"


struct neighbor_status* myRouter::neighbor_list = NULL;
queue<struct msg_package*> myRouter::sending_msg_buf = queue<struct msg_package*>();
int myRouter::neighbor_count = 0;
pthread_mutex_t myRouter::buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t myRouter::buf_cond = PTHREAD_COND_INITIALIZER;


myRouter::myRouter() {
    neighbor_list = new neighbor_status[4];
}
myRouter::~myRouter() {
    delete []neighbor_list;
}
/**
* Set neighbor address (ip, port) to make prepare with neighbor.
*/
void myRouter::addNeighborRouter(const char* neighbor_ip, int neighbor_port) {
    strcpy(neighbor_list[neighbor_count].neighbor_ip, neighbor_ip);
    neighbor_list[neighbor_count].neighbor_port = neighbor_port;
    
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
void myRouter::launchRouter() {
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
    printf("Server wait client accept...\n");
    pthread_t thread;
    if (pthread_create(&thread, NULL, startListenPort, &server_socket) != 0) {
        perror("pthread create fail");
        return;
    }

    // Connect neighbors.
    connectedNeighbor();
    
    // Input command to send message to other neighbor.
    struct msg_package *command;
    command = new msg_package();
    printf("Server:>");
    scanf("%s",command->msg);
    // TODO: Parser command

    pthread_mutex_lock(&buf_mutex);
    sending_msg_buf.push(command);
    pthread_cond_signal(&buf_cond);
    pthread_mutex_unlock(&buf_mutex);

    
    // Thread for sending message.
    pthread_t sending_thread;
    if (pthread_create(&sending_thread, NULL, sendData, NULL)) {
        perror("pthread create fail");
        return;
    }

    command = new msg_package();
    // Loop for user to input message for sending.
    while(strncmp(command->msg, "quit", 4) != 0) {
        // Send message.
        printf("Server:>");
        scanf("%s",command->msg);
        printf("Wait to sending...\n");

        pthread_mutex_lock(&buf_mutex);
        sending_msg_buf.push(command);
        pthread_cond_signal(&buf_cond);
        pthread_mutex_unlock(&buf_mutex);

        printf("Sending successfully...\n");
        command = new msg_package();
    }
    printf("Router done...\n");

}

/**
 * Thread to send data in loop.
 */
void* myRouter::sendData(void *fd) {
    // Send message.
    while (1) {
        // pthread_mutex_lock(&buf_mutex);
        // pthread_cond_wait(&buf_cond, &buf_mutex);
        while (sending_msg_buf.size() != 0) {
            struct msg_package* sending_message = sending_msg_buf.front();
            sending_msg_buf.pop();
            
            for (int i = 0; i < neighbor_count; i++)
                if (neighbor_list[i].is_connected)
                    send(neighbor_list[i].client_socket, sending_message->msg, strlen(sending_message->msg)+1, 0);
            delete sending_message;
        }
        // pthread_mutex_unlock(&buf_mutex);
    }
}

/**
 * Thread to wait for connection from neighbor.
 */
void* myRouter::startListenPort(void *v_server_socket) {
    
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

        
        printf("\n");
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
void* myRouter::receiveData(void *v_session_socket) {
    char recvbuf[256];         // Receive message buffer.
    int session_socket = *((int*)v_session_socket);
    
    while (1) {
        // Receive message and print it.
        if (recv(session_socket, recvbuf, 256, 0) <= 0) {
            perror("\nClient done...\n");
            printf("Server:>");
            break;
        }
        else {
            printf("\nCli:> %s\n",recvbuf);
            // TODO: Parse the data and deal with it by it style.
            printf("Server:>");
        }
    }

    
    free(v_session_socket);
    close(session_socket);
    pthread_exit(NULL);
    
}


/**
 * Try to connect all neighbors.
 */
void myRouter::connectedNeighbor(){
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
void myRouter::initialServerAddress() {
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
void myRouter::initialClientAddress(){
     // Create and initial server_address.
    memset(&client_address, 0, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_addr.s_addr = inet_addr(CLIENT_IP);
    // Server_address.sin_addr.s_addr = INADDR_ANY;
    client_address.sin_port = htons(CLIENT_PORT);
    bzero(client_address.sin_zero, sizeof(client_address.sin_zero));
}   
/**
 * Initial the address of neighbor server.
 */
void myRouter::initialNeigborAddress() {
    // Initial neighbor address.
}

/**
 * Create socket for receive socket.
 * In this case, server plays as the server that listens request from other client.
 * The server will use the socket to accept connection request from other client.
 * Bind the server address with server_socket
 */
void myRouter::bindServerSocket() {
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
void myRouter::bindClientSocket() {
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
