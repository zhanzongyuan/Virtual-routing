//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//
#include "VirtualRouter/VirtualRouter.h"


int main(int argc, const char * argv[]) {
    //VirtualRouter router1("127.0.0.1", 2333, 23334, VirtualRouter::DV);
    //VirtualRouter router1("127.0.0.1", 2333, 23334, VirtualRouter::LS);
    VirtualRouter router1("172.20.10.3", 2333, 23333, VirtualRouter::RCC, "172.20.10.7", 2333);
    //srouter1.addNeighborRouter("127.0.0.1", 2334);
    router1.launchRouter();
    return 0;
}
