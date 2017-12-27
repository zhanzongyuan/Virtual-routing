//
//  VirtualRouter.h
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//
#ifndef MY_ROUTER
#define MY_ROUTER
////////////////////////////////
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind, and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() and getpid() */
#include <fcntl.h>      /* for fcntl() */
#include <sys/file.h>   /* for O_NONBLOCK and FASYNC */
#include <signal.h>     /* for signal() and SIGALRM */
#include <errno.h>      /* for errno */

#include<unistd.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<pthread.h>
#include<queue>
#include<string>
#include<map>
////////////////////////////////
#include <iostream>
#include "../VirtualMessage/VirtualMessage.h"
#include "../RouteTableRCC/RouteTableRCC.h"

using namespace std;

/**
 * Message that communicate between routers with.
 * code: 
 *  000: Broadcast route table in LS mode.
 *  100: Tell neighbors status change in DV mode.
 *  200: Send msg to dst_host.
 *  300: Detect message.
 *  301: Reply ok message.
 *  400: Routing Control Center message to renew table.
 *
 * Command set :
 *  send
 *  route
 *  router
 *  config
 *  exit
 *  help
 *
 */


/**
 * Manage status of neighbor server.
 */
struct neighbor_status {
    char neighbor_ip[16];
    int neighbor_port;
    struct sockaddr_in neighbor_address;  // Neigbor address used to send msg.
    int client_socket;
    bool is_connected;
    // TODO: Record time when detect neighbor that can be used to judge if timeout.
    
};


/**
 * This the class simulate router by socket connection.
 * You need to setNeighborAddress() first, then launch router by launchRouter().
 */
class VirtualRouter {
    
public:
    /**
     * Set neighbor address (ip, port) to make prepare with neighbor.
     */
    void addNeighborRouter(const char* neighbor_ip, int neighbor_port);
    /**
     * Launch router to connect with neighbor router.
     */
    void launchRouter();
    VirtualRouter();
    ~VirtualRouter();
private:
    const int QUEUE_SIZE = 20;
    
    static const char* RCC_IP;
    static const int RCC_PORT;

    static const char* SERVER_IP;
    static const int SERVER_PORT;
    struct sockaddr_in server_address;   // Local address used to receive msg.
    int server_socket;

    static const char* CLIENT_IP;
    static const int CLIENT_PORT;
    static struct sockaddr_in client_address;   // Local address used to send msg.


    // TODO: Complete multi neighbor.
    static struct neighbor_status* neighbor_list;
    static int neighbor_count;

    static queue<VirtualMessage*> sending_msg_buf;
    static pthread_mutex_t buf_mutex;
    static pthread_cond_t buf_cond;
    static char* transport_result;
    
    static int debug_mode;
    static string routing_algo;
    
    static map<string, string> broadcast_mark;
    
    static RouteTableRCC *rcc_route_table;
    
    /**
     * Thread to send data in loop.
     */
    static void* sendData(void *fd);
    
    /**
     * This function pointer is used to wait for connection from neighbor.
     */
    static void *startListenPort(void *v_server_socket);
    /**
     * Deal with message receive.
     */
    static void *receiveData(void *v_session_socket);
    /**
     * This function detect neighbors connectable periodically.
     */
    static void *detectNeighbor(void* fd);
    
    
    /**
     * Rebuild some socket when the connect is done.
     */
    static void rebuildNeighborSocket(int i);
    /**
     * Try to connect all neighbors.
     */
    void connectAllNeighbors();
    
    
    
    /**
     * Initial the address of server.
     */
    void initialServerAddress();
    /**
     * Initial the address of client.
     */
    void initialClientAddress();
    
    /**
     * Create socket for receive socket.
     * In this case, server plays as the server that listens request from other client.
     * The server will use the socket to accept connection request from other client.
     * Bind the server address with server_socket
     */
    void bindServerSocket();
    
    /**
     * Create neighbor socket that used to request connection to neighbor.
     */
    void bindClientSocket();
    
    
    /**
     * Print result of transport.
     */
    void printResult(bool has_result);
    
    
    /**
     * Input IO manage
     */
    void commandIOManage();
    bool isValidCommand(string command);
    void executeCommand(string command);
};

#endif
