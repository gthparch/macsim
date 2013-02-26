/**
This program tests the Schedule() functions of the Manifold class.
*/
#include <TestFixture.h>
#include <TestAssert.h>
#include <TestSuite.h>
#include <Test.h>
#include <TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>

#include "manifold.h"
#include "component.h"
#include "link.h"

using namespace std;
using namespace manifold::kernel;


//####################################################################
// helper classes, functions, and data
//####################################################################
class MyDataType1 {
    public:
        MyDataType1() : x(0) {}
        MyDataType1(int i) : x(i) {}
	int getX() { return x; }
	bool operator==(MyDataType1& other) { return x == other.x; }
	//These two functions are only here so the program will compile.
	static int Serialize(MyDataType1&, unsigned char*){return 0;}
	static MyDataType1 Deserialize(unsigned char*, int) { return MyDataType1(); }
    private:
        int x;
};

class MyDataType2 {
    public:
        MyDataType2() : x(0) {}
        MyDataType2(int i) : x(i) {}
	int getX() { return x; }
	bool operator==(MyDataType2& other) { return x == other.x; }
    private:
        int x;
};

class MyDataType3 {
    public:
        MyDataType3() : x(0) {}
        MyDataType3(int i) : x(i) {}
	int getX() { return x; }
	bool operator==(MyDataType3& other) { return x == other.x; }
    private:
        int x;
};

class MyDataType4 {
    public:
        MyDataType4() : x(0) {}
        MyDataType4(int i) : x(i) {}
	int getX() { return x; }
	bool operator==(MyDataType4& other) { return x == other.x; }
    private:
        int x;
};




class MyObj1 {
    private:
        Clock& m_clock;
	// m_tick is used to record the tick of associated clock at the
	// time when the handler is called.
        Ticks_t m_tick;

	// m_time is used to record the simulation time at the
	// time when the handler is called.
	double m_time;
	MyDataType1 m_d1;
	MyDataType2 m_d2;
	MyDataType3 m_d3;
	MyDataType4 m_d4;
    public:
        MyObj1(Clock& clk) : m_clock(clk) {}

	Clock& getClock() const { return m_clock; }

        Ticks_t getTick() const { return m_tick; }
        double getTime() const { return m_time; }
	MyDataType1 getD1() const { return m_d1; }
	MyDataType2 getD2() const { return m_d2; }
	MyDataType3 getD3() const { return m_d3; }
	MyDataType4 getD4() const { return m_d4; }

	void handler0()
	{
	    m_tick = m_clock.NowTicks();
	    m_time = Manifold::Now();
	}

	void handler1(MyDataType1 d1)
	{
	    m_tick = m_clock.NowTicks();
	    m_time = Manifold::Now();
	    m_d1 = d1;
	}

	void handler2(MyDataType1 d1, MyDataType2 d2)
	{
	    m_tick = m_clock.NowTicks();
	    m_time = Manifold::Now();
	    m_d1 = d1;
	    m_d2 = d2;
	}

	void handler3(MyDataType1 d1, MyDataType2 d2, MyDataType3 d3)
	{
	    m_tick = m_clock.NowTicks();
	    m_time = Manifold::Now();
	    m_d1 = d1;
	    m_d2 = d2;
	    m_d3 = d3;
	}

	void handler4(MyDataType1 d1, MyDataType2 d2, MyDataType3 d3, MyDataType4 d4)
	{
	    m_tick = m_clock.NowTicks();
	    m_time = Manifold::Now();
	    m_d1 = d1;
	    m_d2 = d2;
	    m_d3 = d3;
	    m_d4 = d4;
	}
};


class StaticHandlers {
    private:
	// Static_tick is used to record the tick of associated clock at the
	// time when the handler is called.
	static Ticks_t Static_tick;
	// Static_time is used to record the simulation time at the
	// time when the handler is called.
	static double Static_time;
	static Clock* Static_clock;

	static MyDataType1 Static_d1;
	static MyDataType2 Static_d2;
	static MyDataType3 Static_d3;
	static MyDataType4 Static_d4;

    public:
	static Clock& GetClock() { return *Static_clock; }
        static void SetClock(Clock& clk) { Static_clock = &clk; }

        static Ticks_t getTick() { return Static_tick; }
        static double getTime() { return Static_time; }
        static MyDataType1 getD1() { return Static_d1; }
        static MyDataType2 getD2() { return Static_d2; }
        static MyDataType3 getD3() { return Static_d3; }
        static MyDataType4 getD4() { return Static_d4; }

	static void Static_handler0()
	{
	    Static_tick = Static_clock->NowTicks();
	    Static_time = Manifold::Now();
	}

	static void Static_handler1(MyDataType1 d1)
	{
	    Static_tick = Static_clock->NowTicks();
	    Static_time = Manifold::Now();
	    Static_d1 = d1;
	}

	static void Static_handler2(MyDataType1 d1, MyDataType2 d2)
	{
	    Static_tick = Static_clock->NowTicks();
	    Static_time = Manifold::Now();
	    Static_d1 = d1;
	    Static_d2 = d2;
	}

	static void Static_handler3(MyDataType1 d1, MyDataType2 d2, MyDataType3 d3)
	{
	    Static_tick = Static_clock->NowTicks();
	    Static_time = Manifold::Now();
	    Static_d1 = d1;
	    Static_d2 = d2;
	    Static_d3 = d3;
	}

	static void Static_handler4(MyDataType1 d1, MyDataType2 d2, MyDataType3 d3, MyDataType4 d4)
	{
	    Static_tick = Static_clock->NowTicks();
	    Static_time = Manifold::Now();
	    Static_d1 = d1;
	    Static_d2 = d2;
	    Static_d3 = d3;
	    Static_d4 = d4;
	}
};

Clock* StaticHandlers::Static_clock = 0;
Ticks_t StaticHandlers::Static_tick = 0;
double StaticHandlers::Static_time = 0;
MyDataType1 StaticHandlers::Static_d1;
MyDataType2 StaticHandlers::Static_d2;
MyDataType3 StaticHandlers::Static_d3;
MyDataType4 StaticHandlers::Static_d4;



//####################################################################
// ManifoldTest is the unit test class for the class Manifold.
//####################################################################
class ManifoldTest : public CppUnit::TestFixture {
    private:
	static Clock MasterClock;  //clock has to be global or static.
	static Clock Clock1;  

	enum { MASTER_CLOCK_HZ = 10 };
	enum { CLOCK1_HZ = 4 };

	MyObj1* comp1;
	MyObj1* comp2;
	// The following are parameters passed to the handlers.
	MyDataType1 m_d1;
	MyDataType2 m_d2;
	MyDataType3 m_d3;
	MyDataType4 m_d4;

	static const double DOUBLE_COMP_DELTA = 1.0E-5;

    public:
        /**
	 * Initialization function. Inherited from the CPPUnit framework.
	 */
        void setUp()
	{
	    comp1 = new MyObj1(MasterClock);
	    comp2 = new MyObj1(Clock1);    // component 2 uses Clock1
	    m_d1 = 567;   //randomly selected numbers.
	    m_d2 = 41;
	    m_d3 = 239;
	    m_d4 = 9013;
	}



        //======================================================================
	// The following set of functions test Schedule() for an object.
        //======================================================================
	/**
	 *  Test Schedule(Ticks_t, void(T::*handler)(void), OBJ*)
	 */
	void testSchedule_0()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;
	    //Note events are scheduled relative to current clock ticks.
	    TickEventId te = Manifold::Schedule(WHEN, &MyObj1::handler0, comp1);
	    Ticks_t scheduledAt = WHEN + comp1->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());
	}

	/**
	 * Test Schedule(Ticks_t, void(T::*handler)(U1), OBJ*, T1)
	 */
	void testSchedule_1()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 5;
	    TickEventId te = Manifold::Schedule(WHEN, &MyObj1::handler1, comp1, m_d1);
	    Ticks_t scheduledAt = WHEN + comp1->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1);
	}

	/**
	 * Test Schedule(Ticks_t, void(T::*handler)(U1, U2), OBJ*, T1, T2)
	 */
	void testSchedule_2()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 6;
	    TickEventId te = Manifold::Schedule(WHEN, &MyObj1::handler2, comp1, m_d1, m_d2);
	    Ticks_t scheduledAt = WHEN + comp1->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1 && comp1->getD2() == m_d2);
	}

	/**
	 * Test Schedule(Ticks_t, void(T::*handler)(U1, U2, U3), OBJ*, T1, T2, T3)
	 */
	void testSchedule_3()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 2;
	    TickEventId te = Manifold::Schedule(WHEN, &MyObj1::handler3, comp1, m_d1, m_d2, m_d3);
	    Ticks_t scheduledAt = WHEN + comp1->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1 && comp1->getD2() == m_d2 && comp1->getD3() == m_d3);
	}

	/**
	 * Test Schedule(Ticks_t, void(T::*handler)(U1, U2, U3, U4), OBJ*, T1, T2, T3, T4)
	 */
	void testSchedule_4()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;
	    TickEventId te = Manifold::Schedule(WHEN, &MyObj1::handler4, comp1, m_d1, m_d2, m_d3, m_d4);
	    Ticks_t scheduledAt = WHEN + comp1->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1 && comp1->getD2() == m_d2 &&
	                   comp1->getD3() == m_d3 && comp1->getD4() == m_d4);

	}


        //======================================================================
	// The following set of functions test ScheduleHalf() for an object.
        //======================================================================
	/**
	 * Test ScheduleHalf(Ticks_t, void(T::*handler)(void), OBJ*)
	 */
	void testScheduleHalf_0()
	{
	    Ticks_t WHEN = 1;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, &MyObj1::handler0, comp1);
		Ticks_t scheduledAt = WHEN/2 + comp1->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(T::*handler)(U1), OBJ*, T1)
	 */
	void testScheduleHalf_1()
	{
	    Ticks_t WHEN = 2;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, &MyObj1::handler1, comp1, m_d1);
		Ticks_t scheduledAt = WHEN/2 + comp1->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp1->getD1() == m_d1);
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(T::*handler)(U1, U2), OBJ*, T1, T2)
	 */
	void testScheduleHalf_2()
	{
	    Ticks_t WHEN = 2;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, &MyObj1::handler2, comp1, m_d1, m_d2);
		Ticks_t scheduledAt = WHEN/2 + comp1->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp1->getD1() == m_d1);
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(T::*handler)(U1, U2, U3), OBJ*, T1, T2, T3)
	 */
	void testScheduleHalf_3()
	{
	    Ticks_t WHEN = 2;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, &MyObj1::handler3, comp1, m_d1, m_d2, m_d3);
		Ticks_t scheduledAt = WHEN/2 + comp1->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp1->getD1() == m_d1);
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(T::*handler)(U1, U2, U3, U4), OBJ*, T1, T2, T3, T4)
	 */
	void testScheduleHalf_4()
	{
	    Ticks_t WHEN = 2;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, &MyObj1::handler4, comp1, m_d1, m_d2, m_d3, m_d4);
		Ticks_t scheduledAt = WHEN/2 + comp1->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp1->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp1->getD1() == m_d1);
	    }
	}


        //======================================================================
	// The following set of functions test ScheduleClock() for an object.
        //======================================================================
	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(T::*handler)(void), OBJ*)
	 */
	void testScheduleClock_0()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, &MyObj1::handler0, comp2);
	    Ticks_t scheduledAt = WHEN + comp2->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(T::*handler)(U1), OBJ*, T1)
	 */
	void testScheduleClock_1()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, &MyObj1::handler1, comp2, m_d1);
	    Ticks_t scheduledAt = WHEN + comp2->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp2->getD1() == m_d1);
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(T::*handler)(U1, U2), OBJ*, T1, T2)
	 */
	void testScheduleClock_2()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, &MyObj1::handler2, comp2, m_d1, m_d2);
	    Ticks_t scheduledAt = WHEN + comp2->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp2->getD1() == m_d1 && comp2->getD2() == m_d2);
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(T::*handler)(U1, U2, U3), OBJ*, T1, T2, T3)
	 */
	void testScheduleClock_3()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, &MyObj1::handler3, comp2, m_d1, m_d2, m_d3);
	    Ticks_t scheduledAt = WHEN + comp2->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp2->getD1() == m_d1 && comp2->getD2() == m_d2 &&
	                   comp2->getD3() == m_d3);
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(T::*handler)(U1, U2, U3, U4), OBJ*, T1, T2, T3, T4)
	 */
	void testScheduleClock_4()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, &MyObj1::handler4, comp2, m_d1, m_d2, m_d3, m_d4);
	    Ticks_t scheduledAt = WHEN + comp2->getClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp2->getD1() == m_d1 && comp2->getD2() == m_d2 &&
	                   comp2->getD3() == m_d3 && comp2->getD4() == m_d4);
        }

        //======================================================================
	// The following set of functions test ScheduleClockHalf() for an object.
        //======================================================================
	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(T::*handler)(void), OBJ*)
	 */
	void testScheduleClockHalf_0()
	{
	    Ticks_t WHEN = 3;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, &MyObj1::handler0, comp2);
		Ticks_t scheduledAt = WHEN/2 + comp2->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(T::*handler)(U1), OBJ*, T1)
	 */
	void testScheduleClockHalf_1()
	{
	    Ticks_t WHEN = 3;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, &MyObj1::handler1, comp2, m_d1);
		Ticks_t scheduledAt = WHEN/2 + comp2->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp2->getD1() == m_d1);
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(T::*handler)(U1, U2), OBJ*, T1, T2)
	 */
	void testScheduleClockHalf_2()
	{
	    Ticks_t WHEN = 3;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, &MyObj1::handler2, comp2, m_d1, m_d2);
		Ticks_t scheduledAt = WHEN/2 + comp2->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp2->getD1() == m_d1 && comp2->getD2() == m_d2);
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(T::*handler)(U1, U2, U3), OBJ*, T1, T2, T3)
	 */
	void testScheduleClockHalf_3()
	{
	    Ticks_t WHEN = 3;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, &MyObj1::handler3, comp2, m_d1, m_d2, m_d3);
		Ticks_t scheduledAt = WHEN/2 + comp2->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp2->getD1() == m_d1 && comp2->getD2() == m_d2 &&
		               comp2->getD3() == m_d3);
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(T::*handler)(U1, U2, U3, U4), OBJ*, T1, T2, T3, T4)
	 */
	void testScheduleClockHalf_4()
	{
	    Ticks_t WHEN = 3;

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, &MyObj1::handler4, comp2, m_d1, m_d2, m_d3, m_d4);
		Ticks_t scheduledAt = WHEN/2 + comp2->getClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, comp2->getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(comp2->getD1() == m_d1 && comp2->getD2() == m_d2 &&
		               comp2->getD3() == m_d3 && comp2->getD4() == m_d4);
	    }
	}


        //======================================================================
	// The following set of functions test ScheduleTime() for an object.
        //======================================================================
	/**
	 *  Test ScheduleTime(double, void(T::*handler)(void), OBJ*)
	 */
	void testScheduleTime_0()
	{
            Manifold::unhalt();
	    double WHEN = 3.4;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, &MyObj1::handler0, comp1);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, comp1->getTime(), DOUBLE_COMP_DELTA);
	}

	/**
	 *  Test ScheduleTime(double, void(T::*handler)(U1), OBJ*, T1)
	 */
	void testScheduleTime_1()
	{
            Manifold::unhalt();
	    double WHEN = 2.7;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, &MyObj1::handler1, comp1, m_d1);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, comp1->getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1);
	}

	/**
	 *  Test ScheduleTime(double, void(T::*handler)(U1, U2), OBJ*, T1, T2)
	 */
	void testScheduleTime_2()
	{
            Manifold::unhalt();
	    double WHEN = 2.7;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, &MyObj1::handler2, comp1, m_d1, m_d2);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, comp1->getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1 && comp1->getD2() == m_d2);
	}

	/**
	 *  Test ScheduleTime(double, void(T::*handler)(U1, U2, U3), OBJ*, T1, T2, T3)
	 */
	void testScheduleTime_3()
	{
            Manifold::unhalt();
	    double WHEN = 2.7;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, &MyObj1::handler3, comp1, m_d1, m_d2, m_d3);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, comp1->getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1 && comp1->getD2() == m_d2 &&
	                   comp1->getD3() == m_d3);
	}

	/**
	 *  Test ScheduleTime(double, void(T::*handler)(U1, U2, U3, U4), OBJ*, T1, T2, T3, T4)
	 */
	void testScheduleTime_4()
	{
            Manifold::unhalt();
	    double WHEN = 2.7;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, &MyObj1::handler4, comp1, m_d1, m_d2, m_d3, m_d4);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, comp1->getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(comp1->getD1() == m_d1 && comp1->getD2() == m_d2 &&
	                   comp1->getD3() == m_d3 && comp1->getD4() == m_d4);
	}




        //======================================================================
	// The following set of functions test Schedule() for static callbacks.
        //======================================================================
	/**
	 * Test Schedule(Ticks_t, void(*handler)(void))
	 */
	void testSchedule_s0()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 2;
	    //Note events are scheduled relative to current clock ticks.
	    StaticHandlers::SetClock(MasterClock);

	    TickEventId te = Manifold::Schedule(WHEN, &StaticHandlers::Static_handler0);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());
	}

	/**
	 * Test Schedule(Ticks_t, void(*handler)(U1), T1)
	 */
	void testSchedule_s1()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 5;
	    //Note events are scheduled relative to current clock ticks.
	    StaticHandlers::SetClock(MasterClock);

	    TickEventId te = Manifold::Schedule(WHEN, &StaticHandlers::Static_handler1, m_d1);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1);
	}

	/**
	 * Test Schedule(Ticks_t, void(*handler)(U1, U2), T1, T2)
	 */
	void testSchedule_s2()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 6;
	    //Note events are scheduled relative to current clock ticks.
	    StaticHandlers::SetClock(MasterClock);

	    TickEventId te = Manifold::Schedule(WHEN, &StaticHandlers::Static_handler2, m_d1, m_d2);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2);
	}

	/**
	 * Test Schedule(Ticks_t, void(*handler)(U1, U2, U3), T1, T2, T3)
	 */
	void testSchedule_s3()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 2;
	    //Note events are scheduled relative to current clock ticks.
	    StaticHandlers::SetClock(MasterClock);

	    TickEventId te = Manifold::Schedule(WHEN, &StaticHandlers::Static_handler3, m_d1, m_d2, m_d3);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
	                   StaticHandlers::getD3() == m_d3);
	}

	/**
	 * Test Schedule(Ticks_t, void(*handler)(U1, U2, U3, U4), T1, T2, T3, T4)
	 */
	void testSchedule_s4()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;
	    //Note events are scheduled relative to current clock ticks.
	    StaticHandlers::SetClock(MasterClock);

	    TickEventId te = Manifold::Schedule(WHEN, &StaticHandlers::Static_handler4, m_d1, m_d2, m_d3, m_d4);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt(WHEN + 1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
	                   StaticHandlers::getD3() == m_d3 && StaticHandlers::getD4() == m_d4);
	}


        //======================================================================
	// The following set of functions test ScheduleHalf() for static callbacks.
        //======================================================================
	/**
	 * Test ScheduleHalf(Ticks_t, void(*handler)(void))
	 */
	void testScheduleHalf_s0()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(MasterClock);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, StaticHandlers::Static_handler0);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(*handler)(U1), T1)
	 */
	void testScheduleHalf_s1()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(MasterClock);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, StaticHandlers::Static_handler1, m_d1);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1);
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(*handler)(U1, U2), T1, T2)
	 */
	void testScheduleHalf_s2()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(MasterClock);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, StaticHandlers::Static_handler2, m_d1, m_d2);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2);
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(*handler)(U1, U2, U3), T1, T2, T3)
	 */
	void testScheduleHalf_s3()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(MasterClock);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, StaticHandlers::Static_handler3, m_d1, m_d2, m_d3);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
		               StaticHandlers::getD3() == m_d3);
	    }
	}

	/**
	 * Test ScheduleHalf(Ticks_t, void(*handler)(U1, U2, U3, U4), T1, T2, T3, T4)
	 */
	void testScheduleHalf_s4()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(MasterClock);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleHalf(WHEN, StaticHandlers::Static_handler4, m_d1, m_d2, m_d3, m_d4);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt(WHEN+1);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
		               StaticHandlers::getD3() == m_d3 && StaticHandlers::getD4() == m_d4);
	    }
	}


        //======================================================================
	// The following set of functions test ScheduleClock() for static callbacks.
        //======================================================================
	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(*handler)(void))
	 */
	void testScheduleClock_s0()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, StaticHandlers::Static_handler0);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(*handler)(U1), T1)
	 */
	void testScheduleClock_s1()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, StaticHandlers::Static_handler1, m_d1);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1);
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(*handler)(U1, U2), T1, T2)
	 */
	void testScheduleClock_s2()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, StaticHandlers::Static_handler2, m_d1, m_d2);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2);
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(*handler)(U1, U2, U3), T1, T2, T3)
	 */
	void testScheduleClock_s3()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, StaticHandlers::Static_handler3, m_d1, m_d2, m_d3);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
	                   StaticHandlers::getD3() == m_d3);
        }

	/**
	 * Test ScheduleClock(Ticks_t, Clock&, void(*handler)(U1, U2, U3, U4), T1, T2, T3, T4)
	 */
	void testScheduleClock_s4()
	{
            Manifold::unhalt();
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

	    TickEventId te = Manifold::ScheduleClock(WHEN, Clock1, StaticHandlers::Static_handler4, m_d1, m_d2, m_d3, m_d4);
	    Ticks_t scheduledAt = WHEN + StaticHandlers::GetClock().NowTicks();

	    CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

	    Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
	                   StaticHandlers::getD3() == m_d3 && StaticHandlers::getD4() == m_d4);
        }


        //======================================================================
	// The following set of functions test ScheduleClockHalf() for static callbacks.
        //======================================================================
	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(*handler)(void))
	 */
	void testScheduleClockHalf_s0()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, StaticHandlers::Static_handler0);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(*handler)(U1), T1)
	 */
	void testScheduleClockHalf_s1()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, StaticHandlers::Static_handler1, m_d1);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1);
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(*handler)(U1, U2), T1, T2)
	 */
	void testScheduleClockHalf_s2()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, StaticHandlers::Static_handler2, m_d1, m_d2);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2);
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(*handler)(U1, U2, U3), T1, T2, T3)
	 */
	void testScheduleClockHalf_s3()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, StaticHandlers::Static_handler3, m_d1, m_d2, m_d3);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
		               StaticHandlers::getD3() == m_d3);
	    }
	}

	/**
	 * Test ScheduleClockHalf(Ticks_t, Clock&, void(*handler)(U1, U2, U3, U4), T1, T2, T3, T4)
	 */
	void testScheduleClockHalf_s4()
	{
	    Ticks_t WHEN = 3;

	    StaticHandlers::SetClock(Clock1);

            // There are 2 test cases here. We test both even and odd number of half ticks.
	    for(int i=0; i<2; i++) {
		Manifold::unhalt();
	        WHEN += i;
		//Note events are scheduled relative to current clock ticks.
		TickEventId te = Manifold::ScheduleClockHalf(WHEN, Clock1, StaticHandlers::Static_handler4, m_d1, m_d2, m_d3, m_d4);
		Ticks_t scheduledAt = WHEN/2 + StaticHandlers::GetClock().NowTicks();

		CPPUNIT_ASSERT_EQUAL(scheduledAt, te.time); // CPPUNIT_ASSERT_EQUAL(expected, actual)

		Manifold::StopAt((WHEN+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduledAt, StaticHandlers::getTick());

		//verify handler called with right parameters
		CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
		               StaticHandlers::getD3() == m_d3 && StaticHandlers::getD4() == m_d4);
	    }
	}


        //======================================================================
	// The following set of functions test ScheduleTime() for static callbacks.
        //======================================================================
	/**
	 *  Test ScheduleTime(double, void(*handler)(void))
	 */
	void testScheduleTime_s0()
	{
            Manifold::unhalt();
	    double WHEN = 3.4;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, StaticHandlers::Static_handler0);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, StaticHandlers::getTime(), DOUBLE_COMP_DELTA);
	}

	/**
	 *  Test ScheduleTime(double, void(*handler)(U1), T1)
	 */
	void testScheduleTime_s1()
	{
            Manifold::unhalt();
	    double WHEN = 3.4;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, StaticHandlers::Static_handler1, m_d1);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, StaticHandlers::getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1);
	}

	/**
	 *  Test ScheduleTime(double, void(*handler)(U1, U2), T1, T2)
	 */
	void testScheduleTime_s2()
	{
            Manifold::unhalt();
	    double WHEN = 3.4;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, StaticHandlers::Static_handler2, m_d1, m_d2);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, StaticHandlers::getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2);
	}

	/**
	 *  Test ScheduleTime(double, void(*handler)(U1, U2, U3), T1, T2, T3)
	 */
	void testScheduleTime_s3()
	{
            Manifold::unhalt();
	    double WHEN = 3.4;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, StaticHandlers::Static_handler3, m_d1, m_d2, m_d3);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, StaticHandlers::getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
	                   StaticHandlers::getD3() == m_d3);
	}

	/**
	 *  Test ScheduleTime(double, void(*handler)(U1, U2, U3, U4), T1, T2, T3, T4)
	 */
	void testScheduleTime_s4()
	{
            Manifold::unhalt();
	    double WHEN = 3.4;
	    //Note events are scheduled relative to current time.
	    EventId ev = Manifold::ScheduleTime(WHEN, StaticHandlers::Static_handler4, m_d1, m_d2, m_d3, m_d4);
	    double scheduledAt = WHEN + Manifold::Now();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, ev.time, DOUBLE_COMP_DELTA); // CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, actual, delta)

	    Manifold::StopAtTime(WHEN+1);
	    Manifold::Run();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt, StaticHandlers::getTime(), DOUBLE_COMP_DELTA);

	    //verify handler called with right parameters
	    CPPUNIT_ASSERT(StaticHandlers::getD1() == m_d1 && StaticHandlers::getD2() == m_d2 &&
	                   StaticHandlers::getD3() == m_d3 && StaticHandlers::getD4() == m_d4);
	}


        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("ManifoldTest");




	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_0", &ManifoldTest::testSchedule_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_1", &ManifoldTest::testSchedule_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_2", &ManifoldTest::testSchedule_2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_3", &ManifoldTest::testSchedule_3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_4", &ManifoldTest::testSchedule_4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_0", &ManifoldTest::testScheduleHalf_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_1", &ManifoldTest::testScheduleHalf_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_2", &ManifoldTest::testScheduleHalf_2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_3", &ManifoldTest::testScheduleHalf_3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_4", &ManifoldTest::testScheduleHalf_4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_0", &ManifoldTest::testScheduleClock_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_1", &ManifoldTest::testScheduleClock_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_2", &ManifoldTest::testScheduleClock_2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_3", &ManifoldTest::testScheduleClock_3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_4", &ManifoldTest::testScheduleClock_4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_0", &ManifoldTest::testScheduleClockHalf_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_1", &ManifoldTest::testScheduleClockHalf_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_2", &ManifoldTest::testScheduleClockHalf_2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_3", &ManifoldTest::testScheduleClockHalf_3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_4", &ManifoldTest::testScheduleClockHalf_4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_0", &ManifoldTest::testScheduleTime_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_1", &ManifoldTest::testScheduleTime_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_2", &ManifoldTest::testScheduleTime_2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_3", &ManifoldTest::testScheduleTime_3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_4", &ManifoldTest::testScheduleTime_4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_s0", &ManifoldTest::testSchedule_s0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_s1", &ManifoldTest::testSchedule_s1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_s2", &ManifoldTest::testSchedule_s2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_s3", &ManifoldTest::testSchedule_s3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testSchedule_s4", &ManifoldTest::testSchedule_s4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_s0", &ManifoldTest::testScheduleHalf_s0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_s1", &ManifoldTest::testScheduleHalf_s1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_s2", &ManifoldTest::testScheduleHalf_s2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_s3", &ManifoldTest::testScheduleHalf_s3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleHalf_s4", &ManifoldTest::testScheduleHalf_s4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_s0", &ManifoldTest::testScheduleClock_s0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_s1", &ManifoldTest::testScheduleClock_s1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_s2", &ManifoldTest::testScheduleClock_s2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_s3", &ManifoldTest::testScheduleClock_s3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClock_s4", &ManifoldTest::testScheduleClock_s4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_s0", &ManifoldTest::testScheduleClockHalf_s0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_s1", &ManifoldTest::testScheduleClockHalf_s1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_s2", &ManifoldTest::testScheduleClockHalf_s2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_s3", &ManifoldTest::testScheduleClockHalf_s3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleClockHalf_s4", &ManifoldTest::testScheduleClockHalf_s4));

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_s0", &ManifoldTest::testScheduleTime_s0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_s1", &ManifoldTest::testScheduleTime_s1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_s2", &ManifoldTest::testScheduleTime_s2));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_s3", &ManifoldTest::testScheduleTime_s3));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testScheduleTime_s4", &ManifoldTest::testScheduleTime_s4));
	    /*
	    */

	    return mySuite;
	}
};

Clock ManifoldTest::MasterClock(MASTER_CLOCK_HZ);
Clock ManifoldTest::Clock1(CLOCK1_HZ);



int main()
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( ManifoldTest::suite() );
    if(runner.run("", false))
	return 0; //all is well
    else
	return 1;
}

