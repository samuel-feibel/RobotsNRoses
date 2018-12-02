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

output [5:0] RESULT;
reg [5:0] reg_result;
assign RESULT = reg_result;

reg [3:0] red_frame;
reg [3:0] blue_frame;
reg [3:0] null_frame;

reg b1;
reg r1;
reg n1;

reg [15:0] countBLUE; 
reg [15:0] countRED; 
reg [15:0] countNULL; 
reg [2:0] R_THRESHOLD = 3'b100;
reg [2:0] B_THRESHOLD = 3'b100;
reg [15:0] FRAME_THRESHOLD = 16'd15000;
reg [15:0] R_CNT_THRESHOLD = 16'd6000; 
reg [15:0] B_CNT_THRESHOLD = 16'd6000; 	// 6000 for square (bigger area), 3000 for smaller triangle (smaller area)
reg lastsync = 1'b0; 
reg [7:0] lastY = 8'b0; //not used?

// Blue edge Points
reg [7:0] BLUE_LINE_0 = 8'b0;
reg [7:0] BLUE_LINE_1 = 8'b0;
reg [7:0] BLUE_LINE_2 = 8'b0;
reg [7:0] BLUE_CNT_LINE = 8'b0;
// Red edge Points
reg [7:0] RED_LINE_0 = 8'b0;
reg [7:0] RED_LINE_1 = 8'b0;
reg [7:0] RED_LINE_2 = 8'b0;
reg [7:0] RED_CNT_LINE = 8'b0;

reg BLUE_T;
reg RED_T;
reg BLUE_D;
reg RED_D;
reg BLUE_S;
reg RED_S;

reg [3:0] toggle; //10 frames

//always @(negedge VGA_VSYNC_NEG) begin
//	if (toggle==4'd10) begin
//		toggle=0;
//		
//	end 
//	else begin
//		toggle=toggle+1;
//	end
//end 


always @(posedge CLK) begin 

    	
		if (PIXEL_IN== 8'b11111111)begin countNULL=countNULL+1; end
		
		else begin 
			
			if(PIXEL_IN[7:5] > 3'b000   ) begin 
				countRED = countRED + 16'd1; 
			
					
			end 
			
			// Usually, we have brighter red, so the red value threshold is higher.
			 else if(PIXEL_IN[1:0] > 3'b000 ) begin 
				
				countBLUE = countBLUE + 16'd1; 
				
			end 
		end
			
			
		
	

	if(VGA_VSYNC_NEG == 1'b0 && lastsync == 1'b1) begin //negedge VSYNC 
		if(toggle==4'd10) begin
		   r1<=red_frame;
			b1<=blue_frame;
			n1<=null_frame;
			blue_frame<=0;
			red_frame<=0;
			null_frame<=0;
			toggle<=0;
			end
			
		else begin
			toggle<=toggle+1;
			/*----------------------- Testing lor ---------------------------*/
			if(countRED > countBLUE  && countRED>16'd5000) begin  
				red_frame<=red_frame+1;
				blue_frame<=blue_frame;
				null_frame<=null_frame;
//				reg_result[3] <= 1'b1; //isRed
//			   reg_result[4] <= 1'b0;
//			   reg_result[5] <= 1'b0;
			
			end 
				
			else if(countBLUE > countRED && countBLUE>16'd5000) begin 
				blue_frame<=blue_frame+1;
				red_frame<=red_frame;
				null_frame<=null_frame;
//		   	reg_result[4] <= 1'b1; //isBlue
//		     	reg_result[3] <= 1'b0;
//			   reg_result[5] <= 1'b0;
			end 
			
		
			else  begin //if(countNULL > countBLUE +16'd1000 && countNULL > countRED + 16'd1000)
				null_frame<=null_frame+1;
				blue_frame<=blue_frame;
				red_frame<=red_frame;
			end
		end 
		

		if(r1 > b1 && r1>=n1) begin  
			reg_result[3] <= 1'b1; //isRed
			reg_result[4] <= 1'b0;
			reg_result[5] <= 1'b0;
			end
			
		else if(b1 > r1 && b1>n1) begin 
			reg_result[4] <= 1'b1; //isBlue
			reg_result[3] <= 1'b0;
			reg_result[5] <= 1'b0;
			end
			
		else begin 
			reg_result[5] <= 1'b1; //null
			reg_result[3] <= 1'b0;
			reg_result[4] <= 1'b0;
			end 
		
		countBLUE = 16'b0; 
		countRED = 16'b0; 
		countNULL = 16'b0; 
	
	end 
	lastsync = VGA_VSYNC_NEG; 
	lastY = VGA_PIXEL_Y;

end

endmodule
