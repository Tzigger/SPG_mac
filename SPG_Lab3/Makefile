CXX      = g++
CXXFLAGS = -I. -I/usr/local/include
LDFLAGS  = -L/usr/local/lib -lGLEW -lglfw -framework OpenGL -framework Cocoa -framework IOKit
COMPAT   = freeglut_compat.cpp

all: ex1 ex2 ex3 ex4

ex1: main.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) main.cpp $(COMPAT) $(LDFLAGS) -o ex1

ex2: ex2.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) ex2.cpp $(COMPAT) $(LDFLAGS) -o ex2

ex3: ex3.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) ex3.cpp $(COMPAT) $(LDFLAGS) -o ex3

ex4: ex4.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) ex4.cpp $(COMPAT) $(LDFLAGS) -o ex4

clean:
	rm -f ex1 ex2 ex3 ex4

.PHONY: all ex1 ex2 ex3 ex4 clean
