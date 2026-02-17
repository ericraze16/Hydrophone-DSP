#include "Vbutterfly_unit.h"
#include "verilated.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>

// Fixed-point scale factor (Q1.15)
const int SCALE = 32768;

struct TestCase {
    std::string name;
    int a_re, a_im, b_re, b_im;
    double w_re_f, w_im_f;
};

void run_test(Vbutterfly_unit* dut, TestCase tc) {
    // 1. Setup inputs
    dut->rst_n = 0; dut->clk = 0; dut->eval();
    dut->rst_n = 1; dut->clk = 1; dut->eval(); // Reset pulse

    dut->a_re = tc.a_re; dut->a_im = tc.a_im;
    dut->b_re = tc.b_re; dut->b_im = tc.b_im;
    dut->w_re = (int)(tc.w_re_f * (SCALE - 1)); 
    dut->w_im = (int)(tc.w_im_f * (SCALE - 1));
    dut->en = 1;
    dut->valid_in = 1;

    // 2. Run for LATENCY + 1 cycles
    for (int i = 0; i < 5; i++) {
        dut->clk = 0; dut->eval();
        dut->clk = 1; dut->eval();
    }

    // 3. Print Results
    std::cout << "TEST: " << tc.name << std::endl;
    std::cout << "  Input A: (" << tc.a_re << " + " << tc.a_im << "j)" << std::endl;
    std::cout << "  Input B: (" << tc.b_re << " + " << tc.b_im << "j) rotated by " << tc.w_re_f << " + " << tc.w_im_f << "j" << std::endl;
    std::cout << "  Result A: (" << (int16_t)dut->out_a_re << " + " << (int16_t)dut->out_a_im << "j)" << std::endl;
    std::cout << "  Result B: (" << (int16_t)dut->out_b_re << " + " << (int16_t)dut->out_b_im << "j)" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;
}

int main(int argc, char** argv) {
    Verilated::commandArgs(argc, argv);
    Vbutterfly_unit* dut = new Vbutterfly_unit;

    std::vector<TestCase> tests = {
        // Simple Add/Sub (W = 1 + 0j)
        {"Identity (Sum/Diff)", 1000, 0, 500, 0, 1.0, 0.0}, 

        // 90 degree rotation (W = 0 + 1j)
        // B(500+0j) * W(0+1j) = (0 + 500j)
        // OutA = (1000+0j) + (0+500j) = 1000 + 500j
        {"90 deg Rotation", 1000, 0, 500, 0, 0.0, 1.0},

        // Full Cancellation
        // A=500, B=500, W=1 -> OutB should be 0
        {"Perfect Cancellation", 500, 0, 500, 0, 1.0, 0.0},

        // Max Negative Values (Check 2's complement)
        {"Negative Logic", -1000, 0, -500, 0, 1.0, 0.0},

        // Potential Overflow Case
        // 30000 + 30000 = 60000 (Should wrap or clip in 16-bit)
        {"Overflow Check", 30000, 0, 30000, 0, 1.0, 0.0}
    };

    for (const auto& tc : tests) {
        run_test(dut, tc);
    }

    delete dut;
    return 0;
}