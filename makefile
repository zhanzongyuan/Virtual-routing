CC = gcc
CXX = g++
RM = rm -rf
SRC = ./src
V_MSG = $(SRC)/VirtualMessage
V_ROUTER = $(SRC)/VirtualRouter
V_RCC = $(SRC)/VirtualRCC

V_LSRT = $(SRC)/RouteTableLS
V_DVRT = $(SRC)/RouteTableDV
V_RCCRT = $(SRC)/RouteTableRCC

BUILD = ./build

C11 = -std=c++11
PTHREAD = -lpthread

VR_CPP = $(V_ROUTER)/VirtualRouter.cpp
VR_H = $(V_ROUTER)/VirtualRouter.h

VRCC_CPP = $(V_RCC)/VirtualRCC.cpp
VRCC_H = $(V_RCC)/VirtualRCC.h

VM_H = $(MSG)/VirtualMessage.h

VLS_CPP = $(V_LSRT)/RouteTableLS.cpp
VLS_H = $(V_LSRT)/RouteTableLS.h

VDV_CPP = $(V_DVRT)/RouteTableDV.cpp
VDV_H = $(V_DVRT)/RouteTableDV.h

VRCC_RT_CPP = $(V_RCCRT)/RouteTableRCC.cpp
VRCC_RT_H = $(V_RCCRT)/RouteTableRCC.h

LAUNCH_ROUTER = $(SRC)/launch-router.cpp
LAUNCH_RCC = $(SRC)/launch-rcc.cpp

ROUTER_OUT = ./virtual-router
RCC_OUT = ./virtual-rcc

vpath %.o $(BUILD)
vpath %.h $(VR_H)

rcc:  $(BUILD)/VirtualRCC.o $(BUILD)/launch-rcc.o $(BUILD)/RouteTableRCC.o
	$(CXX) -o $(RCC_OUT) $(BUILD)/launch-rcc.o $(BUILD)/VirtualRCC.o $(BUILD)/RouteTableRCC.o $(C11) $(PTHREAD)

router: $(BUILD)/VirtualRouter.o $(BUILD)/RouteTableLS.o $(BUILD)/RouteTableDV.o $(BUILD)/launch-router.o $(BUILD)/RouteTableRCC.o
	$(CXX) -o $(ROUTER_OUT) $(BUILD)/launch-router.o $(BUILD)/VirtualRouter.o $(BUILD)/RouteTableLS.o $(BUILD)/RouteTableDV.o $(BUILD)/RouteTableRCC.o $(C11) $(PTHREAD)

$(BUILD)/VirtualRouter.o: $(VR_CPP) $(VR_H) 
	$(CXX) -c $(VR_CPP) $(C11) $(PTHREAD) -o $(BUILD)/VirtualRouter.o 

$(BUILD)/VirtualRCC.o: $(VRCC_CPP) $(VRCC_H)
	$(CXX) -c $(VRCC_CPP) $(C11) $(PTHREAD) -o $(BUILD)/VirtualRCC.o 

$(BUILD)/RouteTableLS.o: $(VLS_CPP) $(VLS_H)
	$(CXX) -c $(VLS_CPP) $(C11) $(PTHREAD) -o $(BUILD)/RouteTableLS.o 

$(BUILD)/RouteTableDV.o: $(VDV_CPP) $(VDV_H)
	$(CXX) -c $(VDV_CPP) $(C11) $(PTHREAD) -o $(BUILD)/RouteTableDV.o 

$(BUILD)/RouteTableRCC.o: $(VRCC_RT_CPP) $(VRCC_RT_H)
	$(CXX) -c $(VRCC_RT_CPP) $(C11) $(PTHREAD) -o $(BUILD)/RouteTableRCC.o 

$(BUILD)/launch-router.o: $(LAUNCH_ROUTER) $(VR_H) $(VLS_H) $(VDV_H) 
	$(CXX) -c $(LAUNCH_ROUTER) $(C11) $(PTHREAD) -o $(BUILD)/launch-router.o 

$(BUILD)/launch-rcc.o: $(LAUNCH_RCC) $(VRCC_H) 
	$(CXX) -c $(LAUNCH_RCC) $(C11) $(PTHREAD) -o $(BUILD)/launch-rcc.o 

clean:
	rm -rf $(BUILD)/*.o