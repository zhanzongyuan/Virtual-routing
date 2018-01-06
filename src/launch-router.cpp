//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//
#include "VirtualRouter/VirtualRouter.h"


int main(int argc, const char * argv[]) {
    VirtualRouter router1("192.168.43.230", 2346, 23333, VirtualRouter::DV);
    
    //VirtualRouter router1("192.168.43.230", 2341, 23333, VirtualRouter::LS);
    
    // VirtualRouter router1("192.168.199.230", 2333, 23333, VirtualRouter::RCC, "192.168.199.201", 2333);
    
    router1.addNeighborRouter("192.168.43.220", 2346);
    router1.addNeighborRouter("192.168.43.17", 2346);
    router1.addNeighborRouter("192.168.43.235", 2346);
    
    router1.launchRouter();
    return 0;
}
