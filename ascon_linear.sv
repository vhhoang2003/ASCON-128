module ascon_linear (
	input logic [63:0] x0, x1, x2, x3, x4,
	output logic [63:0] y0, y1, y2, y3, y4
);

// Shift left logical function
function automatic logic [63 : 0] rotate (logic [63:0] x, int l);
	return (x >> l) | (x << (64 - l));
endfunction
// Linear diffusion layer
assign y0 = x0 ^ rotate (x0, 19) ^ rotate (x0, 28);
assign y1 = x1 ^ rotate (x1, 61) ^ rotate (x0, 39);
assign y2 = x2 ^ rotate (x2, 1)  ^ rotate (x0, 6);
assign y3 = x3 ^ rotate (x3, 10) ^ rotate (x0, 17);
assign y4 = x4 ^ rotate (x4, 7)  ^ rotate (x0, 41);

endmodule