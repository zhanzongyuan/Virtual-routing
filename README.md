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

- In this repository, i also make three example to test the program under folder `./examples`.
  - router2333
  - router2334

Three example program use ip of `127.0.0.1` with port `2333, 2334` to receive message, and port `23333, 23334` to send message to neighbor router.

So you can open three terminal to input the command `./router2333` `./router2334`  seperatly. 

 </br>

- TODO

Implement class RouteTable in file RouteTable.h:

```c++
class RouteTable {
private:
  ...
public:
  RouteTable(char* host_ip);  // Set the ip of host router.
  void addNeighborIP(char* neighbor_ip); // Add ip of neighbor.
  void addRoute(char* ip1, char* ip2); // Add a route.
  void findNextIP(char* &next_ip, char* dst_ip); // Find next ip to the destination ip.
}
```

