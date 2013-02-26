//!
//! @brief This program is a component test (as opposed to unit test) program,
//! since it involves multiple classes.
//! We create two components c0, c1 for two different LPs and set up the connection
//! between the components. For each component, a few events are scheduled.
//! c0's handler for the events causes messages to be sent to c1. On the other hand,
//! c1's handler for the events does nothing. When messages are received by c1,
//! new events are automatically scheduled. c1 uses a different handler to handle
//! those events.
//!
//! Classes involved include: Manifold, Clock, Component, LinkBase, Link,
//! LinkOutputBase, LinkOutput, LinkOutputRemote, LinkInputBase, LinkInputBaseT,
//! LinkInput, Messenger, Message_s.
//!
//! In this program, the components are clock based. The clock used is one other than
//! the master clock.
//! The delay is specified in half ticks.
//! This program uses a template class, so it can be used to test all primitive types
//! that are sent between two components.
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
#include "messenger.h"
#include "component.h"
#include "manifold.h"
#include "link.h"

using namespace std;
using namespace manifold::kernel;


//comment the following line if debug info is not desired.
#define DBG

int Mytid; //task id

//####################################################################
// helper classes
//####################################################################
template<typename T>
class MyObj1 : public Component {
    private:
        Clock& m_clock;
	vector<Ticks_t> m_recvHalfTick;

	vector<T> m_incoming;

	#ifdef DBG
	ofstream&  m_dbg_log;
	#endif
    public:
        static const int C0_OUT = 2;
        static const int C1_IN = 5;
        static const int C0_OUT_LATENCY = 1;

	#ifdef DBG
	MyObj1(const ofstream & dbg_log, const Clock& c) : m_clock(const_cast<Clock&>(c)), m_dbg_log(const_cast<ofstream&>(dbg_log)) {}
	#else
	MyObj1(const Clock& c) : m_clock(const_cast<Clock&>(c)) {}
	#endif

        vector<Ticks_t>& getRecvHalfTick() { return m_recvHalfTick; }

        vector<T>& getIncoming() { return m_incoming; }

	//! Handler for initially scheduled events.
	//! Note that only task 0 sends out messages.
	void handler(T data)
	{
	    #ifdef DBG
	    Ticks_t now = Manifold :: NowHalfTicks(m_clock);
	    m_dbg_log << "@@@ " << Manifold :: NowTicks(m_clock) << " (half tick " << now << ")" << endl;
	    #endif

            if(Mytid == 0) { //c0 sends messages to c1
	        Send(C0_OUT, data);
		#ifdef DBG
		m_dbg_log << "    handler called "<< endl;
		m_dbg_log << "    sendHalfTick= " << now
			  << " recvHalfTick= " << now + C0_OUT_LATENCY
			  << " data= " << data
			  << endl;
		#endif
	    }
	}

        //! Handler for incoming message.
	void process_incoming(int inputIndex, T data)
	{
	    #ifdef DBG
	    m_dbg_log << "@@@ " << Manifold :: NowTicks(m_clock) << " (half tick " << Manifold :: NowHalfTicks(m_clock) << ")" << endl;
	    m_dbg_log << "    process_incoming  "
	              << " inputIdex= " << inputIndex
	              << " data= " << data
		      << endl;
	    #endif
	    // save the received data and time for verification later.
            m_incoming.push_back(data);
	    m_recvHalfTick.push_back(Manifold :: NowHalfTicks(m_clock));
	}
};


//####################################################################
//####################################################################
class MessagingTest : public CppUnit::TestFixture {
    private:
	static Clock MasterClock;  //clock has to be global or static.
	static Clock Clock1;
	enum { MASTER_CLOCK_HZ = 10 };
	enum { CLOCK1_HZ = 4 };

        static const double DOUBLE_COMP_DELTA = 1.0E-5;

    public:
        //!
	//! Initialization function. Inherited from the CPPUnit framework.
        //!
        void setUp()
	{
	}

        template<typename T>
        void test_messaging()
	{
	    MPI_Comm_rank(MPI_COMM_WORLD, &Mytid);

	    #ifdef DBG
	    // create a file into which to write debug info.
	    char buf[10];
	    sprintf(buf, "DBG_LOG%d", Mytid);
	    ofstream DBG_LOG(buf);
	    #endif

	    const int SIZE=5; // number of output messages to send from c0 to c1.

	    struct timeval ts;
	    gettimeofday(&ts, NULL);
	    srandom(ts.tv_usec + 1234 * Mytid);

	    Ticks_t when[SIZE]; // when output occurs
	    when[0] = (Ticks_t)(random()/(RAND_MAX+1.0) * 10);  //a number between 0 and 10
	    for(int i=1; i<SIZE; i++) {
		//when[i] = when[i-1] + d;   1 <= d < 6
		when[i] = when[i-1] + (Ticks_t)(random()/(RAND_MAX+1.0) * 5 + 1);
	    }

	    #ifdef DBG
            DBG_LOG << "Events scheduled at (half ticks): ";
	    for(int i=0; i<SIZE; i++) {
		DBG_LOG << when[i] << ", ";
	    }
	    DBG_LOG << endl;
	    DBG_LOG << "Link delay= " << MyObj1<T>::C0_OUT_LATENCY << endl;
	    #endif


	    #ifdef DBG
	    CompId_t c0 = Component :: Create<MyObj1<T> >(0, DBG_LOG, Clock1); //c0 created for LP 0
	    CompId_t c1 = Component :: Create<MyObj1<T> >(1, DBG_LOG, Clock1); //c1 created for LP 1
	    #else
	    CompId_t c0 = Component :: Create<MyObj1<T> >(0, Clock1); //c0 created for LP 0
	    CompId_t c1 = Component :: Create<MyObj1<T> >(1, Clock1); //c1 created for LP 1
	    #endif

	    MyObj1<T>* c0p = Component :: GetComponent<MyObj1<T> >(c0);
	    MyObj1<T>* c1p = Component :: GetComponent<MyObj1<T> >(c1);

	    //connect the two
	    Manifold :: ConnectClockHalf(c0, MyObj1<T>::C0_OUT, c1, MyObj1<T>::C1_IN, Clock1, &MyObj1<T>::process_incoming, MyObj1<T>::C0_OUT_LATENCY);

	    MyObj1<T>* comp = (Mytid == 0) ? c0p : c1p;

	    Manifold :: unhalt();

	    T send_data[SIZE];
	    // schedule a few events
	    for(int i=0; i<SIZE; i++) {
		send_data[i] = (T)(random() & 0xffffffff);
		Manifold :: ScheduleClockHalf(when[i], Clock1, &MyObj1<T>::handler, comp, send_data[i]);
	    }
	    const unsigned int STOP = 1000;
	    CPPUNIT_ASSERT(when[SIZE-1] < STOP);

	    Ticks_t ScheduleAt = Manifold :: NowHalfTicks(Clock1); // Tick time when we schedule the events.

	    Manifold :: StopAtClock(STOP, Clock1);
	    Manifold :: Run();

	    //Now for verification
	    if(Mytid == 0) {
		//get the received data back for verification.
		//use long long so it's big enough to hold any primitive data
		unsigned long long recv_data[SIZE];
		MPI_Status status;
		MPI_Recv(recv_data, SIZE, MPI_UNSIGNED_LONG_LONG, 1, 0, MPI_COMM_WORLD, &status);

		//get the received data back for verification.
		Ticks_t ticks[SIZE];
		MPI_Recv(ticks, SIZE, MPI_UNSIGNED_LONG_LONG, 1, 0, MPI_COMM_WORLD, &status);

		for(int i=0; i<SIZE; i++) {
		    CPPUNIT_ASSERT_EQUAL(send_data[i], (T)(recv_data[i])); // cast back to T
		    CPPUNIT_ASSERT_EQUAL(ScheduleAt + when[i] + MyObj1<T>::C0_OUT_LATENCY, ticks[i]);
		}
	    }
	    else {
		//send the received data back for verification.
		unsigned long long data[SIZE];
		vector<T>& in = c1p->getIncoming();
		for(int i=0; i<SIZE; i++) {
		    data[i] = in[i];
		}
		MPI_Send(data, SIZE, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD);

		//send the receive tick back for verification.
		Ticks_t ticks[SIZE];
		vector<Ticks_t>& recv_ticks = c1p->getRecvHalfTick();
		for(int i=0; i<SIZE; i++) {
		    ticks[i] = recv_ticks[i];
		}
		MPI_Send(ticks, SIZE, MPI_UNSIGNED_LONG_LONG, 0, 0, MPI_COMM_WORLD);
	    }
	}




        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("MessagingTest");

	    mySuite->addTest(new CppUnit::TestCaller<MessagingTest>("test_messaging_u32", &MessagingTest::test_messaging<uint32_t>));
	    mySuite->addTest(new CppUnit::TestCaller<MessagingTest>("test_messaging_u64", &MessagingTest::test_messaging<uint64_t>));

	    return mySuite;
	}
};


Clock MessagingTest::MasterClock(MASTER_CLOCK_HZ);
Clock MessagingTest::Clock1(CLOCK1_HZ);


int main(int argc, char** argv)
{
    Manifold :: Init(argc, argv);
    if(2 != TheMessenger.get_node_size()) {
        cerr << "ERROR: Must specify \"-np 2\" for mpirun!" << endl;
	return 1;
    }

    CppUnit::TextUi::TestRunner runner;
    runner.addTest( MessagingTest::suite() );
    runner.run();

    MPI_Finalize();

    return 0;
}

