#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <assert.h>

#include "memory.hpp"
#include "buffer.hpp"
#include "mac.hpp"
#include "controller.hpp"

int main(int argc, char *argv[]) {
    int opt;
    // configurations
    float clock, mem_to_buffer_bw;
    int ub_capacity, mac_number, A, B, C;
    float buffer_to_mac_bw = 100000000; //supposedly infinite bandwidth
    std::string filename;
    
    while ((opt = getopt(argc, argv, "b:c:m:n:s:x:y:z:")) != -1) {
        switch(opt) {
            case 'b':
                // memory BANDWIDTH [GB/s]
                mem_to_buffer_bw = (float)atoi(optarg);
                break;
            case 'c':
                // CLOCK [GHz]
                clock = (float)atoi(optarg);
                break;
            case 'm':
                // number of MAC units
                mac_number = atoi(optarg);
                break;
            case 'n':
                // name of output file
                //strcpy(filename, optarg);
                filename = optarg;
            case 's':
                // SIZE of unified buffer [MB]
                ub_capacity = atoi(optarg);
                break;
            case 'x':
                // X, leading dimension of first matrix
                A = atoi(optarg);
                break;
            case 'y':
                // Y, leading dimension of second matrix
                B = atoi(optarg);
                break;
            case 'z':
                // Z, number of columns of second matrix
                C = atoi(optarg);
                break;
            default:
                // shouldn't reach here
                std::cerr << "?" << std::endl;
                assert(0);
        }
    }

    // setting
    Memory *mem_p =         new Memory(clock, mem_to_buffer_bw);
    UnifiedBuffer *ub_p =   new UnifiedBuffer(clock, buffer_to_mac_bw, ub_capacity, mem_p);
    Mac *mac_p =            new Mac(ub_p, mac_number, clock);
    Controller *ctrl_p =    new Controller(mem_p, ub_p, mac_p, clock);
    //connection
    mem_p->SetBufferConnection(ub_p);
    ub_p->SetMacConnection(mac_p);
    std::cerr << "Test name - MAC: "<< mac_number
              << ", Mem-BW: " << mem_to_buffer_bw
              << " in process." << std::endl;
    // inject a matrix multiplication
    ctrl_p->MatrixMultiply(A, B, C);
    
    while(true) {
        mac_p->Cycle();
        ub_p->Cycle();
        mem_p->Cycle();

        if (mac_p->DoingAbsolutelyNothing())
            break;
    }

    // open file in append mode
    std::ofstream outputfile;
    outputfile.open(filename.c_str(), std::ios::app);  // open filename in append mode
    
    float tflops = clock * mac_p->GetOperations() / (mac_p->GetTotalCycle() * 1000);
    outputfile << mac_number << ", " << mem_to_buffer_bw << ", " << tflops << "," << std::endl;
    outputfile.close();
    return 0;
}
