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
localparam RED = 8'b111_000_00;
localparam GREEN = 8'b000_111_00;
localparam BLUE = 8'b000_000_11;

//=======================================================
//  PORT declarations
//=======================================================

//////////// CLOCK - DON'T NEED TO CHANGE THIS //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:18]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
wire [7:0]	pixel_data_RGB332;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
reg [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 

//assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [9:0]	PIXEL_X;
wire [9:0]	PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;
reg			VGA_WRITE_MEM_EN;
assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];
assign GPIO_0_D[0] = clk0;

///// CAMERA INPUTS/OUTPUTS /////
wire [33:26] CAMDATA;



///// I/O for Img Proc /////
wire [8:0] RESULT;

/* WRITE ENABLE */
wire W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////
wire clk0;
wire clk1;
wire clk2;
reg newx;
reg newy;

///////* INSTANTIATE YOUR PLL HERE *///////
robots_n_roses_3400	robots_n_roses_3400_inst (
	.inclk0 (CLOCK_50),
	.c0 ( clk0 ),
	.c1 ( clk1 ),
	.c2 ( clk2 )
	);
	
///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(GPIO_1_D[22]), // PCLK from camera 
	.clk_R(clk1), // same as VGA read clock? used to be clk0// DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(clk1), // same as mem read clk?
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : BLUE),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

///////* Image Processor *///////
IMAGE_PROCESSOR proc(
	.PIXEL_IN(MEM_OUTPUT),
	.CLK(clk1),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);


///////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
		READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
		if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_READ_MEM_EN = 1'b0;
		end
		else begin
				VGA_READ_MEM_EN = 1'b1;
		end
end

///////* Update Write Address */////// // x and y from camera 
always @ (PIXEL_X, PIXEL_Y) begin
		WRITE_ADDRESS = (PIXEL_X + PIXEL_Y*`SCREEN_WIDTH);
		if(PIXEL_X>(`SCREEN_WIDTH-1) || PIXEL_Y>(`SCREEN_HEIGHT-1))begin
				VGA_WRITE_MEM_EN = 1'b0;
		end
		else begin
				VGA_WRITE_MEM_EN = 1'b1;
		end
end

CAMERA cam(
	.DATA(CAMDATA),
	.CLK(clk0),
	.PCLK(GPIO_1_D[22]), // same as mem write clock
	.VSYNC(GPIO_1_D[20]),
	.HREF(GPIO_1_D[18]),
	.W_EN(W_EN),
	.PIXEL_COLOR(pixel_data_RGB332), 
	.X(PIXEL_X),
	.Y(PIXEL_Y),
);
	
	
/*
always @(*) begin
	W_EN = 1;
	if(VGA_PIXEL_Y < 18) begin
		pixel_data_RGB332 = 8'b111_000_00;
	end
	else if (VGA_PIXEL_Y < 36) begin
		pixel_data_RGB332 = 8'b111_011_00;
	end
	else if (VGA_PIXEL_Y < 54) begin
		pixel_data_RGB332 = 8'b111_111_00;
	end
	else if (VGA_PIXEL_Y < 72) begin
		pixel_data_RGB332 = 8'b000_111_00;
	end
	else if (VGA_PIXEL_Y < 90) begin
		pixel_data_RGB332 = 8'b000_000_11;
	end
	else if (VGA_PIXEL_Y < 118) begin
		pixel_data_RGB332 = 8'b011_000_10;
	end
	else if (VGA_PIXEL_Y < 146) begin
		pixel_data_RGB332 = 8'b111_000_11;
	end
	else begin
		pixel_data_RGB332 = 8'b111_111_11;
	end
end
*/

/*
always @(*) begin
	W_EN = 1;
	if(VGA_PIXEL_X < 18) begin
		pixel_data_RGB332 = 8'b111_111_11;
	end

	else if (VGA_PIXEL_X > 18)begin
		pixel_data_RGB332 = 8'b111_000_00;
	end
	else begin
		pixel_data_RGB332 = 8'b000_000_00;
	end
	
end
*/



endmodule 
