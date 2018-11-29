`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VGA_VSYNC_NEG,
	RESULT
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VGA_VSYNC_NEG;

output [8:0] RESULT;

reg [14:0] blue_px;
reg [14:0] red_px;
reg [14:0] px_count;

//always @ (posedge CLK) begin
//	// reset count when we reach end of frame
//	if (px_count > (`SCREEN_WIDTH * `SCREEN_HEIGHT) begin
//		blue_px <= 0;
//		red_px <= 0;
//		px_count <= 0;
//	end
//	else begin
//		if () begin
//		
//		end
//		if () begin
//		
//		end
//	end
//
//end

endmodule