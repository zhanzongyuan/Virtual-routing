//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//


#include "VirtualRCC/VirtualRCC.h"

int main(int argc, const char * argv[]) {
    VirtualRCC rcc("127.0.0.1", 2333, 23333);
    rcc.addNeighborRouter("127.0.0.1", 2334);
    //router1.addNeighborRouter(neighbor_ip, 2335);
    rcc.launchRouter();
    return 0;
}
