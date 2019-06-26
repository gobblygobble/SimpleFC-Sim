#include <iostream>
#include <cstdlib>
#include "memory.hpp"

int main(int argc, char* argv[]) {
    float clock;
    float bw;

    clock = 1;  // 1GHz
    bw = 16;    // 16GB/s
    
    // check constructor
    std::cout << "Making new Memory module with clock " << clock << "GHz and bandwidth " << bw << "GBps." << std::endl;
    Memory* mem1_p = new Memory(clock, bw);
    Memory mem1 = *mem1_p;
    
    // print stats
    mem1.PrintStats();

    // check idleness
    if (!mem1.IsIdle()) std::cout << "WTF? New memory module isn't idle?" << std::endl;
    
/*
    std::cout << "Request for 100B from buffer 1." << std::endl;
    mem1.ReceiveRequest(1, 100);
    if (mem1.IsIdle()) std::cout << "WTF? Working Memory module is idle?" << std::endl;
    mem1.PrintStats();
    while (!mem1.IsIdle()) {
        mem1.Cycle();
        //mem1.PrintStats();
    }
    mem1.Cycle();
    mem1.PrintStats();

    std::cout << "Request for 50B from buffer 2." << std::endl;
    mem1.ReceiveRequest(2, 50);
    mem1.PrintStats();
    mem1.Cycle();
    mem1.Cycle();
    mem1.Cycle();
    mem1.PrintStats();
    std::cout << "Request for 100B from buffer 1." << std::endl;
    mem1.ReceiveRequest(1, 100);
    mem1.PrintStats();
    while (!mem1.IsIdle()) {
        mem1.Cycle();
    }
    mem1.PrintStats();
*/
    int while_loop_count = 0;
    while (while_loop_count < 100) {
        if (while_loop_count % 30 == 0) mem1.ReceiveRequest(1, 150);
        if (while_loop_count % 40 == 0) mem1.ReceiveRequest(2, 320);
        mem1.Cycle();
        while_loop_count++;
    }
    mem1.PrintStats();

    while (!mem1.IsIdle())
        mem1.Cycle();
    mem1.PrintStats();
    
    return 0;
}
