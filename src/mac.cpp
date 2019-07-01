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
    connected_buffer = 0;   // not receiving anything
    latest_rcv_index = 2;   // because we want to start at 1
    btr = 0;
    // dynamic instances used for statistics
    busy_cycle = 0;
    idle_cycle = 0;
    total_received_rounds = 0;
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

    // receiving part
    if (connected_buffer) {
        assert(btr > 0); // connected_buffer should be set to 0 only when btr is 0
        btr = ((btr - bpc) < 0) ? 0 : (btr - bpc);
        if (btr == 0) {
            // update
            latest_rcv_index = connected_buffer;
            total_received_rounds++;
            connected_buffer = (req_queue.empty()) ? 0 : (3 - connected_buffer); // no pending: 0, pending: 1->2, 2->1
            if (!req_queue.empty()) {
                btr = req_queue.front();
                req_queue.pop();
            }
        }
    }

#ifdef DEBUG
    std::cout << "Mac::Cycle() end." << std::endl;
#endif
}

bool Mac::IsIdle() {
    return (connected_buffer == 0);
}

void Mac::SendRequest(float _btr) {
    if (IsIdle() && req_queue.empty()) {
        if (latest_rcv_index == 0)
            connected_buffer = 1;
        else
            connected_buffer = 3 - latest_rcv_index; // 1->2, 2->1
        btr = _btr;
        // make sure things are going as expected
        assert(CheckConsistencyWithBufferIndex());
        // signal to unified buffer.
        ub->ReceiveRequest(_btr);
    }
    else {
        req_queue.push(_btr);
    }
}

/* This seems unnecessary
void Mac::ReceiveDoneSignal(int rc_buffer_index, bool pending) {
    return;
}
*/

bool Mac::CheckConsistencyWithBufferIndex() {
    return (ub->GetLatestSendIndex() == latest_rcv_index);
}

void Mac::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "=========================== Mac Unit Stats ===========================" << std::endl;
    if (connected_buffer == 0)
        std::cout << "Currently not receiving from any buffer." << std::endl;
    else
        std::cout << "Currently receiving from buffer #" << connected_buffer << std::endl;

    std::cout << "Total of " << req_queue.size() << " pending requests to send to buffer." << std::endl;
    std::cout << "Total busy cycles : idle cycles\t" << busy_cycle << " : " << idle_cycle << std::endl;
    std::cout << "Has been serviced a total of " << total_received_rounds << " chunks." << std::endl;
    std::cout << "======================================================================" << std::endl;
}


