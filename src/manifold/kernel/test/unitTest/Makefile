CXX = mpic++
CXXFLAGS += -DKERNEL_UTEST -DNO_MPI -I/usr/include/cppunit -I../../include/kernel
CXXFLAGS_MESSENGER = -DKERNEL_UTEST -I/usr/include/cppunit -I../../include/kernel
LDFLAGS += -lcppunit
EXECS = ClockTest ComponentTest LinkTest LinkOutputTest ManifoldConnectTest ManifoldScheduleTest ManifoldTest \
        MessengerTest0 MessengerTest_big_data1 MessengerTest_big_data2 tickObjTest 

VPATH = ../../src

ALL: $(EXECS)

ClockTest: ClockTest.o  clock.o manifold.o component.o
	$(CXX) -o$@ $(LDFLAGS) $^

ComponentTest: ComponentTest.o component.o manifold.o clock.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

LinkTest: LinkTest.o  clock.o manifold.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

LinkOutputTest: LinkOutputTest.o  clock.o manifold.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

ManifoldTest: ManifoldTest.o  clock.o manifold.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

ManifoldConnectTest: ManifoldConnectTest.o  clock.o manifold.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

ManifoldScheduleTest: ManifoldScheduleTest.o  clock.o manifold.o component.o link.o
	$(CXX) -o$@ $(LDFLAGS) $^

tickObjTest: tickObjTest.o  clock.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessengerTest0: MessengerTest0.o messenger.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessengerTest0.o: MessengerTest0.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS_MESSENGER) -MMD -MF dep/$*.d -c $< -o $*.o

MessengerTest_big_data1: MessengerTest_big_data1.o messenger.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessengerTest_big_data1.o: MessengerTest_big_data1.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS_MESSENGER) -MMD -MF dep/$*.d -c $< -o $*.o

MessengerTest_big_data2: MessengerTest_big_data2.o messenger.o
	$(CXX) -o$@ $(LDFLAGS) $^

MessengerTest_big_data2.o: MessengerTest_big_data2.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS_MESSENGER) -MMD -MF dep/$*.d -c $< -o $*.o

messenger.o: messenger.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS_MESSENGER) -MMD -MF dep/$*.d -c $< -o $*.o

%.o: %.cc
	@[ -d dep ] || mkdir dep
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MF dep/$*.d -c $< -o $*.o

-include $(wildcard dep/*.d)

.PHONY: clean
clean:
	rm -f $(EXECS) *.o DBG*
	rm -rf dep
