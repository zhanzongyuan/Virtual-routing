//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//
#include "VirtualRouter/VirtualRouter.h"


int main(int argc, const char * argv[]) {
    VirtualRouter router1("127.0.0.1", 2334, 23334, VirtualRouter::DV);
    //VirtualRouter router1("127.0.0.1", 2334, 23334, VirtualRouter::LS);
    //VirtualRouter router1("127.0.0.1", 2334, 23334, VirtualRouter::RCC, "127.0.0.1", 2333);
    router1.addNeighborRouter("127.0.0.1", 2333);
    router1.launchRouter();
    return 0;
}
