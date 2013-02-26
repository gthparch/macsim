/**
This program tests member functions of the Manifold class other than the Connect()
and Schedule() functions.
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

class MyObj2 {
    private:
        Clock& m_clock;
	// m_tick is used to record the tick of associated clock at the
	// time when the handler is called.
        vector<Ticks_t> m_ticks;

	// m_time is used to record the simulation time at the
	// time when the handler is called.
	vector<double> m_times;

    public:
        MyObj2(Clock& clk) : m_clock(clk) {}

	Clock& getClock() const { return m_clock; }

        const vector<Ticks_t>& getTicks() const { return m_ticks; }
        const vector<double>& getTimes() const { return m_times; }

	void handler()
	{
	    m_ticks.push_back(m_clock.NowTicks());
	    m_times.push_back(Manifold::Now());
	}

	void testStop()
	{
	    m_ticks.push_back(m_clock.NowTicks());
	    m_times.push_back(Manifold::Now());
	    Manifold::Stop();
	}

	void testCancelEvent(EventId& id)
	{
	    m_ticks.push_back(m_clock.NowTicks());
	    m_times.push_back(Manifold::Now());
	    Manifold::Cancel(id);
	}

};



//####################################################################
// ManifoldTest is the unit test class for the class Manifold.
//####################################################################
class ManifoldTest : public CppUnit::TestFixture {
    private:
	static Clock MasterClock;  //clock has to be global or static.
	static Clock Clock1;  

	enum { MASTER_CLOCK_HZ = 10 };
	enum { CLOCK1_HZ = 4 };

	static const double DOUBLE_COMP_DELTA = 1.0E-5;

    public:
        /**
	 * Initialization function. Inherited from the CPPUnit framework.
	 */
        void setUp()
	{
	}


        //======================================================================
	// The following set of functions test functions such as Now(), NowTicks()
        //======================================================================
	/**
	 * Test Now()
	 */
        void testNow()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    double WHEN1 = 3.4;
	    //schedule 2 events
	    EventId ev = Manifold::ScheduleTime(WHEN1, &MyObj2::handler, comp);
	    double WHEN2 = WHEN1 + 1.2;
	    EventId ev2 = Manifold::ScheduleTime(WHEN2, &MyObj2::handler, comp);

	    double scheduling = Manifold::Now();  //current time

	    Manifold::StopAtTime(WHEN2+1);
	    Manifold::Run();

            const vector<double>& times = comp->getTimes();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduling + WHEN1, times[0], DOUBLE_COMP_DELTA);
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduling + WHEN2, times[1], DOUBLE_COMP_DELTA);

	    delete comp;
	}

	/**
	 * Test NowTicks()
	 */
        void testNowTicks()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    Ticks_t WHEN1 = 3;
	    //schedule 2 events
	    TickEventId ev = Manifold::Schedule(WHEN1, &MyObj2::handler, comp);
	    Ticks_t WHEN2 = WHEN1 + 2;
	    TickEventId ev2 = Manifold::Schedule(WHEN2, &MyObj2::handler, comp);

	    Ticks_t scheduling = Manifold::NowTicks();  //current time

	    Manifold::StopAt(WHEN2+1);
	    Manifold::Run();

            const vector<Ticks_t>& ticks = comp->getTicks();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduling + WHEN1, ticks[0]);
	    CPPUNIT_ASSERT_EQUAL(scheduling + WHEN2, ticks[1]);

	    delete comp;
	}

	/**
	 * Test Stop()
	 */
        void testStop()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    Ticks_t WHEN1 = 3;
	    TickEventId ev = Manifold::Schedule(WHEN1, &MyObj2::testStop, comp);

	    Ticks_t scheduling = Manifold::NowTicks();  //current time

	    Manifold::Run();

            const vector<Ticks_t>& ticks = comp->getTicks();

	    //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduling + WHEN1, ticks[0]);

	    //verify current tick is the tick when Stop() was called.
	    CPPUNIT_ASSERT_EQUAL(scheduling + WHEN1, Manifold::NowTicks());


	    delete comp;
	}

	/**
	 * Test StopAt()
	 */
        void testStopAt()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    Ticks_t WHEN = 3;
	    //schedule 3 events
	    TickEventId ev1 = Manifold::Schedule(WHEN, &MyObj2::handler, comp);
	    TickEventId ev2 = Manifold::Schedule(WHEN + 4, &MyObj2::handler, comp);
	    TickEventId ev3 = Manifold::Schedule(WHEN + 7, &MyObj2::handler, comp);

	    Ticks_t scheduling = Manifold::NowTicks();  //current time

            Ticks_t stopTick = WHEN+10;
	    Manifold::StopAt(stopTick);
	    Manifold::Run();

	    //verify current tick is same as the parameter for StopAt()
	    CPPUNIT_ASSERT_EQUAL(stopTick + scheduling, Manifold::NowTicks());

	    delete comp;
	}

	/**
	 * Test StopAtClock()
	 */
        void testStopAtClock()
	{
	    MyObj2* comp = new MyObj2(Clock1);

            Manifold::unhalt();
	    Ticks_t WHEN = 3;
	    //schedule 3 events
	    TickEventId ev1 = Manifold::ScheduleClock(WHEN, comp->getClock(), &MyObj2::handler, comp);
	    TickEventId ev2 = Manifold::ScheduleClock(WHEN + 4, comp->getClock(), &MyObj2::handler, comp);
	    TickEventId ev3 = Manifold::ScheduleClock(WHEN + 7, comp->getClock(), &MyObj2::handler, comp);

	    Ticks_t scheduling = comp->getClock().NowTicks();  //current time

            Ticks_t stopTick = WHEN+10;
	    Manifold::StopAtClock(stopTick, comp->getClock());
	    Manifold::Run();

	    //verify current tick is same as the parameter for StopAt()
	    CPPUNIT_ASSERT_EQUAL(stopTick + scheduling, comp->getClock().NowTicks());

	    delete comp;
	}

	/**
	 * Test StopAtTime()
	 */
        void testStopAtTime()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    double WHEN = 3.6;
	    //schedule 3 events
	    EventId ev1 = Manifold::ScheduleTime(WHEN, &MyObj2::handler, comp);
	    EventId ev2 = Manifold::ScheduleTime(WHEN + 1.3, &MyObj2::handler, comp);
	    EventId ev3 = Manifold::ScheduleTime(WHEN + 5.2, &MyObj2::handler, comp);

	    double scheduling = Manifold::Now();  //current time

            double stopTime = WHEN+10;
	    Manifold::StopAtTime(stopTime);
	    Manifold::Run();

	    //verify current time is same as the parameter for StopAt()
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(stopTime + scheduling, Manifold::Now(), DOUBLE_COMP_DELTA);

	    delete comp;
	}

	/**
	 * Test Cancel(EventId&)
	 */
        void testCancel_0()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    double WHEN1 = 0.36;
	    double WHEN2 = WHEN1 + 0.13;
	    EventId ev2 = Manifold::ScheduleTime(WHEN2, &MyObj2::handler, comp);
	    double WHEN3 = WHEN2 + 0.52;
	    EventId ev3 = Manifold::ScheduleTime(WHEN3, &MyObj2::handler, comp);

            //this event's handler will cancel ev2
	    EventId ev1 = Manifold::ScheduleTime(WHEN1, &MyObj2::testCancelEvent, comp, ev2);

	    double scheduling = Manifold::Now();  //current time

            double stopTime = WHEN3+1;
	    Manifold::StopAtTime(stopTime);
	    Manifold::Run();

            const vector<double>& times = comp->getTimes();
	    CPPUNIT_ASSERT(times.size() == 2);

	    //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduling + WHEN1, times[0], DOUBLE_COMP_DELTA);
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduling + WHEN3, times[1], DOUBLE_COMP_DELTA);

	    delete comp;
	}

	/**
	 * Test Peek()
	 */
        void testPeek()
	{
	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
	    double WHEN1 = 0.36;
	    double WHEN2 = WHEN1 + 0.13;
	    EventId ev2 = Manifold::ScheduleTime(WHEN2, &MyObj2::handler, comp);
	    double WHEN3 = WHEN2 + 0.52;
	    EventId ev3 = Manifold::ScheduleTime(WHEN3, &MyObj2::handler, comp);

	    EventId ev1 = Manifold::ScheduleTime(WHEN1, &MyObj2::handler, comp);

            EventId ei = Manifold::Peek();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(ev1.time, ei.time, DOUBLE_COMP_DELTA); //verify ev1 is 1st event
	    CPPUNIT_ASSERT_EQUAL(ev1.uid, ei.uid);

	    Manifold::StopAtTime(WHEN3+1);
	    Manifold::Run();
	    delete comp;
	}

	/**
	 * Test GetEarliestEvent()
	 */
        void testGetEarliestEvent()
	{
	    //No events scheduled, so GetEarliestEvent should return 0
	    EventBase* e = Manifold::GetEarliestEvent();

	    CPPUNIT_ASSERT_EQUAL_MESSAGE("Either the tested function is buggy or there are left-over events from previous testcases.", (EventBase*)0, e);


	    MyObj2* comp = new MyObj2(MasterClock);

            Manifold::unhalt();
            //now schedule some events
	    double WHEN1 = 0.36;
	    double WHEN2 = WHEN1 + 0.13;
	    EventId ev2 = Manifold::ScheduleTime(WHEN2, &MyObj2::handler, comp);
	    double WHEN3 = WHEN2 + 0.52;
	    EventId ev3 = Manifold::ScheduleTime(WHEN3, &MyObj2::handler, comp);

	    EventId ev1 = Manifold::ScheduleTime(WHEN1, &MyObj2::handler, comp);

            EventBase* eb1 = Manifold::GetEarliestEvent();

	    CPPUNIT_ASSERT_DOUBLES_EQUAL(ev1.time, eb1->time, DOUBLE_COMP_DELTA);
	    CPPUNIT_ASSERT_EQUAL(ev1.uid, eb1->uid);

            //run the simulation to process the scheduled events.
	    Manifold::StopAtTime(WHEN3+1);
	    Manifold::Run();

	    delete comp;
	}


        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("ManifoldTest");



	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testNow", &ManifoldTest::testNow));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testNowTicks", &ManifoldTest::testNowTicks));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testStop", &ManifoldTest::testStop));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testStopAt", &ManifoldTest::testStopAt));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testStopAtClock", &ManifoldTest::testStopAtClock));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testStopAtTime", &ManifoldTest::testStopAtTime));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testCandel_0", &ManifoldTest::testCancel_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testPeek", &ManifoldTest::testPeek));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testGetEarliestEvent", &ManifoldTest::testGetEarliestEvent));


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

