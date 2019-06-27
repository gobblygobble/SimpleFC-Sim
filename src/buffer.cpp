#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <string>
#include <queue>

#include "memory.hpp"
#include "buffer.hpp"

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

void Buffer::Cycle() {
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
            NotifyChangeInReceiver(buffer_index); // the other buffer needs to receive data
        }
    }
    // sending part
    if (bring_out) {
        bts = ((bts - bpc) < 0) ? 0 : (bts - bpc);
        if (bts == 0) {
            bring_out = false;
            // let the unified buffer know
            NotifyChangeInSender(buffer_index); // the other buffer needs to send data
        }
    }
}

/* In this function it is assumed that UnifiedBuffer::SendRequest() picks the right index of buffer
 * and calls (this) Buffer::SendRequest() on the right buffer. */
void Buffer::SendRequest(float _btr) {
    memory->ReceiveRequest(buffer_index, _btr);
    bring_in = true;
}

/* In this function it is assumed that UnifiedBuffer::ReceiveRequest() picks the right index of buffer
 * and calls (this) Buffer::ReceiveRequest() on the right buffer.
 * Contacting with MAC should also be done at UnifiedBuffer-level. */
void Buffer::ReceiveRequest(float _bts) {
    bring_out = true;
}

/* TODO:
 * Note that this function needs to be modified if a different measure of idle and busy cycles is to be taken. */
bool Buffer::IsIdle() {
    if (bring_out)
        return false;
    else
        return true;
}

int Buffer::GetBusyCycle() {
    return busy_cycle;
}

int Buffer::GetIdleCycle() {
    return idle_cycle;
}

int Buffer::GetTotalCycle() {
    return GetBusyCycle() + GetIdleCycle();
}

float Buffer::GetBandwidth() {
    return bw;
}

float Buffer::GetBytesPerCycle() {
    return bpc;
}

float Buffer::GetBytesToSend() {
    return bts;
}

float Buffer::GetBytesToReceive() {
    return btr;
}

void Buffer::SetBringIn(bool flag) {
    if (bring_in == flag) {
        std::cerr << "Trying to change bring_in to identical value" << std::endl;
        assert(0);
    }
    bring_in = flag;
}

void Buffer::SetBringOut(bool flag) {
    if (bring_out == flag) {
        std::cerr << "Trying to change bring_out to identical value" << std::endl;
        assert(0);
    }
    bring_out = flag;
}

void Buffer::NotifyChangeInReceiver(int finished_index) {
    ub->ChangeInReceiver(int finished_index);
}

void Buffer::NotifyChangeInSender(int finished_index) {
    ub->ChangeInSender(int finished_index);
}

void Buffer::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    if (buffer_index == 1)
        std::cout << "=========================== Buffer 1 Stats ===========================" << std::endl;
    else
        std::cout << "=========================== Buffer 2 Stats ===========================" << std::endl;
    
    std::cout << "Total busy cycles : idle cycles\t" << busy_cycle << " : " << idle_cycle << std::endl;
    
    std::string rcv = (bring_in) ? "receiving from memory" : "NOT receiving anything";
    std::string send = (bring_out) ? "sending to MAC" : "NOT sending anything";

    std::cout << "Currently " << rcv << " and " << send << "." << std::endl;
    if (bring_in)
        std::cout << "Need to receive " << btr << "B." << std::endl;
    if (bring_out)
        std::cout << "Need to send " << bts << "B." << std::endl;
    std::cout << "======================================================================" << std::endl;;
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

    pending_receive = false;
    pending_send = false;
    latest_rcv_index = 2;   // because we want to start at 1
    latest_send_index = 2;  // because we want to start at 1
}

void UnifiedBuffer::Cycle() {
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

    // check queue for pending requests to send to memory
    HandleQueue();

    // TODO: these two updates may be redundant...

    // update receiving buffer
    if (buffer1->IsReceiving())
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

void UnifiedBuffer::HandleQueue() {
    float btr;
    if (memory->IsIdle()){
        if (!req_queue.empty()) {
            btr = req_queue.pop()
            SendRequest(btr);

            if (req_queue.empty())
                pending_receive = false;
        }
    }
}

void UnifiedBuffer::SendRequest(float _btr) {
    if (memory->IsIdle()) {
        if (latest_rcv_index == 1) {
            buffer2->SendRequest(_btr);
            rcv_buffer = 2;
        }
        else {
            buffer1->SendRequest(_btr);
            rcv_buffer = 1;
        }
    }
    else
        req_queue.push(_btr);
}

void UnifiedBuffer::ReceieveRequest(float _bts) {
    if (!IsIdle()) {
        std::cerr << "Request should only be sent when the provider is idle!" << std::endl;
        assert(0);
    }
    if (latest_send_index == 1) {
        buffer2->ReceiveRequest(_bts);
        send_buffer = 2;
    }
    else {
        buffer1->ReceiveRequest(_bts);
        send_buffer = 1;
    }
}

// TODO: this function may need to be changed
void UnifiedBuffer::ReceiveDoneSignal(int rcv_buffer_index, bool pending) {
    UpdateLatestReceivedIndex(rcv_buffer_index);
    if (pending) {
        // memory has more to send
        rcv_buffer = 3 - rcv_buffer; // 1->2, 2->1
        assert(pending_receive);
        pending_receive = true; // redundant
    }
    else {
        // memory is now idle
        rcv_buffer = 0;
        pending_receive = false;
    }
}

void UnifiedBuffer::UpdateLatestReceivedIndex(int finished_index) {
    // correctness check
    assert(finished_index != latest_rcv_index);
    latest_rcv_index = finished_index;
}

void UnifiedBuffer::UpdateLatestSentIndex(int finished_index) {
    //correctness check
    assert(finished_index != latest_send_index);
    latest_send_index = finished_index;
}

void UnifiedBuffer::ChangeInReceiver(int finished_index) {
    Buffer* buffer_to_change;
    if (finished_index == 1)
        buffer_to_change = buffer2;
    else if (finished_index == 2)
        buffer_to_change = buffer1;
    else {
        std::cerr << "Unexpected index of buffer that finished receiving: " << finished_index << std::endl;
        assert(0);
    }

    if (pending_receive) {
        // change index of UnifiedBuffer class and bring_in of Buffer that'll start bringing in
        rcv_buffer = 3 - finished_index; // 2->1, 1->2
        buffer_to_change->SetBringIn(true);
        pending_receive = false;
    }
    else {
        // change index of UnifiedBuffer class
        rcv_buffer = 0;
    }
    UpdateLatestReceivedIndex(finished_index);
}

void UnifiedBuffer::ChangeInSender(int finished_index) {
    Buffer* buffer_to_change;
    if (finished_index == 1)
        buffer_to_change = buffer2;
    else if (finished_index == 2)
        buffer_to_change = buffer1;
    else {
        std::cerr << "Unexpected index of buffer that finished sending: " << finished_index << std::endl;
        assert(0);
    }

    if (pending_send) {
        // change index of UnifiedBuffer class and bring_out of Buffer that'll start bringing out
        send_buffer = 3 - finished_index; // 2->1, 1->2
        buffer_to_change->SetBringOut(true);
        pending_send = false;
    }
    else {
        // change index of UnifiedBuffer class
        send_buffer = 0;
    }
    UpdateLatestSentIndex(finished_index);
}

void UnifiedBuffer::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "========================= UnifiedBuffer Stats ========================" << std::endl;
    if (rcv_buffer)
        std::cout << "Buffer #" << rcv_buffer << " is receiving data from memory." << std::endl;
    if (send_buffer)
        std::cout << "Buffer #" << send_buffer << " is sending data to MAC." << std::endl;
    if (pending_receive)
        std::cout << "A pending receive exists." << std::endl;
    if (pending_send)
        std::cout << "A pending send exists." << std::endl;
    std::cout << "Total busy cycles : idle cycles\t" << busy_cycle << " : " << idle_cycle << std::endl;
    buffer1->PrintStats();
    buffer2->PrintStats();
}

    if (buffer_index == 1)
        std::cout << "=========================== Buffer 1 Stats ===========================" << std::endl;
    else
        std::cout << "=========================== Buffer 2 Stats ===========================" << std::endl;
    
    std::cout << "Total busy cycles : idle cycles\t" << busy_cycle << " : " << idle_cycle << std::endl;
    
    std::string rcv = (bring_in) ? "receiving from memory" : "NOT receiving anything";
    std::string send = (bring_out) ? "sending to MAC" : "NOT sending anything";

    std::cout << "Currently " << rcv << " and " << send << "." << std::endl;
    if (bring_in)
        std::cout << "Need to receive " << btr << "B." << std::endl;
    if (bring_out)
        std::cout << "Need to send " << bts << "B." << std::endl;
    std::cout << "======================================================================" << std::endl;;
