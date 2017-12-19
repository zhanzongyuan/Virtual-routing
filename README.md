# myRouter

> 2017-12-19
>
> myRouter v 2.0

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

You can input `quit` to quit program.

</br>

- In this repository, i also make three example to test the program.
  - router2333
  - router2334
  - router2335

Three example program use ip of `127.0.0.1` with port `2333, 2334, 2335` to receive message, and port `23333, 23334, 23335` to send message to neighbor router.

So you can open three terminal to input the command `./router2333` `./router2334` `./router2335` seperatly. 

 