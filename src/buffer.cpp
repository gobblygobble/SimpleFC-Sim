#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <string>
#include <queue>

#include "memory.hpp"
#include "buffer.hpp"
#include "mac.hpp"

//#define DEBUG

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
#ifdef DEBUG
    std::cout << "buffer #" << buffer_index << ": Buffer::Cycle() start." << std::endl;
    std::cout << "bring_in: " << bring_in << ": , bring_out: " << bring_out << std::endl;
#endif
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
#ifdef DEBUG
    std::cout << "buffer #" << buffer_index << " Buffer::Cycle() finished." << std::endl;
#endif
}

/* In this function it is assumed that UnifiedBuffer::SendRequest() picks the right index of buffer
 * and calls (this) Buffer::SendRequest() on the right buffer. */
void Buffer::SendRequest(float _btr) {
    bring_in = true;
    btr = _btr;
    memory->ReceiveRequest(buffer_index, _btr);
#ifdef DEBUG
    std::cout << "After Buffer::SendRequest() stats:" << std::endl;
    memory->PrintStats();
#endif
}

/* In this function it is assumed that UnifiedBuffer::ReceiveRequest() picks the right index of buffer
 * and calls (this) Buffer::ReceiveRequest() on the right buffer.
 * Contacting with MAC should also be done at UnifiedBuffer-level. */
void Buffer::ReceiveRequest(float _bts) {
    bring_out = true;
    bts = _bts;
}

bool Buffer::IsSending() {
    return bring_out;
}

bool Buffer::IsReceiving() {
    return bring_in;
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

void Buffer::SetBringIn(bool flag, float size) {
    if (bring_in == flag) {
        std::cerr << "Trying to change bring_in to identical value" << std::endl;
        assert(0);
    }
    bring_in = flag;
    if (bring_in) {
        assert(btr == 0);
        btr = size;
    }
}

void Buffer::SetBringOut(bool flag) {
    if (bring_out == flag) {
        std::cerr << "Trying to change bring_out to identical value" << std::endl;
        assert(0);
    }
    bring_out = flag;
}

void Buffer::NotifyChangeInReceiver(int finished_index) {
    ub->ChangeInReceiver(finished_index);
}

void Buffer::NotifyChangeInSender(int finished_index) {
    ub->ChangeInSender(finished_index);
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

    receiving_req_num = 0;
}

void UnifiedBuffer::Cycle() {
#ifdef DEBUG
    std::cout << "UnifiedBuffer::Cycle() start." << std::endl;
#endif
    if (IsIdle())
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
#ifdef DEBUG
    std::cout << "UnifiedBuffer::Cycle() finished." << std::endl;
#endif
}

bool UnifiedBuffer::IsIdle() {
    return (buffer1->IsIdle() && buffer2->IsIdle());
}

bool UnifiedBuffer::IsNotReceiving() {
    return (!(buffer1->IsReceiving()) && !(buffer2->IsReceiving()) && (req_queue.empty()));
}

bool UnifiedBuffer::DoingAbsolutelyNothing() {
    return (IsIdle() && IsNotReceiving());
}

void UnifiedBuffer::HandleQueue() {
    req r;
    if (memory->IsIdle()) {
        if (!req_queue.empty()) {
            r = req_queue.front();
            req_queue.pop();
            SendRequest(r.size, r.order);

            if (req_queue.empty())
                pending_receive = false;
        }
        else
            rcv_buffer = 0;
    }
}

void UnifiedBuffer::SendRequest(float _btr, int req_number) {
    if (memory->IsIdle()) {
        if (latest_rcv_index == 1) {
            rcv_buffer = 2;
            buffer2->SendRequest(_btr);
            receiving_req_num = req_number;
        }
        else {
            rcv_buffer = 1;
            buffer1->SendRequest(_btr);
            receiving_req_num = req_number;
        }
    }
    else {
        req r;
        r.size = _btr;
        r.order = req_number;
        req_queue.push(r);
    }
}

void UnifiedBuffer::ReceiveRequest(float _bts, int req_num) {
    req r;
    r.size = _bts;
    r.order = req_num;
    req_queue.push(r);
}

void UnifiedBuffer::ReceiveDoneSignal(int rcv_buffer_index, bool pending) {
#ifdef DEBUG
    std::cout << "UnifiedBuffer::ReceiveDoneSignal() start." << std::endl;
#endif
    UpdateLatestReceivedIndex(rcv_buffer_index);
    ready_queue.push(receiving_req_num);
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
    ChangeInReceiver(rcv_buffer_index);

#ifdef DEBUG
    std::cout << "UnifiedBuffer::ReceiveDoneSignal() finished." << std::endl;
#endif
}

void UnifiedBuffer::UpdateLatestReceivedIndex(int finished_index) {
#ifdef DEBUG
    std::cout << "finished_index, latest_rcv_index: " << finished_index << ", " << latest_rcv_index<< std::endl;
#endif
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
    HandleQueue();
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

bool UnifiedBuffer::CheckExistenceInReadyQueue(int order) {
    assert(order >= ready_queue.front());
    return (order == ready_queue.front());
}

void UnifiedBuffer::MacUsingChunk(int order) {
    assert(CheckExistenceInReadyQueue(order));
    ready_queue.pop();
}

void UnifiedBuffer::SetMacConnection(Mac *_mac) {
    mac = _mac;
}
