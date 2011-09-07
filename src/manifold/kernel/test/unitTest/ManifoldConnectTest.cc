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


//####################################################################
// The following classes are used to test the Connect() functions.
//####################################################################

//    | c0 |--------------|    |
//                        | c2 |
//    | c1 |--------------|    |


class MyC0 : public Component 
{
    private:
        uint64_t m_data;
    public:
	enum { OutData };

        void setData(uint64_t d) { m_data= d; }
	void SendIt() {
	Send(OutData, m_data); }
};

class MyC1 : public Component 
{
    private:
        MyDataType1 m_d1;

    public:
	enum { OutData };

        void setData(MyDataType1 d) { m_d1 = d; }
	void SendIt() { 
	Send(OutData, m_d1); }
};


class MyC2 : public Component 
{
    private:
        Ticks_t m_tick;
        Ticks_t m_halftick;
        double m_time;
	uint64_t m_data_0;
	MyDataType1 m_data_1;
	Clock* m_clock;
    public:
	enum { InData0,  InData1 };
	enum { OutData0, OutData1};
  
        MyC2() : m_clock(0) {}
        void setClock(Clock& c) { m_clock = &c; }

	Ticks_t getTick() { return m_tick; }
	Ticks_t getHalfTick() { return m_halftick; }
	double getTime() { return m_time; }
	uint64_t getData_0() { return m_data_0; }
	MyDataType1 getData_1() { return m_data_1; }

	void GotInputOn0(int inputId, uint64_t data)
	{
	    assert(m_clock != 0);
	    m_tick = m_clock->NowTicks();
	    m_halftick = m_clock->NowHalfTicks();
	    m_time = Manifold::Now();
	    m_data_0 = data;
	}

	void GotInputOn1(int inputId, MyDataType1 data)
	{
	    assert(m_clock != 0);
	    m_tick = m_clock->NowTicks();
	    m_halftick = m_clock->NowHalfTicks();
	    m_time = Manifold::Now();
	    m_data_1 = data;
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
	// The following set of functions test Connect()
        //======================================================================
	/**
	 * Test Connect(CompId_t, int, CompId_t, int, void (T::*handler)(int, uint64_t), Ticks_t)
	 */
	 void testConnect_0()
	 {
	    CompId_t c0 = Component::Create<MyC0>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC0* c0p = Component::GetComponent<MyC0>(c0);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            uint64_t C0_OutputData = 123;

	    c0p->setData(C0_OutputData);
	    c2p->setClock(MasterClock);

            Ticks_t LinkLatency = 1;

	    Manifold::Connect(c0, MyC0::OutData, c2, MyC2::InData0, &MyC2::GotInputOn0, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 1;
	    Manifold::Schedule(When, &MyC0::SendIt, c0p);
	    Ticks_t scheduledAt = When + MasterClock.NowTicks();

	    Manifold::StopAt(scheduledAt + LinkLatency + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT_EQUAL(C0_OutputData, c2p->getData_0());
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getTick());
	 }

	/**
	 * Test Connect(CompId_t, int, CompId_t, int, void (T::*handler)(int, T2), Ticks_t)
	 */
	 void testConnect_1()
	 {
	    CompId_t c1 = Component::Create<MyC1>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC1* c1p = Component::GetComponent<MyC1>(c1);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            MyDataType1 d1(328); // a random number

	    c1p->setData(d1);
	    c2p->setClock(MasterClock);

            Ticks_t LinkLatency = 1;

	    Manifold::Connect(c1, MyC1::OutData, c2, MyC2::InData1, &MyC2::GotInputOn1, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 1;
	    Manifold::Schedule(When, &MyC1::SendIt, c1p);
	    Ticks_t scheduledAt = When + MasterClock.NowTicks();

	    Manifold::StopAt(scheduledAt + LinkLatency + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT(c2p->getData_1() == d1);
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getTick());
	 }


	/**
	 * Test ConnectHalf(CompId_t, int, CompId_t, int, void (T::*handler)(int, uint64_t), Ticks_t)
	 */
	 void testConnectHalf_0()
	 {
	    CompId_t c0 = Component::Create<MyC0>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC0* c0p = Component::GetComponent<MyC0>(c0);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            uint64_t C0_OutputData = 123;

	    c0p->setData(C0_OutputData);
	    c2p->setClock(MasterClock);

            Ticks_t LinkLatency = 1;

	    Manifold::ConnectHalf(c0, MyC0::OutData, c2, MyC2::InData0, &MyC2::GotInputOn0, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 3;
	    Manifold::ScheduleHalf(When, &MyC0::SendIt, c0p);
	    Ticks_t scheduledAt = When + MasterClock.NowHalfTicks(); //in half ticks

	    Manifold::StopAt((scheduledAt + LinkLatency)/2 + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT_EQUAL(C0_OutputData, c2p->getData_0());
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getHalfTick());
	 }

	/**
	 * Test ConnectHalf(CompId_t, int, CompId_t, int, void (T::*handler)(int, T2), Ticks_t)
	 */
	 void testConnectHalf_1()
	 {
	    CompId_t c1 = Component::Create<MyC1>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC1* c1p = Component::GetComponent<MyC1>(c1);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            MyDataType1 d1(328); // a random number

	    c1p->setData(d1);
	    c2p->setClock(MasterClock);

            Ticks_t LinkLatency = 1;

	    Manifold::ConnectHalf(c1, MyC1::OutData, c2, MyC2::InData1, &MyC2::GotInputOn1, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 1;
	    Manifold::ScheduleHalf(When, &MyC1::SendIt, c1p);
	    Ticks_t scheduledAt = When + MasterClock.NowHalfTicks(); //in half ticks

	    Manifold::StopAt((scheduledAt + LinkLatency)/2 + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT(c2p->getData_1() == d1);
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getHalfTick());
	 }

	/**
	 * Test ConnectClock(CompId_t, int, CompId_t, int, Clock&, void (T::*handler)(int, uint64_t), Ticks_t)
	 */
	 void testConnectClock_0()
	 {
	    CompId_t c0 = Component::Create<MyC0>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC0* c0p = Component::GetComponent<MyC0>(c0);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            uint64_t C0_OutputData = 123;

	    c0p->setData(C0_OutputData);
	    c2p->setClock(Clock1);

            Ticks_t LinkLatency = 1;

	    Manifold::ConnectClock(c0, MyC0::OutData, c2, MyC2::InData0, Clock1, &MyC2::GotInputOn0, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 1;
	    Manifold::ScheduleClock(When, Clock1, &MyC0::SendIt, c0p); //schedule c0 on Clock1
	    Ticks_t scheduledAt = When + Clock1.NowTicks();

	    Manifold::StopAt((scheduledAt+LinkLatency+1) * (MASTER_CLOCK_HZ/CLOCK1_HZ + 1));
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT_EQUAL(C0_OutputData, c2p->getData_0());
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getTick());
	 }

	/**
	 * Test ConnectClock(CompId_t, int, CompId_t, int, Clock&, void (T::*handler)(int, T2), Ticks_t)
	 */
	 void testConnectClock_1()
	 {
	    CompId_t c1 = Component::Create<MyC1>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC1* c1p = Component::GetComponent<MyC1>(c1);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            MyDataType1 d1(328); // a random number

	    c1p->setData(d1);
	    c2p->setClock(Clock1);

            Ticks_t LinkLatency = 1;

	    Manifold::ConnectClock(c1, MyC1::OutData, c2, MyC2::InData1, Clock1, &MyC2::GotInputOn1, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 1;
	    Manifold::ScheduleClock(When, Clock1, &MyC1::SendIt, c1p);
	    Ticks_t scheduledAt = When + Clock1.NowTicks();

	    Manifold::StopAt(scheduledAt + LinkLatency + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT(c2p->getData_1() == d1);
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getTick());
	 }

	/**
	 * Test ConnectClockHalf(CompId_t, int, CompId_t, int, Clock&, void (T::*handler)(int, uint64_t), Ticks_t)
	 */
	 void testConnectClockHalf_0()
	 {
	    CompId_t c0 = Component::Create<MyC0>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC0* c0p = Component::GetComponent<MyC0>(c0);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            uint64_t C0_OutputData = 1371;

	    c0p->setData(C0_OutputData);
	    c2p->setClock(Clock1);

            Ticks_t LinkLatency = 1;

	    Manifold::ConnectClockHalf(c0, MyC0::OutData, c2, MyC2::InData0, Clock1, &MyC2::GotInputOn0, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 3;
	    Manifold::ScheduleClockHalf(When, Clock1, &MyC0::SendIt, c0p);
	    Ticks_t scheduledAt = When + Clock1.NowHalfTicks(); //in half ticks

	    Manifold::StopAtClock((scheduledAt + LinkLatency)/2 + 1, Clock1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT_EQUAL(C0_OutputData, c2p->getData_0());
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getHalfTick());
	 }

	/**
	 * Test ConnectClockHalf(CompId_t, int, CompId_t, int, Clock&, void (T::*handler)(int, T2), Ticks_t)
	 */
	 void testConnectClockHalf_1()
	 {
	    CompId_t c1 = Component::Create<MyC1>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC1* c1p = Component::GetComponent<MyC1>(c1);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            MyDataType1 d1(328); // a random number

	    c1p->setData(d1);
	    c2p->setClock(Clock1);

            Ticks_t LinkLatency = 1;

	    Manifold::ConnectClockHalf(c1, MyC1::OutData, c2, MyC2::InData1, Clock1, &MyC2::GotInputOn1, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 3;
	    Manifold::ScheduleClockHalf(When, Clock1, &MyC1::SendIt, c1p);
	    Ticks_t scheduledAt = When + Clock1.NowHalfTicks(); //in half ticks

	    Manifold::StopAtClock((scheduledAt + LinkLatency)/2 + 1, Clock1);
	    Manifold::Run();


            //verify c2 get correct data
	    CPPUNIT_ASSERT(c2p->getData_1() == d1);
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_EQUAL(scheduledAt + LinkLatency, c2p->getHalfTick());
	 }

	/**
	 * Test ConnectTime(CompId_t, int, CompId_t, int, void (T::*handler)(int, uint64_t), Time_t)
	 */
	 void testConnectTime_0()
	 {
	    CompId_t c0 = Component::Create<MyC0>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC0* c0p = Component::GetComponent<MyC0>(c0);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            uint64_t C0_OutputData = 789;

	    c0p->setData(C0_OutputData);
	    c2p->setClock(MasterClock);

            Time_t LinkLatency = 1.07;

	    Manifold::ConnectTime(c0, MyC0::OutData, c2, MyC2::InData0, &MyC2::GotInputOn0, LinkLatency);

            Manifold::unhalt();
            Ticks_t When = 1;
	    Manifold::ScheduleTime(When, &MyC0::SendIt, c0p);
	    Time_t scheduledAt = When + Manifold::Now();

	    Manifold::StopAtTime(scheduledAt + LinkLatency + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT_EQUAL(C0_OutputData, c2p->getData_0());
	    //verify c2 handler called at correct moment
	    CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt + LinkLatency, c2p->getTime(), DOUBLE_COMP_DELTA);
	 }

	/**
	 * Test ConnectTime(CompId_t, int, CompId_t, int, void (T::*handler)(int, T2), Time_t)
	 */
	 void testConnectTime_1()
	 {
	    CompId_t c1 = Component::Create<MyC1>(0);
	    CompId_t c2 = Component::Create<MyC2>(0);
	    MyC1* c1p = Component::GetComponent<MyC1>(c1);
	    MyC2* c2p = Component::GetComponent<MyC2>(c2);

            MyDataType1 d1(328); // a random number

	    c1p->setData(d1);
	    c2p->setClock(MasterClock);

            Time_t LinkLatency = 1.26;

	    Manifold::ConnectTime(c1, MyC1::OutData, c2, MyC2::InData1, &MyC2::GotInputOn1, LinkLatency);

            Manifold::unhalt();
            Time_t When = 1;
	    Manifold::ScheduleTime(When, &MyC1::SendIt, c1p);
	    Time_t scheduledAt = When + Manifold::Now();

	    Manifold::StopAtTime(scheduledAt + LinkLatency + 1);
	    Manifold::Run();

            //verify c2 get correct data
	    CPPUNIT_ASSERT(c2p->getData_1() == d1);
	    //verify c2 handler called at correct moment
	    //CPPUNIT_ASSERT_DOUBLES_EQUAL(scheduledAt + LinkLatency, c2p->getTime(), DOUBLE_COMP_DELTA);
	 }




        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("ManifoldTest");



	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnect_0", &ManifoldTest::testConnect_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnect_1", &ManifoldTest::testConnect_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectHalf_0", &ManifoldTest::testConnectHalf_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectHalf_1", &ManifoldTest::testConnectHalf_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectClock_0", &ManifoldTest::testConnectClock_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectClock_1", &ManifoldTest::testConnectClock_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectClockHalf_0", &ManifoldTest::testConnectClockHalf_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectClockHalf_1", &ManifoldTest::testConnectClockHalf_1));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectTime_0", &ManifoldTest::testConnectTime_0));
	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testConnectTime_1", &ManifoldTest::testConnectTime_1));

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

