#include <iostream>
#include <cstdlib>
#include <queue>

#pragma once

class Memory;
class Buffer;
class UnifiedBuffer;

class Mac {
private:
    // unchanged, constant instances
    UnifiedBuffer *ub;              // pointer to unified buffer connected with this buffer
    Buffer *buffer1;                // pointer to first buffer of unified buffer
    Buffer *buffer2;                // pointer to second buffer of unified buffer
    float bpc;                      // bytes per cycle, serviced by buffer to MAC
    int size;                       // how much of data it requests at once (in KBytes)
    // dynamic instances
    float btr;                      // number of bytes left to receive
    // dynamic instances used for statistics;
    int busy_cycle;                 // number of cycles that MAC was serviced by any of the buffers
    int idle_cycle;                 // number of cycles that MAC was left unserviced
    int stall_cycle;                // number of cycles that a request was stalled
    
    std::queue <req> wait_queue;    // queue of requests waiting to be received
    std::queue <req> req_queue;     // queue of requests to send to buffer

public:
    int req_num;                    // index to give to the next generated request

    Mac(UnifiedBuffer *_ub, int capacity);
    void Cycle();
    bool IsComputing();
    bool IsIdle();
    bool DoingAbsolutelyNothing();
   
    void Compute();

    void ReceiveRequest(float _btr);
    bool CheckReadiness(int order);
    void SignalUsing(int order);

    void PrintStats();

    UnifiedBuffer *GetUnifiedBufferPointer() {return ub;}
    Buffer *GetFirstBufferPointer() {return buffer1;}
    Buffer *GetSecondBufferPointer() {return buffer2;}
};
