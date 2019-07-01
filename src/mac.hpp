#include <iostream>
#include <cstdlib>
#include <queue>

#pragma once

class Memory;
class Buffer;
class UnifiedBuffer;

class Mac {
private:
    // unchanged, constanc instances
    UnifiedBuffer *ub;              // pointer to unified buffer connected with this buffer
    Buffer *buffer1;                // pointer to first buffer of unified buffer
    Buffer *buffer2;                // pointer to second buffer of unified buffer
    float bpc;                      // bytes per cycle, serviced by buffer to MAC
    int size;                       // how much of data it requests at once (in KBytes)
    // dynamic instances
    int connected_buffer;           // index of the buffer currently servicing this MAC
    int latest_rcv_index;           // index of buffer that has finished receiving latest
    float btr;                      // number of bytes left to receive
    // dynamic instances used for statistics;
    int busy_cycle;                 // number of cycles that MAC was serviced by any of the buffers
    int idle_cycle;                 // number of cycles that MAC was left unserviced
    int total_received_rounds;      // number of total 'requests' that MAC has been serviced
    std::queue <float> req_queue;   // queue of requests to send to buffer

public:
    Mac(UnifiedBuffer *_ub, int capacity);
    void Cycle();
    bool IsIdle();
    
    void SendRequest(float _btr);
    void ReceiveDoneSignal(int rc_buffer_index, bool pending);

    bool CheckConsistencyWithBufferIndex();

    void PrintStats();

    UnifiedBuffer *GetUnifiedBufferPointer() {return ub;}
    Buffer *GetFirstBufferPointer() {return buffer1;}
    Buffer *GetSecondBufferPointer() {return buffer2;}
};
