CXX = mpic++ -Wall
CXXFLAGS += -DKERNEL_UTEST -I/usr/include/cppunit -I../../include/kernel
LDFLAGS += -lcppunit

EXECS = LBTSTest1 LBTSTest2 LBTSTest3 MessagingClockTest MessagingClockTest_ser MessagingClockTest_ser_pointer MessagingHalfClockTest MessagingHalfClockTest_ser MessagingHalfClockTest_ser_pointer MessagingHalfTest MessagingHalfTest_ser MessagingHalfTest_ser_pointer MessagingTest MessagingTest_ser MessagingTest_ser_pointer MessagingTimeTest MessagingTimeTest_ser MessagingTimeTest_ser_pointer

VPATH = ../../src

ALL: $(EXECS)


M: M.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

LBTSTest1: LBTSTest1.o  manifold.o clock.o messenger.o component.o
	$(CXX) -o$@ $(LDFLAGS) $^

LBTSTest2: LBTSTest2.o  manifold.o clock.o messenger.o component.o
	$(CXX) -o$@ $(LDFLAGS) $^

LBTSTest3: LBTSTest3.o  manifold.o clock.o messenger.o component.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingClockTest: MessagingClockTest.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingClockTest_ser: MessagingClockTest_ser.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingClockTest_ser_pointer: MessagingClockTest_ser_pointer.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingHalfClockTest: MessagingHalfClockTest.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingHalfClockTest_ser: MessagingHalfClockTest_ser.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingHalfClockTest_ser_pointer: MessagingHalfClockTest_ser_pointer.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingHalfTest: MessagingHalfTest.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingHalfTest_ser: MessagingHalfTest_ser.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingHalfTest_ser_pointer: MessagingHalfTest_ser_pointer.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingTest: MessagingTest.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingTest_ser: MessagingTest_ser.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingTest_ser_pointer: MessagingTest_ser_pointer.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingTimeTest: MessagingTimeTest.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingTimeTest_ser: MessagingTimeTest_ser.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessagingTimeTest_ser_pointer: MessagingTimeTest_ser_pointer.o  messenger.o manifold.o clock.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^


force: ;

%.o: %.cc
	@[ -d dep ] || mkdir dep
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MF dep/$*.d -c $< -o $*.o

-include $(wildcard dep/*.d)

.PHONY: clean
clean:
	rm -f $(EXECS) *.d *.o DBG*
	rm -rf dep
