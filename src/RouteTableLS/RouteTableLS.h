// Created by Song Siting
// Link State Routing with Dijkstra algorithm

#ifndef ROUTE_TABLE_LS
#define ROUTE_TABLE_LS

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <sstream>
#include <string.h>
#include <limits.h>

using namespace std;

class RouteTableLS {
private:
    string mhost_ip;
    vector<string> mNeighbor;
    map<string, int> mRouter;
    vector<pair<string, int> > graph[10001];
    int ip_num;
    int dis[10001];  //Stores shortest distance
    bool vis[10001]; //Determines whether the node has been visited or not
    vector<string> vi[10001];
    bool isRemove[10001];

    void dijkstra();
    string encode();
    vector<pair<string, string>> decode(string message);

public:
    RouteTableLS(const char* host_ip);  // Set the ip of host router.
    void addNeighbor(const char* neighbor_ip); // Add ip of neighbor.
    void addRoute(const char* router_ip, string message); // Add a route.
    void findNextIP(char next_ip[], const char* dst_ip); // Find next ip to the destination ip.
    void printRouteTableLS();
    void removeNeighbor(char* neighbor_ip);
    string getBroadcastMessage();
};

#endif
