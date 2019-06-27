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
    Memory* mem_p = new Memory(clock, mem_to_buffer_bw);
    Memory mem = *mem_p;

    UnifiedBuffer* ub_p = new UnifiedBuffer(clock, buffer_to_mac_bw, ub_capacity, mem_p);
    UnifiedBuffer ub = *ub_p;

    mem.SetBufferConnection(ub_p);

    // default stat check
    mem.PrintStats();
    ub.PrintStats();

    // test start
    std::cout << "Test start!" << std::endl;
    ub.SendRequest(100);
    bool middle = false;
    while (true) {
        ub.Cycle();
        mem.Cycle();
        mem.PrintStats();
        ub.PrintStats();
        if (middle && ub.IsIdle() && mem.IsIdle()) {
            break;
        }
        middle = true;
    }
    return 0;
}
