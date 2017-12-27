# Virtual Routing 项目报告

> 2017-12-19
>
> VirtualRouting v 3.0.0
>
> 开发环境：Linux，MacOS
>
> 开发语言：C++

| 组长   | 詹宗沅  |
| ---- | ---- |
| 组员   | 宋思亭  |
| 组员   | 高晨   |

## 一、项目介绍

这是一个模拟路由器行为的c++项目，运用套接字编程、多线程编程等方法，模拟实现了多台主机路由网络。

模拟路由网络中路由器的广播，数据报转发等功能。并且能够通过命令行，直接在终端输入特定信息发送到指定的路由。

实现三种路由算法：Distance Vector、Link Stated、Routing Control Center

</br>

## 二、项目设计分工

**技术分析**

1. C++ socket套节字编程，采用建立tcp双向连接的方式，实现固定虚拟路由器发送端口和接收端口
2. 多线程编程，引用pthread.h头文件，创建多个线程处理路由信息的接收、发送和探测等功能
3. 最短路径算法：Dijkstra算法
4. 距离矢量算法 Distance Vector
5. 广播风暴控制

<br>

**虚拟链路协议设计**

1. 路由器与相邻路由器建立双向TCP套接字连接，实现双向数据发送；

2. 路由固定两个端口

   1. Server端：等待来自相邻路由器的连接请求，一旦连接建立就保持接听来自该路由器的信息；
   2. Client端：主动请求连接相邻路由器，一旦建立连接则随时可以发送信息；

3. Server端每次接收一则数据，则返回code-“301”，表示接收成功；如果接收到空数据，则对方路由器当机。

4. Client发送信息后检查是否有确认信息“301”返回，返回超时则代表对方下线或当机，连接断开。

5. 周期性检测邻居路由状态，通过发送“300”探测报文，检查是否返回“301”应答报文。

6. 报文状态码

   1. ”000“ 状态码：广播报文
   2. ”100“ 状态码：DV模式路由通知报文
   3. “200” 状态码：转发信息报文
   4. “300” 状态码：周期性探测报文
   5. “301” 状态码：OK回复报文
   6. "400" 状态码：RCC信息交换报文

   <br>

**虚拟路由协议设计**

- Distance Vector
  - 当邻居链路状态发生变化，立即通过Distance Vector算法更新本地路由表，将更新的链路状态向邻居发送。
  - 链路状态变化包括：与相邻路由建立连接，相邻路由当机，相邻路由链路状态引发链路状态变化
- Linked Stated
  - 周期性广播链路状态，使得整个网络中的每个路由的拓扑图保持一致
  - 每当接收新的广播信息，更新拓扑图，通过最短路径算法计算新路由表
- Routing Control Center
  - 周期性向RCC发送链路状态，同时获得来自RCC计算的拓扑图。

<br>

**类设计**

- VirtualRouter类设计：


- RoutingControlCenter类设计：
- VirtualMessage类设计：
- RouteTableDV类设计：
- RouteTableLS类设计：
- RouteTableRCC类设计：


<br>

**分工**

| 成员   | 分工                                       |
| ---- | ---------------------------------------- |
| 詹宗沅  | 1. 项目框架搭建及分工组织<br>2. VirtualRouter、RoutingControlCenter、RouteTableRCC相关类设计、实现<br>3. RouteTableDV、RouteTableLS类接口设计<br> |
| 宋思亭  | 1. RouteTableLS实现<br>2. 最短路径算法实现<br>     |
| 高晨   | 1. RouteTableDV实现<br>2. 距离矢量算法实现<br>     |
</br>

## 三、项目部署安装

**环境：**Linux/MacOS

**部署** *（一个主机只能运行一个项目程序）*：

1. 选择一个project进入到main文件夹
2. 设置文件`main.cpp`配置邻居路由的ip和端口
3. 设置文件`VirtualRouter.cpp`/`RoutingControlCentering.cpp`文件头中对Server和Client的ip和端口的设置。
4. 设置文件`VirtualRouter.cpp`中`routing_algo`参数调整路由算法模式
5. 编译安装
   - 在终端输入`projects/router`文件夹，输入`make`编译项目
   - 输入`./virtual-router`安装虚拟路由

</br>

## 四、项目运行结果



</br>

## 五、项目成员贡献及评分

| 成员   | 评分   |
| ---- | ---- |
| 詹宗沅  |      |
| 宋思亭  |      |
| 高晨   |      |

