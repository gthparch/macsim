#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "common-defs.h"

namespace manifold {
namespace kernel {

//const int MAX_DATA_SIZE = 1024;

struct Message_s {
    enum { M_UINT32, M_UINT64, M_SERIAL
         };

    unsigned type;
    int compIndex;  //component index
    int inputIndex; //index of input receiving the message
    int isTick;     //a boolean indicating whether sendTime/recvTime is tick or seconds
    Ticks_t sendTick;
    Ticks_t recvTick;
    Time_t sendTime;
    Time_t recvTime;
    uint32_t uint32_data;
    uint64_t uint64_data;
    //unsigned char data[MAX_DATA_SIZE];
    unsigned char* data;
    int data_len;
};


//const int MAX_MSG_SIZE = sizeof(Message_s);

} //namespace kernel
} //namespace manifold


#endif
