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
    UnifiedBuffer *ub;                  // pointer to unified buffer connected with this buffer
    Buffer *buffer1;                    // pointer to first buffer of unified buffer
    Buffer *buffer2;                    // pointer to second buffer of unified buffer
    float bpc;                          // bytes per cycle, serviced by buffer to MAC

    int power;                          // number of small MAC units
    float clock;                        // frequency, in GHz (cycles per second)
    // dynamic instances
    float btr;                          // number of bytes left to receive
    // dynamic instances used for statistics;
    int busy_cycle;                     // number of cycles that MAC was serviced by any of the buffers
    int idle_cycle;                     // number of cycles that MAC was left unserviced
    int stall_cycle;                    // number of cycles that a request was stalled
    
    std::queue <req> wait_queue;        // queue of requests waiting to be received
    std::queue <req> req_queue;         // queue of requests to send to buffer
    std::queue <int> tile_size_queue;   // queue of respective tile sizes
    std::queue <int> op_queue;          // queue to keep track of the tile size

    int operations;                     // total number of operations executed so far
    int remaining_computation;          // how much of computation left (not 2x-ed)
    int req_num;                        // index to give to the next generated request
    bool computed;                      // whether MAC has performed computation this cycle

public:

    Mac(UnifiedBuffer *_ub, float _power, float _clock);
    void Cycle();
    bool IsComputing();
    bool IsIdle();
    bool DoingAbsolutelyNothing();
   
    void Compute(int size);

    void ReceiveRequest(float _btr, int tile_size, int op_per_tile);
    bool CheckReadiness(int order);
    void SignalUsing(int order);

    void PrintStats();

    int GetBusyCycle() {return busy_cycle;}
    int GetIdleCycle() {return idle_cycle;}
    int GetTotalCycle() {return busy_cycle + idle_cycle;}
    int GetPower() {return power;}
    int GetOperations() {return operations;}

    UnifiedBuffer *GetUnifiedBufferPointer() {return ub;}
    Buffer *GetFirstBufferPointer() {return buffer1;}
    Buffer *GetSecondBufferPointer() {return buffer2;}
};
