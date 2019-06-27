#include <iostream>
#include <cstdlib>
#include <assert.h>

#include "memory.hpp"
#include "buffer.hpp"

Memory::Memory(float clock, float _bw) {
    busy_cycle = 0;
    idle_cycle = 0;
    bw = _bw;
    bpc = _bw / clock;

    bts1 = 0;
    bts2 = 0;
    rcv_buffer = 0;
}

void Memory::Cycle() {
    // take care of cycle
    if (IsIdle()) {
        idle_cycle++;
        return;
    }
    
    // send data
    if (rcv_buffer == 1) {
        bts1 = ((bts1 - bpc) < 0) ? 0 : (bts1 - bpc);
        if (bts1 == 0) {
            // buffer 1 has completed
            if (bts2 == 0) rcv_buffer = 0;  // no pending work
            else rcv_buffer = 2;            // pending work at buffer 2
            
            SignalDoneReceiving(1);
        }
    }
    else if (rcv_buffer == 2) {
        bts2 = ((bts2 - bpc) < 0) ? 0 : (bts2 - bpc);
        if (bts2 == 0) {
            // buffer 2 has completed
            if (bts1 == 0) rcv_buffer = 0;  // no pending work
            else rcv_buffer = 1;            // pending work at buffer 1
            
            SignalDoneReceiving(2);
        }
    }
    else if (rcv_buffer == 0) {
        std::cerr << "rcv_buffer shouldn't be 0 if it is not idle." << std::endl;
        assert(0);
    }
    else {
        std::cerr << "invalid rcv_buffer: " << rcv_buffer << std::endl;
        assert(0);
    }
    busy_cycle++;
    // TODO: what else should go here?
}

void Memory::ReceiveRequest(int bufnum, float _bts) {
    assert(_bts >= 0);
    if (IsIdle())
        rcv_buffer = bufnum;
    if (bufnum == 1) {
        assert(bts1 == 0);
        bts1 = _bts;
    }
    else if (bufnum == 2) {
        assert(bts2 == 0);
        bts2 = _bts;
    }
    else
        assert(0);
}

bool Memory::IsIdle() {
    if ((bts1 == 0) && (bts2 == 0)) {
        if (rcv_buffer == 0) return true;
        std::cerr << "rcv_buffer should be 0 if both bts are 0." << std::endl;
        assert(0);
    }
    else
        return false;
}

int Memory::GetBusyCycle() {
    return busy_cycle;
}

int Memory::GetIdleCycle() {
    return idle_cycle;
}

int Memory::GetTotalCycle() {
    return GetBusyCycle() + GetIdleCycle();
}

float Memory::GetBandwidth() {
    return bw;
}

float Memory::GetBytesPerCycle() {
    return bpc;
}

float Memory::GetBytesToSendBuffer1() {
    return bts1;
}

float Memory::GetBytesToSendBuffer2() {
    return bts2;
}

float Memory::GetBytesToSendTotal() {
    return GetBytesToSendBuffer1() + GetBytesToSendBuffer2();
}

int Memory::GetServicingBuffer() {
    return rcv_buffer;
}

void Memory::SetBufferConnection(UnifiedBuffer *_buffer) {
    buffer = _buffer;
}

void Memory::SignalDoneReceiving(int finished_index) {
    //assert(finished_index == rcv_buffer);
    bool pending = IsIdle() ? false : true;
    buffer->ReceiveDoneSignal(finished_index, pending);
}

void Memory::PrintStats() {
    std::cout << "======================================================================" << std::endl;
    std::cout << "Total busy cycles : idle cycles\t" << busy_cycle << " : " << idle_cycle << std::endl;
    if (IsIdle()) std::cout << "Currently idle." << std::endl;
    else std::cout << "Currently servicing buffer #" << rcv_buffer << "." << std::endl;
    std::cout << "Need to send " << bts1 + bts2 << "B (buffer 1: " << bts1 << "B, buffer 2: " << bts2 << "B)" << std::endl;
    std::cout << "======================================================================" << std::endl;
}
