#include <iostream>
#include <verilated.h>
#include "Vdual_port_bram.h" 

void tick(Vdual_port_bram* dut) {
    dut->clk = 0; dut->eval();
    dut->clk = 1; dut->eval();
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vdual_port_bram* dut = new Vdual_port_bram;

    const int DEPTH = 1024;
    const uint32_t BASE_PATTERN = 0xABC00000;
    
    std::cout << "Starting BRAM Comprehensive Test..." << std::endl;

    // --- PHASE 1: FILL MEMORY (Port A) ---
    std::cout << "Writing patterns via Port A..." << std::endl;
    dut->we_b = 0;
    dut->we_a = 1;
    for (int i = 0; i < DEPTH; i++) {
        dut->addr_a = i;
        dut->din_a = i + BASE_PATTERN;
        tick(dut);
        std::cout << "Wrote Addr " << i << " with Data: " << std::hex << dut->din_a << std::dec << std::endl;
    }
    dut->we_a = 0;

    // --- PHASE 2: VERIFY MEMORY (Corrected Timing) ---
    std::cout << "Verifying patterns via Port B..." << std::endl;
    for (int i = 0; i < DEPTH; i++) {
        // 1. Set the address while clock is LOW
        dut->addr_b = i; 
        tick(dut);

        if (i > 0) {
            uint32_t expected = i + BASE_PATTERN;
            if (dut->dout_b != expected) {
                std::cout << "FAIL: Addr " << (i - 1) 
                          << " | Expected: " << std::hex << expected 
                          << " | Got: " << dut->dout_b << std::dec << std::endl;
            } else {
                std::cout << "PASS: Addr " << (i - 1) 
                          << " | Data: " << std::hex << dut->dout_b << std::dec << std::endl;
            }
        }
    }
    // Final tick to see the very last address (DEPTH-1)
    dut->clk = 0; dut->eval();
    dut->clk = 1; dut->eval();
    if (dut->dout_b != (DEPTH - 1 + BASE_PATTERN)) { std::cout << "Failed at last addr"; }

    // --- TEST 2: Simultaneous Dual-Port Write ---
    std::cout << "TEST 2: Simultaneous Dual-Port Write..." << std::endl;
    dut->we_a = 1; dut->we_b = 1;
    dut->addr_a = 10; dut->din_a = 0x11112222;
    dut->addr_b = 20; dut->din_b = 0x33334444;
    tick(dut);
    dut->we_a = 0; dut->we_b = 0;
    
    // Verify Port A
    dut->addr_a = 10; 
    tick(dut);
    if(dut->dout_a != 0x11112222) std::cout << "Port A Write Failed!" << std::endl;
    else std::cout << "Port A Write Verified" << std::endl;
    
    // Verify Port B
    dut->addr_b = 20;
    tick(dut);
    if(dut->dout_b != 0x33334444) std::cout << "Port B Write Failed!" << std::endl;
    else std::cout << "Port B Write Verified" << std::endl;

    std::cout << "Tests Complete." << std::endl;
    delete dut;
    return 0;
}