module CAMERA(D, PCLK, VSYNC, HREF, W_EN, PIXEL_COLOR, X, Y, SHAPE);
input [7:0] D;
//input [7:0] DATA;
input PCLK;
input VSYNC;
input HREF;

output [7:0] PIXEL_COLOR;
output W_EN;
output X;
output Y;
output [1:0] SHAPE;
reg W_EN = 1'b0;
reg CYCLE = 1'b0;
reg [15:0] TEMP;
reg [7:0] PIXEL_COLOR;
reg [9:0] X = 10'b0;
reg [9:0] Y = 10'b0; 
reg last_sync = 0;
reg last_href = 0;
reg sample = 1'b0;
reg [7:0] colorCount0 = 8'b0;
reg [7:0] colorCount = 8'b0;
reg [9:0] threshold = 10'd100;
reg colorGreen = 1'b0;
reg [1:0] SHAPE;
always @(posedge PCLK) begin
	if(VSYNC & ~last_sync) begin // posedge vsync
		Y = 10'b0;	
		X = 10'b0;
		CYCLE = 1'b0;
	end 
	else if (~HREF & last_href) begin // negedge href
		Y = Y + 10'b1;  
		X = 10'b0;
		CYCLE = 1'b0;
		if(Y == 10'd72) begin sample = 1'b1; end
		else begin sample = 1'b0; end
		if(colorCount > threshold & Y == 10'd73) begin colorGreen = 1'b1; end
		else begin colorGreen = 1'b0; end
	end
	else begin
		Y = Y; 
		if (HREF) begin			
			if (CYCLE == 1'b0) begin
				CYCLE = 1'b1;
				W_EN = 1'b0;
				X = X;
				PIXEL_COLOR[0] = D[3];
				PIXEL_COLOR[1] = D[4];
			end 
			else begin				
				PIXEL_COLOR[2] = D[0];
				PIXEL_COLOR[3] = D[1];
				PIXEL_COLOR[4] = D[2];
				PIXEL_COLOR[5] = D[5];
				PIXEL_COLOR[6] = D[6];
				PIXEL_COLOR[7] = D[7];
				W_EN = 1'b1;
				CYCLE = 1'b0;
				X = X + 1'b1;
				
				// LOOK FOR EXCLUSIVES //
				/*if(colorGreen) begin PIXEL_COLOR = 8'b00011100; end
				else if (PIXEL_COLOR == 8'b0) begin //black
					if(sample) begin colorCount = colorCount + 8'b1; end
					else begin colorCount = colorCount; end
					PIXEL_COLOR = 8'b00000011;
				end
				else if (PIXEL_COLOR[7:5] > 3'b000 && PIXEL_COLOR[4:2] < 3'b001 && PIXEL_COLOR[1:0] < 2'b10) begin
					if(sample) begin colorCount = colorCount + 8'b1; end
					else begin colorCount = colorCount; end
					PIXEL_COLOR = 8'b11100000;
				end
				else begin
					colorCount = colorCount;
					PIXEL_COLOR = 8'b0;
				end*/

	
			end
			
		end else begin
			X = 10'b0;
		end 
	end
	last_sync = VSYNC;
	last_href = HREF;
end
endmodule
