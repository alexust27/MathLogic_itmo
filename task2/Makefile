CXXFLAGS+=-std=c++14 -Wall -O2
CXX=g++

all: out out/main.o

run: out out/main.o
	./out/main.o

out/main.o: main.cpp
	$(CXX) $^ $(CXXFLAGS) -o out/main.o

out:
	mkdir -p out

pack:
	zip task0.zip *.cpp include/*.h

clean:
	rm -rf out
	rm -f include/*.h