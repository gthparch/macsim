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
//! This program tests non-primitive types that are sent between the two components.
//! The data type must implement Serialize() and Deserialize().
//! In this program data is passed by pointer.
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
class MyPkt {
    public:
        unsigned char f1;
	int f2;
	unsigned char f3;
	static int Serialize(const MyPkt& p, unsigned char** buf);
	static int Serialize(MyPkt* p, unsigned char** buf);
	static MyPkt* Deserialize(const unsigned char* data);
	static MyPkt Deserialize(const unsigned char* data, int);
	bool operator==(const MyPkt& p)
	{
	    return (f1==p.f1) && (f2==p.f2) && (f3==p.f3);
	}
        friend ostream& operator<<(ostream& stream, const MyPkt& pkt);
};

static unsigned char Sbuf[sizeof(MyPkt)];

ostream& operator<<(ostream& stream, const MyPkt& pkt)
{
    stream << "f1: " << (unsigned)pkt.f1 << " f2: " << pkt.f2 << " f3: " << (unsigned)pkt.f3;
    return stream;
}

int MyPkt :: Serialize(MyPkt* p, unsigned char** buf)
{
    int pos = 0;
    Sbuf[pos++] = p->f1;
    Sbuf[pos++] =  p->f2 &       0xff;
    Sbuf[pos++] = (p->f2 &     0xff00) >> 8;
    Sbuf[pos++] = (p->f2 &   0xff0000) >> 16;
    Sbuf[pos++] = (p->f2 & 0xff000000) >> 24;
    Sbuf[pos++] = p->f3;

    *buf = Sbuf;
    return pos;
}

int MyPkt :: Serialize(const MyPkt& p, unsigned char** buf)
{
    int pos = 0;
    Sbuf[pos++] = p.f1;
    Sbuf[pos++] =  p.f2 &       0xff;
    Sbuf[pos++] = (p.f2 &     0xff00) >> 8;
    Sbuf[pos++] = (p.f2 &   0xff0000) >> 16;
    Sbuf[pos++] = (p.f2 & 0xff000000) >> 24;
    Sbuf[pos++] = p.f3;

    *buf = Sbuf;
    return pos;
}

MyPkt* MyPkt :: Deserialize(const unsigned char* data)
{
    MyPkt* p = new MyPkt;
    int pos = 0;
    p->f1 = data[pos++];
    p->f2 =0;
    p->f2 |= (data[pos++]);
    p->f2 |= ((int)(data[pos++])) << 8;
    p->f2 |= ((int)(data[pos++])) << 16;
    p->f2 |= ((int)(data[pos++])) << 24;
    p->f3 = data[pos];

    return p;
}

MyPkt MyPkt :: Deserialize(const unsigned char* data, int)
{
    MyPkt p;
    int pos = 0;
    p.f1 = data[pos++];
    p.f2 =0;
    p.f2 |= (data[pos++]);
    p.f2 |= ((int)(data[pos++])) << 8;
    p.f2 |= ((int)(data[pos++])) << 16;
    p.f2 |= ((int)(data[pos++])) << 24;
    p.f3 = data[pos];

    return p;
}



class MyObj1 : public Component {
    private:
        Clock& m_clock;
	vector<Ticks_t> m_recvHalfTick;

	vector<MyPkt> m_incoming;

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

        vector<MyPkt>& getIncoming() { return m_incoming; }

	//! Handler for initially scheduled events.
	//! Note that only task 0 sends out messages.
	void handler(MyPkt* data)
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
			  << " data= " << hex << *data << dec
			  << endl;
		#endif
	    }
	}

        //! Handler for incoming message.
	void process_incoming(int inputIndex, MyPkt* data)
	{
	    #ifdef DBG
	    m_dbg_log << "@@@ " << Manifold :: NowTicks(m_clock) << " (half tick " << Manifold :: NowHalfTicks(m_clock) << ")" << endl;
	    m_dbg_log << "    process_incoming  "
	              << " inputIdex= " << inputIndex
	              << " data= " << hex << *data << dec
		      << endl;
	    #endif
	    // save the received data and time for verification later.
            m_incoming.push_back(*data);
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
	    DBG_LOG << "Link delay= " << MyObj1::C0_OUT_LATENCY << endl;
	    #endif


	    #ifdef DBG
	    CompId_t c0 = Component :: Create<MyObj1>(0, DBG_LOG, Clock1); //c0 created for LP 0
	    CompId_t c1 = Component :: Create<MyObj1>(1, DBG_LOG, Clock1); //c1 created for LP 1
	    #else
	    CompId_t c0 = Component :: Create<MyObj1>(0, Clock1); //c0 created for LP 0
	    CompId_t c1 = Component :: Create<MyObj1>(1, Clock1); //c1 created for LP 1
	    #endif

	    MyObj1* c0p = Component :: GetComponent<MyObj1>(c0);
	    MyObj1* c1p = Component :: GetComponent<MyObj1>(c1);

	    //connect the two
	    Manifold :: ConnectClockHalf(c0, MyObj1::C0_OUT, c1, MyObj1::C1_IN, Clock1, &MyObj1::process_incoming, MyObj1::C0_OUT_LATENCY);

	    MyObj1* comp = (Mytid == 0) ? c0p : c1p;

	    Manifold :: unhalt();

	    MyPkt send_data[SIZE];
	    // schedule a few events
	    for(int i=0; i<SIZE; i++) {
		send_data[i].f1 = (char)(random() & 0xffffffff);
		send_data[i].f2 = (int)(random() & 0xffffffff);
		send_data[i].f3 = (char)(random() & 0xffffffff);

		Manifold :: ScheduleClockHalf(when[i], Clock1, &MyObj1::handler, comp, &send_data[i]);
	    }
	    const unsigned int STOP = 1000;
	    CPPUNIT_ASSERT(when[SIZE-1] < STOP);

	    Ticks_t ScheduleAt = Manifold :: NowHalfTicks(Clock1); // Tick time when we schedule the events.

	    Manifold :: StopAt(STOP);
	    Manifold :: Run();

	    //Now for verification
	    if(Mytid == 0) {
		//get the received data back for verification.
		unsigned char recv_data[SIZE][sizeof(MyPkt)];
		MPI_Status status;
		for(int i=0; i<SIZE; i++) {
		    MPI_Recv(&recv_data[i], sizeof(MyPkt), MPI_UNSIGNED_CHAR, 1, 0, MPI_COMM_WORLD, &status);
		}

		//get the received data back for verification.
		Ticks_t ticks[SIZE];
		MPI_Recv(ticks, SIZE, MPI_UNSIGNED_LONG_LONG, 1, 0, MPI_COMM_WORLD, &status);

		for(int i=0; i<SIZE; i++) {
		    //CPPUNIT_ASSERT_EQUAL(send_data[i], MyPkt :: Deserialize(recv_data[i], sizeof(MyPkt)));
		    CPPUNIT_ASSERT(send_data[i] == MyPkt :: Deserialize(recv_data[i], sizeof(MyPkt)));
		    CPPUNIT_ASSERT_EQUAL(ScheduleAt + when[i] + MyObj1::C0_OUT_LATENCY, ticks[i]);
		}
	    }
	    else {
		//send the received data back for verification.
		vector<MyPkt>& in_pkt = c1p->getIncoming();
		for(int i=0; i<SIZE; i++) {
		    unsigned char* buf;
		    MyPkt :: Serialize(in_pkt[i], &buf);
		    MPI_Send(buf, sizeof(MyPkt), MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
		}

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

	    mySuite->addTest(new CppUnit::TestCaller<MessagingTest>("test_messaging", &MessagingTest::test_messaging));

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

