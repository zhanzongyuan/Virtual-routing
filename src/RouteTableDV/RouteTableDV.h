#ifndef ROUTE_TABLE_DV
#define ROUTE_TABLE_DV

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <stdio.h>      /* for printf() and fprintf() */
#include <stdlib.h>     /* for atoi() */ 
#include <string.h>     /* for memset() */
#include <signal.h>     /* for signal() and SIGALRM */
#include <errno.h>      /* for errno */
#include <limits.h>

using namespace std;

struct DVTableItem
{
  string next_ip;
  string dst_ip;
  int cost; //Default neighbor cost is 1
};

/*
 * 1. innitialize the route
 * 2. listen for table require
 * 3. send table requist to neighbor
 *     do loop(every 30s) 
 *        send DV table to neighbor
 *     do loop(every 180s)
 *        delete unreachable neighbor
*/
class RouteTableDV {
private:
  string router_ip; // Local router address.

  vector<string> neighbor_list; //neighbors that directly connect to this router.
  vector<string> router_list;  // Router in network.
    
  // DVTableItem.next_ip DVTableItem.dst_ip DVTableItem.cost
  vector<vector<int> > cost_table;
    
  //same as DVTalbe, just the next_ip has some change;
  vector<struct DVTableItem> route_table;
    
    
  // Decode.
  vector<pair<string, int>> decode(string table_msg);
  // Encode.
  string encode();
    
  // Update route table.
  void updateRouteTable();

public:
  RouteTableDV(const char* host_ip);  // Set the ip of host router.
  //void addNeighborIP(const char* neighbor_ip); // Add ip of neighbor.
  void findNextIP(char next_ip[], const char* dst_ip); // Find next ip to the destination ip.
  // 获取邻居的路由变化信息, 返回自己到其他路由跳数信息，如果自己到其他路由的跳数没变，则返回“”空字符。
  string routeChangeMessage(const char* neighbor_ip, string change);
  //连接邻居时更新路由表，同时返回自己到其他路由跳数信息
  bool removeNeighbor(const char* neighbor_ip);
  bool addNeighbor(const char* neighbor_ip);
  void print(); //show route message right now.
  string linkState();
};
#endif
