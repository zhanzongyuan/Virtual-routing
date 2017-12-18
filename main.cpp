//
//  main.cpp
//  myRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//
#include "myRouter.h"

int main(int argc, const char * argv[]) {
    myRouter router1;
    const char *neighbor_ip = "192.168.199.201";
    router1.setNeighborAddress(neighbor_ip, 2333);
    router1.launchRouter();
    return 0;
}
