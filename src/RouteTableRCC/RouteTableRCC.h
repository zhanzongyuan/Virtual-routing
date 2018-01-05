// Created by Song Siting
// Link State Routing with Dijkstra algorithm

#ifndef ROUTE_TABLE_RCC
#define ROUTE_TABLE_RCC

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <string.h>
#include <limits.h>

using namespace std;

struct route {
    string dst_ip;
    string next_ip;
};


class RouteTableRCC {
private:
    string host_ip;
    bool RCC_mode;
    
    // RCC
    bool dirty;
    vector<string> routers;
    vector<vector<struct route> > routers_table;
    vector<vector<int> > graph;
    void decodeLinkState(int index, string message);
    
    // Router
    vector<struct route> router_table;
    string str_table;
    vector<string> nei_ip;
public:
    RouteTableRCC(const char* host_ip, bool RCC_mode);
    
    // RCC
    string getRouterTable(const char* ip);
    // 获取该路由的路由表
    void addLinkState(const char* router_ip, string message);
    // 获取来自其他路由(不一定是邻居)广播的信息，信息说明该路由的连接状况。更新拓扑图，更新路由表。
    void updateRouteTable();
    
    
    // Router
    void addNeighbor(char* nei_ip);
    void removeNeighbor(char* nei_ip);
    string getLinkState();
    
    void findNextIP(char next_ip[], const char* dst_ip);
    // 根据目的路由ip获取下一跳路由ip
    void renewRouteTable(string message);
    void print(); // 打印路由表
    bool isDirty();
};

#endif
