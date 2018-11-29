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
	RESULT2,
	RESULT1,
	RESULT0,
	HREF,
	SHAPE
);


//=======================================================
//  PORT declarations
//=======================================================
input	[7:0]	PIXEL_IN;
input 		CLK;
input [1:0] SHAPE;
input [9:0] VGA_PIXEL_X;
input [9:0] VGA_PIXEL_Y;
input			VSYNC;
input			HREF;


output reg RESULT0;
output reg RESULT1;
output reg RESULT2;
reg [15:0] countBLUE;
reg [15:0] countRED;
reg [15:0] countNULL;
reg [15:0] R_CNT_THRESHOLD = 16'd7000;
reg [15:0] B_CNT_THRESHOLD = 16'd10000;
reg lastsync = 1'b0;
reg lasthref = 1'b0;
reg [175:0] first_row;
reg [175:0] middle_row;
reg [175:0] last_row;
always @(posedge CLK) begin
	if(HREF) begin
	
		if(PIXEL_IN == 8'b00000011 ) begin countBLUE = countBLUE + 16'd1; end
		else if(PIXEL_IN == 8'b11100000) begin countRED = countRED + 16'b1; end
		else begin countNULL = countNULL + 16'd1; end
		
	end
	if(VSYNC && ~lastsync) begin //posedge VSYNC (end of frame - assign shapes & colors)
		RESULT1 = SHAPE[0];
		RESULT2 = SHAPE[1];
		if(countBLUE > B_CNT_THRESHOLD) begin RESULT0 = 1'b0; end
		else if(countRED > R_CNT_THRESHOLD) begin RESULT0 = 1'b1; end
		else begin RESULT0 = 1'b0; end
	end	
	if(VSYNC == 1'b0 && lastsync == 1'b1) begin //negedge VSYNC (reset count)
		countBLUE = 16'b0;
		countRED = 16'b0;
		countNULL = 16'b0;
	end
	lastsync = VSYNC;
end

endmodule



