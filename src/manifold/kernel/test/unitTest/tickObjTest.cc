#include <TestFixture.h>
#include <TestAssert.h>
#include <TestSuite.h>
#include <Test.h>
#include <TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>

#include "clock.h"

using namespace manifold::kernel;

//####################################################################
// helper classes
//####################################################################
// class MyObj1 is like a component
class MyObj1 {
    private:
	int m_risingTickCalled; //number of time risingTick() has been called
	int m_fallingTickCalled; //number of time fallingTick() has been called
    public:
        MyObj1() : m_risingTickCalled(0), m_fallingTickCalled(0) {}

	int getRisingTickCalled() const { return m_risingTickCalled; }
	int getFallingTickCalled() const { return m_fallingTickCalled; }

	void risingTick()
	{
	    m_risingTickCalled += 1;
	}

	void fallingTick()
	{
	    m_fallingTickCalled += 1;
	}
};




//####################################################################
// Class tickObjTest is the unit test class for class tickObj. 
//####################################################################
class tickObjTest : public CppUnit::TestFixture {
    public:
        /**
	 * Initialization function. Inherited from the CPPUnit framework.
	 */
        void setUp()
	{
	}


        /**
	 * Test CallRisingTick()
	 */
	void testCallRisingTick_0()
	{
            MyObj1* comp = new MyObj1();
            tickObj<MyObj1>* tobj = new tickObj<MyObj1>(comp, &MyObj1::risingTick, &MyObj1::fallingTick);

            int before = comp->getRisingTickCalled();

            tobj->CallRisingTick();

            //verify risingTick() called.
	    CPPUNIT_ASSERT_EQUAL(before + 1, comp->getRisingTickCalled());

	    delete tobj;
	    delete comp;
	}

        /**
	 * Test CallRisingTick()
	 */
	void testCallRisingTick_1()
	{
            MyObj1* comp = new MyObj1();
	    //no functions for rising/falling tick specified
            tickObj<MyObj1>* tobj = new tickObj<MyObj1>(comp);

            int before = comp->getRisingTickCalled();

            tobj->CallRisingTick();

            //verify risingTick() NOT called.
	    CPPUNIT_ASSERT_EQUAL(before, comp->getRisingTickCalled());

	    delete tobj;
	    delete comp;
	}

        /**
	 * Test CallFallingTick()
	 */
	void testCallFallingTick_0()
	{
            MyObj1* comp = new MyObj1();
            tickObj<MyObj1>* tobj = new tickObj<MyObj1>(comp, &MyObj1::risingTick, &MyObj1::fallingTick);

            int before = comp->getFallingTickCalled();

            tobj->CallFallingTick();

            //verify fallingTick() called.
	    CPPUNIT_ASSERT_EQUAL(before + 1, comp->getFallingTickCalled());

	    delete tobj;
	    delete comp;
	}

        /**
	 * Test CallFallingTick()
	 */
	void testCallFallingTick_1()
	{
            MyObj1* comp = new MyObj1();
	    //no functions for rising/falling tick specified
            tickObj<MyObj1>* tobj = new tickObj<MyObj1>(comp);

            int before = comp->getFallingTickCalled();

            tobj->CallFallingTick();

            //verify fallingTick() NOT called.
	    CPPUNIT_ASSERT_EQUAL(before, comp->getFallingTickCalled());

	    delete tobj;
	    delete comp;
	}


        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("tickObjTest");

	    mySuite->addTest(new CppUnit::TestCaller<tickObjTest>("testCallRisingTick_0", &tickObjTest::testCallRisingTick_0));
	    mySuite->addTest(new CppUnit::TestCaller<tickObjTest>("testCallRisingTick_1", &tickObjTest::testCallRisingTick_1));
	    mySuite->addTest(new CppUnit::TestCaller<tickObjTest>("testCallFallingTick_0", &tickObjTest::testCallFallingTick_0));
	    mySuite->addTest(new CppUnit::TestCaller<tickObjTest>("testCallFallingTick_1", &tickObjTest::testCallFallingTick_1));

	    return mySuite;
	}
};



int main()
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( tickObjTest::suite() );
    if(runner.run("", false))
	return 0; //all is well
    else
	return 1;

}



