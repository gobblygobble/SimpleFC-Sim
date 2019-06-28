#include <iostream>
#include <cstdlib>
//#include "buffer.hpp"

//#include <queue>

#pragma once

class Memory;
class Buffer;
class UnifiedBuffer;

class Mac {
private:
    UnifiedBuffer *ub;              // pointer to unified buffer connected with this buffer
    Buffer *buffer1;                // pointer to first buffer of unified buffer
    Buffer *buffer2;                // pointer to second buffer of unified buffer
    int connected_buffer;           // index of the buffer currently servicing this MAC
    //std::queue <float> req_queue; // queue of requests to send to unified buffer

    int busy_cycle;                 // number of cycles that MAC was serviced by any of the buffers
    int idle_cycle;                 // number of cycles that MAC was left unserviced
    
    int size;                       // how much of data it requests at once (in Bytes)

    bool pending;                   // whether data to receive exists after the current receiving wave

    int latest_rcv_index;           // index of buffer that has finished receiving latest

public:
    Mac();
    void Cycle();
    bool IsIdle();
    
    //void HandleQueue();
    void SendRequest(float _btr);
    void ReceiveDoneSignal(int rc_buffer_index, bool pending);

    void PrintStats();

    UnifiedBuffer *GetUnifiedBufferPointer() {return ub;}
    Buffer *GetFirstBufferPointer() {return buffer1;}
    Buffer *GetSecondBufferPointer() {return buffer2;}
};
