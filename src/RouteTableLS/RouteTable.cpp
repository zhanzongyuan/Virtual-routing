#include "RouteTable.h"

#define INF INT_MAX //Infinity

class prioritize
{
public:
  bool operator()(pair<string, int> &p1, pair<string, int> &p2)
  {
    return p1.second > p2.second;
  }
};

RouteTable::RouteTable(const char *host_ip)
{
  ip_num = 1;
  mhost_ip = host_ip;
  mRouter[mhost_ip] = ip_num;
}

void RouteTable::addNeighborIP(char *neighbor_ip)
{
  if (ip_num < 10001)
  {
    ip_num++;
    string tmp = neighbor_ip;
    mRouter[tmp] = ip_num;
    mNeighbor.push_back(tmp);
  }
  else
  {
    cout << "Add neighbor IP failed, exceed max IP number!" << endl;
  }
}

void RouteTable::addRoute(char *ip1, char *ip2, int time)
{
  string temp_ip1 = ip1, temp_ip2 = ip2;
  if (mRouter[temp_ip1] > 0 && mRouter[temp_ip2] > 0)
  {
    pair<string, int> tmp = make_pair(temp_ip2, time);
    graph[mRouter[temp_ip1]].push_back(tmp);
    graph[mRouter[temp_ip2]].push_back(make_pair(temp_ip1, time));
  }
}

void RouteTable::findNextIP(char *&next_ip, char *dst_ip)
{
  if (dst_ip == mhost_ip)
  {
    next_ip = dst_ip;
    return;
  }
  string temp_ip = dst_ip;
  int dst = mRouter[temp_ip];

  dijkstra();

  if (vi[dst].size() == 0)
  {
    next_ip = (char *)malloc((mhost_ip.length() + 1) * sizeof(char));
    mhost_ip.copy(next_ip, mhost_ip.length(), 0);
  }
  else if (vi[dst].size() == 1)
    next_ip = dst_ip;
  else
    next_ip = (char *)vi[dst][1].data();
  return;
}

void RouteTable::printRouteTable()
{
  dijkstra();
  cout << "Source is: " << mhost_ip << ". The shortest distance to every other vertex from here is: \n";
  for (int i = 1; i < ip_num; i++) //Printing final shortest distances from source
  {
    cout << "Vertex: " << mNeighbor[i - 1] << " , Distance: ";
    dis[mRouter[mNeighbor[i - 1]]] != INF ? cout << dis[mRouter[mNeighbor[i - 1]]] << "\n" : cout << "-1\n";
    cout << "Across vertex: ";
    for (int j = 0; j < vi[mRouter[mNeighbor[i - 1]]].size(); j++)
      cout << vi[mRouter[mNeighbor[i - 1]]][j] << " -> ";
    cout << mNeighbor[i - 1] << endl << endl;
  }
}

void RouteTable::dijkstra()
{
  for (int i = 0; i < 10001; i++)
    dis[i] = INF;                                                              //Set initial distances to Infinity
  priority_queue<pair<string, int>, vector<pair<string, int>>, prioritize> pq; //Priority queue to store vertex,weight pairs
  dis[1] = 0;
  pq.push(make_pair(mhost_ip, dis[1])); //Pushing the source with distance from itself as 0
  while (!pq.empty())
  {
    pair<string, int> curr = pq.top(); //Current vertex. The shortest distance for this has been found
    pq.pop();
    int cv = mRouter[curr.first];
    int cw = curr.second; //'cw' the final shortest distance for this vertex
    if (vis[cv])
      continue; //If the vertex is already visited, no point in exploring adjacent vertices
    vis[cv] = true;
    for (int i = 0; i < graph[cv].size(); i++)                                                              //Iterating through all adjacent vertices
      if (!vis[mRouter[graph[cv][i].first]] && graph[cv][i].second + cw < dis[mRouter[graph[cv][i].first]]) //If this node is not visited and the current parent node distance+distance from there to this node is shorted than the initial distace set to this node, update it
      {
        pq.push(make_pair(graph[cv][i].first, (dis[mRouter[graph[cv][i].first]] = graph[cv][i].second + cw))); //Set the new distance and add to priority queue
        vi[mRouter[graph[cv][i].first]].clear();
        vi[mRouter[graph[cv][i].first]].assign(vi[cv].begin(), vi[cv].end());
        vi[mRouter[graph[cv][i].first]].push_back(curr.first);
      }
  }
}