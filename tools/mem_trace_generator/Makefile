CXX = g++ 

XEDDIR = ./xed2-intel64/include
CFLAGS = -I$(XEDDIR)
LDFLAGS =-lz 
TARGET = mem_trace 
OBJECTS = mem_trace.o

all	: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)  $(LDFLAGS) 

clean :
	rm *.o $(TARGET)
