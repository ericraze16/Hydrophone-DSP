module butterfly_unit #(
    parameter int WIDTH = 16,
    parameter int LATENCY = 3
)(
    input  logic clk,
    input  logic rst_n,
    input  logic en,
    
    // Data inputs (A and B points)
    input  logic signed [WIDTH-1:0] a_re, a_im,
    input  logic signed [WIDTH-1:0] b_re, b_im,
    
    // Twiddle factor (W)
    input  logic signed [WIDTH-1:0] w_re, w_im,
    
    // Control
    input  logic valid_in,
    
    // Outputs
    output logic signed [WIDTH-1:0] out_a_re, out_a_im,
    output logic signed [WIDTH-1:0] out_b_re, out_b_im,
    output logic valid_out
);

    // --- Pipeline Stage 1: Multiplication ---
    // In hardware, this maps to the DSP48 Multiplier (MREG)
    // We use WIDTH*2 to prevent overflow during intermediate math
    logic signed [(WIDTH*2)-1:0] mult_re_aa, mult_re_bb;
    logic signed [(WIDTH*2)-1:0] mult_im_ab, mult_im_ba;
    
    // Delay registers for 'A' to match multiplier latency
    logic signed [WIDTH-1:0] a_re_d1, a_im_d1;
    logic signed [WIDTH-1:0] a_re_d2, a_im_d2;

    always_ff @(posedge clk) begin
        if (en) begin
            // Complex Mult: (b_re + j b_im) * (w_re + j w_im)
            // Re = b_re*w_re - b_im*w_im
            // Im = b_re*w_im + b_im*w_re
            mult_re_aa <= b_re * w_re;
            mult_re_bb <= b_im * w_im;
            mult_im_ab <= b_re * w_im;
            mult_im_ba <= b_im * w_re;

            // Pipeline 'A' to meet the product at the adder
            a_re_d1 <= a_re;
            a_im_d1 <= a_im;
            a_re_d2 <= a_re_d1;
            a_im_d2 <= a_im_d1;
        end
    end

    // --- Pipeline Stage 2: Summing the Products ---
    logic signed [WIDTH-1:0] twid_re, twid_im;

    always_ff @(posedge clk) begin
        if (en) begin
            // Fixed-point shift: we divide by 2^(WIDTH-1) to keep scale
            // Assuming Q1.15 format
            twid_re <= WIDTH'((mult_re_aa - mult_re_bb) >>> (WIDTH-1));
            twid_im <= WIDTH'((mult_im_ab + mult_im_ba) >>> (WIDTH-1));
        end
    end

    // --- Pipeline Stage 3: The Final Addition/Subtraction ---
    // This maps to the DSP48 Post-Adder (PREG)
    always_ff @(posedge clk) begin
        if (!rst_n) begin
        end else if (en) begin
            out_a_re  <= a_re_d2 + twid_re;
            out_a_im  <= a_im_d2 + twid_im;
            out_b_re  <= a_re_d2 - twid_re;
            out_b_im  <= a_im_d2 - twid_im;
        end
    end

    // Simple valid signal delay line
    logic [LATENCY-1:0] v_pipe;
    always_ff @(posedge clk) begin
        if (!rst_n) v_pipe <= '0;
        else if (en) v_pipe <= {v_pipe[LATENCY-2:0], valid_in};
    end
    assign valid_out = v_pipe[LATENCY-1];

endmodule
