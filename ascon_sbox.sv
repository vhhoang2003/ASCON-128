module ascon_sbox (
    // 5 registers of 64 bits input
    input  logic [63:0] x0, x1, x2, x3, x4,
    // 5 registers of 64 bits output
    output logic [63:0] y0, y1, y2, y3, y4
);

    // temporary registers for the sbox
    logic [63:0] t0, t1, t2, t3, t4;

    assign t0 = ~x0 & x1;
    assign t1 = ~x1 & x2;
    assign t2 = ~x2 & x3;
    assign t3 = ~x3 & x4;
    assign t4 = ~x4 & x0;

    assign tmp0 = x0 ^ t1;
    assign tmp1 = x1 ^ t2;
    assign tmp2 = x2 ^ t3;
    assign tmp3 = x3 ^ t4;
    assign tmp4 = x4 ^ t0;

    assign y1 = tmp1 ^ tmp0;
    assign y0 = tmp0 ^ tmp4;
    assign y3 = tmp3 ^ tmp2;
    assign tmp2 = ~tmp2;

endmodule
