CC = gcc
CXX = g++
RM = rm -rf
SRC = ./src
V_MSG = $(SRC)/VirtualMessage
V_ROUTER = $(SRC)/VirtualRouter
BUILD = ./build

C11 = -std=c++11
PTHREAD = -lpthread

VR_CPP = $(V_ROUTER)/VirtualRouter.cpp
VR_H = $(V_ROUTER)/VirtualRouter.h

MAIN = $(SRC)/main.cpp

OUTPUT = ./virtual-router 

vpath %.o $(BUILD)
vpath %.h $(VR_H)

build: $(BUILD)/VirtualRouter.o $(BUILD)/main.o
	$(CXX) -o $(OUTPUT) $(BUILD)/main.o $(BUILD)/VirtualRouter.o $(C11) $(PTHREAD)

$(BUILD)/VirtualRouter.o: $(VR_CPP) $(VR_H)
	$(CXX) -c $(VR_CPP) $(C11) $(PTHREAD) -o $(BUILD)/VirtualRouter.o 

$(BUILD)/main.o: $(MAIN) $(VR_H)
	$(CXX) -c $(MAIN) $(C11) $(PTHREAD) -o $(BUILD)/main.o 


clean:
	rm -rf $(BUILD)/*.o