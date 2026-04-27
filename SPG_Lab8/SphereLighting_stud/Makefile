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
TARGET   = sphere_lighting

all: $(TARGET)

$(TARGET): $(SRCDIR)/main.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) $(SRCDIR)/main.cpp $(COMPAT) $(LDFLAGS) -o $(TARGET)
	cp $(SRCDIR)/pixel_light.vert ./pixel_light.vert
	cp $(SRCDIR)/pixel_light.frag ./pixel_light.frag

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) pixel_light.vert pixel_light.frag

.PHONY: all run clean
