.PHONY: clean

all: sample

sample: sample.o promise.o
	$(CXX) -pthread $^ -o $@

%.o: %.cpp
	$(CXX) -std=c++0x -pipe -Wall -c $< -o $@

clean:
	rm -f *.o sample

sample.o: promise.hpp Makefile
promise.o: promise.hpp Makefile
