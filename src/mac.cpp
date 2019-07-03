#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <string>
#include <queue>

#include "buffer.hpp"
#include "mac.hpp"

//#define DEBUG

/* Note that a Mac unit should be constructed after a Unifiedbuffer unit is properly initialized, 
 * and should be passed on the pointer with type UnifiedBuffer*, just like it was for Memory and UnifiedBuffer.
 */
Mac::Mac(UnifiedBuffer *_ub, float _power, float _clock) {
    // unchanged, constant instances
    ub = _ub;
    buffer1 = _ub->GetFirstBufferPointer();
    buffer2 = _ub->GetSecondBufferPointer();
    bpc = _ub->GetBytesPerCycle();
    
    power = _power;
    clock = _clock;
    // dynamic instances
    btr = 0;
    // dynamic instances used for statistics
    busy_cycle = 0;
    idle_cycle = 0;
    stall_cycle = 0;

    operations = 0;
    remaining_computation = 0;
    req_num = 1;
}

void Mac::Cycle() {
#ifdef DEBUG
    std::cout << "Mac::Cycle() start." << std::endl;
#endif

    req r;
    if (IsComputing())
        Compute(0);
    // not computing
    else if (!wait_queue.empty()) {
        r = wait_queue.front();
        int check_num = r.order;
        int size = r.size;

        // if ready, start computation immediately
        if (CheckReadiness(check_num)) {
            wait_queue.pop();
            SignalUsing(check_num);
            Compute(size);
        }
    }
    // if pending requests exist, request them all to UnifiedBuffer
    while (!req_queue.empty()) {
        r = req_queue.front();
        req_queue.pop();
        wait_queue.push(r);
        ub->ReceiveRequest(r.size, r.order);
    }

    // take care of cycle
    if (IsIdle())
        idle_cycle++;
    else
        busy_cycle++;

    computed = false;

#ifdef DEBUG
    std::cout << "Mac::Cycle() end." << std::endl;
#endif
}

bool Mac::IsComputing() {
    return (remaining_computation != 0);
}

bool Mac::IsIdle() {
    //return !IsComputing();
    return !computed;
}

bool Mac::DoingAbsolutelyNothing() {
    return (req_queue.empty() && wait_queue.empty() && !IsComputing());
}

void Mac::Compute(int size) {
    if (size > 0)
        remaining_computation = size;
    assert(!op_queue.empty());
    assert(!tile_size_queue.empty());
    int comp_power = op_queue.front();
    int tile_size = tile_size_queue.front();
    int remaining_computation_before_computation = remaining_computation;

    remaining_computation = ((remaining_computation - comp_power) < 0) ? 0 : (remaining_computation - tile_size);
    if (remaining_computation == 0) {
        op_queue.pop();
        tile_size_queue.pop();
    }

    operations += 2 * (remaining_computation_before_computation - remaining_computation) * (comp_power / tile_size);
    computed = true;
}

void Mac::ReceiveRequest(float _btr, int tile_size, int op_per_tile) {
    assert(2 * power > op_per_tile);
    req r;
    r.size = _btr;
    r.order = req_num++;
    req_queue.push(r);
    op_queue.push(op_per_tile);
    tile_size_queue.push(tile_size);
}

bool Mac::CheckReadiness(int order) {
    return ub->CheckExistenceInReadyQueue(order);
}

void Mac::SignalUsing(int order) {
    ub->MacUsingChunk(order);
}

void Mac::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "=========================== Mac Unit Stats ===========================" << std::endl;
    std::cout << "Total of " << req_queue.size() << " pending requests to send to buffer." << std::endl;
    std::cout << "Total of " << wait_queue.size() << " requests waiting to receive from buffer." << std::endl;
    std::cout << "Total busy cycles : idle cycles\t" << busy_cycle << " : " << idle_cycle << std::endl;
    std::cout << "Total of " << operations << " operations performed by MAC." << std::endl;
    std::cout << "======================================================================" << std::endl;
}


