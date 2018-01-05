#include "VirtualRouter.h"

const string VirtualRouter::RCC = "ROUTING_CONTROL_CENTER";
const string VirtualRouter::DV = "DISTANCE_VECTOR";
const string VirtualRouter::LS = "LINK_STATE";

char* VirtualRouter::RCC_IP = new char[16];
int VirtualRouter::RCC_PORT = 8080;

char* VirtualRouter::SERVER_IP = new char[16];
int VirtualRouter::SERVER_PORT = 2333;

char* VirtualRouter::CLIENT_IP = new char[16];
int VirtualRouter::CLIENT_PORT = 23333;

struct neighbor_status* VirtualRouter::neighbor_list = NULL;
queue<VirtualMessage*> VirtualRouter::sending_msg_buf = queue<VirtualMessage*>();
int VirtualRouter::neighbor_count = 0;

pthread_mutex_t VirtualRouter::buf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t VirtualRouter::buf_cond = PTHREAD_COND_INITIALIZER;

struct sockaddr_in VirtualRouter::client_address = {0};
char* VirtualRouter::transport_result = new char[256];

int VirtualRouter::debug_mode = 0;
// string VirtualRouter::routing_algo = VirtualRouter::DV;
// string VirtualRouter::routing_algo = VirtualRouter::LS;
string VirtualRouter::routing_algo = VirtualRouter::RCC;


RouteTableDV* VirtualRouter::dv_route_table = NULL;
RouteTableLS* VirtualRouter::ls_route_table = NULL;
RouteTableRCC* VirtualRouter::rcc_route_table = NULL;

map<string, string> VirtualRouter::broadcast_mark = map<string, string>();


VirtualRouter::VirtualRouter(const char* host_ip, 
    const int server_port, const int client_port, string routing_algo,
    const char* rcc_ip, const int rcc_port) {

    this->routing_algo = routing_algo;
    strncpy(SERVER_IP, host_ip, 16);
    strncpy(CLIENT_IP, host_ip, 16);
    SERVER_PORT = server_port;
    CLIENT_PORT = client_port;

    neighbor_list = new neighbor_status[4];
    
    if (routing_algo == VirtualRouter::DV) {
        dv_route_table = new RouteTableDV(SERVER_IP);
    }
    else if (routing_algo == VirtualRouter::LS) {
        ls_route_table = new RouteTableLS(SERVER_IP);
    }
    else if (routing_algo == VirtualRouter::RCC) {
        // Create route table in rcc mode that peer router.
        strncpy(RCC_IP, rcc_ip, 16);
        RCC_PORT = rcc_port;
        rcc_route_table = new RouteTableRCC(SERVER_IP, false);
        addNeighborRouter(RCC_IP, RCC_PORT);
    }
    broadcast_mark.insert(pair<string, string>(SERVER_IP, ""));
}
VirtualRouter::~VirtualRouter() {
    delete []neighbor_list;
    if (routing_algo == VirtualRouter::DV) ;
    else if (routing_algo == VirtualRouter::LS) ;
    else if (routing_algo == VirtualRouter::RCC) {
        delete rcc_route_table;
    }
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
    
    pthread_cancel(receiving_thread);
    pthread_cancel(detecting_thread);
    pthread_cancel(sending_thread);
    shutdown(server_socket, 2);
    close(server_socket);
    for (int i = 0; i < neighbor_count; i++) {
        if (neighbor_list[i].is_connected) {
            try {
                shutdown(neighbor_list[i].client_socket, 2);
                close(neighbor_list[i].client_socket);
            }
            catch (exception e) {
                printf("%s\n", e.what());
            }
        }
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
    char code[4];
    char next_neighbor_ip[16];
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

            if (strncmp(sending_message->getDst(), "0.0.0.0", 16) != 0) {
                
                // Find next neighbor.
                if (routing_algo == VirtualRouter::RCC) {
                    if (strncmp(sending_message->getDst(), RCC_IP, 16) != 0) {
                        rcc_route_table->findNextIP(next_neighbor_ip, sending_message->getDst());
                    }
                    else strncpy(next_neighbor_ip, sending_message->getDst(), 16);
                }
                else if (routing_algo == VirtualRouter::DV)
                    dv_route_table->findNextIP(next_neighbor_ip, sending_message->getDst());
                else if (routing_algo == VirtualRouter::LS)
                    ls_route_table->findNextIP(next_neighbor_ip, sending_message->getDst());


                if (next_neighbor_ip[0] != '\0') {
                    for (int i = 0; i < neighbor_count; i++) {
                        if (strncmp(next_neighbor_ip, neighbor_list[i].neighbor_ip, 16) == 0) {
                            next_neighbor_index.push_back(i);
                            break;
                        }
                    }
                }
            }
            else {
                // Put all index in vector.
                for (int i = 0; i < neighbor_count; i++)
                    next_neighbor_index.push_back(i);
            }
            
            // Try send message.
            if (next_neighbor_index.size() != 0) {
                for (int i = 0; i < next_neighbor_index.size(); i++)
                    if (neighbor_list[next_neighbor_index[i]].is_connected) {
                        send(neighbor_list[next_neighbor_index[i]].client_socket,
                             encode_message, VirtualMessage::STR_MSG_LEN, 0);
                        
                        if (recv(neighbor_list[next_neighbor_index[i]].client_socket, response, 256, 0) <= 0) {
                            // If send data error, it infer that the neighbor host is done.
                            rebuildNeighborSocket(next_neighbor_index[i]);
                            neighbor_list[i].is_connected = false;
                            
                            // Sending result.
                            strncpy(transport_result, "Error: Neighbor ", 256);
                            size_t result_len = strlen(transport_result);
                            strncpy(transport_result+result_len, neighbor_list[next_neighbor_index[i]].neighbor_ip, 256-result_len);
                            result_len = strlen(transport_result);
                            strncpy(transport_result+result_len, " is done", 256-result_len);
                        }
                        else {
                            strncpy(transport_result, "Send message successfully to ", 256);
                            size_t result_len = strlen(transport_result);
                            strncpy(transport_result+result_len, neighbor_list[next_neighbor_index[i]].neighbor_ip, 256-result_len);
                        }
                    }
                    else {
                        strncpy(transport_result, "Error: Connect refused from ", 256);
                        size_t result_len = strlen(transport_result);
                        strncpy(transport_result+result_len, neighbor_list[next_neighbor_index[i]].neighbor_ip, 256-result_len);
                    }
            }
            else {
                strncpy(transport_result, "Error: No route to the router ", 256);
                size_t result_len = strlen(transport_result);
                strncpy(transport_result+result_len, sending_message->getDst(), 256-result_len);
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
            if (strncmp(recvbuf, "000", 3) == 0 || strncmp(recvbuf, "111", 3) == 0) {
                // Broadcast, check if it has been broadcasted.
                string src(v_message->getDst());
                string msg(v_message->getMsg());
                
                // TODO: LS update graph and route table.
                if (strncmp(recvbuf, "000", 3) == 0)
                    ls_route_table->addRoute(v_message->getSrc(), v_message->getMsg());
                
                if (broadcast_mark.find(src) == broadcast_mark.end()) {
                    // First get broadcast message.
                    broadcast_mark.insert(pair<string, string>(src, msg));
                    // Add to message queue.
                    pthread_mutex_lock(&buf_mutex);
                    sending_msg_buf.push(v_message);
                    pthread_cond_signal(&buf_cond);
                    pthread_mutex_unlock(&buf_mutex);
                }
                else if (broadcast_mark.find(src)->second != msg) {
                    // Get new broadcast message.
                    broadcast_mark.find(src)->second = msg;
                    // Add to message queue.
                    pthread_mutex_lock(&buf_mutex);
                    sending_msg_buf.push(v_message);
                    pthread_cond_signal(&buf_cond);
                    pthread_mutex_unlock(&buf_mutex);
                }
                else {
                    // Do nothing, drop package.
                    delete v_message;
                }
            }
            else if (strncmp(recvbuf, "100", 3) == 0) {
                // mode = Distance Vector
                // TODO: Neighbor change route table, decode msg to route and update route table.
                string route_change = dv_route_table->routeChangeMessage(v_message->getSrc(),
                                                                         string(v_message->getMsg()));
                
                if (route_change.size() != 0) {
                    // If route change tell neighbor.
                    v_message->setCode("100");
                    v_message->setSrc(SERVER_IP);
                    v_message->setMsg(route_change.c_str());
                    // Add to message queue.
                    pthread_mutex_lock(&buf_mutex);
                    sending_msg_buf.push(v_message);
                    pthread_cond_signal(&buf_cond);
                    pthread_mutex_unlock(&buf_mutex);
                }
                else delete v_message;
                // BUG
            }
            else if (strncmp(recvbuf, "200", 3) == 0) {
                // If it doesn't arrives destination put it into queue
                if (strncmp(v_message->getDst(), SERVER_IP, 16) == 0) {
                    // Do nothing.
                    delete v_message;
                }
                else {
                    // Add to message queue.
                    pthread_mutex_lock(&buf_mutex);
                    sending_msg_buf.push(v_message);
                    pthread_cond_signal(&buf_cond);
                    pthread_mutex_unlock(&buf_mutex);
                }
            }
            else if (strncmp(recvbuf, "400", 3) == 0) {
                // TODO: CENTER mode
                rcc_route_table->renewRouteTable(string(v_message->getMsg()));
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
void *VirtualRouter::detectNeighbor(void* fd){
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
                    
                    // TODO: Change route table and tell neighbors route change.
                    VirtualMessage *v_message;
                    v_message = new VirtualMessage();

                    // Remove neighbor.
                    if (routing_algo == VirtualRouter::LS) {
                        // Delete a neighbor in route table.
                        ls_route_table->removeNeighbor(neighbor_list[i].neighbor_ip);
                        
                        v_message->setCode("000");
                        v_message->setSrc(SERVER_IP);
                        v_message->setDst("0.0.0.0");
                        v_message->setMsg(ls_route_table->getBroadcastMessage().c_str());
                    }
                    else if (routing_algo == VirtualRouter::DV) {
                        // Delete a neighbor in route table.
                        v_message->setCode("100");
                        v_message->setSrc(SERVER_IP);
                        v_message->setDst("0.0.0.0");
                        v_message->setMsg(dv_route_table->removeNeighbor(neighbor_list[i].neighbor_ip).c_str());
                    }
                    else if (routing_algo == VirtualRouter::RCC) {
                        // Delete a neighbor in route table.
                        if (strncmp(neighbor_list[i].neighbor_ip, RCC_IP, 16) != 0)
                            rcc_route_table->removeNeighbor(neighbor_list[i].neighbor_ip);
                        
                        // Transport link state to rcc.
                        v_message->setCode("400");
                        v_message->setSrc(SERVER_IP);
                        v_message->setDst(RCC_IP);
                        v_message->setMsg(rcc_route_table->getLinkState().c_str());
                        
                    }
                    // Add to message queue.
                    pthread_mutex_lock(&buf_mutex);
                    sending_msg_buf.push(v_message);
                    pthread_cond_signal(&buf_cond);
                    pthread_mutex_unlock(&buf_mutex);
                    
                    
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
                    perror("connect fail");
                    neighbor_list[i].is_connected = false;
                    rebuildNeighborSocket(i);
                }
                else {
                    neighbor_list[i].is_connected = true;
                    
                    // TODO: Change route table and tell neighbor route change.
                    // Transport link state to rcc.
                    VirtualMessage *v_message;
                    v_message = new VirtualMessage();
                    
                    if (routing_algo == VirtualRouter::LS) {
                        // Add a neighbor in route table.
                        ls_route_table->addNeighbor(neighbor_list[i].neighbor_ip);
                        
                        // Transport link state to rcc.
                        v_message->setCode("000");
                        v_message->setSrc(SERVER_IP);
                        v_message->setDst("0.0.0.0");
                        v_message->setMsg(ls_route_table->getBroadcastMessage().c_str());
                    }
                    else if (routing_algo == VirtualRouter::DV) {
                        // Add a neighbor in route table.
                        v_message->setCode("100");
                        v_message->setSrc(SERVER_IP);
                        v_message->setDst("0.0.0.0");
                        string _msg = dv_route_table->addNeighbor(neighbor_list[i].neighbor_ip);
                        v_message->setMsg(_msg.c_str());

                    }
                    else if (routing_algo == VirtualRouter::RCC) {
                        // Add a neighbor in route table.
                        if (strncmp(neighbor_list[i].neighbor_ip, RCC_IP, 16) != 0)
                            rcc_route_table->addNeighbor(neighbor_list[i].neighbor_ip);
                        
                        v_message->setCode("400");
                        v_message->setSrc(SERVER_IP);
                        v_message->setDst(RCC_IP);
                        v_message->setMsg(rcc_route_table->getLinkState().c_str());
                    }
                    // Add to message queue.
                    pthread_mutex_lock(&buf_mutex);
                    sending_msg_buf.push(v_message);
                    pthread_cond_signal(&buf_cond);
                    pthread_mutex_unlock(&buf_mutex);
                    
                    
                    printf("Connect neighbor %s successfully.\n", neighbor_list[i].neighbor_ip);
                    printf("router@name# ");
                    // Reflesh input to screen.
                    fflush(stdout);
                }
            }
        }
        // 1 sec = 1000 ms = 1000000 us
        // 5000000 us = 5 sec
        usleep(3000000);
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
        throw(new exception());
    }
    // Make socket can reuse the same port.
    // int opt = 1;
    // if(setsockopt(client_socket, SOL_SOCKET,SO_REUSEPORT, (const void *) &opt, sizeof(opt))){
    //     perror("setsockopt");
    //     throw(new exception());
    // }
    // bind !!!must use ::bind , for std::bind is default.
    // Just bind socket with address.
    // socklen_t addrlen = sizeof(struct sockaddr);
    // if (::bind(client_socket, (struct sockaddr *)&client_address, addrlen)) {
    //     perror("bind fail");
    //     throw(new exception());
    // }
    close(neighbor_list[i].client_socket);
    neighbor_list[i].client_socket = client_socket;
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
void VirtualRouter::bindClientSocket() {
    for (int i = 0; i < neighbor_count; i++) {
        // Create client socket that can be use to connet and send message to server.
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket == -1) {
            perror("create socket fail");
            throw(new exception());
        }
        // Make socket can reuse the same port.
        // int opt = 1;
        // if(setsockopt(client_socket, SOL_SOCKET,SO_REUSEPORT, (const void *) &opt, sizeof(opt))){
        //     perror("setsockopt");
        //     throw(new exception());
        // }
        // bind !!!must use ::bind , for std::bind is default.
        // Just bind socket with address.
        // socklen_t addrlen = sizeof(struct sockaddr);
        // if (::bind(client_socket, (struct sockaddr *)&client_address, addrlen)) {
        //     perror("bind fail");
        //     throw(new exception());
        // }
        
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


/**
 * Input IO manage.
 */
void VirtualRouter::commandIOManage() {
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
bool VirtualRouter::isValidCommand(string command) {
    if (command == "send"
        || command == "exit"
        || command == "route"
        || command == "router"
        || command == "config"
        || command == "help") return true;
    printf("Wrong command ‘%s’, try command 'help'.\n", command.c_str());
    return false;
}
void VirtualRouter::executeCommand(string command) {
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
            v_message->setCode("111");
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
        printf("Virtual Router v3.1.0\n");
        printf("Server address : %s:%d\n", SERVER_IP, SERVER_PORT);
        printf("Client address : %s:%d\n\n", CLIENT_IP, CLIENT_PORT);
        
    }
    else if (command == "route") {
        // route table
        if (routing_algo == VirtualRouter::DV) {
            dv_route_table->print();
        }
        else if (routing_algo == VirtualRouter::LS) {
            ls_route_table->printRouteTableLS();
        }
        else if (routing_algo == VirtualRouter::RCC) {
            rcc_route_table->print();
        }
    }
    else {
        printf("Command '%s' dosen't exist.\n", command.c_str());
    }
}



