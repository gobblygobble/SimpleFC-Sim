#include <iostream>
#include <cstdlib>
#include <assert.h>
#include <string>

#include "memory.hpp"
#include "buffer.hpp"
#include "mac.hpp"
#include "controller.hpp"

#define DEBUG

Controller::Controller(Memory *_memory, UnifiedBuffer *_ub, Mac *_mac, float _clock) {
    memory = _memory;
    ub = _ub;
    mac = _mac;
    clock = _clock;
    power = mac->GetPower();
    buffer_size = ub->GetCapacity() / 2 * 1000000;
}

void Controller::MatrixMultiply(int _A, int _B, int _C) {
    A = _A;
    B = _B;
    C = _C;
    Tile();
    SendRequestToMac();
}

void Controller::Tile() {
    a = GreatestCommonDivisor(A, C);
    while (true) {
        b = (int)(power / (a * a));
        if (b != 0)
            break;
        a = (int)(a / 2);
    }
    while (B % b) {
        b--;
    }
    c = a;
}

void Controller::SendRequestToMac() {
    int tile_size = (a * b + b * c);
    int tiles_in_chunk = (int)(buffer_size / tile_size);    // number of tiles in a chunk of request
    long long int T = (long long int)A * (long long int)B * (long long int)C;
    long long int s = (long long int)(a * b * c);
    long long int t = s * (long long int)tiles_in_chunk;
    int whole_chunk_num = (int)(T / t);                     // number of whole chunks
    int chunk_size = tiles_in_chunk * tile_size;            // total size of chunk of request
    int comp_per_tile = 2 * a * b * c;
    // send whole chunks
    for (int i = 0; i < whole_chunk_num; i++)
        mac->ReceiveRequest(chunk_size, tile_size, comp_per_tile);
    // send remaining
    int remaining = (int)(tile_size * (float)T / s) - chunk_size * whole_chunk_num;
    if (remaining)
        mac->ReceiveRequest(remaining, tile_size, comp_per_tile);
}

int Controller::GreatestCommonDivisor(int x, int y) {
    if (y == 0)
        return x;
    return GreatestCommonDivisor(y, x % y);
}


