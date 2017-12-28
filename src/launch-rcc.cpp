//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//


#include "VirtualRCC/VirtualRCC.h"

int main(int argc, const char * argv[]) {
    VirtualRCC rcc("192.168.199.230", 2333, 23333);
    rcc.addNeighborRouter("192.168.199.201", 2333);
    //router1.addNeighborRouter(neighbor_ip, 2335);
    rcc.launchRouter();
    return 0;
}
