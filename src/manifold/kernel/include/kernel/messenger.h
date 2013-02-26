#ifndef __MESSENGER_H__
#define __MESSENGER_H__

#ifndef NO_MPI

#include <stdint.h>

#include "message.h"
#include "mpi.h"

namespace manifold {
namespace kernel {

class Messenger {
public:
    Messenger();
    void init(int argc, char** argv);

    //! Return the MPI rank, i.e., the ID of the node.
    int get_node_id() const { return m_nodeId; }

    //! Return the number of nodes.
    int get_node_size() const { return m_nodeSize; }

    //! Return the number of messages sent.
    int get_numSent() const { return m_numSent; }

    //! Return the number of messages received.
    int get_numReceived() const { return m_numReceived; }

    //! Enter a synchronization barrier.
    void barrier();

    //! Perform an AllGather.
    void allGather(char* item, int itemSize, char* array);

    void send_uint32_msg(int dest, int compIndex, int inputIndex,
                         Ticks_t sendTick, Ticks_t recvTick, uint32_t data);

    void send_uint32_msg(int dest, int compIndex, int inputIndex,
                         double sendTime, double recvTime, uint32_t data);

    void send_uint64_msg(int dest, int compIndex, int inputIndex,
                         Ticks_t sendTick, Ticks_t recvTick, uint64_t data);

    void send_uint64_msg(int dest, int compIndex, int inputIndex,
                         double sendTime, double recvTime, uint64_t data);

    void send_serial_msg(int dest, int compIndex, int inputIndex,
                         Ticks_t sendTick, Ticks_t recvTick, unsigned char* data, int len);

    void send_serial_msg(int dest, int compIndex, int inputIndex,
                         double sendTime, double recvTime, unsigned char* data, int len);

    //void start_irecv();
    Message_s& irecv_message(int* received);


private:
    void send_message(int dest, unsigned char* buf, int position);
    Message_s& unpack_message(unsigned char*);

    int m_nodeId;
    int m_nodeSize;
    int m_numSent; //number of sent messages.
    int m_numReceived; //number of received messages.
    Message_s m_msg; // message holder; avoid allocating/deallocating all the time
    //char** m_buf; //message receiving buffer
    unsigned char* m_recv_buf;
    int m_recv_buf_size;
    unsigned char* m_send_buf;
    int m_send_buf_size;
    //MPI_Request* m_requests;
    //bool m_irecv_started;
};



extern Messenger TheMessenger;

} //namespace kernel
} //namespace manifold


#endif //#ifndef NO_MPI



#endif
