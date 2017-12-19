router: myRouter.o main.o
	g++ -o router2333 main.o myRouter.o -std=c++11 -lpthread

myRouter.o: myRouter.cpp myRouter.h
	g++ -c myRouter.cpp -std=c++11 -lpthread

main.o: main.cpp myRouter.h 
	g++ -c main.cpp -std=c++11 -lpthread


clean:
	rm -rf *.o