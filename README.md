# VirtualRouting

> 2017-12-19
>
> VirtualRouting v 3.0.0

- How to install

Just input the command below.

```
make
```
Then it will generate a file "router", you can run the program in terminal by input follow command:

```
./router
```

Edit main.cpp and Router.cpp to change the host address(ip and port) and neigbor address(ip and port).

</br>

- How to use

Just input the message after `Server:>`, then it will send you input message to neighbor routers.

You will see the message you send in the terminal of neighbor routers.

Notice that in this version the router can only broadcast message you input to neighbor router, it has no route table to choose send message to which router.

Controll router by input command.

```
In this version, router add command set as follow:

- 'send'  : send message to router with ip. 
- 'router': list neighbor routers information. 
- 'config': list router config. 
- 'route' : list route table. 
- 'exit'  : shutdown router and exit system. 
- 'help'  : list avaliable commands in system. 

```

</br>

- Examples

In this repository, i also make three example to test the program under folder `./examples`.


```
router2333
router2334
```


Three example program use ip of `127.0.0.1` with port `2333, 2334` to receive message, and port `23333, 23334` to send message to neighbor router.

So you can open three terminal to input the command `./router2333` `./router2334`  seperatly. 

 </br>

- TODO

1. Implement class RouteTable in file RouteTable.h:

```c++
/**
 * @高晨
 * TODO: 实现下面距离向量类
 * 记录邻居路由和自己到其他路由需要的跳数
 * 
 */
class RouteTableDV {
private:
  
  ...
   // 内部编码解码
   // decode  
   // encode  
  ...
  
public:
  RouteTableDV(char* host_ip); 
  
  string routeChangeMessage(char* neighbor_ip, string change); 
  // 获取邻居的路由变化信息, 返回自己到其他路由跳数信息，如果自己到其他路由的跳数没变，则返回“”空字符。
  
  string addNeighborIP(char* neighbor_ip); 
  // 连接邻居时更新路由表，同时返回自己到其他路由跳数信息
  
  void findNextIP(char* &next_ip, char* dst_ip); 
  // 传入一个目的ip，找到下一跳路由ip，如果自己到其他路由的跳数没变，则返回“”空字符。
  
  void print(); // 打印路由表
}

/**
 * @思亭
 * TODO: 实现下面最短路类
 * 动态保存整个网络的拓扑图，用链接表记录
 * 通过最短路径算法，计算每次更新拓扑图后的路由表 <目标ip，下一跳ip>
 *
 */
class RouteTableLS {
private:
  ...
  // 这里传入和传出的信息都是字符串，需要在内部实现编码和解码
  // decode
  // encode
  ...
public:
  RouteTableLS(char* host_ip); 
  
  string getBroadcastMessage();
  // 获取该路由的连接情况，定期广播发送到整个网络
  
  void addRoute(char* router_ip, string message);
  // 获取来自其他路由(不一定是邻居)广播的信息，信息说明该路由的连接状况。更新拓扑图，更新路由表。
  
  void findNextIP(char* &next_ip, char* dst_ip); 
  // 根据目的路由ip获取下一跳路由ip
  
  void print(); // 打印路由表
}
```

2. Add function to send neighbor its route table periodically for algorithm *Distance Vector* and *Link State*
3. Add debug mode and *Distance Vector/Link State* mode
4. Add name to each router.
5. Add command to connect or disconnect new neighbor in command, when the router is still running.