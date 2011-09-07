#ifndef NO_MPI

#include <iostream>
#include <stdlib.h>
#include <assert.h>

#include "messenger.h"

using namespace std;

namespace manifold {
namespace kernel {

//====================================================================
//====================================================================
const int MSG_TAG = 12;  //just a number.


//====================================================================
//====================================================================
Messenger :: Messenger()
{
    m_recv_buf_size = sizeof(Message_s) + 1024;
    m_recv_buf = new unsigned char[m_recv_buf_size];
    m_send_buf_size = sizeof(Message_s) + 1024;
    m_send_buf = new unsigned char[m_recv_buf_size];
}

//====================================================================
//====================================================================
void Messenger :: init(int argc, char** argv)
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &m_nodeId);
    MPI_Comm_size(MPI_COMM_WORLD, &m_nodeSize);

    //m_irecv_started = false;
}

//====================================================================
//====================================================================
void Messenger :: barrier()
{
    MPI_Barrier(MPI_COMM_WORLD);
}

//====================================================================
//! Perform an AllGater.
//! @param item   address of the data to send
//! @param itemSize    size of the data in terms of bytes
//! @param recvbuf    this is where the gathered data are stored
//====================================================================
void Messenger :: allGather(char* item, int itemSize, char* recvbuf)
{
    MPI_Allgather(item, itemSize, MPI_BYTE, recvbuf,
                  itemSize, MPI_BYTE, MPI_COMM_WORLD);
}


//====================================================================
//====================================================================
void Messenger :: send_uint32_msg(int dest, int compIndex, int inputIndex,
				  Ticks_t sendTick, Ticks_t recvTick, uint32_t data)
{
    const int Buf_size = sizeof(Message_s);

    unsigned char buf[Buf_size];

    unsigned msg_type = Message_s :: M_UINT32;
    int isTick = 1;  //time unit for sendTime/recvTime is tick

    int position = 0;

    MPI_Pack(&msg_type, 1, MPI_UNSIGNED, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&compIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&inputIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&isTick, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    //Ticks_t == uint64_t
    MPI_Pack(&sendTick, 1, MPI_UNSIGNED_LONG_LONG, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&recvTick, 1, MPI_UNSIGNED_LONG_LONG, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&data, 1, MPI_UNSIGNED, buf, Buf_size, &position, MPI_COMM_WORLD);

    send_message(dest, buf, position);

    #ifdef DBG_MSG
    #endif

}

//====================================================================
//====================================================================
void Messenger :: send_uint32_msg(int dest, int compIndex, int inputIndex,
                                  double sendTime, double recvTime, uint32_t data)
{
    const int Buf_size = sizeof(Message_s);
    unsigned char buf[Buf_size];  

    unsigned msg_type = Message_s :: M_UINT32;
    int isTick = 0;  //time unit for sendTime/recvTime is not tick

    int position = 0;

    MPI_Pack(&msg_type, 1, MPI_UNSIGNED, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&compIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&inputIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&isTick, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&sendTime, 1, MPI_DOUBLE, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&recvTime, 1, MPI_DOUBLE, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&data, 1, MPI_UNSIGNED, buf, Buf_size, &position, MPI_COMM_WORLD);

    send_message(dest, buf, position);

    #ifdef DBG_MSG
    #endif

}


//====================================================================
//====================================================================
void Messenger :: send_uint64_msg(int dest, int compIndex, int inputIndex,
				  Ticks_t sendTick, Ticks_t recvTick, uint64_t data)
{
    const int Buf_size = sizeof(Message_s);
    unsigned char buf[Buf_size];  

    unsigned msg_type = Message_s :: M_UINT64;
    int isTick = 1;  //time unit for sendTime/recvTime is tick

    int position = 0;

    MPI_Pack(&msg_type, 1, MPI_UNSIGNED, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&compIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&inputIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&isTick, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    //Ticks_t == uint64_t
    MPI_Pack(&sendTick, 1, MPI_UNSIGNED_LONG_LONG, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&recvTick, 1, MPI_UNSIGNED_LONG_LONG, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&data, 1, MPI_UNSIGNED_LONG_LONG, buf, Buf_size, &position, MPI_COMM_WORLD);

    send_message(dest, buf, position);

    #ifdef DBG_MSG
    #endif

}

//====================================================================
//====================================================================
void Messenger :: send_uint64_msg(int dest, int compIndex, int inputIndex,
                                  double sendTime, double recvTime, uint64_t data)
{
    const int Buf_size = sizeof(Message_s);
    unsigned char buf[Buf_size];  

    unsigned msg_type = Message_s :: M_UINT64;
    int isTick = 0;  //time unit for sendTime/recvTime is not tick

    int position = 0;

    MPI_Pack(&msg_type, 1, MPI_UNSIGNED, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&compIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&inputIndex, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&isTick, 1, MPI_INT, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&sendTime, 1, MPI_DOUBLE, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&recvTime, 1, MPI_DOUBLE, buf, Buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&data, 1, MPI_UNSIGNED_LONG_LONG, buf, Buf_size, &position, MPI_COMM_WORLD);

    send_message(dest, buf, position);

    #ifdef DBG_MSG
    #endif

}


//====================================================================
//====================================================================
void Messenger :: send_serial_msg(int dest, int compIndex, int inputIndex,
				  Ticks_t sendTick, Ticks_t recvTick, unsigned char* data, int len)
{
    if(len + sizeof(Message_s) > m_send_buf_size) {
	m_send_buf_size = len + sizeof(Message_s);
        delete[] m_send_buf;
	m_send_buf = new unsigned char[m_send_buf_size];
    }

    unsigned msg_type = Message_s :: M_SERIAL;
    int isTick = 1;  //time unit for sendTime/recvTime is tick

    int position = 0;

    MPI_Pack(&msg_type, 1, MPI_UNSIGNED, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&compIndex, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&inputIndex, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&isTick, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    //Ticks_t == uint64_t
    MPI_Pack(&sendTick, 1, MPI_UNSIGNED_LONG_LONG, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&recvTick, 1, MPI_UNSIGNED_LONG_LONG, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&len, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(data, len, MPI_UNSIGNED_CHAR, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);

    send_message(dest, m_send_buf, position);

    #ifdef DBG_MSG
    #endif

}


//====================================================================
//====================================================================
void Messenger :: send_serial_msg(int dest, int compIndex, int inputIndex,
                                  double sendTime, double recvTime, unsigned char* data, int len)
{

    if(len + sizeof(Message_s) > m_send_buf_size) {
	m_send_buf_size = len + sizeof(Message_s);
        delete[] m_send_buf;
	m_send_buf = new unsigned char[m_send_buf_size];
    }

    unsigned msg_type = Message_s :: M_SERIAL;
    int isTick = 0;  //time unit for sendTime/recvTime is not tick

    int position = 0;

    MPI_Pack(&msg_type, 1, MPI_UNSIGNED, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&compIndex, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&inputIndex, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&isTick, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&sendTime, 1, MPI_DOUBLE, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&recvTime, 1, MPI_DOUBLE, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(&len, 1, MPI_INT, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);
    MPI_Pack(data, len, MPI_UNSIGNED_CHAR, m_send_buf, m_send_buf_size, &position, MPI_COMM_WORLD);

    send_message(dest, m_send_buf, position);

    #ifdef DBG_MSG
    #endif

}


//====================================================================
//====================================================================
void Messenger :: send_message(int dest, unsigned char* buf, int position)
{
    if(MPI_Send(buf, position, MPI_PACKED, dest, MSG_TAG, MPI_COMM_WORLD) !=
                                                    MPI_SUCCESS) {
        cerr << "send_message failed!" << endl;
        exit(-1);
    }

    m_numSent++;
}


//====================================================================
//====================================================================
/*
void Messenger :: start_irecv()
{
    if(m_irecv_started)
        return;

    m_buf = new char*[m_nodeSize];
    for(int i=0; i<m_nodeSize; i++) {
        m_buf[i] = new char[MAX_MSG_SIZE];
    }

    m_requests = new MPI_Request[m_nodeSize];

    for(int i=0; i<m_nodeSize; i++) {
	MPI_Irecv(m_buf[i], MAX_MSG_SIZE, MPI_CHAR, i, MSG_TAG, MPI_COMM_WORLD, &m_requests[i]);
    }

    m_irecv_started = true;
}
*/

//====================================================================
//====================================================================
Message_s& Messenger :: irecv_message(int* received)
{
/*
    assert(m_irecv_started == true);

    int index=0;
    *received = 0;
    MPI_Status status;
    MPI_Testany(m_nodeSize, m_requests, &index, received, &status);
    if(*received != 0) { 
	unpack_message(m_buf[index]);

	m_requests[index]=0;
	MPI_Irecv(m_buf[index], MAX_MSG_SIZE, MPI_CHAR, index, MSG_TAG, MPI_COMM_WORLD,
	          &m_requests[index]);
	m_numReceived++;
    }
    else {
        //no message available; do nothing
    }
    return m_msg;
*/
    *received = 0;

    MPI_Status status;
    int flag;
    MPI_Iprobe(MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &flag, &status);
    if(flag != 0) {
        *received = 1;

        int size;
	MPI_Get_count(&status, MPI_CHAR, &size);
	if(size > m_recv_buf_size) { // msg size > receiving buffer size
	    m_recv_buf_size = size;
	    delete[] m_recv_buf;
	    m_recv_buf = new unsigned char[m_recv_buf_size];
	}
	MPI_Recv(m_recv_buf, m_recv_buf_size, MPI_PACKED, status.MPI_SOURCE, MSG_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	unpack_message(m_recv_buf);
	m_numReceived++;
    }

    return m_msg;
}


//====================================================================
//====================================================================
Message_s& Messenger :: unpack_message(unsigned char* buf)
{
    int position;
    //char buf[MAX_MSG_SIZE];
    //MPI_Status status;
    //message_s msg;

    //MPI_Recv(buf, MAX_MSG_SIZE, MPI_PACKED, MPI_ANY_SOURCE, MSG_TAG, MPI_COMM_WORLD, &status);
 
    position = 0;
    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.type), 1, MPI_UNSIGNED, MPI_COMM_WORLD); 

    switch(m_msg.type) {
	case Message_s :: M_UINT32:
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.compIndex), 1, MPI_INT, MPI_COMM_WORLD);
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.inputIndex), 1, MPI_INT, MPI_COMM_WORLD);
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.isTick), 1, MPI_INT, MPI_COMM_WORLD);
	    if(m_msg.isTick == 0) {
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.sendTime), 1, MPI_DOUBLE, MPI_COMM_WORLD);
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.recvTime), 1, MPI_DOUBLE, MPI_COMM_WORLD);
	    }
	    else {
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.sendTick), 1,
		           MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.recvTick), 1,
		           MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
	    }

	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.uint32_data), 1, MPI_UNSIGNED,
	                                                             MPI_COMM_WORLD);
	break;
	case Message_s :: M_UINT64:
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.compIndex), 1, MPI_INT, MPI_COMM_WORLD);
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.inputIndex), 1, MPI_INT, MPI_COMM_WORLD);
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.isTick), 1, MPI_INT, MPI_COMM_WORLD);
	    if(m_msg.isTick == 0) {
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.sendTime), 1, MPI_DOUBLE, MPI_COMM_WORLD);
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.recvTime), 1, MPI_DOUBLE, MPI_COMM_WORLD);
	    }
	    else {
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.sendTick), 1,
		           MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.recvTick), 1,
		           MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
	    }

	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.uint64_data), 1, MPI_UNSIGNED_LONG_LONG,
	                                                             MPI_COMM_WORLD);
	break;
	case Message_s :: M_SERIAL:
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.compIndex), 1, MPI_INT, MPI_COMM_WORLD);
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.inputIndex), 1, MPI_INT, MPI_COMM_WORLD);
	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.isTick), 1, MPI_INT, MPI_COMM_WORLD);
	    if(m_msg.isTick == 0) {
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.sendTime), 1, MPI_DOUBLE, MPI_COMM_WORLD);
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.recvTime), 1, MPI_DOUBLE, MPI_COMM_WORLD);
	    }
	    else {
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.sendTick), 1,
		           MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
		MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.recvTick), 1,
		           MPI_UNSIGNED_LONG_LONG, MPI_COMM_WORLD);
	    }

	    MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.data_len), 1, MPI_INT, MPI_COMM_WORLD);
	    m_msg.data = &m_recv_buf[position];
	    //MPI_Unpack(buf, m_recv_buf_size, &position, &(m_msg.data), m_msg.data_len, MPI_UNSIGNED_CHAR,
	     //                                                        MPI_COMM_WORLD);
	break;

	default:
	    cerr << "Unknown message type: " << m_msg.type << endl;
	    exit(1);
    }//switch

    return m_msg;
}





Messenger TheMessenger;

} //namespace kernel
} //namespace manifold

#endif //#ifndef NO_MPI





