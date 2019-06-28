#include <iostream>
#include <cstdlib>

#include "memory.hpp"
#include "buffer.hpp"

int main(int argc, char* argv[]) {
    // configurations
    float clock = 1;                // 1GHz
    float mem_to_buffer_bw = 16;    // 16GB/s
    float buffer_to_mac_bw = 16;    // 16GB/s
    int ub_capacity = 60;           // 60MB (30MB each)
    // setting

/* REALLY STUPID SETTING
    Memory* mem_p = new Memory(clock, mem_to_buffer_bw);
    Memory mem = *mem_p;

    UnifiedBuffer* ub_p = new UnifiedBuffer(clock, buffer_to_mac_bw, ub_capacity, mem_p);
    UnifiedBuffer ub = *ub_p;

    mem.SetBufferConnection(ub_p);

    std::cout << "memory pointer address: " << mem_p << std::endl;
    std::cout << "memory object address: " << &mem << std::endl;
    std::cout << "memory pointer inside UB address: " << ub.GetMemoryPointer() << std::endl;
    std::cout << "memory pointer inside UB pointer address: " << ub_p->GetMemoryPointer() << std::endl;
*/

    Memory* mem_p = new Memory(clock, mem_to_buffer_bw);    
    UnifiedBuffer* ub_p = new UnifiedBuffer(clock, buffer_to_mac_bw, ub_capacity, mem_p);
    mem_p->SetBufferConnection(ub_p);

    // default stat check
    //mem.PrintStats();
    //ub.PrintStats();

    // test start
    std::cout << "Test start!" << std::endl;
    std::cout << "Request: " << std::endl;
    ub_p->SendRequest(100);
    //bool middle = false;
    //ub_p->GetMemoryPointer()->PrintStats();
    mem_p->PrintStats();
    std::cout << "Cycle start!" << std::endl;
    while (true) {
        ub_p->Cycle();
        mem_p->Cycle();
        mem_p->PrintStats();
        ub_p->PrintStats();
        //if (middle && ub_p->IsIdle() && mem_p->IsIdle()) {
        if (ub_p->IsIdle() && mem_p->IsIdle()) {
            break;
        }
        //middle = true;
    }
    return 0;
}
