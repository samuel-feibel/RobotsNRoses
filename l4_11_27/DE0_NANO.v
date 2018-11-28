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
wire [1:0] SHAPE;
///// CAMERA INPUTS/OUTPUTS /////
wire [2:0] R;
wire [2:0] G;
wire [1:0] B;
wire pclk;
wire HREF;
wire vsync;
wire RESULT2, RESULT1, RESULT0;
assign pclk = GPIO_1_D[21];
assign R[2] = GPIO_1_D[27]; //D7 brown wire
assign R[1] = GPIO_1_D[28]; //D6 orange wire
assign R[0] = GPIO_1_D[29]; //D5 blue wire
assign G[2] = GPIO_1_D[30]; //D2 green wire
assign G[1] = GPIO_1_D[32]; //D1 white wire
assign G[0] = GPIO_1_D[33]; //D0 grey wire
assign B[1] = GPIO_1_D[31]; //D3 purple wire
assign B[0] = GPIO_1_D[30]; //D2 green wire
assign HREF = GPIO_1_D[23];
assign vsync = GPIO_1_D[19];




///// I/O for Img Proc /////
assign GPIO_0_D[29] = RESULT2;
assign GPIO_0_D[27] = RESULT1;
assign GPIO_0_D[25] = RESULT0;

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
	.clk_W(clk2), 
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
	.PIXEL_IN(pixel_data_RGB332),
	.CLK(pclk),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VSYNC(vsync),
	.RESULT2(RESULT2),
	.RESULT1(RESULT1),
	.RESULT0(RESULT0),
	.HREF(HREF),
	.SHAPE(SHAPE)
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
	.R(R),
	.G(G),
	.B(B),
	.PCLK(pclk),
	.VSYNC(vsync),
	.HREF(HREF),
	.W_EN(W_EN),
	.PIXEL_COLOR(pixel_data_RGB332), 
	.X(PIXEL_X),
	.Y(PIXEL_Y),
	.SHAPE(SHAPE)
);

endmodule 