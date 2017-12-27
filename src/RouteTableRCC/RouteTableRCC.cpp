#include "RouteTableRCC.h"

RouteTableRCC::RouteTableRCC(const char* host_ip, bool RCC_mode) {
    this->host_ip = string(host_ip);
    this->RCC_mode = RCC_mode;
    str_table = "";
}

// RCC
string RouteTableRCC::getRouterTable(const char* ip) {
    if (RCC_mode) {
        int index = -1;
        for (int i = 0; i<routers.size(); i++) {
            if (strncmp(ip, routers[i].c_str(), 16) == 0) {
                index = i;
                break;
            }
        }
        if (index == -1) return "";
        
        string result = "";
        // Encode route table.
        for (int i = 0; i < routers_table[index].size(); i++) {
            result += routers_table[index][i].dst_ip;
            result += "&";
            result += routers_table[index][i].next_ip;
            result += "#";
        }
        return result;
    }
    return "";
}
void RouteTableRCC::updateRouteTable() {
    // Initial route table.
    for (int i = 0; i < routers_table.size(); i++) routers_table[i].clear();
    
    for (int i = 0; i < routers.size(); i++) {
        vector<int> dst = vector<int>(routers.size(), INT_MAX);
        vector<int> pre = vector<int>(routers.size(), -1);
        vector<bool> in_set = vector<bool>(routers.size(), false);
        dst[i] = 0;
        in_set[i] = true;
        pre[i] = 0;
        int set_num = 1;
        
        // Dijkstra
        while(set_num < routers.size()) {
            int min = INT_MAX;
            int index = -1;
            for (int j = 0; j < routers.size(); j++) {
                if (in_set[j]) {
                    for (int p = 0; p < graph[j].size(); p++) {
                        if (!in_set[graph[j][p]] && dst[graph[j][p]] > dst[j] + 1) {
                            dst[p] = dst[j] + 1;
                            pre[p] = j;
                            if (dst[p] > min) {
                                min = dst[p];
                                index = p;
                            }
                        }
                    }
                }
            }
            if (index == -1) break;
            in_set[index] = true;
            set_num++;
        }
        
        // Update route table.
        for (int j = 0; j < pre.size(); j++) {
            if (j != i) {
                struct route a_route;
                a_route.dst_ip = routers[i];
                a_route.next_ip = routers[pre[j]];
                routers_table[j].push_back(a_route);
            }
        }
    }
}
void RouteTableRCC::addLinkState(const char* router_ip, string message) {
    if (RCC_mode) {
        int index = -1;
        for (int i = 0; i < routers.size(); i++)
            if (strncmp(router_ip, routers[i].c_str(), 16) == 0) {
                index = i;
                break;
            }
        if (index == -1) {
            index = int(routers.size());
            routers.push_back(string(router_ip));
            graph.push_back(vector<int>());
            routers_table.push_back(vector<struct route>());
        }
        decodeLinkState(index, message);
        
        updateRouteTable();
    }
}
void RouteTableRCC::decodeLinkState(int index, string message) {
    // Initial index renew router state.
    for (int i = 0; i < graph.size(); i++) {
        if (i == index) {
            graph[i].clear();
            continue;
        }
        for (int j = 0; j < graph[i].size(); j++) {
            if (graph[i][j] == index) {
                graph[i].erase(graph[i].begin()+j);
                break;
            }
        }
    }
    
    // Make new router state to graph.
    string ip = "";
    for (int i = 0; i < message.size(); i++) {
        if (message[i] == '#') {
            int index_nei = -1;
            for (int j = 0; j < routers.size(); j++) {
                if (routers[j] == ip) {
                    index_nei = j;
                    break;
                }
            }
            if (index_nei == -1) {
                index_nei = int(routers.size());
                routers.push_back(ip);
                graph.push_back(vector<int>());
                routers_table.push_back(vector<struct route>());
            }
            graph[index_nei].push_back(index);
            graph[index].push_back(index_nei);
            ip = "";
        }
        else {
            ip += message[i];
        }
    }
}

// Router
void RouteTableRCC::addNeighbor(char* nei_ip) {
    // Add neighbor routers ip;
    for (int i = 0; i < this->nei_ip.size(); i++) {
        if (strncmp(this->nei_ip[i].c_str(), nei_ip, 16) == 0) {
            return;
        }
    }
    this->nei_ip.push_back(nei_ip);
}
void RouteTableRCC::removeNeighbor(char* nei_ip) {
    // Delete neighbor routers ip.
    for (int i = 0; i < this->nei_ip.size(); i++) {
        if (strncmp(this->nei_ip[i].c_str(), nei_ip, 16) == 0) {
            this->nei_ip.erase(this->nei_ip.begin()+i);
            return;
        }
    }
}
string RouteTableRCC::getLinkState() {
    // Encode link state.
    string result = "";
    for (int i = 0; i < nei_ip.size(); i++) {
        result += nei_ip[i];
        result += "#";
    }
    return result;
}
void RouteTableRCC::findNextIP(char next_ip[], const char* dst_ip) {
    // Find next router ip;
    strncpy(next_ip, "", 16);
    for (int i = 0; i < router_table.size(); i++) {
        if (strncmp(dst_ip, router_table[i].dst_ip.c_str(), 16) == 0) {
            strncpy(next_ip, router_table[i].next_ip.c_str(), 16);
            return;
        }
    }
}
void RouteTableRCC::renewRouteTable(string message) {
    // Decode route table.
    if (message == str_table) return;
    string ip = "";
    for (int i = 0; i < message.size(); i++) {
        if (message[i] == '&') {
            struct route a_route;
            a_route.dst_ip = ip;
            router_table.push_back(a_route);
            ip = "";
        }
        else if (message[i] == '#') {
            router_table[router_table.size()-1].next_ip = ip;
            ip = "";
        }
        else {
            ip += message[i];
        }
    }
}
void RouteTableRCC::print() {
    // Print route table.
    if (!RCC_mode) {
        printf("\n");
        printf("********* %s Route Table ********\n", host_ip.c_str());
        printf("    Destination ip  |  Next ip  \n");
        printf("--------------------|--------------------\n");
        for (int i = 0; i < router_table.size(); i++) {
            printf("  %16s  |  %-16s  \n", router_table[i].dst_ip.c_str(), router_table[i].next_ip.c_str());
        }
        printf("\n");
    }
        
}
