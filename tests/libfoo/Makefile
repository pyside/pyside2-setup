all: foo.o bar.o
	g++ -fPIC -shared foo.o bar.o -o libfoo.so

foo.o: foo.h foo.cpp
	g++ -fPIC foo.cpp -c

bar.o: bar.h bar.cpp
	g++ -fPIC bar.cpp -c

test: main.cpp
	g++ main.cpp -L. -lfoo -I. -o footest

clean:
	rm *.o *.so footest -rf

