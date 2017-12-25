#include "RouteTable.h"
#define INF INT32_MAX/2

int neighbor_count = 0;
// Set the ip of host router.
RouteTableDV::RouteTableDV(const char* host_ip) {
  router_ip = host_ip;
}

// Add ip of neighbor.(without push it into table)
void RouteTableDV::addNeighborIP(const char* neighbor_ip) {
  string neighbor = neighbor_ip;
  struct neighbor_item temp;
  temp.neighbor_ip = neighbor;
  time(&temp.update);
  neighbor_list.push_back(temp);
}

// Add a route item into table.
// if time_cost == INF, means ip1->ip2 down
void RouteTableDV::addRoute(const char* ip1, const char* ip2, int time_cost) {
  int dst_ip1 = -1, dst_ip2 = -1;
  int next_ip1 = -1, next_ip2 = -1;
  string sip1 = ip1, sip2 = ip2;
  if(sip1 == sip2) return;
    for (int i = 0; i < DVTable.size(); ++i)
    {
    if(DVTable[i].dst_ip == sip1) dst_ip1 = i;
    if(DVTable[i].dst_ip == sip2) dst_ip2 = i;
    if(DVTable[i].next_ip == sip1) next_ip1 = i;
    if(DVTable[i].next_ip == sip2) next_ip2 = i;
    }

    //down ip1->ip2 or ip2->ip1
    if(time_cost == INF) {
    if(next_ip1 != -1 && DVTable[next_ip1].dst_ip == sip2) {
    DVTable[next_ip1].time_cost = time_cost;
    }
    else if(next_ip2 != -1 && DVTable[next_ip2].dst_ip == sip1) 
    DVTable[next_ip2].time_cost = time_cost;
    return;
    }

    //host->ip1
    if(sip2 == router_ip) {
    //router get connect with ip1
    if(dst_ip1 == -1) {
    struct DVTableItem temp;
    temp.dst_ip = sip1;
    temp.next_ip = router_ip;
    temp.time_cost = time_cost;
    DVTable.push_back(temp);
    //router->ip1 update
    } else if(DVTable[dst_ip1].time_cost > time_cost) {
    DVTable[dst_ip1].time_cost = time_cost;
    DVTable[dst_ip1].next_ip = sip2;
    }
    //host->ip2
    } else if(sip1 == router_ip) {
  //router get connect with ip2
    if(dst_ip2 == -1) {
    struct DVTableItem temp;
    temp.dst_ip = sip2;
    temp.next_ip = router_ip;
    temp.time_cost = time_cost;
    DVTable.push_back(temp);
    //router->ip2 update
    } else if(DVTable[dst_ip2].time_cost > time_cost) {
    DVTable[dst_ip2].time_cost = time_cost;
    DVTable[dst_ip2].next_ip = sip1;
    }
    } else {
    //ip1->ip2 or ip2->ip1 in table
    //router->ip1->ip2 better
    if(next_ip1 != -1 && DVTable[next_ip1].dst_ip == sip2 && DVTable[next_ip1].time_cost > time_cost) {
    DVTable[dst_ip1].time_cost = time_cost;
    //router->ip2->ip1 better
    } else if(next_ip2 != -1 && DVTable[next_ip2].dst_ip == sip1 && DVTable[next_ip2].time_cost > time_cost) {
    DVTable[next_ip2].time_cost = time_cost;
    }
    //router->x->ip1 and router->x->ip2 in table while router->ip1->ip2 or router->ip2->ip1 not
    if(dst_ip1 != -1 && dst_ip2 != -1 && DVTable[next_ip1].dst_ip != sip2 && DVTable[next_ip2].dst_ip != sip1) {
    //router->ip2->ip1 better than router->x->ip1
    if(DVTable[dst_ip1].time_cost > DVTable[dst_ip2].time_cost + time_cost) {
    DVTable[dst_ip1].next_ip = sip2;
    DVTable[dst_ip1].time_cost = time_cost;
    //router->ip1->ip2 better than router->x->ip2
    } else if(DVTable[dst_ip2].time_cost > DVTable[dst_ip1].time_cost + time_cost) {
    DVTable[dst_ip2].next_ip = sip1;
    DVTable[dst_ip2].time_cost = time_cost;
    }
    }
    //router->x->ip1 in table while router->ip2 not
    if(dst_ip1 != -1 && dst_ip2 == -1) {
    struct DVTableItem temp;
    temp.dst_ip = sip2;
    temp.next_ip = sip1;
    temp.time_cost = time_cost;
    DVTable.push_back(temp);
    }
    //router->x->ip2 in table while router->ip1 not
    if(dst_ip2 != -1 && dst_ip1 == -1) {
    struct DVTableItem temp;
    temp.dst_ip = sip1;
    temp.next_ip = sip2;
    temp.time_cost = time_cost;
    DVTable.push_back(temp);
    }
    }
}

// Find next ip to the destination ip.
// next_ip = host_ip while destionation is router's neighbor.
// next_ip = "" while there is no way to go.
void RouteTableDV::findNextIP(char* &next_ip, const char* dst_ip) {
  string sdst_ip = dst_ip;
  for (int i = 0; i < DVTable.size(); ++i)
  {
  if(DVTable[i].dst_ip == sdst_ip && DVTable[i].time_cost != INF) {
  const char* temp = DVTable[i].next_ip.c_str();
  strcpy(next_ip, temp);
  return;
  }
  }
  strcpy(next_ip, "");
}