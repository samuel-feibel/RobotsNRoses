module CAMERA(DATA, PCLK, VSYNC, HREF, W_EN, PIXEL_COLOR, X, Y);

input [7:0] DATA;
input PCLK;
input VSYNC;
input HREF;

output [7:0] PIXEL_COLOR;
output W_EN;
output X;
output Y;
reg W_EN = 1'b0;
reg CYCLE = 1'b0;
reg [15:0] TEMP;
reg [7:0] PIXEL_COLOR;
reg [9:0] X = 10'b0;
reg [9:0] Y = 10'b0; 

reg last_sync = 0;
reg last_href = 0;

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
	end
	else begin
		Y = Y; 
		if (HREF) begin			
			if (CYCLE == 1'b0) begin
				TEMP[7:0] = DATA; // 444
				CYCLE = 1'b1;
				W_EN = 1'b0;
				X = X;
				PIXEL_COLOR[7:5] = DATA[3:1];
			end 
			else begin				
				TEMP[15:8] = DATA; // 444
				PIXEL_COLOR = {TEMP[15:13], TEMP[11:9], TEMP[3:2]};				
				W_EN = 1'b1;
				CYCLE = 1'b0;
				X = X + 1'b1;			
			end
			
		end else begin
			X = 10'b0;
		end 
	end
	last_sync = VSYNC;
	last_href = HREF;
end

endmodule
