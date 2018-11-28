/*module CAMERA(DATA, PCLK, VSYNC, HREF, W_EN, PIXEL_COLOR, X, Y, SHAPE);

input [7:0] DATA;
input PCLK;
input VSYNC;
input HREF;
output [1:0] SHAPE;
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
reg seenColor = 1'b0;
integer r;
integer g;
integer b;
integer rs;
integer bs;
integer yiq;
reg [1:0] SHAPE = 2'b0;
reg [9:0] topX = 10'b0;
reg [9:0] mid1X = 10'b0;
reg [9:0] mid2X = 10'b0;
reg [9:0] botX = 10'b0;

//ALWAYS MAKE SURE CAMERA IS ORIENTED THE RIGHT WAY OR ELSE IT WILL AFFECT SHAPE DETECTION
always @(posedge PCLK) begin
	if(VSYNC & ~last_sync) begin // posedge VSYNC (end of frame)
		Y = 10'b0;	
		X = 10'b0;
		CYCLE = 1'b0;
		
		//SHAPE DETECTION
		if((topX + mid1X + mid2X + botX) > 10'd700) begin
			if((topX+mid1X) > (mid2X + botX)) begin SHAPE = 2'b01; end //TRIANGLE 
			else if((mid1X + mid2X) > (topX + mid1X) &&  (mid1X + mid2X) > (botX + mid2X)) begin SHAPE = 2'b10; end //DIAMOND
			else begin SHAPE = 2'b11; end //SQUARE
		end
		else begin
			SHAPE = 2'b0;
		end
			
	end 
	else if (~HREF & last_href) begin // negedge HREF
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
			end 
			else begin				
				TEMP[15:8] = DATA; // 444			
				W_EN = 1'b1;
				CYCLE = 1'b0;
				X = X + 1'b1;
				
				// COLOR CORRECTION //
				/*r = (TEMP[15:12]/4'b1111)*255;
				g = (TEMP[7:4]/4'b1111)*255;
				b = (TEMP[3:0]/4'b1111)*255;
				yiq = ((r*299)+(g*587)+(b*114))/1000;		
				if(yiq > 140) begin 
					rs = r/4;
					bs = b/4;
				end
				else if (yiq > 100) begin 
					rs = r/2;
					bs = b/2;
				end
				else if (yiq > 60) begin
					rs = (r*3)/4;
					bs = (b*3)/4;
				end
				else begin 
					rs = TEMP[15:13];
					bs = TEMP[3:2];
				end
				PIXEL_COLOR = {TEMP[15:13], 3'b0, TEMP[3:2]};
				end
				//PIXEL_COLOR = {rs, TEMP[11:9], bs};	//no green, compensated red and blue
				
				// LOOK FOR EXCLUSIVES & STORE EDGE X//
				//if(rs < 20 && bs < 20) begin
				/*if (TEMP[15:12] < 4'b0100 && TEMP[7:4] < 4'b0100) begin //black
					if(~seenColor && X > 20 && Y > 23 && Y < 151) begin
						PIXEL_COLOR = 8'b00011100;
						seenColor = 1'b1;
						if(Y >= 95) begin botX = botX + X; end
						else if (Y >= 71) begin mid2X = mid2X + X; end
						else if (Y >= 47) begin mid1X = mid1X + X; end
						else if(Y > 23) begin topX = topX + X; end
						else begin
							botX = botX;
							mid2X = mid2X;
							mid1X = mid1X;
							topX = topX;
						end
					end
					else begin
						PIXEL_COLOR = 8'b00000011;
						seenColor = seenColor;
					end
				end
				//else if(rs > 50 && bs < 20) begin
				//else if (PIXEL_COLOR[7:5] > 3'b000 && PIXEL_COLOR[4:2] < 3'b001 && PIXEL_COLOR[1:0] < 2'b10) begin
				else if(TEMP[15:12] > 4'b1110) begin
					if(~seenColor && X > 20 && Y > 23 && Y < 151) begin
						PIXEL_COLOR = 8'b00011100;
						seenColor = 1'b1;
						if(Y >= 95) begin botX = botX + X; end
						else if (Y >= 71) begin mid2X = mid2X + X; end
						else if (Y >= 47) begin mid1X = mid1X + X; end
						else if(Y > 23) begin topX = topX + X; end
						else begin
							botX = botX;
							mid2X = mid2X;
							mid1X = mid1X;
							topX = topX;
						end
					end
					else begin
						PIXEL_COLOR = 8'b11100000;
						seenColor = seenColor;
					end
				end
				else begin
					botX = botX;
					mid2X = mid2X;
					mid1X = mid1X;
					topX = topX;
					seenColor = seenColor;
					PIXEL_COLOR = 8'b0;
				end
			end
		end
		else begin //RESET X & SEENCOLOR
			X = 10'b0;
			seenColor = 1'b0;
		end 
	end
	last_sync = VSYNC;
	last_href = HREF;
end

endmodule*/