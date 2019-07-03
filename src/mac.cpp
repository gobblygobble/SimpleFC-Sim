#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <string>
#include <queue>

#include "buffer.hpp"
#include "mac.hpp"

#define DEBUG

/* Note that a Mac unit should be constructed after a Unifiedbuffer unit is properly initialized, 
 * and should be passed on the pointer with type UnifiedBuffer*, just like it was for Memory and UnifiedBuffer.
 */
Mac::Mac(UnifiedBuffer *_ub, int capacity) {
    // unchanged, constant instances
    ub = _ub;
    buffer1 = _ub->GetFirstBufferPointer();
    buffer2 = _ub->GetSecondBufferPointer();
    bpc = _ub->GetBytesPerCycle();
    size = capacity;
    // dynamic instances
    btr = 0;
    // dynamic instances used for statistics
    busy_cycle = 0;
    idle_cycle = 0;
    stall_cycle = 0;

    req_num = 1;
}

void Mac::Cycle() {
#ifdef DEBUG
    std::cout << "Mac::Cycle() start." << std::endl;
#endif
    // take care of cycle
    if (IsIdle())
        idle_cycle++;
    else
        busy_cycle++;

    req r;

    if (!wait_queue.empty()) {
        r = wait_queue.front();
        //int check_num = r.order;
        int check_num = wait_queue.front().order;
        
        // if ready, start computation immediately
        if (CheckReadiness(check_num)) {
            wait_queue.pop();
            SignalUsing(check_num);
            Compute();
        }
    }
    // if pending requests exist, request them all to UnifiedBuffer
    while (!req_queue.empty()) {
        r = req_queue.front();
        req_queue.pop();
        wait_queue.push(r);
        ub->ReceiveRequest(r.size, r.order);
    }
#ifdef DEBUG
    std::cout << "Mac::Cycle() end." << std::endl;
#endif
}

// TODO: change function
bool Mac::IsComputing() {
    return false;
}

bool Mac::IsIdle() {
    return !IsComputing();
}

bool Mac::DoingAbsolutelyNothing() {
    return (req_queue.empty() && wait_queue.empty());
}

// TODO: change function
void Mac::Compute() {
    return;
}

void Mac::ReceiveRequest(float _btr) {
    req r;
    r.size = _btr;
    r.order = req_num++;
    req_queue.push(r);
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
    std::cout << "======================================================================" << std::endl;
}


