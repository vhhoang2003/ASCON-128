module ascon_permutation (
    input  logic clk,
    input  logic rst,
    input  logic start,
    input  logic [63:0] state_in [4:0],
    output logic [63:0] state_out [4:0]
);

    logic [63:0] state [4:0];

    logic [3:0] round_counter;

always_ff @ (posedge clk or posedge rst) begin
    if (rst) begin
        round_counter <= 0; 
        state <= state_in;

        // In ASCON-128, the first 12 rounds are identical to the first 12 rounds of ASCON-128a
    end else if (start && round_counter < 12) begin

        // Add constant to regirster 2 and update round counter
        state[2] <= state[2] ^ 64'hF0E1D2C3B4A59687;
        round_counter <= round_counter + 1;
    end
end

ascon_sbox sbox_inst(
	.x0(state[0]), .x1(state[1]), .x2(state[2]), .x3(state[3]), .x4(state[4]),
	.y0(state[0]), .y1(state[1]), .y2(state[2]), .y3(state[3]), .y4(state[4])   
);

ascon_linear lin_inst (
        .x0(state[0]), .x1(state[1]), .x2(state[2]), .x3(state[3]), .x4(state[4]),
        .y0(state[0]), .y1(state[1]), .y2(state[2]), .y3(state[3]), .y4(state[4])
    );

assign state_out = state;

endmodule