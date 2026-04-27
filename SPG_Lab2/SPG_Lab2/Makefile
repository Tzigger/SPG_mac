CXX = g++
CXXFLAGS = -std=c++11 -g $(shell fltk-config --cxxflags)
LDFLAGS = $(shell fltk-config --ldflags)

TARGET = SPG_Lab2
SRCS = main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp MyWidget.h MyWidget2.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	lldb ./$(TARGET)
