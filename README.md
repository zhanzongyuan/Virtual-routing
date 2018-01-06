# VirtualRouting

> 2017-12-19
>
> VirtualRouting v 3.0.0

- How to install

Just input the command below.

```
make router
```
Then it will generate a file "router", you can run the program in terminal by input follow command:

```
./virtual-router
```

Edit main.cpp and Router.cpp to change the host address(ip and port) and neigbor address(ip and port).

> In RCC mode, you need to make rcc router additionally.

```c++
make rcc
```

Launch rcc router as router control center.

```c++
./virtual-rcc
```

</br>

- How to use

Just input the message after `router@name# `, then it will send you input message to neighbor routers.

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

> DV Mode

Config `launch-router.cpp` 

```c++
//VirtualRouter router1([IP], [SERVER_PORT], [CLIENT_PORT], MODE);
VirtualRouter router1("192.168.43.230", 2333, 23333, VirtualRouter::DV);

//router1.addNeighborRouter([Neighbor_IP], [Neighbor_PORT])
router1.addNeighborRouter("192.168.43.220", 2346);
router1.addNeighborRouter("192.168.43.17", 2346);
router1.addNeighborRouter("192.168.43.235", 2346);

router1.launchRouter();
```

> LS Mode

```c++
//VirtualRouter router1([IP], [SERVER_PORT], [CLIENT_PORT], MODE);
VirtualRouter router1("192.168.43.230", 2333, 23333, VirtualRouter::LS);

//router1.addNeighborRouter([Neighbor_IP], [Neighbor_PORT])
router1.addNeighborRouter("192.168.43.220", 2346);
router1.addNeighborRouter("192.168.43.17", 2346);
router1.addNeighborRouter("192.168.43.235", 2346);

router1.launchRouter();
```

> RCC Mode

```c++
// peer router config

//VirtualRouter router1([IP], [SERVER_PORT], [CLIENT_PORT], [RCC], [RCC_IP], [RCC_PORT]);
VirtualRouter router1("192.168.43.230", 2346, 23333, VirtualRouter::RCC, "192.168.43.191", 2333);

//router1.addNeighborRouter([Neighbor_IP], [Neighbor_PORT])
router1.addNeighborRouter("192.168.43.220", 2346);
router1.addNeighborRouter("192.168.43.17", 2346);
router1.addNeighborRouter("192.168.43.235", 2346);

router1.launchRouter();
```

Router control center config (in file `launch-rcc.cpp`) as follows:

```c++
// rcc router config

//VirtualRouter router1([IP], [SERVER_PORT], [CLIENT_PORT]);
VirtualRouter router1("192.168.43.191", 2346, 23333);

//Add peer router
//router1.addNeighborRouter([Neighbor_IP], [Neighbor_PORT])
router1.addNeighborRouter("192.168.43.230", 2346);
router1.addNeighborRouter("192.168.43.220", 2346);
router1.addNeighborRouter("192.168.43.17", 2346);
router1.addNeighborRouter("192.168.43.235", 2346);

router1.launchRouter();
```

</br>
