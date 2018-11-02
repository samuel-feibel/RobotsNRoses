module CAMERA( DATA, CLK, PCLK, VSYNC, HREF, W_EN, PIXEL_COLOR, X, Y);

input [7:0] DATA;
input CLK;
input PCLK;
input VSYNC;
input HREF;

output [7:0] PIXEL_COLOR;
output W_EN;
output X;
output Y;
reg W_EN = 1'b0;

reg CYCLE = 1'b0;
reg [7:0] TEMP;
reg [7:0] PIXEL_COLOR;
reg [9:0] X;
reg [9:0] Y; 
reg pclk_posedge;

	always @(posedge PCLK) begin
		if (~VSYNC) begin 
			if (HREF) begin 
				if (X < 9'd176) begin // 176 columns  
					X <= X + 1'b1; // inc X count
				end else begin 
					X <= 9'b0; // reset X count 
				end 	
				if (CYCLE == 1'b0) begin
					TEMP <= DATA;
					CYCLE <= 1'b1;
					W_EN <= 1'b0;
				end else begin
					PIXEL_COLOR[7:5] <= TEMP[7:3]/5'b00101;
					PIXEL_COLOR[4:2] <= {TEMP[2:0], DATA[7:5]}/6'b001001;
					PIXEL_COLOR[1:0] <= DATA[4:0]/5'b00101;
					W_EN <= 1'b1;
				end
				
			end
		end 
	end
	always @(posedge HREF) begin 
		if (~VSYNC) begin 
			if (Y < 9'd144) begin // 144 rows  
				Y <= Y + 1'b1; // inc Y count
			end else begin 
				Y <= 9'b0; // reset Y count
			end 				
		end 	
	end 

endmodule

