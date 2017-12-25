CC = gcc
CXX = g++
RM = rm -rf
SRC = ./src
V_MSG = $(SRC)/VirtualMessage
V_ROUTER = $(SRC)/VirtualRouter
V_LSRT = $(SRC)/RouteTableLS
V_DVRT = $(SRC)/RouteTableDV
BUILD = ./build

C11 = -std=c++11
PTHREAD = -lpthread

VR_CPP = $(V_ROUTER)/VirtualRouter.cpp
VR_H = $(V_ROUTER)/VirtualRouter.h
VM_H = $(V_ROUTER)/VirtualMessage.h

VLS_CPP = $(V_LSRT)/RouteTable.cpp
VLS_H = $(V_LSRT)/RouteTable.h

VDV_CPP = $(V_DVRT)/RouteTable.cpp
VDV_H = $(V_DVRT)/RouteTable.h

MAIN = $(SRC)/main.cpp

OUTPUT = ./virtual-router2333

vpath %.o $(BUILD)
vpath %.h $(VR_H)

build: $(BUILD)/VirtualRouter.o $(BUILD)/RouteTableLS.o $(BUILD)/RouteTableDV.o $(BUILD)/main.o 
	$(CXX) -o $(OUTPUT) $(BUILD)/main.o $(BUILD)/VirtualRouter.o $(BUILD)/RouteTableLS.o $(BUILD)/RouteTableDV.o $(C11) $(PTHREAD)

$(BUILD)/VirtualRouter.o: $(VR_CPP) $(VR_H) $(VM_H)
	$(CXX) -c $(VR_CPP) $(C11) $(PTHREAD) -o $(BUILD)/VirtualRouter.o 

$(BUILD)/RouteTableLS.o: $(VLS_CPP) $(VLS_H)
	$(CXX) -c $(VLS_CPP) $(C11) $(PTHREAD) -o $(BUILD)/RouteTableLS.o 

$(BUILD)/RouteTableDV.o: $(VDV_CPP) $(VDV_H)
	$(CXX) -c $(VDV_CPP) $(C11) $(PTHREAD) -o $(BUILD)/RouteTableDV.o 

$(BUILD)/main.o: $(MAIN) $(VR_H) $(VLS_H) $(VDV_H) 
	$(CXX) -c $(MAIN) $(C11) $(PTHREAD) -o $(BUILD)/main.o 

clean:
	rm -rf $(BUILD)/*.o