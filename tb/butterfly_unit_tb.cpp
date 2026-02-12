#include "Vbutterfly_unit.h"
#include "verilated.h"
#include "verilated_vcd_c.h"
#include <vector>
#include <iostream>
#include <cmath>

int main(int argc, char** argv) {
    std::cout << "Starting Butterfly Unit Testbench" << std::endl;

    Verilated::commandArgs(argc, argv);
    Vbutterfly_unit* top = new Vbutterfly_unit;

    // Fixed-point scale factor (2^15)
    const int SCALE = 32768;

    // Test Case: Rotate 45 degrees
    // B = 1000 + 0j
    // W = cos(45) + j sin(45) -> 0.707 + 0.707j
    top->a_re = 5000; top->a_im = 0;   // A point
    top->b_re = 1000; top->b_im = 0;   // B point
    top->w_re = 0.707 * SCALE;         // Twiddle Real
    top->w_im = 0.707 * SCALE;         // Twiddle Imag
    top->rst_n = 0;
    top->valid_in = 1;
    top->en = 1;


    // Initialize Tracing
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    top->trace(tfp, 99); // Trace 99 levels of hierarchy
    tfp->open("waveform.vcd");

    int time = 0;


    for (int i = 0; i < 100; i++) {
        if (i == 2) top->rst_n = 1; // Release reset after 2 cycles
        
        top->clk = 0; top->eval();
        top->clk = 1; top->eval();
        tfp->dump(time++);

        // if (top->valid_out) {
            std::cout << "Cycle " << i << " Output Ready!" << std::endl;
            std::cout << "Inputs: " << top->a_re << " + " << top->a_im << "j, "
                      << top->b_re << " + " << top->b_im << "j, W: "
                      << top->w_re << " + " << top->w_im << "j" << std::endl;
            std::cout << "Out A: " << top->out_a_re << " + " << top->out_a_im << "j" << std::endl;
            std::cout << "Out B: " << top->out_b_re << " + " << top->out_b_im << "j" << std::endl;
        // }
    }

    delete top;
    return 0;
}