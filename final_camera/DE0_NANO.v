`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

///////* DON'T CHANGE THIS PART *///////
module DE0_NANO(
	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY
);

//=======================================================
//  PARAMETER declarations
//=======================================================

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:20]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 ;
///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR = 0;
reg [14:0] Y_ADDR = 0;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;
reg  [7:0]  PIXEL_COLOR;
reg [2:0] TREASURE;
assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

///// I/O for Img Proc /////
wire [5:0] RESULT;

/* WRITE ENABLE */
reg W_EN;


///////* CREATE ANY_ADDRLOCAL WIRES YOU NEED FOR YOUR PLL *///////

reg [7:0] BACKGROUND;

///////* INSTANTIATE YOUR PLL HERE *///////
///////* INSTANTIATE YOUR PLL HERE *///////
sweetPLL	sweetPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( c0_sig ), // 24 MHz
	.c1 ( c1_sig ), // 25 MHz
	.c2 ( c2_sig ) // 50 MHz
	);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(c2_sig),
	.clk_R(c1_sig),
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(c1_sig),
	//.PIXEL_COLOR_IN(COLOR),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BACKGROUND),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(c1_sig),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.X_ADDR(X_ADDR),
	.Y_ADDR(Y_ADDR),
	.RESULT(RESULT)
);


assign GPIO_0_D[29] = TREASURE[0];
assign GPIO_0_D[31] = TREASURE[1];
assign GPIO_0_D[33] = TREASURE[2];




// Downsampling variables
assign D0 = GPIO_1_D[20];
assign D1 = GPIO_1_D[21];

assign D2 = GPIO_1_D[22];
assign D3 = GPIO_1_D[23];
assign D4 = GPIO_1_D[24];
assign D5 = GPIO_1_D[25];
assign D6 = GPIO_1_D[26];

assign D7 = GPIO_1_D[27];
assign PCLK = GPIO_1_D[28];
assign HREF = GPIO_1_D[29];
assign VSYNC = GPIO_1_D[30];

assign GPIO_0_D[0] = c0_sig;



always @ (VGA_PIXEL_Y, VGA_PIXEL_X) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end

reg flag = 0;
reg reach2byte = 0; //not in use?
assign GPIO_0_D[1] = flag;
reg is_image_started = 1'b1;
reg is_new_row = 1'b0;
reg is_new_byte = 1'b1;

reg [3:0] RED = 4'b0;
reg [3:0] BLU = 4'b0;
reg [3:0] GRE = 4'b0;
reg prev_href = 1'b0;
reg prev_vsync = 1'b0;
reg seenColor;

reg [15:0] top = 16'b0;
reg [15:0] md1 = 16'b0;
reg [15:0] md2 = 16'b0;
reg [15:0] bot = 16'b0;
reg [14:0] B_COUNT = 15'b0;
reg [14:0] R_COUNT = 15'b0;

always @ (posedge PCLK) begin
   // Handling when an image frame starts or ends
   if (VSYNC && ~prev_vsync) begin // Image TX on falling edge started
		  X_ADDR = 0;
	     Y_ADDR = 0;
		  flag = 1'b0; //reset flag
		  
		  if(R_COUNT > 15'd1400 || B_COUNT > 15'd1400) begin //if there is a shape
				if (((top>md1) && (bot>md2)) && ((md2-bot) > 1'b0)) begin //diamond
					if(B_COUNT > 15'd1400) begin TREASURE = 3'b100; end //blue diamond
					else begin TREASURE = 3'b101; end //red diamond
				end	
				else if((top>md1) && (md1>md2) && (md2>bot) && ((top-bot) > 10'd100)) begin //triangle
					if(B_COUNT > 15'd1600) begin TREASURE = 3'b010; end //blue triangle
					else begin TREASURE = 3'b011; end //red triangle
				end 
				else begin
					if(B_COUNT > 15'd2600) begin TREASURE = 3'b110; end //blue square
					else begin TREASURE = 3'b111; end //red square
				end
		  end
		  else begin
			TREASURE = 3'b000;
		  end
	end
	
	else begin
	
   	if (~HREF & prev_href) begin // row TX ends on falling edge, must be a new row (HREF == 1'b0)
		     X_ADDR = 0;
			  seenColor = 1'b0;
		     Y_ADDR = Y_ADDR + 1; 
	   end
	   else if (HREF) begin  // new row TX on rising edge
			 if(X_ADDR == 15'b0 && Y_ADDR == 15'b0) begin //reset all
				  top = 16'b0;
				  md1 = 16'b0;
				  md2 = 16'b0;
				  bot = 16'b0;
				  B_COUNT = 15'B0;
				  R_COUNT = 15'B0;
			 end
			 if (~flag) begin
			     W_EN = 1'b0;
				  flag = 1'b1;
				  X_ADDR = X_ADDR;
				  Y_ADDR = Y_ADDR;
				  BLU = {D3,D2, D1, D0};
				  pixel_data_RGB332[1:0] = {D3, D2};
			 end
			 else begin
				  flag = 1'b0;	  
				  RED = {D3, D2, D1, D0};
				  GRE = {D7, D6, D5, D4};	
				  pixel_data_RGB332[7:2] = {D3, D2, D1, D7, D6, D5};
				  if(BLU > 4'b0000 && RED < 4'b0110 && GRE < 4'b0110 && X_ADDR > 15'd20 && X_ADDR < 15'd155 && Y_ADDR > 15'd15 && Y_ADDR < 15'd127) begin 
						if(~seenColor) begin //if this is the first pixel of color in the row
							if(Y_ADDR > 15'd110) begin
								bot = bot + X_ADDR; 
								md2 = md2;
								md1 = md1;
								top = top;
							end
							else if (Y_ADDR > 15'd86) begin 
								bot = bot;
								md2 = md2 + X_ADDR; 
								md1 = md1;
								top = top;
							end
							else if (Y_ADDR > 15'd62) begin 
								bot = bot;
								md2 = md2; 
								md1 = md1 + X_ADDR; 
								top = top;
							end
							else if(Y_ADDR > 15'd38) begin 
								bot = bot;
								md2 = md2;
								md1 = md1;
								top = top + X_ADDR; 
							end
							else begin
								bot = bot;
								md2 = md2;
								md1 = md1; 
								top = top;
							end
							seenColor = 1'b1;
							pixel_data_RGB332 = 8'b00011100;
						end
						else begin
							seenColor = seenColor;
							pixel_data_RGB332[7:0] = 8'b00000011; 
						end
						B_COUNT = B_COUNT + 1'b1;
				  end
				  
				  else if(RED > 4'b0000 && GRE < 4'b0111 && BLU < 4'b0111 && X_ADDR > 15'd20 && X_ADDR < 15'd155 && Y_ADDR > 15'd15 && Y_ADDR < 15'd134) begin
						if(~seenColor) begin
							if(Y_ADDR > 15'd110) begin
								bot = bot + X_ADDR; 
								md2 = md2;
								md1 = md1;
								top = top;
							end
							else if (Y_ADDR > 15'd86) begin 
								bot = bot;
								md2 = md2 + X_ADDR; 
								md1 = md1;
								top = top;
							end
							else if (Y_ADDR > 15'd62) begin 
								bot = bot;
								md2 = md2; 
								md1 = md1 + X_ADDR; 
								top = top;
							end
							else if(Y_ADDR > 15'd38) begin 
								bot = bot;
								md2 = md2;
								md1 = md1;
								top = top + X_ADDR; 
							end
							else begin
								bot = bot;
								md2 = md2;
								md1 = md1; 
								top = top;
							end
							seenColor = 1'b1;
							pixel_data_RGB332 = 8'b00011100;
						end
						else begin
							seenColor = seenColor;
							pixel_data_RGB332[7:0] = 8'b11100000;
						end	
						R_COUNT = R_COUNT + 1'b1;
				  end 
				  
				  else begin 
						pixel_data_RGB332[7:0] = 8'b11111111;
						seenColor = seenColor;
				  end
				  
				  X_ADDR = X_ADDR + 1;
				  Y_ADDR = Y_ADDR;
				  W_EN = 1'b1;
			end 
		 end
	end
	X_ADDR = X_ADDR;
	Y_ADDR = Y_ADDR;
	prev_href = HREF;
	prev_vsync = VSYNC;
	
end

endmodule 
