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

// Fixed point math gives quantization errors
bool is_close(int actual, int expected, int tolerance = 2) {
    return std::abs(actual - expected) <= tolerance;
}

void run_test(Vbutterfly_unit* dut, TestCase tc) {
    // --- 1. Stimulus ---
    dut->rst_n = 0; dut->clk = 0; dut->eval();
    dut->rst_n = 1; dut->eval(); // De-assert reset
    
    // Apply inputs
    dut->a_re = tc.a_re; dut->a_im = tc.a_im;
    dut->b_re = tc.b_re; dut->b_im = tc.b_im;
    dut->w_re = (int)std::round(tc.w_re_f * (SCALE - 1)); 
    dut->w_im = (int)std::round(tc.w_im_f * (SCALE - 1));
    dut->en = 1;
    dut->valid_in = 1;

    // --- 2. Step through Latency ---
    // Note: We toggle clk here. Ensure inputs stay stable if DUT is registered
    for (int i = 0; i < 6; i++) {
        dut->clk = !dut->clk;
        dut->eval();
    }

    // --- 3. Calculate Expected (The Golden Model) ---
    // DIF Math:
    // sum = A + B
    // diff = A - B
    // A_out = sum / 2
    // B_out = (diff * W) / SCALE
    
    int sum_re = tc.a_re + tc.b_re;
    int sum_im = tc.a_im + tc.b_im;
    int exp_a_re = sum_re >> 1;
    int exp_a_im = sum_im >> 1;

    int diff_re = tc.a_re - tc.b_re;
    int diff_im = tc.a_im - tc.b_im;
    int w_re_int = (int)std::round(tc.w_re_f * (SCALE - 1));
    int w_im_int = (int)std::round(tc.w_im_f * (SCALE - 1));

    // Complex Multiply: (dr + jdi) * (wr + jwi)
    // Re: (dr*wr - di*wi), Im: (dr*wi + di*wr)
    long long prod_re = ((long long)diff_re * w_re_int) - ((long long)diff_im * w_im_int);
    long long prod_im = ((long long)diff_re * w_im_int) + ((long long)diff_im * w_re_int);
    
    int exp_b_re = (int)(prod_re >> 15);
    int exp_b_im = (int)(prod_im >> 15);

    // --- 4. Comparison ---
    int actual_a_re = (int16_t)dut->out_a_re;
    int actual_a_im = (int16_t)dut->out_a_im;
    int actual_b_re = (int16_t)dut->out_b_re;
    int actual_b_im = (int16_t)dut->out_b_im;

    bool pass = is_close(actual_a_re, exp_a_re) && is_close(actual_a_im, exp_a_im) &&
                is_close(actual_b_re, exp_b_re) && is_close(actual_b_im, exp_b_im);

    // --- 5. Print Results ---
    std::cout << (pass ? "[PASS] " : "[FAIL] ") << tc.name << std::endl;
    if (!pass) {
        std::cout << "   Expected A: (" << exp_a_re << " + " << exp_a_im << "j)" << std::endl;
        std::cout << "   Actual A:   (" << actual_a_re << " + " << actual_a_im << "j)" << std::endl;
        std::cout << "   Expected B: (" << exp_b_re << " + " << exp_b_im << "j)" << std::endl;
        std::cout << "   Actual B:   (" << actual_b_re << " + " << actual_b_im << "j)" << std::endl;
    }
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

    for (int i = 0; i < 100; i++) {
        TestCase random_test = {
            "Random_" + std::to_string(i),
            (rand() % 20000) - 10000, // Random A
            (rand() % 20000) - 10000, 
            (rand() % 20000) - 10000, // Random B
            (rand() % 20000) - 10000,
            ((rand() % 2000) - 1000) / 1000.0, // Random W real (-1.0 to 1.0)
            ((rand() % 2000) - 1000) / 1000.0  // Random W imag
        };
        run_test(dut, random_test);
    }

    delete dut;
    return 0;
}