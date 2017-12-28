#include "VirtualRCC.h"

char* VirtualRCC::SERVER_IP = new char[16];
int VirtualRCC::SERVER_PORT = 2333;

char* VirtualRCC::CLIENT_IP = new char[16];
int VirtualRCC::CLIENT_PORT = 23333;

struct neighbor_status* VirtualRCC::neighbor_list = NULL;
queue<VirtualMessage*> VirtualRCC::sending_msg_buf = queue<VirtualMessage*>();
int VirtualRCC::neighbor_count = 0;

pthread_mutex_t VirtualRCC::buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t VirtualRCC::buf_cond = PTHREAD_COND_INITIALIZER;

struct sockaddr_in VirtualRCC::client_address = {0};
char* VirtualRCC::transport_result = new char[256];

int VirtualRCC::debug_mode = 0;
string VirtualRCC::routing_algo = "CENTER";
RouteTableRCC *VirtualRCC::rcc_route_table = NULL;

map<string, string> VirtualRCC::broadcast_mark = map<string, string>();


VirtualRCC::VirtualRCC(const char* host_ip, 
        const int server_port, 
        const int client_port) {
    strncpy(SERVER_IP, host_ip, 16);
    strncpy(CLIENT_IP, host_ip, 16);
    SERVER_PORT = server_port;
    CLIENT_PORT = client_port;


    neighbor_list = new neighbor_status[4];
    
    // Create route table in rcc mode that routing control center.
    rcc_route_table = new RouteTableRCC(SERVER_IP, true);
    broadcast_mark.insert(pair<string, string>(SERVER_IP, ""));
}
VirtualRCC::~VirtualRCC() {
    delete []neighbor_list;
    delete rcc_route_table;
}
/**
* Set neighbor address (ip, port) to make prepare with neighbor.
*/
void VirtualRCC::addNeighborRouter(const char* neighbor_ip, int neighbor_port) {
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
void VirtualRCC::launchRouter() {
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
    executeCommand("config");
    
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
    commandIOManage();
    
    printf("Router done...\n");
    pthread_cancel(sending_thread);
    pthread_cancel(receiving_thread);
}

/**
 * Thread to send data in loop.
 */
void* VirtualRCC::sendData(void *fd) {
    // RCC send message to all neihgbor.
    char response[256];
    char code[4];
    vector<int> next_neighbor_index;
    char encode_message[VirtualMessage::STR_MSG_LEN];
    while (1) {
        pthread_mutex_lock(&buf_mutex);
        pthread_cond_wait(&buf_cond, &buf_mutex);
        while (sending_msg_buf.size() != 0) {
            VirtualMessage* sending_message = sending_msg_buf.front();
            sending_msg_buf.pop();
            
            // Encode message
            VirtualMessage::encode(sending_message, encode_message);
            
            next_neighbor_index.clear();
            strncpy(code, sending_message->getCode(), 4);
            // Put all index in vector.
            for (int i = 0; i < neighbor_count; i++)
                if (neighbor_list[i].is_connected) {
                    send(neighbor_list[i].client_socket,
                         encode_message, VirtualMessage::STR_MSG_LEN, 0);
                    
                    if (recv(neighbor_list[i].client_socket, response, 256, 0) <= 0) {
                        // If send data error, it infer that the neighbor host is done.
                        rebuildNeighborSocket(i);
                        neighbor_list[i].is_connected = false;
                        
                        // Sending result.
                        strncpy(transport_result, "Error: Neighbor ", 256);
                        size_t result_len = strlen(transport_result);
                        strncpy(transport_result+result_len, neighbor_list[i].neighbor_ip, 256-result_len);
                        result_len = strlen(transport_result);
                        strncpy(transport_result+result_len, " is done", 256-result_len);
                    }
                    else {
                        strncpy(transport_result, "Send message successfully to ", 256);
                        size_t result_len = strlen(transport_result);
                        strncpy(transport_result+result_len, neighbor_list[i].neighbor_ip, 256-result_len);
                    }
                }
                else {
                    strncpy(transport_result, "Error: Connect refused from ", 256);
                    size_t result_len = strlen(transport_result);
                    strncpy(transport_result+result_len, neighbor_list[i].neighbor_ip, 256-result_len);
                }
            
            
            
            delete sending_message;
        }
        pthread_mutex_unlock(&buf_mutex);
    }
}

/**
 * Thread to wait for connection from neighbor.
 */
void* VirtualRCC::startListenPort(void *v_server_socket) {
    
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
void* VirtualRCC::receiveData(void *v_session_socket) {
    char recvbuf[512];         // Receive message buffer.
    int session_socket = *((int*)v_session_socket);
    
    while (1) {
        // Receive message and print it.
        if (recv(session_socket, recvbuf, 512, 0) <= 0) {
            // Client done in silence.
            break;
        }
        else {
            if (strncmp(recvbuf, "300", 3) == 0) {
                // Get message to detect neighbor.
                send(session_socket, "301", 3, 0);
                continue;
            }
            
            // Decode receive message.
            VirtualMessage *v_message = new VirtualMessage();
            VirtualMessage::decode(v_message, recvbuf);
            v_message->print();
            if (strncmp(recvbuf, "400", 3) == 0) {
                // TODO: RCC update graph and renew table, put new table into queue.
                rcc_route_table->addLinkState(v_message->getSrc(), string(v_message->getMsg()));
                pthread_mutex_lock(&buf_mutex);
                for (int i = 0; i < neighbor_count; i++) {
                    if (neighbor_list[i].is_connected) {
                        VirtualMessage *route_message = new VirtualMessage();
                        route_message->setCode("400");
                        route_message->setSrc(SERVER_IP);
                        route_message->setDst(neighbor_list[i].neighbor_ip);
                        route_message->setMsg(rcc_route_table->getRouterTable(neighbor_list[i].neighbor_ip).c_str());
                        sending_msg_buf.push(route_message);
                    }
                }
                pthread_cond_signal(&buf_cond);
                pthread_mutex_unlock(&buf_mutex);

                delete v_message;
            }
            
            // Response request.
            send(session_socket, "301", 4, 0);
            
            // Reflesh input to screen.
            printf("\nrouter@name# ");
            fflush(stdout);
        }
    }

    
    close(session_socket);
    free(v_session_socket);
    pthread_exit(NULL);
    
}

/**
 * This function detect neighbors connectable periodically.
 */
void *VirtualRCC::detectNeighbor(void* fd){
    char response[256];
    while (1) {
        //printf("\nDetecting neighbors...\n");
        for (int i = 0; i < neighbor_count; i++) {
            if (neighbor_list[i].is_connected) {
                send(neighbor_list[i].client_socket, "300", 4, 0);
                
                if (recv(neighbor_list[i].client_socket, response, 256, 0) <= 0) {
                    close(neighbor_list[i].client_socket);
                    rebuildNeighborSocket(i);
                    neighbor_list[i].is_connected = false;
                    
                    printf("Neighbor %s done.\n", neighbor_list[i].neighbor_ip);
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
                    neighbor_list[i].is_connected = true;
                    
                    printf("Connect neighbor %s successfully.\n", neighbor_list[i].neighbor_ip);
                    printf("router@name# ");
                    // Reflesh input to screen.
                    fflush(stdout);
                }
            }
        }
        // 1 sec = 1000 ms = 1000000 us
        // 5000000 us = 5 sec
        usleep(5000000);
    }
}


/**
 * Rebuild some socket when the connect is done.
 */
void VirtualRCC::rebuildNeighborSocket(int i){
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
void VirtualRCC::connectAllNeighbors(){
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
void VirtualRCC::initialServerAddress() {
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
void VirtualRCC::initialClientAddress(){
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
void VirtualRCC::bindServerSocket() {
    // Create socket.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("create socket fail");
        throw(new exception());
    }
    // bind !!!must use ::bind , for std::bind is default.
    // Just bind socket with address.
    socklen_t addrlen = sizeof(struct sockaddr);
    if (::bind(server_socket, (struct sockaddr *)&server_address, addrlen)) {
        perror("bind fail");
        throw(new exception());
    }
}

/**
 * Create neighbor socket that used to request connection to neighbor.
 */
void VirtualRCC::bindClientSocket() {
    for (int i = 0; i < neighbor_count; i++) {
        // Create client socket that can be use to connet and send message to server.
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            perror("create socket fail");
            throw(new exception());
        }
        // Make socket can reuse the same port.
        int opt = 1;
        if(setsockopt(client_socket, SOL_SOCKET,SO_REUSEPORT, (const void *) &opt, sizeof(opt))){
            perror("setsockopt");
            throw(new exception());
        }
        // bind !!!must use ::bind , for std::bind is default.
        // Just bind socket with address.
        socklen_t addrlen = sizeof(struct sockaddr);
        if (::bind(client_socket, (struct sockaddr *)&client_address, addrlen)) {
            perror("bind fail");
            throw(new exception());
        }
        
        neighbor_list[i].client_socket = client_socket;
    }
    
}

/**
 * Print result of transport.
 */
void VirtualRCC::printResult(bool has_result) {
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


/**
 * Input IO manage.
 */
void VirtualRCC::commandIOManage() {
    string command = "";
    
    // Send message.
    do {
        printf("router@name# ");
        getline(cin, command);
    } while(!isValidCommand(command));
    
    // Loop for user to input message for sending.
    while(command != "exit") {
        executeCommand(command);
        do {
            printf("router@name# ");
            getline(cin, command);
        } while(!isValidCommand(command));
    }
}
bool VirtualRCC::isValidCommand(string command) {
    if (command == "send"
        || command == "exit"
        || command == "route"
        || command == "router"
        || command == "config"
        || command == "help") return true;
    printf("Wrong command ‘%s’, try command 'help'.\n", command.c_str());
    return false;
}
void VirtualRCC::executeCommand(string command) {
    if (command == "help") {
        // Show help.
        printf("\n");
        printf(" Command | Description \n");
        printf("---------|-------------\n");
        printf(" 'send'  | send message to router with ip. \n");
        printf(" 'router'| list neighbor routers information. \n");
        printf(" 'config'| list router config. \n");
        printf(" 'route' | list route table. \n");
        printf(" 'exit'  | shutdown router and exit system. \n");
        printf(" 'help'  | list avaliable commands in system. \n\n");
    }
    else if (command == "send") {
        // Send message to queue.
        string ip, msg;
        printf("Destination ip : ");
        getline(cin, ip);
        if (ip.size() > 15) {
            printf("Invalid ip length.\n");
            return;
        }
        printf("Message : ");
        getline(cin, msg);
        if (msg.size() > 127) {
            printf("Invalid message length.\n");
            return;
        }
        
        // Create VirtualMessage that add to queue;
        VirtualMessage *v_message;
        v_message = new VirtualMessage();
        if (ip == "0.0.0.0") {
            v_message->setCode("000");
            broadcast_mark.find(SERVER_IP)->second = msg;
        }
        else v_message->setCode("200");
        v_message->setSrc(SERVER_IP);
        v_message->setDst(ip.c_str());
        v_message->setMsg(msg.c_str());
        
        // Add to message queue.
        pthread_mutex_lock(&buf_mutex);
        sending_msg_buf.push(v_message);
        pthread_cond_signal(&buf_cond);
        pthread_mutex_unlock(&buf_mutex);
        
        // Show sending result.
        printResult(true);
    }
    else if (command == "router") {
        printf("\n");
        printf("       Router address    |  Status  \n");
        printf("-------------------------|-----------\n");
        const char *connectStatus = "connected";
        const char *disconnectStatus = "disconnected";
        for (int i = 0; i < neighbor_count; i++) {
            if (neighbor_list[i].is_connected)
                printf("%16s:%-8d| %s\n", neighbor_list[i].neighbor_ip,
                       neighbor_list[i].neighbor_port, connectStatus);
            else printf("%16s:%-8d| %s\n", neighbor_list[i].neighbor_ip,
                       neighbor_list[i].neighbor_port, disconnectStatus);
        }
        printf("\n");
    }
    else if (command == "config") {
        // Show config of router.
        printf("\n");
        printf("Virtual RCC v3.1.0\n");
        printf("Server address : %s:%d\n", SERVER_IP, SERVER_PORT);
        printf("Client address : %s:%d\n\n", CLIENT_IP, CLIENT_PORT);
        
    }
    else if (command == "route") {
        // route table
        rcc_route_table->print();
        
    }
    else {
        printf("Command '%s' dosen't exist.\n", command.c_str());
    }
}



