CXXFLAGS+=-std=c++14 -Wall -O2
CXX=g++

all: out out/main.o

run:
	./out/main.o

out/main.o: main.cpp
	$(CXX) $< $(CXXFLAGS) -o out/main.o

out:
	mkdir -p out

clean:
	rm -rf out

pack:
	zip task7.zip *.cpp *.h Makefile