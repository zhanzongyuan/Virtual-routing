#ifndef ROUTE_TABLE_DV
#define ROUTE_TABLE_DV

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <string>
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* for memset() */
#include <signal.h>     /* for signal() and SIGALRM */
#include <errno.h>      /* for errno */

using namespace std;

struct DVTableItem
{
  string next_ip;
  string dst_ip;
  int time_cost;
};

/*
 * 1. innitialize the route
 * 2. listen for table require
 * 3. send table requist to neighbor
 *     do loop(every 30s) 
 *         send DV table to neighbor
 *     do loop(every 180s)
 *        delete unreachable neighbor
*/
class RouteTableDV {
private:
  string router_ip; // Local router address.

  std::vector<string> neighbor_list; //neighbors that directly connect to this router.

  // DVTableItem.next_ip DVTableItem.dst_ip DVTableItem.time_cost
  vector<struct DVTableItem> DVTable;
public:
  RouteTableDV(const char* host_ip);  // Set the ip of host router.
  void addNeighborIP(const char* neighbor_ip); // Add ip of neighbor.
  void addRoute(const char* ip1, const char* ip2, int time_cost); // Add a route.
  void findNextIP(char* &next_ip, const char* dst_ip); // Find next ip to the destination ip.
};
#endif