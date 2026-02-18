module butterfly_unit #(
    // Uses DIF - capture signal from ADC in natural order
    // With DIF, we want to compute A' = (A+B), B' = (A-B)*W

    parameter int WIDTH = 16,
    parameter int LATENCY = 3
)(
    input  logic clk,
    input  logic rst_n,
    input  logic en,
    
    // A and B complex valued points
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

    // Stage 1: sum and diff
    // Use width+1 bits to prevent overflowing the sum
    logic signed [WIDTH:0] sum_re, sum_im, diff_re, diff_im;
    logic signed [WIDTH-1:0] w_re_d1, w_im_d1;

    always_ff @(posedge clk) begin
        if (en) begin
            sum_re <= a_re + b_re;
            sum_im <= a_im + b_im;
            diff_re <= a_re - b_re;
            diff_im <= a_im - b_im;
            w_re_d1 <= w_re;
            w_im_d1 <= w_im;
        end
    end

    // Stage 2: multiply diff by W, pass through sum
    logic signed [(WIDTH*2)-1:0] b_prime_re, b_prime_im;
    logic signed [WIDTH:0] a_prime_re, a_prime_im;

    always_ff @(posedge clk) begin
        if (en) begin
            a_prime_re <= sum_re;
            a_prime_im <= sum_im;
            b_prime_re <= diff_re * w_re_d1 - diff_im * w_im_d1;
            b_prime_im <= diff_re * w_im_d1 + diff_im * w_re_d1;
        end
    end

    // Stage 3: Scale outputs
    always_ff @(posedge clk) begin
        if (!rst_n) begin
            out_a_re <= '0; out_a_im <= '0;
            out_b_re <= '0; out_b_im <= '0;
        end else if (en) begin
            // Account for 17 bit accumulator by right shifting 1
            out_a_re <= WIDTH'(a_prime_re >>> 1);
            out_a_im <= WIDTH'(a_prime_im >>> 1);
            out_b_re <= WIDTH'(b_prime_re >>> (WIDTH-1));
            out_b_im <= WIDTH'(b_prime_im >>> (WIDTH-1));
        end
    end

    // Left shift valid signal, MSB is current value
    logic [LATENCY-1:0] v_pipe;
    always_ff @(posedge clk) begin
        if (!rst_n) v_pipe <= '0; // flush
        else if (en) v_pipe <= {v_pipe[LATENCY-2:0], valid_in};
    end
    assign valid_out = v_pipe[LATENCY-1];

endmodule
