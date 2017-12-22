// Created by Song Siting
// Link State Routing with Dijkstra algorithm

#ifndef ROUTE_TABLE
#define ROUTE_TABLE

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <string.h>
#include <limits.h>

using namespace std;

class RouteTable {
private:
    string mhost_ip;
    map<string, int> mRouter;
    vector<pair<string, int> > graph[10001];
    int ip_num;
public:
    RouteTable(const char* host_ip);  // Set the ip of host router.
    void addNeighborIP(char* neighbor_ip); // Add ip of neighbor.
    void addRoute(char* ip1, char* ip2, int time); // Add a route.
    void findNextIP(char* &next_ip, char* dst_ip); // Find next ip to the destination ip.
};

#endif
