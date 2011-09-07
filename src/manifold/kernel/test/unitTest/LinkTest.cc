#include <TestFixture.h>
#include <TestAssert.h>
#include <TestSuite.h>
#include <Test.h>
#include <TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>

#include "link-decl.h"
#include "link.h"
#include "manifold.h"
#include "clock.h"

using namespace manifold::kernel;

//####################################################################
// helper classes
//####################################################################
class MyDataType1 {
    public:
        MyDataType1() : x(0) {}
        MyDataType1(int i) : x(i) {}
	bool operator==(const MyDataType1& other) { return x == other.x; }
    private:
        int x;
};

// class MyObj1 is like a component
class MyObj1 {
    private:
        Clock& m_clock;
	Ticks_t m_tickCalled; //time (in tick) when handler is called
	Time_t m_timeCalled; //time when handler is called
	MyDataType1 m_d1;
    public:
        MyObj1(Clock& c) : m_clock(c) {}

	void handler1(int i, MyDataType1 d)
	{
	    m_tickCalled = m_clock.NowTicks();
	    m_timeCalled = Manifold::Now();
	    m_d1 = d;
	}

        Clock& getClock() { return m_clock; }
	Ticks_t getTickCalled() { return m_tickCalled; }
	Time_t getTimeCalled() { return m_timeCalled; }
	MyDataType1 getD1() { return m_d1; }
};




//####################################################################
// Class LinkTest is the unit test class for class Link. 
//####################################################################
class LinkTest : public CppUnit::TestFixture {
    private:
        static Clock MasterClock;
	static const double DOUBLE_COMP_DELTA = 1.0E-5;

    public:
        /**
	 * Initialization function. Inherited from the CPPUnit framework.
	 */
        void setUp()
	{
	}


        /**
	 * Test Send()
	 */
	void testSend_0()
	{
	    Link<MyDataType1>* link1 = new Link<MyDataType1>();
	    MyObj1* obj1 = new MyObj1(MasterClock);

	    Clock& clock = obj1->getClock();

            Ticks_t latency = 1;
	    int inputIdx = 1;
            Time_t tLatency = 0;
	    bool isTimed = false;
	    bool isHalf = false;

	    link1->AddOutput<MyObj1>(&MyObj1::handler1, obj1, latency, inputIdx, &clock, tLatency, isTimed, isHalf);

	    Manifold::unhalt();
	    MyDataType1 d1(1234);
	    link1->Send(d1);
	    
	    Ticks_t scheduling = clock.NowTicks();

	    Manifold::StopAtClock(latency+1, clock);
	    Manifold::Run();

            //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduling + latency, obj1->getTickCalled());
	    //verify handler called with right parameter
	    CPPUNIT_ASSERT(d1 == obj1->getD1());

	    delete link1;
	    delete obj1;
	}

        /**
	 * Test Send()
	 * isTimed = false; isHalf = true;
	 */
	void testSend_1()
	{
	    //This test case uses SchuduleClockHalf; we should test different latencies.
	    Ticks_t lats[] = {1, 5, 2}; //various latency values

	    for(unsigned int i=0; i<sizeof(lats)/sizeof(Ticks_t); i++) {
		Link<MyDataType1>* link1 = new Link<MyDataType1>();
		MyObj1* obj1 = new MyObj1(MasterClock);

		Clock& clock = obj1->getClock();

		Ticks_t latency = lats[i];
		int inputIdx = 1;
		Time_t tLatency = 0;
		bool isTimed = false;
		bool isHalf = true;

		link1->AddOutput<MyObj1>(&MyObj1::handler1, obj1, latency, inputIdx, &clock, tLatency, isTimed, isHalf);

		Manifold::unhalt();
		MyDataType1 d1(1234);
		link1->Send(d1);
		
		Ticks_t scheduling = clock.NowTicks();

		Manifold::StopAtClock(latency+1, clock);
		Manifold::Run();

		//verify handler called at right time
		CPPUNIT_ASSERT_EQUAL(scheduling + latency/2, obj1->getTickCalled());
		//verify handler called with right parameter
		CPPUNIT_ASSERT(d1 == obj1->getD1());

		delete link1;
		delete obj1;
	    }
	}

        /**
	 * Test Send()
	 * isTimed = true; isHalf = false;
	 */
	void testSend_2()
	{
	    Link<MyDataType1>* link1 = new Link<MyDataType1>();
	    MyObj1* obj1 = new MyObj1(MasterClock);
	    Clock& clock = obj1->getClock();

            Ticks_t latency = 1;
	    int inputIdx = 1;
            Time_t tLatency = 0.23;
	    bool isTimed = true;
	    bool isHalf = false;

	    link1->AddOutput<MyObj1>(&MyObj1::handler1, obj1, latency, inputIdx, &clock, tLatency, isTimed, isHalf);

	    Manifold::unhalt();
	    MyDataType1 d1(1234);
	    link1->Send(d1);
	    
	    Time_t scheduling = Manifold::Now();

	    Manifold::StopAtTime(latency+1);
	    Manifold::Run();

            //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduling + tLatency, obj1->getTimeCalled(), DOUBLE_COMP_DELTA);
	    //verify handler called with right parameter
	    CPPUNIT_ASSERT(d1 == obj1->getD1());

	    delete link1;
	    delete obj1;
	}

        /**
	 * Test Send()
	 * isTimed = true; isHalf = true;
	 */
	void testSend_3()
	{
	    Link<MyDataType1>* link1 = new Link<MyDataType1>();
	    MyObj1* obj1 = new MyObj1(MasterClock);
	    Clock& clock = obj1->getClock();

            Ticks_t latency = 1;
	    int inputIdx = 1;
            Time_t tLatency = 0.23;
	    bool isTimed = true;
	    bool isHalf = true;

	    link1->AddOutput<MyObj1>(&MyObj1::handler1, obj1, latency, inputIdx, &clock, tLatency, isTimed, isHalf);

	    Manifold::unhalt();
	    MyDataType1 d1(1234);
	    link1->Send(d1);
	    
	    Time_t scheduling = Manifold::Now();

	    Manifold::StopAtTime(latency+1);
	    Manifold::Run();

            //verify handler called at right time
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduling + tLatency, obj1->getTimeCalled(), DOUBLE_COMP_DELTA);
	    //verify handler called with right parameter
	    CPPUNIT_ASSERT(d1 == obj1->getD1());

	    delete link1;
	    delete obj1;
	}

        /**
	 * Test SendTick()
	 * In this test case, the dynamically given tick latency is greater than the default latency.
	 */
	void testSendTick_0()
	{
	    Link<MyDataType1>* link1 = new Link<MyDataType1>();
	    MyObj1* obj1 = new MyObj1(MasterClock);

	    Clock& clock = obj1->getClock();

            Ticks_t latency = 1;
	    int inputIdx = 1;
            Time_t tLatency = 0;
	    bool isTimed = false;
	    bool isHalf = false;

	    link1->AddOutput<MyObj1>(&MyObj1::handler1, obj1, latency, inputIdx, &clock, tLatency, isTimed, isHalf);

	    Manifold::unhalt();
	    MyDataType1 d1(1234);
	    Ticks_t dynamic_latency = 2;
	    CPPUNIT_ASSERT(dynamic_latency > latency);

	    link1->SendTick(d1, dynamic_latency);
	    
	    Ticks_t scheduling = clock.NowTicks();

	    Manifold::StopAtClock(dynamic_latency+1, clock);
	    Manifold::Run();

            //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduling + dynamic_latency, obj1->getTickCalled());
	    //verify handler called with right parameter
	    CPPUNIT_ASSERT(d1 == obj1->getD1());

	    delete link1;
	    delete obj1;
	}

        /**
	 * Test SendTick()
	 * In this test case, the dynamically given tick latency is smaller than the default latency.
	 */
	void testSendTick_1()
	{
	    Link<MyDataType1>* link1 = new Link<MyDataType1>();
	    MyObj1* obj1 = new MyObj1(MasterClock);

	    Clock& clock = obj1->getClock();

            Ticks_t latency = 3;
	    int inputIdx = 1;
            Time_t tLatency = 0;
	    bool isTimed = false;
	    bool isHalf = false;

	    link1->AddOutput<MyObj1>(&MyObj1::handler1, obj1, latency, inputIdx, &clock, tLatency, isTimed, isHalf);

	    Manifold::unhalt();
	    MyDataType1 d1(1234);
	    Ticks_t dynamic_latency = 2;
	    CPPUNIT_ASSERT(dynamic_latency < latency);

	    link1->SendTick(d1, dynamic_latency);
	    
	    Ticks_t scheduling = clock.NowTicks();

	    Manifold::StopAtClock(dynamic_latency+1, clock);
	    Manifold::Run();

            //verify handler called at right time
	    CPPUNIT_ASSERT_EQUAL(scheduling + dynamic_latency, obj1->getTickCalled());
	    //verify handler called with right parameter
	    CPPUNIT_ASSERT(d1 == obj1->getD1());

	    delete link1;
	    delete obj1;
	}


        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("LinkTest");

	    mySuite->addTest(new CppUnit::TestCaller<LinkTest>("testSend_0", &LinkTest::testSend_0));
	    mySuite->addTest(new CppUnit::TestCaller<LinkTest>("testSend_1", &LinkTest::testSend_1));
	    mySuite->addTest(new CppUnit::TestCaller<LinkTest>("testSend_2", &LinkTest::testSend_2));
	    mySuite->addTest(new CppUnit::TestCaller<LinkTest>("testSend_3", &LinkTest::testSend_3));

	    mySuite->addTest(new CppUnit::TestCaller<LinkTest>("testSendTick_0", &LinkTest::testSendTick_0));
	    mySuite->addTest(new CppUnit::TestCaller<LinkTest>("testSendTick_1", &LinkTest::testSendTick_1));

	    return mySuite;
	}
};

Clock LinkTest::MasterClock(10);


int main()
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( LinkTest::suite() );
    if(runner.run("", false))
	return 0; //all is well
    else
	return 1;
}



