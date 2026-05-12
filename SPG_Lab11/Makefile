CXX      = g++
BREW_PREFIX := $(shell brew --prefix)
CXXFLAGS = -std=c++14 \
            -ISPGOpenGL \
            -ISPGOpenGL/include \
            -I$(BREW_PREFIX)/include \
            -DGL_SILENCE_DEPRECATION

LDFLAGS  = -L$(BREW_PREFIX)/lib \
            -lGLEW -lglfw \
            -framework OpenGL \
            -framework Cocoa \
            -framework IOKit

SRCDIR   = SPGOpenGL
COMPAT   = freeglut_compat.cpp
TARGET   = lab11

all: $(TARGET)

$(TARGET): $(SRCDIR)/main_old.cpp $(COMPAT)
	$(CXX) $(CXXFLAGS) $(SRCDIR)/main_old.cpp $(COMPAT) $(LDFLAGS) -o $(TARGET)
	cp $(SRCDIR)/vertex.vert ./vertex.vert
	cp $(SRCDIR)/fragment.frag ./fragment.frag
	cp $(SRCDIR)/wallg.jpg ./wallg.jpg
	cp $(SRCDIR)/texture-1874580_640.jpg ./texture-1874580_640.jpg

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) vertex.vert fragment.frag wallg.jpg texture-1874580_640.jpg

.PHONY: all run clean
