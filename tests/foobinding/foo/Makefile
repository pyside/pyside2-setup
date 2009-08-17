CXX_FLAGS=-DBOOST_PYTHON_NO_PY_SIGNATURES -g -fPIC -I/usr/include/python2.5 -I../../libfoo `pkg-config pyside --cflags`
CXX_LDFLAGS=-DBOOST_PYTHON_NO_PY_SIGNATURES -fPIC -shared -L../../libfoo -lfoo `pkg-config pyside --libs`

all: foo_wrapper.o bar_wrapper.o foo_globals_wrapper.o foo_module_wrapper.o
	g++ $(CXX_LDFLAGS) bar_wrapper.o foo_wrapper.o foo_globals_wrapper.o foo_module_wrapper.o -Wl,-soname,foo.so -o foo.so

foo_wrapper.o: foo_wrapper.cpp foo_wrapper.hpp
	g++ $(CXX_FLAGS) foo_wrapper.cpp -c

bar_wrapper.o: bar_wrapper.cpp bar_wrapper.hpp
	g++ $(CXX_FLAGS) bar_wrapper.cpp -c

foo_globals_wrapper.o: foo_globals_wrapper.cpp
	g++ $(CXX_FLAGS) foo_globals_wrapper.cpp -c

foo_module_wrapper.o: foo_module_wrapper.cpp
	g++ $(CXX_FLAGS) foo_module_wrapper.cpp -c

clean:
	rm *.o *.so -rf

