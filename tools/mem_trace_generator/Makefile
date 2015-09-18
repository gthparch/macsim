CXX = g++ 

XEDDIR = ./xed2-intel64/include
CFLAGS = -I$(XEDDIR) 
CXXFLAGS := -std=c++11 
LDFLAGS =-lz 
TARGET = mem_trace 
OBJECTS = mem_trace.o

all	: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJECTS)  $(LDFLAGS) 

clean :
	rm *.o $(TARGET)
