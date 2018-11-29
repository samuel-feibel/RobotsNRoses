`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144
`define NUM_BARS 3
`define BAR_HEIGHT 48
`define X_LOW 20
`define X_HIGH 150
`define THRESHOLD 7000
`define SHAPE_T 100

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
// RESULT BITS: ( USING PARALLEL COMMUNICATION )
// [2]   TREASURE COLOR ( 0 = BLUE , 1 = RED ) 
// [1:0] TREASURE SHAPE / PRESENCE OF TREASURE ( 00 = NO TREASURE, 01 = TRIANGLE, 10 = DIAMOND, 11 = SQUARE ) 
output [2:0] RESULT; 

reg [2:0] RESULT;
// counters are 15 bits because 176x144 screen size
// need to store up to 25k values, so 15 bits needed
reg [14:0] red_count  = 15'd0; // red pixels
reg [14:0] blue_count = 15'd0; // blue pixels
reg [14:0] blue_top   = 15'd0; // blue pixels in upper third
reg [14:0] blue_mid   = 15'd0; // blue pixels in middle third
reg [14:0] blue_bot   = 15'd0; // blue pixels in bottom third
reg [14:0] red_top    = 15'd0; // red pixels in upper third
reg [14:0] red_mid    = 15'd0; // red pixels in middle third
reg [14:0] red_bot    = 15'd0; // red pixels in bottom third
reg VGA_VSYNC_P;
// check at clock edges (new pixel) and vysnc edges (new image)
always @ ( posedge CLK ) begin
	// vysnc low means end of image, check shape
		if ( !VGA_VSYNC_NEG ) begin
		// reset registers to prep for new image
		blue_count = 15'd0;
		blue_top   = 15'd0;
		blue_mid   = 15'd0;
		blue_bot   = 15'd0;
		red_count  = 15'd0;
		red_top    = 15'd0;
		red_mid    = 15'd0;
		red_bot    = 15'd0;
		RESULT = RESULT;
	end
	else begin
		// check if pixel in bounds
		if ( VGA_PIXEL_Y < `SCREEN_HEIGHT && VGA_PIXEL_X < `SCREEN_WIDTH ) begin
			// check color of pixel, red here
			if ( PIXEL_IN[7:6] > PIXEL_IN[4:3] && PIXEL_IN[7:6] > PIXEL_IN[1:0] ) begin
			// if ( PIXEL_IN[7:6] == 2'b11 && PIXEL_IN[4:3] != 2'b11 && PIXEL_IN[1:0] != 2'b11 ) begin
				red_count = red_count + 15'd1;
				// check if pixel in top
				if ( VGA_PIXEL_Y > 10'd20 && VGA_PIXEL_Y < 10'd56 && VGA_PIXEL_X < `X_HIGH && VGA_PIXEL_X > `X_LOW ) begin
					red_top = red_top + 15'd1;
				end
				//check if pixel in mid
				else if ( VGA_PIXEL_Y > 10'd55 && VGA_PIXEL_Y < 10'd90 && VGA_PIXEL_X < `X_HIGH && VGA_PIXEL_X > `X_LOW ) begin
					red_mid = red_mid + 15'd1;
				end
				// check if pixel in bot
				else if ( VGA_PIXEL_Y > 10'd89 && VGA_PIXEL_Y < 10'd121 && VGA_PIXEL_X < `X_HIGH && VGA_PIXEL_X > `X_LOW ) begin
					red_bot = red_bot + 15'd1;
				end
				else begin
					red_top = red_top;
					red_mid = red_mid;
					red_bot = red_bot;
				end
			end
			// check blue
			else if ( PIXEL_IN[1:0] < 2'b11 && PIXEL_IN[7:6] < 2'b01 && PIXEL_IN[7:6] < 2'b01 ) begin
			// else if ( PIXEL_IN[1:0] > PIXEL_IN[7:6]  && PIXEL_IN[1:0] > PIXEL_IN[4:3] ) begin
				blue_count = blue_count + 15'd1;
				// check if pixel in top
				if ( VGA_PIXEL_Y > 10'd20 && VGA_PIXEL_Y < 10'd56 && VGA_PIXEL_X < `X_HIGH && VGA_PIXEL_X > `X_LOW ) begin
					blue_top = blue_top + 15'd1;
				end
				//check if pixel in mid
				else if ( VGA_PIXEL_Y > 10'd55 && VGA_PIXEL_Y < 10'd90 && VGA_PIXEL_X < `X_HIGH && VGA_PIXEL_X > `X_LOW ) begin
					blue_mid = blue_mid + 15'd1;
				end
				// check if pixel in bot
				else if ( VGA_PIXEL_Y > 10'd89 && VGA_PIXEL_Y < 10'd121 && VGA_PIXEL_X < `X_HIGH && VGA_PIXEL_X > `X_LOW ) begin
					blue_bot = blue_bot + 15'd1;
				end
				else begin
					blue_top = blue_top;
					blue_mid = blue_mid;
					blue_bot = blue_bot;
				end
			end
			else begin
				red_count = red_count;
				blue_count = blue_count;
				red_top = red_top;
				red_mid = red_mid;
				red_bot = red_bot;
				blue_top = blue_top;
				blue_mid = blue_mid;
				blue_bot = blue_bot;
			end
		end
		else begin
			red_count = red_count;
			blue_count = blue_count;
			red_top = red_top;
			red_mid = red_mid;
			red_bot = red_bot;
			blue_top = blue_top;
			blue_mid = blue_mid;
			blue_bot = blue_bot;
		end
		// check for red
		if ( red_count > 5000 ) begin
			// triangle
			if ( red_top < red_mid && red_mid < red_bot ) begin
				RESULT = 3'b101;
			end
			// diamond
			else if ( red_top < red_mid && red_mid > red_bot ) begin
				RESULT = 3'b110;
			end
			// square
			else begin
				RESULT = 3'b111;
			end
		end
		// check for blue
		else if ( blue_count > `THRESHOLD ) begin
			// triangle
			if ( blue_top < blue_mid && blue_mid < blue_bot ) begin
				RESULT = 3'b001;
			end
			// diamond
			else if ( blue_top < blue_mid && blue_mid > blue_bot ) begin 
				RESULT = 3'b010;
			end
			// square
			else begin
				RESULT = 3'b011;
			end			
		end
		else begin
			RESULT = 3'b000;
		end		
	end
end	
	
endmodule
