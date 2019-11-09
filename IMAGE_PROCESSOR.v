`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48

module IMAGE_PROCESSOR (
	PIXEL_IN,
	CLK,
	VGA_PIXEL_X,
	VGA_PIXEL_Y,
	VSYNC,
	RESULT,
	HREF
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;

input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VSYNC;
input			HREF;


output [2:0] RESULT;
reg RESULT;
reg [15:0] countBLUE;
reg [15:0] countRED;
reg [15:0] countNULL;
//reg RED_THRESHOLD;
reg [15:0] B_CNT_THRESHOLD = 16'd20000;
reg lastsync = 1'b0;
//reg R_CNT_THRESHOLD;
always @(posedge CLK) begin
	if(HREF) begin
		if(PIXEL_IN == 8'b0) begin countBLUE = countBLUE + 16'd1; end
		//else if(PIXEL_IN > RED_THRESHOLD) begin countRED = countRED + 15'b1; end
		else begin countNULL = countNULL + 16'd1; end
	end
	if(VSYNC == 1'b1 && lastsync == 1'b0) begin //posedge VSYNC
		if(countBLUE > B_CNT_THRESHOLD) begin RESULT = 3'b000; end
		//else if(countRED > R_CNT_THRESHOLD) begin RESULT = 3'b001; end
		else begin RESULT = 3'b001; end
	end	
	if(VSYNC == 1'b0 && lastsync == 1'b1) begin //negedge VSYNC
		countBLUE = 16'b0;
		countRED = 16'b0;
		countNULL = 16'b0;
	end
	lastsync = VSYNC;
end

endmodule
