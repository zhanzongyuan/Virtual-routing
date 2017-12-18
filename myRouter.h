//
//  myRouter.h
//  myRouter
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
////////////////////////////////
#include <iostream>


/**
 * This the class simulate router by socket connection.
 * You need to setNeighborAddress() first, then launch router by launchRouter().
 */
class myRouter {
    
public:
    /**
     * Set neighbor address (ip, port) to make prepare with neighbor.
     */
    void setNeighborAddress(const char* neighbor_ip, int neighbor_port);
    /**
     * Launch router to connect with neigbor router.
     */
    void launchRouter();
    myRouter();
    ~myRouter();
private:
    const char* HOST_IP = "192.168.199.230";
    const int HOST_PORT = 2333;
    const int QUEUE_SIZE = 20;
    char* neighbor_ip;
    int neighbor_port;
    struct sockaddr_in host_address;
    struct sockaddr_in neighbor_address;
    int receiver_socket;
    int neigbor_socket;
    
    
    /**
     * This function pointer is used to wait for connection from neighbor.
     */
    static void *startListenPort(void *v_receiver_socket);
    /**
     * Deal with message receive.
     */
    static void *receiveData(void *v_session_socket);

    
    /**
     * Initial the address of host.
     */
    void initialHostAddress();
    /**
     * Initial the address of neigbor host.
     */
    void initialNeigborAddress();
    
    /**
     * Create socket for receive socket.
     * In this case, host plays as the server that listens request from other client.
     * The host will use the socket to accept connection request from other client.
     * Bind the host address with receiver_socket
     */
    void bindReceiverSocket();
    
    /**
     * Create neigbor socket that used to request connection to neigbor.
     */
    void createNeighborSocket();
    
};

#endif
