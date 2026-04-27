CXX      = g++
BREW_PREFIX := $(if $(wildcard /opt/homebrew),/opt/homebrew,/usr/local)
CXXFLAGS = -I. -Iinclude -I$(BREW_PREFIX)/include
LDFLAGS  = -L$(BREW_PREFIX)/lib -lGLEW -lglfw -framework OpenGL -framework Cocoa -framework IOKit
COMPAT   = freeglut_compat.cpp
COMMON   = lab4_common.cpp
TARGETS  = ex1 ex2 ex3 ex4 ex5

all: $(TARGETS)

ex1: ex1.cpp $(COMMON) $(COMPAT)
	$(CXX) $(CXXFLAGS) ex1.cpp $(COMMON) $(COMPAT) $(LDFLAGS) -o ex1

ex2: ex2.cpp $(COMMON) $(COMPAT)
	$(CXX) $(CXXFLAGS) ex2.cpp $(COMMON) $(COMPAT) $(LDFLAGS) -o ex2

ex3: ex3.cpp $(COMMON) $(COMPAT)
	$(CXX) $(CXXFLAGS) ex3.cpp $(COMMON) $(COMPAT) $(LDFLAGS) -o ex3

ex4: ex4.cpp $(COMMON) $(COMPAT)
	$(CXX) $(CXXFLAGS) ex4.cpp $(COMMON) $(COMPAT) $(LDFLAGS) -o ex4

ex5: ex5.cpp $(COMMON) $(COMPAT)
	$(CXX) $(CXXFLAGS) ex5.cpp $(COMMON) $(COMPAT) $(LDFLAGS) -o ex5

run-ex1: ex1
	./ex1

run-ex2: ex2
	./ex2

run-ex3: ex3
	./ex3

run-ex4: ex4
	./ex4

run-ex5: ex5
	./ex5

clean:
	rm -f $(TARGETS)

.PHONY: all clean run-ex1 run-ex2 run-ex3 run-ex4 run-ex5
