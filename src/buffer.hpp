#include <iostream>
#include <cstdlib>
#include "memory.hpp"

/*
enum workstatus {
    NOTHING,
    SEND_AND_RCV,
    SEND,
    RCV,
};
typedef enum workstatus workstatus;
*/

class Buffer {
private:
    int buffer_index;   // index of buffer - 1 or 2
    Memory *memory;     // pointer to memory connected to this buffer
    UnifiedBuffer *ub;  // pointer to unified buffer, which includes this buffer

    bool bring_in;      // whether it is receiving data from Memory
    bool bring_out;     // whether it is sending data to MAC
    int capacity;       // size of capacity in MB
    int busy_cycle;     // number of busy cycles (sending)
    int idle_cycle;     // number of idle cycles
    float bw;           // bandwidth from buffer to MAC
    float bpc;          // bytes per clock, bw/clock(frequency)

    float bts;          // bytes to send (remaining) for MAC
    float btr;          // bytes to receive (remaining) from memory

public:
    Buffer(int _buffer_index, float _bw, float _bpc, int _capacity, Memory *_memory, UnifiedBuffer *_ub);
    void Cycle(bool pending_receive, bool pending_send);
    void SendRequest(float _btr);
    void ReceiveRequest(float _bts);
    bool IsReceiving();
    bool IsSending();
    bool IsIdle();

    int GetBusyCycle();
    int GetIdleCycle();
    int GetTotalCycle();
    float GetBandwidth();
    float GetBytesPerCycle();
    float GetBytesToSend();
    float GetBytesToReceive();

    void PrintStats();
};

class UnifiedBuffer {
private:
    Buffer *buffer1;        // pointer to first buffer
    Buffer *buffer2;        // pointer to second buffer
    Memory *memory;         // pointer to memory connected to this buffer
    int rcv_buffer;         // index of buffer receiving data from memory
    int send_buffer;        // index of buffer sending data to MAC

    int busy_cycle;         // number of cycles that either one of the two buffers were busy (sending)
    int idle_cycle;         // number of cycles that both of the buffers were idle (not sending)
    float bw;               // bandwidth from buffer to MAC
    float bpc;              // bytes per clock, bw/clock(frequency)
    int capacity;           // total capacity of unified buffer

    bool pending_receive;   // whether data to receive exists after the current receiving wave
    bool pending_send;      // whether data to send exists after the current sending wave

public:
    UnifiedBuffer(float clock, float _bw, int _capacity, Memory *_memory);
    void Cycle();
    void SendRequest();

    void ReceiveDoneSignal(int rcv_buffer_index, bool pending);

    void PrintStats();
};
