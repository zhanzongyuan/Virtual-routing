//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//


#include "VirtualRCC/VirtualRCC.h"

int main(int argc, const char * argv[]) {
    VirtualRCC rcc("127.0.0.1", 2341, 23333);
    rcc.addNeighborRouter("127.0.0.1", 2333);
    // VirtualRCC rcc("192.168.43.230", 2341, 23333);
    // rcc.addNeighborRouter("192.168.43.191", 2341);
    // rcc.addNeighborRouter("192.168.43.235", 2336);
    //router1.addNeighborRouter(neighbor_ip, 2335);
    rcc.launchRouter();
    return 0;
}
