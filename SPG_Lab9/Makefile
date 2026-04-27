CXX      = g++
CXXFLAGS = -std=c++14 \
            -ISPGOpenGL/include \
            -I/usr/local/include \
            -DGL_SILENCE_DEPRECATION

LDFLAGS  = -L/usr/local/lib \
            -lGLEW -lglfw \
            -framework OpenGL \
            -framework Cocoa \
            -framework IOKit

SRCDIR   = SPGOpenGL
COMPAT   = freeglut_compat.cpp
TARGET   = lab9

all: $(TARGET)

$(TARGET): $(SRCDIR)/main.cpp $(SRCDIR)/objloader.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) $(SRCDIR)/main.cpp $(SRCDIR)/objloader.cpp $(COMPAT) $(LDFLAGS) -o $(TARGET)
	cp $(SRCDIR)/vertex.vert ./vertex.vert
	cp $(SRCDIR)/fragment.frag ./fragment.frag
	cp -r $(SRCDIR)/obj ./obj

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) vertex.vert fragment.frag
	rm -rf obj

.PHONY: all run clean
