CXX      = g++
CXXFLAGS = -std=c++14 \
            -ISPGOpenGL/SPGOpenGL/include \
            -I/usr/local/include \
            -DGL_SILENCE_DEPRECATION

LDFLAGS  = -L/usr/local/lib \
            -lGLEW -lglfw \
            -framework OpenGL \
            -framework Cocoa \
            -framework IOKit

SRCDIR   = SPGOpenGL/SPGOpenGL
COMPAT   = freeglut_compat.cpp
TARGET   = triunghi

all: $(TARGET)

$(TARGET): $(SRCDIR)/main.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) $(SRCDIR)/main.cpp $(COMPAT) $(LDFLAGS) -o $(TARGET)
	cp $(SRCDIR)/vertex.vert ./vertex.vert
	cp $(SRCDIR)/fragment.frag ./fragment.frag
	cp $(SRCDIR)/wall.jpg ./wall.jpg
	cp $(SRCDIR)/texture-1874580_640.jpg ./texture-1874580_640.jpg

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) vertex.vert fragment.frag wall.jpg texture-1874580_640.jpg

.PHONY: all run clean
