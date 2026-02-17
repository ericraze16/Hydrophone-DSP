// Basic functional model of a dual-port BRAM for use in FFT
// Syhchronous read/write, read after write, no latency
module dual_port_bram #(
    parameter int POINTS = 1024,      // Number of complex samples
    parameter int SAMPLE_WIDTH = 32,   // 16-bit Re + 16-bit Im
    localparam int ADDR_W = $clog2(POINTS)
)(
    input  logic clk,
    
    input  logic [ADDR_W-1:0]        addr_a,
    input  logic [SAMPLE_WIDTH-1:0]  din_a,
    input  logic                     we_a,
    output logic [SAMPLE_WIDTH-1:0]  dout_a,
    
    input  logic [ADDR_W-1:0]        addr_b,
    input  logic [SAMPLE_WIDTH-1:0]  din_b,
    input  logic                     we_b,
    output logic [SAMPLE_WIDTH-1:0]  dout_b
);

    (* ram_style = "block" *)
    logic [SAMPLE_WIDTH-1:0] mem [POINTS];

    always_ff @(posedge clk) begin
        if (we_a) begin
            mem[addr_a] <= din_a;
        end
        dout_a <= mem[addr_a];
    end

    always_ff @(posedge clk) begin
        if (we_b) begin
            mem[addr_b] <= din_b;
        end
        dout_b <= mem[addr_b];
    end

endmodule
