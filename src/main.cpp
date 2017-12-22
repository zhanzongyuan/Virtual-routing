//
//  main.cpp
//  VirtualRouter
//
//  Created by applecz on 2017/12/16.
//  Copyright © 2017年 applecz. All rights reserved.
//

#include "VirtualRouter/VirtualRouter.h"

int main(int argc, const char * argv[]) {
    VirtualRouter router1;
    const char *neighbor_ip = "127.0.0.1";
    router1.addNeighborRouter(neighbor_ip, 2334);
    //router1.addNeighborRouter(neighbor_ip, 2335);
    router1.launchRouter();
    return 0;
}
