CXX      = g++
CXXFLAGS = -std=c++14 \
            -I$(SRCDIR)/include \
            -IGL \
            -I/usr/local/include \
            -DGL_SILENCE_DEPRECATION

LDFLAGS  = -L/usr/local/lib \
            -lGLEW -lglfw \
            -framework OpenGL \
            -framework Cocoa \
            -framework IOKit

SRCDIR   = SPGOpenGL/SPGOpenGL
COMPAT   = freeglut_compat.cpp
TARGET   = bratRobot

all: $(TARGET)

$(TARGET): $(SRCDIR)/main.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) $(SRCDIR)/main.cpp $(COMPAT) $(LDFLAGS) -o $(TARGET)
	cp $(SRCDIR)/vertex.vert ./vertex.vert
	cp $(SRCDIR)/fragment.frag ./fragment.frag

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
