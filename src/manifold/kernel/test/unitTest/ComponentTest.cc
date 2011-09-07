#include <TestFixture.h>
#include <TestAssert.h>
#include <TestSuite.h>
#include <Test.h>
#include <TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>

#include "component.h"
#include "manifold.h"

using namespace manifold::kernel;

//####################################################################
// helper classes
//####################################################################
class MyComp1 : public Component {
    private:
        const int m_val;
    public:
        MyComp1(int x) : m_val(x) {}
	int getVal() const { return m_val; }

	size_t getOutLinkNum()
	{
	    int n=0;
	    for(unsigned i=0; i<outLinks.size(); i++)
	        if(outLinks[i] != 0)
		    n++;
	    return n;
        }
};


//====================================================================
// The following classes are used for testing the Connect() functions.
//====================================================================
class MyObj0 : public Component {
    public:
        MyObj0() {}
};

class MyObj1 : public Component {
    public:
        MyObj1(int a) : m_a(a) {}
	int getA() { return m_a; }
    private:
        int m_a;
};

class MyObj2 : public Component {
    public:
        MyObj2(int a, int b) : m_a(a), m_b(b) {}
	int getA() { return m_a; }
	int getB() { return m_b; }
    private:
        int m_a;
        int m_b;
};

class MyObj3 : public Component {
    public:
        MyObj3(int a, int b, int c) : m_a(a), m_b(b), m_c(c) {}
	int getA() { return m_a; }
	int getB() { return m_b; }
	int getC() { return m_c; }
    private:
        int m_a;
        int m_b;
        int m_c;
};

class MyObj4 : public Component {
    public:
        MyObj4(int a, int b, int c, int d) : m_a(a), m_b(b), m_c(c), m_d(d) {}
	int getA() { return m_a; }
	int getB() { return m_b; }
	int getC() { return m_c; }
	int getD() { return m_d; }
    private:
        int m_a;
        int m_b;
        int m_c;
        int m_d;
};

//====================================================================
// The following classes are used for testing the Send() function.
//====================================================================
static Clock MasterClock(10);

class MySComp0 : public Component {
};

class MySComp1 : public Component {
    public:
	void handler(int, uint32_t val)
	{
	    m_val = val;
	    m_tick = Manifold :: NowTicks();
	}
	Ticks_t getTick() { return m_tick; }
	uint32_t getVal() { return m_val; }
    private:
        Ticks_t m_tick; // record tick when handler is called.
        uint32_t m_val;
};



//####################################################################
// Class ComponentTest is the unit test class for class Component.
//####################################################################
class ComponentTest : public CppUnit::TestFixture {
    public:
        /**
	 * Initialization function. Inherited from the CPPUnit framework.
	 */
        void setUp()
	{
	}


        /**
	 * Test GetComponentId()
	 * Tests GetComponentId, GetComponent and GetComponentName.
	 */
	void testGetComponentId_1()
	{
	    CompName_t name = "testGetComponentId_1";
	    CompName_t noname = "None";
            CompId_t id1 = Component :: Create<MyComp1, int>(0, 123);
            CompId_t id2 = Component :: Create<MyComp1, int>(0, 456, name);

	    MyComp1* comp1 = Component :: GetComponent<MyComp1>(id1);
	    MyComp1* comp2 = Component :: GetComponent<MyComp1>(name);
            MyComp1* comp3 = Component :: GetComponent<MyComp1>(noname);

            //verify
	    CPPUNIT_ASSERT_EQUAL(id1, comp1->GetComponentId());
	    CPPUNIT_ASSERT_EQUAL(123, comp1->getVal());
	    CPPUNIT_ASSERT_EQUAL(id2, comp2->GetComponentId());
	    CPPUNIT_ASSERT_EQUAL(456, comp2->getVal());
	    // You can't retrieve component with the default name "None".
	    CPPUNIT_ASSERT(0 == comp3);
            //Showing components without names have a Name = "None"
	    CPPUNIT_ASSERT(noname == comp1->GetComponentName()); 
	    CPPUNIT_ASSERT(name == comp2->GetComponentName());
	}

        /**
	 * Test GetComponentId()
	 * If a component is created for another LP, then an object is not
	 * created.
	 */
	void testGetComponentId_2()
	{   
	    CompName_t name = "testGetComponentId_2";
            CompId_t id1 = Component :: Create<MyComp1, int>(2, 111);
            Component :: Create<MyComp1, int>(3, 123, name); 
	    MyComp1* comp1 = Component :: GetComponent<MyComp1>(id1);
	    MyComp1* comp2 = Component :: GetComponent<MyComp1>(name);
           
            //verify
	    CPPUNIT_ASSERT(0 == comp1);
	    CPPUNIT_ASSERT(0 == comp2);
	}


        /**
	 * Test GetComponentId()
	 */
	void testAddOutputLink()
	{
	    MyComp1* comp1 = new MyComp1(123);
	    Link<int>* lk1 = comp1->AddOutputLink<int>(1); // add a link to output 1
	    CPPUNIT_ASSERT(lk1 != 0); //lk1 should be added successfully
	    CPPUNIT_ASSERT_EQUAL(1, (int)comp1->getOutLinkNum());

	    Link<int>* lk2 = comp1->AddOutputLink<int>(2); // add a link to output 2
	    CPPUNIT_ASSERT(lk2 != 0); //lk2 should be added successfully
	    CPPUNIT_ASSERT_EQUAL(2, (int)comp1->getOutLinkNum());

	    Link<int>* lk3 = comp1->AddOutputLink<int>(1); // add another link to output 1
	    CPPUNIT_ASSERT(lk3 != 0); //lk3 should be added successfully
	    //In fact, lk3 == lk1
	    CPPUNIT_ASSERT_EQUAL(lk1, lk3);
	    CPPUNIT_ASSERT_EQUAL(2, (int)comp1->getOutLinkNum());

	    // add a different type of link to output 1; an exception is thrown
	    CPPUNIT_ASSERT_THROW(comp1->AddOutputLink<char>(1), LinkTypeMismatchException);
	    CPPUNIT_ASSERT_EQUAL(2, (int)comp1->getOutLinkNum());
	}


        /**
	 * Test isLocal()
	 */
	void testIsLocal()
	{
            CompName_t name1 = "testIsLocal1";
	    CompName_t name2 = "testIsLocal2";
            CompName_t noname = "None";
            CompId_t id1 = Component :: Create<MyComp1, int>(0, 123, name1);
            CompId_t id2 = Component :: Create<MyComp1, int>(2, 456, name2);

            //verify
	    CPPUNIT_ASSERT_EQUAL(true, Component :: IsLocal(id1));
	    CPPUNIT_ASSERT_EQUAL(true, Component :: IsLocal(name1));
	    CPPUNIT_ASSERT_EQUAL(false, Component :: IsLocal(noname));
	    //In the sequential kernel, all components are deemed local
	    CPPUNIT_ASSERT_EQUAL(false, Component :: IsLocal(id2));
	    CPPUNIT_ASSERT_EQUAL(false, Component :: IsLocal(name2));
	}


        /**
	 * Test Create()
	 * Test creating components whose constructors take no parameters.
	 */
	void testCreate_0()
	{
	    CompName_t name = "testCreate_0";
            CompId_t id0 = Component :: Create<MyObj0>(0, name);
	    MyObj0* obj0 = Component :: GetComponent<MyObj0>(id0);
	    CPPUNIT_ASSERT(0 != obj0);
	}

        /**
	 * Test Create()
	 * Test creating components whose constructors take 1 parameter.
	 */
	void testCreate_1()
	{
	    CompName_t name = "testCreate_1";
            CompId_t id = Component :: Create<MyObj1>(0, 123, name);
	    MyObj1* obj = Component :: GetComponent<MyObj1>(id);
	    CPPUNIT_ASSERT_EQUAL(123, obj->getA());
	}

        /**
	 * Test Create()
	 * Test creating components whose constructors take 2 parameters.
	 */
	void testCreate_2()
	{
	    CompName_t name = "testCreate_2";
            CompId_t id = Component :: Create<MyObj2>(0, 123, 45, name);
	    MyObj2* obj = Component :: GetComponent<MyObj2>(id);

	    CPPUNIT_ASSERT_EQUAL(123, obj->getA());
	    CPPUNIT_ASSERT_EQUAL(45, obj->getB());
	}

        /**
	 * Test Create()
	 * Test creating components whose constructors take 3 parameters.
	 */
	void testCreate_3()
	{
	    CompName_t name = "testCreate_3";
            CompId_t id = Component :: Create<MyObj3>(0, 123, 45, 6, name);
	    MyObj3* obj = Component :: GetComponent<MyObj3>(id);
	    CPPUNIT_ASSERT_EQUAL(123, obj->getA());
	    CPPUNIT_ASSERT_EQUAL(45, obj->getB());
	    CPPUNIT_ASSERT_EQUAL(6, obj->getC());
	}



        /**
	 * Test Create()
	 * Test creating components whose constructors take 4 parameters.
	 */
	void testCreate_4()
	{
	    CompName_t name = "testCreate_4";
            CompId_t id = Component :: Create<MyObj4>(0, 123, 45, 6, 789, name);
	    MyObj4* obj = Component :: GetComponent<MyObj4>(id);
	    CPPUNIT_ASSERT_EQUAL(123, obj->getA());
	    CPPUNIT_ASSERT_EQUAL(45, obj->getB());
	    CPPUNIT_ASSERT_EQUAL(6, obj->getC());
	    CPPUNIT_ASSERT_EQUAL(789, obj->getD());
	}


	void testSend_0()
	{
            CompId_t id0 = Component :: Create<MySComp0>(0);
            CompId_t id1 = Component :: Create<MySComp1>(0);

            const Ticks_t LATENCY_TICKS = 1;
	    Manifold :: Connect(id0, 0, id1, 1, &MySComp1::handler, LATENCY_TICKS);

            uint32_t DATA = 3072; // a random number

            //send the data; event would be scheduled for the destination component.
	    Ticks_t sendAt = Manifold::NowTicks();
	    MySComp0* obj0 = Component :: GetComponent<MySComp0>(id0);
	    obj0->Send(0, DATA);

	    Manifold::unhalt();
	    Manifold::StopAt(LATENCY_TICKS + 1);
	    Manifold::Run();

	    MySComp1* obj1 = Component :: GetComponent<MySComp1>(id1);
	    CPPUNIT_ASSERT_EQUAL(sendAt + LATENCY_TICKS, obj1->getTick());
	    CPPUNIT_ASSERT_EQUAL(DATA, obj1->getVal());
	}


        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("ComponentTest");

	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testGetComponentId_1", &ComponentTest::testGetComponentId_1));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testGetComponentId_2", &ComponentTest::testGetComponentId_2));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testAddOutputLink", &ComponentTest::testAddOutputLink));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testIsLocal", &ComponentTest::testIsLocal));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testCreate_0", &ComponentTest::testCreate_0));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testCreate_1", &ComponentTest::testCreate_1));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testCreate_2", &ComponentTest::testCreate_2));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testCreate_3", &ComponentTest::testCreate_3));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testCreate_4", &ComponentTest::testCreate_4));
	    mySuite->addTest(new CppUnit::TestCaller<ComponentTest>("testSend_0", &ComponentTest::testSend_0));

	    return mySuite;
	}
};



int main()
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest( ComponentTest::suite() );
    if(runner.run("", false))
	return 0; //all is well
    else
	return 1;
}



