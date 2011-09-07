//!
//! @brief This program tests Messenger :: send_serial_msg().
//!
//! The data size is bigger than the messenger's send/recv buffer,
//! forcing the messenger to get new buffers.
//!
//! Scheduler is not involved. We only send/recv message using the
//! Messenger class.
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

using namespace std;
using namespace manifold::kernel;


//####################################################################
// Helper classes
//####################################################################
//! MyPkt is the data type being sent in a serial message.
class MyPkt {
    public:
        static const int ARR_SIZE = 2048;

        unsigned char f1;
	int f2;
	unsigned char f3;
	unsigned char array[ARR_SIZE];

	static int Serialize(MyPkt& p, unsigned char** buf);
	static MyPkt Deserialize(const unsigned char* data, int len);
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

int MyPkt :: Serialize(MyPkt& p, unsigned char** buf)
{
    int pos = 0;
    Sbuf[pos++] = p.f1;
    Sbuf[pos++] =  p.f2 &       0xff;
    Sbuf[pos++] = (p.f2 &     0xff00) >> 8;
    Sbuf[pos++] = (p.f2 &   0xff0000) >> 16;
    Sbuf[pos++] = (p.f2 & 0xff000000) >> 24;
    Sbuf[pos++] = p.f3;
    for(int i=0; i<ARR_SIZE; i++)
        Sbuf[pos++] = p.array[i];
    *buf = Sbuf;
    return pos;
}

MyPkt MyPkt :: Deserialize(const unsigned char* data, int len)
{
    MyPkt p;
    int pos = 0;
    p.f1 = data[pos++];
    p.f2 =0;
    p.f2 |= (data[pos++]);
    p.f2 |= ((int)(data[pos++])) << 8;
    p.f2 |= ((int)(data[pos++])) << 16;
    p.f2 |= ((int)(data[pos++])) << 24;
    p.f3 = data[pos++];
    for(int i=0; i<ARR_SIZE; i++)
        p.array[i] = data[pos++];

    return p;
}



//####################################################################
//####################################################################
//! This is the unit testing class for the class Messenger.
class MessengerTest : public CppUnit::TestFixture {
    private:
        static const double DOUBLE_COMP_DELTA = 1.0E-5; //! Delta for comparing doubles.

    public:
        //==========================================================================
        //==========================================================================
        //! Test send_serial_msg with sendTime/recvTime
	//! We send a few messages from one task to another and echo them back.
        void test_send_serial_msg_1()
	{
	    int Mytid; //task id
	    MPI_Comm_rank(MPI_COMM_WORLD, &Mytid);

            //Open a file for writing debug info. Each MPI task has its own file, so
	    //the output won't be mingled.
	    char buf[10];
	    sprintf(buf, "DBG_LOG%d", Mytid);
	    ofstream DBG_LOG(buf);

	    const int SIZE=3;


            if(Mytid == 0) { // task 0 creates messages; sends them and recvs the echo
		struct timeval ts;
		gettimeofday(&ts, NULL);
		srandom(ts.tv_usec);

		double when[SIZE][2]; // first column = sendTime; 2nd column = recvTime
		when[0][0] = random()/(RAND_MAX+1.0) * 10;  //a number between 0 and 10
		when[0][1] = when[0][0] + 1.34;
		for(int i=1; i<SIZE; i++) {
		    //when[i][0] = when[i-1][1] + d;   1 <= d < 6
		    when[i][0] = when[i-1][1] + (random()/(RAND_MAX+1.0) * 5 + 1);
		    when[i][1] = when[i][0] + (random()/(RAND_MAX+1.0) * 5 + 1);
		}

		for(int i=0; i<SIZE; i++) {
		    DBG_LOG << when[i][0] << ", " << when[i][1] << "  ";
		}
		DBG_LOG << endl;

		int cidx[SIZE], iidx[SIZE]; //component index, input indx, data
		MyPkt data[SIZE];
		//unsigned char buffer[1024];
		unsigned char* buffer;

                //TheMessenger.start_irecv();
                for(int i=0; i<SIZE; i++) {
		    cidx[i] = (int)(random() & 0xffffffff);
		    iidx[i] = (int)(random() & 0xff);
		    data[i].f1 = (char)(random() & 0xffffffff);
		    data[i].f2 = (int)(random() & 0xffffffff);
		    data[i].f3 = (char)(random() & 0xffffffff);

		    for(int j=0; j<MyPkt::ARR_SIZE; j++)
		        data[i].array[j] = j && 0xff;

                    int len = MyPkt :: Serialize(data[i], &buffer);
		    //send a message
		    TheMessenger.send_serial_msg(1, cidx[i], iidx[i], when[i][0], when[i][1], buffer, len);

		    int received=0;
		    do{
			//now receive it back
			Message_s& msg = TheMessenger.irecv_message(&received);
			if(received != 0) {
			    CPPUNIT_ASSERT(msg.type == Message_s :: M_SERIAL);
			    CPPUNIT_ASSERT_EQUAL(msg.compIndex, cidx[i]);
			    CPPUNIT_ASSERT_EQUAL(msg.inputIndex, iidx[i]);
			    CPPUNIT_ASSERT_EQUAL(msg.isTick, 0);
			    CPPUNIT_ASSERT_DOUBLES_EQUAL(msg.sendTime, when[i][0], DOUBLE_COMP_DELTA);
			    CPPUNIT_ASSERT_DOUBLES_EQUAL(msg.recvTime, when[i][1], DOUBLE_COMP_DELTA);
			    CPPUNIT_ASSERT(data[i] == MyPkt :: Deserialize(msg.data, msg.data_len));
			}
		    }while(received == 0);
		}
            }
	    else { //task 1 recvs msg, and echoes it back.
                //TheMessenger.start_irecv();

                int num=0;
		while(num < SIZE) {
		    int received=0;
		    do {
			Message_s& msg = TheMessenger.irecv_message(&received);
			if(received != 0) {
			    CPPUNIT_ASSERT(msg.type == Message_s :: M_SERIAL);
			    CPPUNIT_ASSERT_EQUAL(msg.isTick, 0);

			    //send it back
			    TheMessenger.send_serial_msg(0, msg.compIndex, msg.inputIndex, msg.sendTime, msg.recvTime, msg.data, msg.data_len);
			}
		    }while(received==0);
		    num++;
		}
	    }
	    DBG_LOG.close();

	}

        //==========================================================================
        //==========================================================================
        //! Test send_serial_msg with sendTick/recvTick
	//! We send a few messages from one task to another and echo them back.
        void test_send_serial_msg_2()
	{
	    int Mytid; //task id
	    MPI_Comm_rank(MPI_COMM_WORLD, &Mytid);

            //Open a file for writing debug info. Each MPI task has its own file, so
	    //the output won't be mingled.
	    char buf[10];
	    sprintf(buf, "DBG_LOG%d", Mytid);
	    ofstream DBG_LOG(buf);

	    const int SIZE=3;


            if(Mytid == 0) { // task 0 creates messages; sends them and recvs the echo
		struct timeval ts;
		gettimeofday(&ts, NULL);
		srandom(ts.tv_usec);

		Ticks_t when[SIZE][2]; // first column = sendTime; 2nd column = recvTime
		when[0][0] = (Ticks_t)(random()/(RAND_MAX+1.0) * 10);  //a number between 0 and 10
		when[0][1] = when[0][0] + 1;
		for(int i=1; i<SIZE; i++) {
		    //when[i][0] = when[i-1][1] + d;   1 <= d < 6
		    when[i][0] = when[i-1][1] + (Ticks_t)(random()/(RAND_MAX+1.0) * 5 + 1);
		    when[i][1] = when[i][0] + (Ticks_t)(random()/(RAND_MAX+1.0) * 5 + 1);
		}

		for(int i=0; i<SIZE; i++) {
		    DBG_LOG << when[i][0] << ", " << when[i][1] << "  ";
		}
		DBG_LOG << endl;

		int cidx[SIZE], iidx[SIZE]; //component index, input indx, data
		MyPkt data[SIZE];
		unsigned char* buffer;

                //TheMessenger.start_irecv();

                for(int i=0; i<SIZE; i++) {
		    cidx[i] = (int)(random() & 0xffffffff);
		    iidx[i] = (int)(random() & 0xff);
		    data[i].f1 = (char)(random() & 0xffffffff);
		    data[i].f2 = (int)(random() & 0xffffffff);
		    data[i].f3 = (char)(random() & 0xffffffff);

                    int len = MyPkt :: Serialize(data[i], &buffer);
		    //send a message
		    TheMessenger.send_serial_msg(1, cidx[i], iidx[i], when[i][0], when[i][1], buffer, len);

		    int received=0;
		    do{
			//now receive it back
			Message_s& msg = TheMessenger.irecv_message(&received);
			if(received != 0) {

			    CPPUNIT_ASSERT(msg.type == Message_s :: M_SERIAL);
			    CPPUNIT_ASSERT_EQUAL(msg.compIndex, cidx[i]);
			    CPPUNIT_ASSERT_EQUAL(msg.inputIndex, iidx[i]);
			    CPPUNIT_ASSERT_EQUAL(msg.isTick, 1);
			    CPPUNIT_ASSERT_EQUAL(msg.sendTick, when[i][0]);
			    CPPUNIT_ASSERT_EQUAL(msg.recvTick, when[i][1]);
			    CPPUNIT_ASSERT(data[i] == MyPkt :: Deserialize(msg.data, msg.data_len));
			}
		    }while(received == 0);
		}
            }
	    else { //task 1 recvs msg, and echoes it back.
                //TheMessenger.start_irecv();

		int num=0;
		while(num < SIZE) {
		    int received=0;
		    do {
			Message_s& msg = TheMessenger.irecv_message(&received);
			if(received != 0) {

			    CPPUNIT_ASSERT(msg.type == Message_s :: M_SERIAL);
			    CPPUNIT_ASSERT_EQUAL(msg.isTick, 1);

			    //send it back
			    TheMessenger.send_serial_msg(0, msg.compIndex, msg.inputIndex, msg.sendTick, msg.recvTick, msg.data, msg.data_len);
			}
		    }while(received == 0);
		    num++;
		}
	    }
	    DBG_LOG.close();

	}

        /**
	 * Build a test suite.
	 */
	static CppUnit::Test* suite()
	{
	    CppUnit::TestSuite* mySuite = new CppUnit::TestSuite("MessengerTest");

	    mySuite->addTest(new CppUnit::TestCaller<MessengerTest>("test_send_serial_msg_1", &MessengerTest::test_send_serial_msg_1));
	    mySuite->addTest(new CppUnit::TestCaller<MessengerTest>("test_send_serial_msg_2", &MessengerTest::test_send_serial_msg_2));

	    return mySuite;
	}
};



int main(int argc, char** argv)
{
    TheMessenger.init(argc, argv);
    if(2 != TheMessenger.get_node_size()) {
        cerr << "ERROR: Must specify \"-np 2\" for mpirun!" << endl;
	return 1;
    }

    CppUnit::TextUi::TestRunner runner;
    runner.addTest( MessengerTest::suite() );
    runner.run();

    MPI_Finalize();

    return 0;
}

