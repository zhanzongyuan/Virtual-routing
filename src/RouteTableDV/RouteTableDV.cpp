#include "RouteTableDV.h"
#define INF INT32_MAX/2

// Set the ip of host router.
RouteTableDV::RouteTableDV(const char* host_ip) {
    router_ip = string(host_ip);
    
    router_list.push_back(router_ip);
    neighbor_list.push_back(router_ip);
    
    cost_table.push_back(vector<int>());
    cost_table[0].push_back(0);
}


// result format:"next_ip1;next_ip1;cost1;......;host_ipn;dst_ipn;costn;#"
string RouteTableDV::encode() {
    string result = "";
    for (int i = 1; i < cost_table[0].size(); i++) {
        result += router_list[i];
        result += "#";
        result += to_string(cost_table[0][i]);
        result += "#";
    }
    return result;
}

vector<pair<string, int>> RouteTableDV::decode(string table_msg){
    vector<pair<string, int>> result;
    string ip = "";
    string cost = "";
    for (int i = 0; i < table_msg.size(); i++) {
        if (table_msg[i] != '#') {
            ip += table_msg[i];
        }
        else {
            i++;
            while(table_msg[i] != '#') {
                cost += table_msg[i];
                i++;
            }
            int cost_int = atoi(cost.c_str());
            result.push_back(pair<string, int>(ip, cost_int));
            ip = "";
            cost = "";
        }
    }
    return result;
}


// Find next ip to the destination ip.
// next_ip = dst_ip while destionation is router's neighbor.
// next_ip = "" while there is no way to go.
void RouteTableDV::findNextIP(char next_ip[], const char* dst_ip) {
    string dst_str = string(dst_ip);
    for (int i = 0; i < route_table.size(); i++) {
        if (route_table[i].dst_ip == dst_str) {
            strncpy(next_ip, route_table[i].next_ip.c_str(), 16);
            return;
        }
    }
    strncpy(next_ip, "", 16);
}

// 获取邻居的路由变化信息, 返回自己到其他路由跳数信息，如果自己到其他路由的跳数没变，则返回“”空字符。
//change is the DVTable of neighbor. Need to be decode
//func add ip1=neighbor_ip, ip2=DVTable[i].des_ip, cost=DVTable[i].cost
string RouteTableDV::routeChangeMessage(const char* neighbor_ip, string change) {
    string nei_str = string(neighbor_ip);
    
    vector<pair<string, int> > decoded_msg = decode(change);
    bool my_dirty = false;
    
    // Find neighbor router in which row and column.
    int nei_index = -1, router_index = -1;
    for (int i = 0; i < neighbor_list.size(); i++) {
        if (nei_str == neighbor_list[i]) {
            nei_index = i;
            break;
        }
    }
    
    // If it is new neighbor, add it.
    if (nei_index == -1) {
        // Add row.
        neighbor_list.push_back(nei_str);
        cost_table.push_back(vector<int>(router_list.size(), INT_MAX));
        nei_index = int(neighbor_list.size())-1;
        
        // Add column.
        router_list.push_back(nei_str);
        for (int i = 0 ; i < neighbor_list.size(); i++) {
            cost_table[i].push_back(INT_MAX);
        }
        router_index = int(router_list.size())-1;
    }
    else {
        for (int i = 0; i < router_list.size(); i++) {
            if (nei_str == router_list[i]) {
                router_index = i;
            }
            cost_table[nei_index][i] = INT_MAX;
        }
    }
    cost_table[0][router_index] = 1;
    cost_table[nei_index][0] = 1;
    cost_table[nei_index][router_index] = 0;
    
    // Update row of neighbor ip.
    for (int i = 0; i < decoded_msg.size(); i++) {
        int index = -1;
        for (int j = 0; j < router_list.size(); j++) {
            if (router_list[j] == decoded_msg[i].first) {
                index = j;
                break;
            }
        }
        
        if (index == -1) {
            router_list.push_back(decoded_msg[i].first);
            for (int j = 0; j < cost_table.size(); j++) {
                cost_table[j].push_back(INT_MAX);
            }
            index = int(router_list.size())-1;
        }
        cost_table[nei_index][index] = decoded_msg[i].second;
    }
    
    // Update row of host ip
    for (int i = 1; i < router_list.size(); i++) {
        if (cost_table[nei_index][i] != INT_MAX && 1 + cost_table[nei_index][i] < cost_table[0][i]) {
            cost_table[0][i] = 1 + cost_table[nei_index][i];
            my_dirty = true;
        }
    }
    
    updateRouteTable();
    if (my_dirty) return encode();
    else return "";
}

string RouteTableDV::removeNeighbor(const char* neighbor_ip) {
    int nei_index = -1;
    for (int i = 0; i < neighbor_list.size(); i++) {
        if (strncmp(neighbor_list[i].c_str(), neighbor_ip, 16) == 0) {
            nei_index = i;
            
            // Erase row.
            neighbor_list.erase(neighbor_list.begin()+i);
            cost_table[i].clear();
            cost_table.erase(cost_table.begin()+i);
            
            // Erase column.
            for (int j = 0; j < router_list.size(); j++) {
                if (strncmp(router_list[i].c_str(), neighbor_ip, 16) == 0) {
                    router_list.erase(router_list.begin()+j);
                    for (int k = 0; k < cost_table.size(); k++) {
                        cost_table[k].erase(cost_table[k].begin()+j);
                    }
                    break;
                }
            }
            // Update Route Table.
            updateRouteTable();
            break;
        }
    }
    if (nei_index == -1) return "";
    else return encode();
}
void RouteTableDV::updateRouteTable() {
    route_table.clear();
    for (int i = 1; i < router_list.size(); i++) {
        struct DVTableItem item;
        item.dst_ip = router_list[i];
        item.cost = INT_MAX;
        for (int j = 1; j < neighbor_list.size(); j++) {
            if (item.cost > cost_table[j][i]) {
                item.cost = cost_table[j][i];
                item.next_ip = neighbor_list[j];
            }
        }
        if (item.cost == INT_MAX) continue;
        route_table.push_back(item);
    }
}
string RouteTableDV::addNeighbor(const char* neighbor_ip) {
    string nei_str = string(neighbor_ip);
    neighbor_list.push_back(nei_str);
    cost_table.push_back(vector<int>(int(router_list.size()), INT_MAX));

    router_list.push_back(nei_str);
    for (int i = 0; i < neighbor_list.size(); i++) {
        cost_table[i].push_back(INT_MAX);
    }
    cost_table[0][router_list.size()-1] = 1;
    cost_table[neighbor_list.size()-1][0] = 1;
    cost_table[neighbor_list.size()-1][router_list.size()-1] = 0;
    
    printf("\n");
    updateRouteTable();
    return encode();
}
void RouteTableDV::print() { //show route message right now.
    printf("\n\n");
    printf("  Next  Address  |  Goal  Address  | Cost\n");
    printf("-----------------|-----------------|-----\n");
    
    for (int i = 0; i < route_table.size(); i++) {
        printf("%17s|%17s|%5d\n",
               route_table[i].next_ip.c_str(),
               route_table[i].dst_ip.c_str(),
               route_table[i].cost);
    }
    printf("\n\n");
}

