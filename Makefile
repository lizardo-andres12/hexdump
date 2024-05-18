CXX = g++
CXXFLAGS = -Wall -std=c++17
TARGET = xxd

SRCS = cmd_parser.cpp file_utils.cpp

OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)


$(TARGET): main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
