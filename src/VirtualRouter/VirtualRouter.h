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
////////////////////////////////
#include <iostream>

using namespace std;

// TODO: Include route class

/**
 * Message that communicate between routers with.
 * code: 
 *  100: Detect neighbor msg.
 *  101: Echo detect neighbor msg.
 *  200: Send msg to dst_host.
 *  300: Change route table.
 */
struct msg_package {
    char code[4];  
    char src_host[32];
    char dst_host[32];
    char msg[128];
};

/**
 * Manage status of neighbor server.
 */
struct neighbor_status {
    char neighbor_ip[32];
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

    const char* SERVER_IP = "127.0.0.1";
    const int SERVER_PORT = 2333;
    struct sockaddr_in server_address;   // Local address used to receive msg.
    int server_socket;

    const char* CLIENT_IP = "127.0.0.1";
    const int CLIENT_PORT = 23333;
    static struct sockaddr_in client_address;   // Local address used to send msg.


    // TODO: Complete multi neighbor.
    static struct neighbor_status* neighbor_list;
    static int neighbor_count;

    static queue<struct msg_package*> sending_msg_buf;
    static pthread_mutex_t buf_mutex;
    static pthread_cond_t buf_cond;
    static char* transport_result;
    
    
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
};


#endif
