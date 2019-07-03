#include <iostream>
#include <cstdlib>

#pragma once

class Memory;
class Buffer;
class UnifiedBuffer;
class Mac;

class Controller {
private:
    Memory* memory;
    UnifiedBuffer *ub;
    Mac *mac;
    float clock;        // GHz
    int power;
    int buffer_size;    // Bytes
    int A;
    int B;
    int C;
    int a;
    int b;
    int c;

public:
    Controller(Memory *_memory, UnifiedBuffer *_ub, Mac *_mac, float _clock);
    void MatrixMultiply(int _A, int _B, int _C);
    void Tile();
    void SendRequestToMac();

    int GreatestCommonDivisor(int x, int y);

    int GetTotalOperations() {return 2 * A * B * C;}
};
