#include "myRouter.h"

myRouter::myRouter() {
    neighbor_ip = NULL;
}
myRouter::~myRouter() {
    if (neighbor_ip != NULL) delete []neighbor_ip;
}
/**
* Set neighbor address (ip, port) to make prepare with neighbor.
*/
void myRouter::setNeighborAddress(const char* neighbor_ip, int neighbor_port) {
    int n = int(strlen(neighbor_ip));
    this->neighbor_ip = new char(n);
    strcpy(this->neighbor_ip, neighbor_ip);
    this->neighbor_port = neighbor_port;
}
/**
* Launch router to connect with neigbor router.
*/
void myRouter::launchRouter() {
    if (neighbor_ip == NULL) {
        printf("Error: you need to 'setNeighborAddress(char* neighbor_ip, int neighbor_port)' first.\n");
        return;
    }
    // Initial socket for receiver.
    initialHostAddress();
    bindReceiverSocket();

    // Initial socket for sender.
    initialNeigborAddress();
    createNeighborSocket();

    // Open connection port.
    // Then listen and wait for accepting connection request.
    if (listen(receiver_socket, QUEUE_SIZE) == -1) {
        perror("listen fail");
        return;
    }
    printf("Server wait client accept...\n");
    pthread_t thread;
    if (pthread_create(&thread, NULL, startListenPort, &receiver_socket) != 0) {
        perror("pthread create fail");
        return;
    }

    // Input command to send message to other neighbor.
    char command[256];
    printf("Host:>");
    scanf("%s",command);
    // TODO: Parser command

    // Try send message to neighbor.
    socklen_t addrlen = sizeof(neighbor_address);
    while(strncmp(command, "quit", 4) != 0) {
        if(connect(neigbor_socket, (struct sockaddr*)&neighbor_address, addrlen) == -1) {
            perror("connect fail");
            printf("Host:>");
            scanf("%s",command);
            continue;
        }
        // Send message.
        send(neigbor_socket, command, strlen(command)+1, 0);
        // Connect to neigbor in one connection, close connection after sending message.
        close(neigbor_socket);
        createNeighborSocket();
        printf("Host:>");
        scanf("%s",command);
    }
    printf("Router done...\n");

}



/**
 * This function pointer is used to wait for connection from neighbor.
 */
void* myRouter::startListenPort(void *v_receiver_socket) {
    
    int receiver_socket = *(int*)v_receiver_socket;
    while(1) {
        // Wait to accept message from port.
        struct sockaddr_in client_address;
        socklen_t addrlen = sizeof(struct sockaddr);
        int *session_socket = (int*) malloc(sizeof(int));
        (*session_socket) = accept(receiver_socket, (struct sockaddr*)&client_address, &addrlen);
        
        printf("\n");
        if (*session_socket == -1) {
            perror("accept fail");
        }
        else {
            // Get and print message from client.
            printf("Accept client message from %s:%d ....\n",
                   inet_ntoa(client_address.sin_addr),
                   ntohs(client_address.sin_port));
            
            // New a thread to deal with the message from client.
            pthread_t thread;
            if (pthread_create(&thread, NULL, *receiveData, session_socket) != 0) {
                perror("pthread create fail");
                break;
            }
        }
    }
    
    free(v_receiver_socket);
    close(receiver_socket);
    pthread_exit(NULL);
    printf("Host:>");
}
/**
 * Deal with message receive.
 */
void* myRouter::receiveData(void *v_session_socket) {
    char recvbuf[256];         //receive message buffer
    int session_socket = *((int*)v_session_socket);
    
    // Receive message and print it.
    recv(session_socket, recvbuf, 256, 0);
    printf("Cli:> %s\nHost:>",recvbuf);
    // TODO: Parse the data and deal with it by it style.
    
    
    free(v_session_socket);
    close(session_socket);
    pthread_exit(NULL);
    
}


/**
 * Initial the address of host.
 */
void myRouter::initialHostAddress() {
    // Create and initial host_address.
    memset(&host_address, 0, sizeof(host_address));
    host_address.sin_family = AF_INET;
    host_address.sin_addr.s_addr = inet_addr(HOST_IP);
    // Server_address.sin_addr.s_addr = INADDR_ANY;
    host_address.sin_port = htons(HOST_PORT);
    bzero(host_address.sin_zero, sizeof(host_address.sin_zero));
}
/**
 * Initial the address of neigbor host.
 */
void myRouter::initialNeigborAddress() {
    // Initial neigbor address.
    memset(&neighbor_address, 0, sizeof(neighbor_address));
    neighbor_address.sin_family = AF_INET;
    neighbor_address.sin_addr.s_addr = inet_addr(neighbor_ip);
    neighbor_address.sin_port = htons(neighbor_port);
    bzero(neighbor_address.sin_zero, sizeof(neighbor_address.sin_zero));
}

/**
 * Create socket for receive socket.
 * In this case, host plays as the server that listens request from other client.
 * The host will use the socket to accept connection request from other client.
 * Bind the host address with receiver_socket
 */
void myRouter::bindReceiverSocket() {
    // Create socket.
    receiver_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (receiver_socket == -1) {
        perror("create socket fail");
    }
    // bind !!!must use ::bind , for std::bind is default.
    // Just bind socket with address.
    socklen_t addrlen = sizeof(struct sockaddr);
    if (::bind(receiver_socket, (struct sockaddr *)&host_address, addrlen)) {
        perror("bind fail");
    }
}

/**
 * Create neigbor socket that used to request connection to neigbor.
 */
void myRouter::createNeighborSocket() {
    // Create client socket that can be use to connet and send message to server.
    neigbor_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (neigbor_socket == -1) {
        perror("create socket fail");
    }
}