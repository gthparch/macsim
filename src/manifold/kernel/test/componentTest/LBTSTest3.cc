//!
//! @brief This program tests the Manifold :: isSafeToProcess() function,
//! which uses LBTS to find the lower bound time stamp in the whole system
//! and then determine whether an event can be safely processed.
//! In this program we schedule a few events for each process. The timestamps
//! of the events are random numbers. The processes don't send messages to
//! each other either, except, of course, the allGather used in LBTS.
//!
//! Only ScheduleClock() is used.
//!
#include <TestFixture.h>
#include <TestAssert.h>
#include <TestSuite.h>
#include <Test.h>
#include <TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include "mpi.h"
#include "manifold.h"
#include "messenger.h"

using namespace std;
using namespace manifold::kernel;


//####################################################################
// helper classes
//####################################################################
class MyObj1 {
    private:
	Clock& m_clock;
        Ticks_t m_tick;

	// m_calledticks is used to record the clock tick at the
	// time when the handler is called.
	vector<Ticks_t> m_calledticks;
	// m_grantedtimes is used to record the LBTS granted time at the
	// time when the handler is called. Obviously we should have
	// time of m_ticks[i] <= m_grantedtimes
	vector<double> m_grantedtimes;
    public:

	MyObj1(Clock& clk) : m_clock(clk) {}

	Clock& getClock() const { return m_clock; }

        vector<Ticks_t>& getCalledTicks() { return m_calledticks; }
        vector<double>& getGrantedTimes() { return m_grantedtimes; }

	void handler0()
	{
	    m_calledticks.push_back(Manifold::NowTicks());
	    m_grantedtimes.push_back(Manifold::grantedTime);
	}

};


//####################################################################
// ManifoldTest is the unit test class for the class Manifold.
//####################################################################
class ManifoldTest : public CppUnit::TestFixture {
    private:
        static Clock MasterClock;
	enum { MASTER_CLOCK_HZ = 10 };

    public:
        /**
	 * Initialization function. Inherited from the CPPUnit framework.
	 */
        void setUp()
	{
	}

        //! Test the Manifold :: isSafeToProcess() function.
	//! Each MPI task schedules a number of events whose timestams are
	//! randomly generated.
	//!
	//! Note: Manifold :: grantedTime must be temporarily changed to public
	//! for this program to compile.
        void testIsSafeToProcess()
	{
	    int Mytid; //task id
	    MPI_Comm_rank(MPI_COMM_WORLD, &Mytid);

	    char buf[10];
	    sprintf(buf, "DBG_LOG%d", Mytid);
	    ofstream DBG_LOG(buf);

	    const int SIZE=3;  // SIZE numbers are generated
	    Ticks_t when[SIZE];
	    Ticks_t STOP_TIME=1000;

            struct timeval ts;
	    gettimeofday(&ts, NULL);
	    srandom(ts.tv_usec + Mytid*123);

	    when[0] = (Ticks_t)(random()/(RAND_MAX+1.0) * 10 + 1);  //a number between 1 and 10
	    for(int i=1; i<SIZE; i++) {
	        //when[i] = when[i-1] + d;   1 <= d < 6
	        when[i] = when[i-1] + (Ticks_t)(random()/(RAND_MAX+1.0) * 5 + 1);
	    }
	    CPPUNIT_ASSERT(when[SIZE-1] < STOP_TIME);

            for(int i=0; i<SIZE; i++) {
	        DBG_LOG << when[i] << " ";
	    }
	    DBG_LOG << endl;


            MyObj1* comp1 = new MyObj1(MasterClock);

            //Now schedule the events.
	    for(int i=0; i<SIZE; i++) {
	        Manifold :: Schedule(when[i], &MyObj1::handler0, comp1);
	    }

	    Manifold::StopAt(STOP_TIME);
	    Manifold::Run();

	    vector<Ticks_t>& calledTicks = comp1->getCalledTicks();
	    vector<double>& grantedTimes = comp1->getGrantedTimes();

	    for(unsigned int i=0; i<calledTicks.size(); i++) {
	        DBG_LOG << "calledTick= " << calledTicks[i] << ", called= " << (double)calledTicks[i]/MASTER_CLOCK_HZ << ", granted= " << grantedTimes[i] << endl;
	        CPPUNIT_ASSERT((double)calledTicks[i]/MASTER_CLOCK_HZ <= grantedTimes[i]);
	    }

	}


        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("ManifoldTest");

	    mySuite->addTest(new CppUnit::TestCaller<ManifoldTest>("testIsSafeToProcess", &ManifoldTest::testIsSafeToProcess));

	    return mySuite;
	}
};

Clock ManifoldTest::MasterClock(MASTER_CLOCK_HZ);


int main(int argc, char** argv)
{
    Manifold :: Init(argc, argv);
    if(2 != TheMessenger.get_node_size()) {
        cerr << "ERROR: Must specify \"-np 2\" for mpirun!" << endl;
	return 1;
    }

    CppUnit::TextUi::TestRunner runner;
    runner.addTest( ManifoldTest::suite() );
    runner.run();

    MPI_Finalize();

    return 0;
}

