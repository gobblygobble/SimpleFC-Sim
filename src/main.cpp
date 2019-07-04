#include <iostream>
#include <cstdlib>

#include "memory.hpp"
#include "buffer.hpp"
#include "mac.hpp"
#include "controller.hpp"

int main(int argc, char* argv[]) {
    // configurations
    float clock = 1;                    // 1GHz
    float mem_to_buffer_bw = 600;       // 16GB/s
    float buffer_to_mac_bw = 1000000;   // supposedly infinite bandwidth
    int ub_capacity = 60;               // 60MB (30MB for each buffer)
    int mac_power = 7500;               // how many mac units per big MAC unit
    // setting
    Memory *mem_p = new Memory(clock, mem_to_buffer_bw);
    UnifiedBuffer *ub_p = new UnifiedBuffer(clock, buffer_to_mac_bw, ub_capacity, mem_p);
    Mac *mac_p = new Mac(ub_p, mac_power, clock);
    Controller* ctrl_p = new Controller(mem_p, ub_p, mac_p, clock);
    // connection
    mem_p->SetBufferConnection(ub_p);
    ub_p->SetMacConnection(mac_p);

    std::cout << "Test start!" << std::endl;
    // inject a matrix multiplication
    int A = 1024;
    int B = 1024;
    int C = 1024;
    ctrl_p->MatrixMultiply(A, B, C);   // (A by B) X (B by C)
    mac_p->PrintStats();
    while(true) {
        mac_p->Cycle();
        ub_p->Cycle();
        mem_p->Cycle();

        if (mac_p->DoingAbsolutelyNothing())
        //if(mac_p->DoingAbsolutelyNothing() && ub_p->DoingAbsolutelyNothing() && mem_p->IsIdle())
            break;
    }
    mac_p->PrintStats();
    std::cout << "A " << A << " by " << B << " matrix X " << B << " by " << C << " matrix multiplication Statistics:" << std::endl;
    std::cout << "MAC has run for a total of " << mac_p->GetTotalCycle() << " cycles" << std::endl;
    std::cout << "MAC unit utilization is "
              << int((float)mac_p->GetBusyCycle() / (float)mac_p->GetTotalCycle() * 100) << "%." << std::endl;
    std::cout << "The matrix multiplication requires " << ctrl_p->GetTotalOperations() << " operations." << std::endl;
    //std::cout << "The matrix multiplication requires " << mac_p->GetOperations() << " operations." << std::endl;
    std::cout << "The MAC resulted in: " << clock * ctrl_p->GetTotalOperations() / mac_p->GetTotalCycle() << " GFLOPs." << std::endl;
    //std::cout << "The MAC resulted in: " << clock * mac_p->GetOperations() / mac_p->GetTotalCycle() << " GFLOPs." << std::endl;
}
