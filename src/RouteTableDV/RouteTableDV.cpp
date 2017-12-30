#include "RouteTableDV.h"
#define INF INT32_MAX/2

// Set the ip of host router.
RouteTableDV::RouteTableDV(const char* host_ip) {
    router_ip = host_ip;
}

// Add a route item into table.
// if cost == INF, means ip1->ip2 down
void RouteTableDV::addRoute(const char* ip1, const char* ip2, int cost) {
    int dst_ip1 = -1, dst_ip2 = -1;
    int next_ip1 = -1, next_ip2 = -1;
    string sip1 = ip1, sip2 = ip2;
    if(sip1 == sip2) return;
    for (int i = 0; i < DVTable.size(); ++i) {
        if(DVTable[i].dst_ip == sip1) dst_ip1 = i;
        if(DVTable[i].dst_ip == sip2) dst_ip2 = i;
        if(DVTable[i].next_ip == sip1) next_ip1 = i;
        if(DVTable[i].next_ip == sip2) next_ip2 = i;
    }

    //down ip1->ip2 or ip2->ip1
    if(cost == INF) {
        if(next_ip1 != -1 && DVTable[next_ip1].dst_ip == sip2) {
            DVTable[next_ip1].cost = cost;
        }
        else if(next_ip2 != -1 && DVTable[next_ip2].dst_ip == sip1)
            DVTable[next_ip2].cost = cost;
        return;
    }

    //host->ip1
    if(sip2 == router_ip) {
    //router get connect with ip1
        if(dst_ip1 == -1) {
            struct DVTableItem temp;
            temp.dst_ip = sip1;
            temp.next_ip = router_ip;
            temp.cost = cost;
            DVTable.push_back(temp);
            //router->ip1 update
        } else if(DVTable[dst_ip1].cost > cost) {
            DVTable[dst_ip1].cost = cost;
            DVTable[dst_ip1].next_ip = sip2;
        }
        //host->ip2
    } else if(sip1 == router_ip) {
      //router get connect with ip2
        if(dst_ip2 == -1) {
            struct DVTableItem temp;
            temp.dst_ip = sip2;
            temp.next_ip = router_ip;
            temp.cost = cost;
            DVTable.push_back(temp);
            //router->ip2 update
        } else if(DVTable[dst_ip2].cost > cost) {
            DVTable[dst_ip2].cost = cost;
            DVTable[dst_ip2].next_ip = sip1;
        }
    } else {
        //ip1->ip2 or ip2->ip1 in table
        //router->ip1->ip2 better
        if(next_ip1 != -1 && DVTable[next_ip1].dst_ip == sip2 && DVTable[next_ip1].cost > cost) {
            DVTable[dst_ip1].cost = cost;
            //router->ip2->ip1 better
        } else if(next_ip2 != -1 && DVTable[next_ip2].dst_ip == sip1 && DVTable[next_ip2].cost > cost) {
            DVTable[next_ip2].cost = cost;
        }
        //router->x->ip1 and router->x->ip2 in table while router->ip1->ip2 or router->ip2->ip1 not
        if(dst_ip1 != -1 && dst_ip2 != -1 && DVTable[next_ip1].dst_ip != sip2 && DVTable[next_ip2].dst_ip != sip1) {
            //router->ip2->ip1 better than router->x->ip1
            if(DVTable[dst_ip1].cost > DVTable[dst_ip2].cost + cost) {
                DVTable[dst_ip1].next_ip = sip2;
                DVTable[dst_ip1].cost = cost;
                //router->ip1->ip2 better than router->x->ip2
            } else if(DVTable[dst_ip2].cost > DVTable[dst_ip1].cost + cost) {
                DVTable[dst_ip2].next_ip = sip1;
                DVTable[dst_ip2].cost = cost;
            }
        }
        //router->x->ip1 in table while router->ip2 not
        if(dst_ip1 != -1 && dst_ip2 == -1) {
            struct DVTableItem temp;
            temp.dst_ip = sip2;
            temp.next_ip = sip1;
            temp.cost = cost;
            DVTable.push_back(temp);
        }
        //router->x->ip2 in table while router->ip1 not
        if(dst_ip2 != -1 && dst_ip1 == -1) {
            struct DVTableItem temp;
            temp.dst_ip = sip1;
            temp.next_ip = sip2;
            temp.cost = cost;
            DVTable.push_back(temp);
        }
    }
}

// Find next ip to the destination ip.
// next_ip = dst_ip while destionation is router's neighbor.
// next_ip = "" while there is no way to go.
void RouteTableDV::findNextIP(char next_ip[], const char* dst_ip) {
    string sdst_ip = dst_ip;
    for (int i = 0; i < DVTable.size(); ++i)
    {
        //the line isn't down
        if(DVTable[i].cost != INF) {
            //find destination ip in table
            if(DVTable[i].dst_ip == sdst_ip) {
                //next ip isn't the destination itself
                if(DVTable[i].next_ip != router_ip) {
                    const char* temp = DVTable[i].next_ip.c_str();
                    strcpy(next_ip, temp);
                //next ip is the destination ip
                } else {
                    strcpy(next_ip, dst_ip);
                }
                return;
            }
        }
    }
    strcpy(next_ip, "");
}

//show route message right now.
void RouteTableDV::print() {
    int count = DVTable.size();
    printf("\n");
    printf("  Next  Address  |  Goal  Address  | Cost\n");
    printf("-----------------|-----------------|-----\n");
    for (int i = 0; i < count; i++) {
        printf("%17s|%17s|%5d\n", DVTable[i].next_ip.c_str(),
         DVTable[i].dst_ip.c_str(), DVTable[i].cost);
    }
    printf("\n");
}

//remove one item in route table
void RouteTableDV::removeRoute(const char* ip1, const char* ip2) {
    string sip1 = ip1, sip2 = ip2;
    for (auto itor = DVTable.begin(); itor != DVTable.end(); itor++) {
        //router->ip1->ip2
        if((*itor).next_ip == sip1 && (*itor).dst_ip == sip2) {
            DVTable.erase(itor);
            return;
        }
        //router->ip2->ip1
        if((*itor).next_ip == sip2 && (*itor).dst_ip == sip1) {
            DVTable.erase(itor);
            return;
        }
    }
    //printf("Error: Route item doesn't exist\n");
}

// result format:"next_ip1;next_ip1;cost1;......;host_ipn;dst_ipn;costn;#"
string RouteTableDV::encode(vector<struct DVTableItem> table) {
    char str_table[180];
    int point = 0;
    //memset is set memory not give memory.
    memset(str_table, 0, 180);
    char str_cost[10];

    if(table.size() == 0) return "";
    for (int i = 0; i < table.size(); ++i)
    {
        strncpy(str_table+point, table[i].next_ip.c_str(), table[i].next_ip.length());
        point += table[i].next_ip.length();
        str_table[point++] = ';';

        strncpy(str_table+point, table[i].dst_ip.c_str(), sizeof(table[i].dst_ip));
        point += table[i].dst_ip.length();
        str_table[point++] = ';';
        
        sprintf(str_cost, "%d", table[i].cost); 
        string test = str_cost;
        strncpy(str_table+point, str_cost, test.length());
        point += test.length();
        str_table[point++] = ';';
    }
    str_table[point++] = '#';
    str_table[point] = '\0';
    string result = str_table;
    return result;
}

// input format:"next_ip1;dst_ip1;cost1;......;next_ipn;dst_ipn;costn;#"
vector<struct DVTableItem> RouteTableDV::decode(string str_table) {
    int point_left = 0, point_right = 0;
    vector<struct DVTableItem> table;
    struct DVTableItem tempItem;
    string temp;
    int temp_cost;

    point_right = str_table.find(';', point_left);
    for(;str_table[point_left]!='#';) {
        temp = str_table.substr(point_left, point_right - point_left);
        tempItem.next_ip = temp;
        point_left = point_right + 1;
        point_right = str_table.find(';', point_left);
        temp = str_table.substr(point_left, point_right - point_left);
        tempItem.dst_ip = temp;
        point_left = point_right + 1;
        point_right = str_table.find(';', point_left);
        temp = str_table.substr(point_left, point_right - point_left);
        //temp = temp + '\0';
        temp_cost = atoi(temp.c_str());
        tempItem.cost = temp_cost;
        table.push_back(tempItem);
        point_left = point_right + 1;
        point_right = str_table.find(';', point_left);
    }
    return table;
}

// 获取邻居的路由变化信息, 返回自己到其他路由跳数信息，如果自己到其他路由的跳数没变，则返回“”空字符。
//change is the DVTable of neighbor. Need to be decode
//func add ip1=neighbor_ip, ip2=DVTable[i].des_ip, cost=DVTable[i].cost
string RouteTableDV::routeChangeMessage(const char* neighbor_ip, string change) {
    vector<struct DVTableItem> table = decode(change);
    string before =  encode(DVTable);
    for(int i = 0; i < table.size(); i++) {
        addRoute(neighbor_ip, table[i].dst_ip.c_str(), table[i].cost);
    }
    string after = encode(DVTable);
    if(before == after) return "";
    else return after;
}

//连接邻居时更新路由表，同时返回自己到其他路由跳数信息
//the same as addNeighborIP, just plus addRoute and return DVTable.
string RouteTableDV::addNeighborIP(char* neighbor_ip, int cost) {
    string neighbor = neighbor_ip;
    auto itor=neighbor_list.begin();
    //TODO: waiting to be simple
    for(;itor!= neighbor_list.end(); itor++) {
        if((*itor) == neighbor) break;
    }
    if(itor == neighbor_list.end()) {
        neighbor_list.push_back(neighbor);
    }
    addRoute(router_ip.c_str(), neighbor_ip, cost);
    string temp =  encode(DVTable);
    return temp;
}
