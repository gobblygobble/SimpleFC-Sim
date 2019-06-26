#include <iostream>
#include <cstdlib>
#include <assert.h>

#include "memory.h"
#include "buffer.h"

Buffer::Buffer(int _buffer_index, float _bw, float _bpc, int _capacity, Memory *_memory, UnifiedBuffer *_ub) {
    buffer_index = _buffer_index;
    memory = _memory;
    ub = _ub;

    bring_in = false;
    bring_out = false;
    capacity = _capacity;
    busy_cycle = 0;
    idle_cycle = 0;
    bw = _bw;
    bpc = _bpc;

    bts = 0;
    btr = 0;
}

Buffer::Cycle() {
    // take care of cycle
    if (IsIdle())
        idle_cycle++;
    else 
        busy_cycle++;

    // receiving part
    if (bring_in) {
        btr = ((btr - memory->GetBytesPerCycle()) < 0) ? 0 : (btr - memory->GetBytesPerCycle());
        if (btr == 0) {
            bring_in = false;
            // let the unified buffer know
            NotifyChangeInReceiver(); // the other buffer needs to receive data
        }
    }
    // sending part
    if (bring_out) {
        bts = ((bts - bpc) < 0) ? 0 : (bts - bpc);
        if (bts == 0) {
            bring_out = false;
            // let the unified buffer know
            NotifyChangeInSender(); // the other buffer needs to send data
        }
    }
}


UnifiedBuffer::UnifiedBuffer(float clock, float _bw, int _capacity, Memory *_memory) {
    busy_cycle = 0;
    idle_cycle = 0;
    bw = _bw;
    bpc = _bw / clock;
    capacity = _capacity;

    rcv_buffer = 0;
    send_buffer = 0;
    memory = _memory;

    buffer1 = new Buffer(1, bw, bpc, capacity / 2, memory, this);
    buffer2 = new Buffer(2, bw, bpc, capacity / 2, memory, this);
}

UnifiedBuffer::Cycle() {
    if (buffer1->IsIdle() && buffer2->IsIdle())
        idle_cycle++;
    else
        busy_cycle++;

    buffer1->Cycle();
    buffer2->Cycle();
    // sanity check
    if (buffer1->IsReceiving() && buffer2->IsReceiving()) {
        std::cerr << "WTF? Both buffers receiving from memory?" << std::endl;
        assert(0);
    }
    if (buffer1->IsSending() && buffer2->IsSending()) {
        std::cerr << "WTF? Both buffers sending to MAC?" << std::endl;
        assert(0);
    }
    // update receiving buffer
    if (buffer2->IsReceiving())
        rcv_buffer = 1;
    else if (buffer2->IsReceiving())
        rcv_buffer = 2;
    else
        rcv_buffer = 0;
    // update sending buffer
    if (buffer1->IsSending())
        send_buffer = 1;
    else if (buffer2->IsSending())
        send_buffer = 2;
    else
        send_buffer = 0;
}
