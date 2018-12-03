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
	X_ADDR,
	Y_ADDR,
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
input X_ADDR;
input Y_ADDR;

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
reg [15:0] countNULL; // 6000 for square (bigger area), 3000 for smaller triangle (smaller area)
reg lastsync = 1'b0; 
reg [7:0] lastY = 8'b0; //not used?



reg BLUE_T;
reg RED_T;
reg BLUE_D;
reg RED_D;
reg BLUE_S;
reg RED_S;

reg [3:0] toggle=4'b0000; //10 frames


always @(posedge CLK) begin 

    	case (PIXEL_IN)
		   8'b00000011:countBLUE = countBLUE + 16'd1; 
			8'b11100000:countRED = countRED + 16'd1; 
			8'b11111111:countNULL = countNULL + 16'd1; 
		endcase 
		
		

	if(VGA_VSYNC_NEG == 1'b0 && lastsync == 1'b1) begin //negedge VSYNC 
//		if(toggle==4'd10) begin
//			//reg_result[0]=1'b1;
//			
//		   r1=red_frame;
//			b1=blue_frame;
//			n1=null_frame;
//			
//			blue_frame=0;
//			red_frame=0;
//			null_frame=0;
//			
//			countBLUE = 16'b0; 
//			countRED = 16'b0; 
//			countNULL = 16'b0;
//			toggle=0;
//			reg_result[0]=1'b0;
//			reg_result[1]=1'b0;
			//end
			
//		else begin
//			toggle=toggle+1;
			//reg_result[0]=1'b0;
			/*----------------------- Testing lor ---------------------------*/
			if(countBLUE> countRED-15'd2000 ) begin 
//				blue_frame=blue_frame+1;
//				red_frame=red_frame;
//				null_frame=null_frame;
				reg_result[4] = 1'b1; //isBlue
				reg_result[3] = 1'b0;
				reg_result[5] = 1'b0;
			end 
			
			else if(countRED>countBLUE+15'd3000 && countRED>countNULL+15'd3000) begin  
//				red_frame =red_frame+1;
//				blue_frame =blue_frame;
//				null_frame =null_frame;
				reg_result[3] = 1'b1; //isRed
				reg_result[4] = 1'b0;
				reg_result[5] = 1'b0;
			end 
				
			else  begin
//				null_frame =null_frame+1;
//				blue_frame=blue_frame;
//				red_frame=red_frame;
				reg_result[5] = 1'b1; //null
				reg_result[3] = 1'b0;
				reg_result[4] = 1'b0;
			end
		//end 
		
//
//		if(r1 >3) begin  
//			reg_result[0]=1'b1;
//			
//			reg_result[3] = 1'b1; //isRed
//			reg_result[4] = 1'b0;
//			reg_result[5] = 1'b0;
//			reg_result[0]=1'b0;
//			end
//			
//		else if(b1 > 3 ) begin 
//			reg_result[1]=1'b1;
//			
//			reg_result[4] = 1'b1; //isBlue
//			reg_result[3] = 1'b0;
//			reg_result[5] = 1'b0;
//			end
//			
//		else  begin 
//			reg_result[5] = 1'b1; //null
//			reg_result[3] = 1'b0;
//			reg_result[4] = 1'b0;
//			end 
//	
		countBLUE = 16'b0; 
		countRED = 16'b0; 
		countNULL = 16'b0; 
	end 
	lastsync = VGA_VSYNC_NEG; 
	lastY = VGA_PIXEL_Y;

end

endmodule
