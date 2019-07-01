#include <iostream>
#include <cstdlib>

#include "memory.hpp"
#include "buffer.hpp"
#include "mac.hpp"

int main(int argc, char* argv[]) {
    // configurations
    float clock = 1;                // 1GHz
    float mem_to_buffer_bw = 16;    // 16GB/s
    float buffer_to_mac_bw = 16;    // 16GB/s
    int ub_capacity = 60;           // 60MB (30MB each)
    int mac_capacity = 16;          // 16KB
    // setting
    Memory *mem_p = new Memory(clock, mem_to_buffer_bw);
    UnifiedBuffer *ub_p = new UnifiedBuffer(clock, buffer_to_mac_bw, ub_capacity, mem_p);
    Mac *mac_p = new Mac(ub_p, mac_capacity);
    mem_p->SetBufferConnection(ub_p);
    ub_p->SetMacConnection(mac_p);
    
    std::cout << "Test start!" << std::endl;
    // inject request to MAC
    for (int i = 0; i < 5; i++)
        mac_p->SendRequest(100);
    mac_p->PrintStats();
    while(true) {
        mac_p->Cycle();
        ub_p->Cycle();
        mem_p->Cycle();

        mac_p->PrintStats();

        if (mac_p->IsIdle() && ub_p->DoingAbsolutelyNothing() && mem_p->IsIdle())
            break;
    }
    std::cout << "Test end!" << std::endl;
    std::cout << "Final MAC statistics:" << std::endl;
    mac_p->PrintStats();
    return 0;
}
