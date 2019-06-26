#include <iostream>
#include <cstdlib>

class Memory {
private:
    int busy_cycle;         // number of busy cycles
    int idle_cycle;         // number of idle cycles
    float bw;               // bandwidth from memory to buffer in GB/s
    float bpc;              // bytes per clock, bw/clock(frequency)

    float bts1;             // bytes to send (remaining) for buffer1
    float bts2;             // bytes to send (remaining) for buffer2
    int rcv_buffer;         // buffer being serviced atm - 0 if none.

    UnifiedBuffer *buffer;  // pointer to buffer connected to this memory, NOT connected in constructor!

public:
    Memory(float clock, float _bw);    // clock should be in GHz
    void Cycle();
    void ReceiveRequest(int bufnum, float _bts);
    bool IsIdle();

    int GetBusyCycle();
    int GetIdleCycle();
    int GetTotalCycle();
    float GetBandwidth();
    float GetBytesPerCycle();
    float GetBytesToSendBuffer1();
    float GetBytesToSendBuffer2();
    float GetBytesToSendTotal();
    int GetServicingBuffer();

    void SetBufferConnection(UnifiedBuffer *_buffer);
    void SignalDoneReceiving();
    
    void PrintStats();
};
