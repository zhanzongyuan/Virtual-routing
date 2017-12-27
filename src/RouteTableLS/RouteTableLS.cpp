#include "RouteTableLS.h"

#define INF INT_MAX //Infinity

class prioritize
{
public:
  bool operator()(pair<string, int> &p1, pair<string, int> &p2)
  {
    return p1.second > p2.second;
  }
};

RouteTableLS::RouteTableLS(const char *host_ip)
{
  ip_num = 1;
  mhost_ip = host_ip;
  mRouter[mhost_ip] = ip_num;
  mNeighbor.push_back(mhost_ip);
  for (int i = 0; i < 10001; i++)
    isRemove[i] = false;
}

void RouteTableLS::addNeighborIP(const char *neighbor_ip)
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

void RouteTableLS::addRoute(char *router_ip, string message)
{
  string tmp = router_ip;
  if (mRouter[tmp] == 0 || isRemove[mRouter[tmp]])
  {
    cout << "Add route failed, can not find tmp in route table!" << endl;
    return;
  }

  vector<pair<string, string>> vp = decode(message);
  for (int i = 0; i < vp.size(); i++)
  {
    string temp_ip1 = vp[i].first, temp_ip2 = vp[i].second;
    if (mRouter[temp_ip1] == 0 || isRemove[mRouter[temp_ip1]])
      addNeighborIP(temp_ip1.c_str());
    if (mRouter[temp_ip2] == 0 || isRemove[mRouter[temp_ip2]])
      addNeighborIP(temp_ip2.c_str());
    bool flag1 = false, flag2 = false;
    for (int j = 0; j < graph[mRouter[temp_ip1]].size(); j++)
      if (graph[mRouter[temp_ip1]][j].first == temp_ip2)
        flag1 = true;
    if (!flag1)
      graph[mRouter[temp_ip1]].push_back(make_pair(temp_ip2, 1));
    for (int j = 0; j < graph[mRouter[temp_ip2]].size(); j++)
      if (graph[mRouter[temp_ip2]][j].first == temp_ip1)
        flag2 = true;
    if (!flag2)
      graph[mRouter[temp_ip2]].push_back(make_pair(temp_ip1, 1));
  }
  dijkstra();
}

void RouteTableLS::findNextIP(char *&next_ip, char *dst_ip)
{
  if (dst_ip == mhost_ip)
  {
    strcpy(next_ip, dst_ip);
    return;
  }
  string temp_ip = dst_ip;
  int dst = mRouter[temp_ip];
  if (vi[dst].size() == 0)
  {
    next_ip = (char *)malloc((mhost_ip.length() + 1) * sizeof(char));
    mhost_ip.copy(next_ip, mhost_ip.length(), 0);
  }
  else if (vi[dst].size() == 1)
    strcpy(next_ip, dst_ip);
  else
    strcpy(next_ip, (char *)vi[dst][1].data());
  return;
}

void RouteTableLS::printRouteTableLS()
{
  cout << "Source is: " << mhost_ip << ". The shortest distance to every other vertex from here is: \n";
  for (int i = 1; i < ip_num; i++) //Printing final shortest distances from source
  {
    if (isRemove[mRouter[mNeighbor[i]]] || mRouter[mNeighbor[i]] != i + 1)
      continue;
    cout << "Vertex: " << mNeighbor[i] << " , Distance: ";
    if (dis[mRouter[mNeighbor[i]]] != INF)
    {
      cout << dis[mRouter[mNeighbor[i]]] << endl;
      cout << "Across vertex: ";
      for (int j = 0; j < vi[mRouter[mNeighbor[i]]].size(); j++)
        cout << vi[mRouter[mNeighbor[i]]][j] << " -> ";
      cout << mNeighbor[i] << endl;
    }
    else
      cout << "-1\n";
    cout << endl;
  }
}

void RouteTableLS::removeRoute(char *neighbor_ip)
{
  string tmp = neighbor_ip;
  if (mRouter[tmp] > 1 && !isRemove[mRouter[tmp]])
  {
    isRemove[mRouter[tmp]] = true;
    graph[mRouter[tmp]].clear();
    for (int i = 0; i < ip_num; i++)
      for (int j = 0; j < graph[mRouter[mNeighbor[i]]].size();)
      {
        if (graph[mRouter[mNeighbor[i]]][j].first == tmp)
          graph[mRouter[mNeighbor[i]]].erase(graph[mRouter[mNeighbor[i]]].begin() + j);
        else
          j++;
      }
    dijkstra();
  }
  else if (mRouter[tmp] == 1)
  {
    cout << "Remove failed! Can not remove host ip!" << endl;
  }
  else
  {
    cout << "Remove failed! Can not find IP: " << tmp << "in route table!" << endl;
  }
}

string RouteTableLS::getBroadcastMessage()
{
  return encode();
}

void RouteTableLS::dijkstra()
{
  for (int i = 0; i < 10001; i++)
  {
    dis[i] = INF; //Set initial distances to Infinity
    vi[i].clear();
    vis[i] = false;
  }

  priority_queue<pair<string, int>, vector<pair<string, int>>, prioritize> pq; //Priority queue to store vertex,weight pairs
  dis[1] = 0;
  pq.push(make_pair(mhost_ip, dis[1])); //Pushing the source with distance from itself as 0
  while (!pq.empty())
  {
    pair<string, int> curr = pq.top(); //Current vertex. The shortest distance for this has been found
    pq.pop();
    int cv = mRouter[curr.first];
    int cw = curr.second; //'cw' the final shortest distance for this vertex
    if (vis[cv] || isRemove[cv])
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

string RouteTableLS::encode()
{
  stringstream ss;

  /*for (int i = 0; i < ip_num; i++) //Printing final shortest distances from source
  {
    if (isRemove[mRouter[mNeighbor[i]]] || (mRouter[mNeighbor[i]] != i + 1))
      continue;
    for (int j = 0; j < graph[mRouter[mNeighbor[i]]].size(); j++)
      ss << "{" << mNeighbor[i] << "," << graph[mRouter[mNeighbor[i]]][j].first << "};";
  }*/
  for (int j = 0; j < graph[mRouter[mhost_ip]].size(); j++)
    ss << "{" << mhost_ip << "," << graph[mRouter[mhost_ip]][j].first << "};";
  return ss.str();
}

vector<pair<string, string>> RouteTableLS::decode(string message)
{
  vector<pair<string, string>> vp;
  int left = 0, right = 0;
  for (int i = 0; i < message.length(); i++)
    if (message[i] == ';')
    {
      right = i - 1;
      string tmp = message.substr(left + 1, right - left - 1);
      for (int j = 0; j < tmp.length(); j++)
        if (tmp[j] == ',')
        {
          string s1 = tmp.substr(0, j);
          string s2 = tmp.substr(j + 1);
          vp.push_back(make_pair(s1, s2));
        }
      left = i + 1;
    }
  return vp;
}