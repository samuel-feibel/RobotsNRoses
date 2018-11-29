`define SCREEN_WIDTH 176
`define SCREEN_HEIGHT 144

module DE0_NANO(
 	CLOCK_50,
	GPIO_0_D,
	GPIO_1_D,
	KEY,
	LED
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

//////////// CLOCK //////////
input 		          		CLOCK_50;

//////////// GPIO_0, GPIO_0 connect to GPIO Default //////////
output 		    [33:0]		GPIO_0_D;
//////////// GPIO_0, GPIO_1 connect to GPIO Default //////////
input 		    [33:0]		GPIO_1_D;
input 		     [1:0]		KEY;
//// LEDs //// 
output [7:0] LED;
reg [7:0] LED;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332;

///// READ/WRITE ADDRESS /////
reg  [14:0] X_ADDR;
reg  [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg  [14:0] READ_ADDRESS; 

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X; // Horizontal Position of Next Pixel
wire [9:0]	VGA_PIXEL_Y; // Vertical Position of Next Pixel
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];
assign GPIO_0_D[0] = CLOCK_24_PLL;
///// I/O for Img Proc /////
wire [2:0] RESULT;
wire       HREF;  // posedge of HREF means new row of pixels. HREF will sit at 1 during row 
wire       VSYNC; // negedge of VSYNC means new image. VSYNC will sit at 0 during image
wire       PCLK;  // PCLK is clock signal that indicates new byte from camera
wire [7:0] CAM_IN;
reg        cam_counter = 0;

//assign inputs from camera
assign CAM_IN[7] = GPIO_1_D[33]; //D7 white wire
assign CAM_IN[6] = GPIO_1_D[32]; //D6 blue wire
assign CAM_IN[5] = GPIO_1_D[31]; //D5 green wire
assign CAM_IN[4] = GPIO_1_D[30]; //D4 purple wire
assign CAM_IN[3] = GPIO_1_D[29]; //D3 orange wire
assign CAM_IN[2] = GPIO_1_D[28]; //D2 yellow
assign CAM_IN[1] = GPIO_1_D[27]; //D1 green
assign CAM_IN[0] = GPIO_1_D[26]; //D3 blue
assign HREF = GPIO_1_D[23]; //grey
assign PCLK = GPIO_1_D[21]; //white
assign VSYNC = GPIO_1_D[19]; //puprple

//assign outputs to Arduino
assign GPIO_0_D[31] = RESULT[2];
assign GPIO_0_D[29] = RESULT[1];
assign GPIO_0_D[27] = RESULT[0];
// assign LEd outputs

/* WRITE ENABLE */
reg W_EN;

//=======================================================
// wires, regs
//=======================================================
wire CLOCK_24_PLL;
wire CLOCK_25_PLL;
wire CLOCK_50_PLL;
reg  HREF_P;
reg  VSYNC_P;
reg  CLOCK_50_PLL_P;
//=======================================================
// PLL subcomponent 
//=======================================================
Lab4PLL	Lab4PLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( CLOCK_24_PLL ),
	.c1 ( CLOCK_25_PLL ),
	.c2 ( CLOCK_50_PLL )
	);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(CLOCK_50_PLL),
	.clk_R(CLOCK_25_PLL), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(CLOCK_25_PLL),
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
	.CLK(CLOCK_25_PLL),
	.VGA_PIXEL_X(VGA_PIXEL_X),
	.VGA_PIXEL_Y(VGA_PIXEL_Y),
	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
	.RESULT(RESULT)
);

// 24 MHz clock
assign GPIO_0_D[33] = CLOCK_24_PLL;

/// DOWNSAMPLE AND UPDATE ADDRESSES ON PCLK ///

always @ ( posedge PCLK ) begin
	// @ negedge VYSNC reset X and Y
	if ( VSYNC && ~HREF && ~HREF_P ) begin
		X_ADDR = 15'd0;
		Y_ADDR = 15'd0;
		cam_counter  = 0;
		W_EN = 1'b0;
	end
	// @ posedge of HREF, reset X_ADDR and increment Y_ADDR
	else begin
		Y_ADDR = Y_ADDR;
		if (HREF) begin
			if (cam_counter == 0) begin
				// increment X_ADDR
				X_ADDR = X_ADDR + 15'd1;
				// take pixel data
				pixel_data_RGB332[1] = CAM_IN[4];
				pixel_data_RGB332[0] = CAM_IN[3];
				// set write enable
				W_EN = 1'b1;
			end
			else begin
				X_ADDR = X_ADDR;
				Y_ADDR = Y_ADDR;
				// take pixel data;
				pixel_data_RGB332[7] = CAM_IN[7];
				pixel_data_RGB332[6] = CAM_IN[6];
				pixel_data_RGB332[5] = CAM_IN[5];
				pixel_data_RGB332[4] = CAM_IN[2];
				pixel_data_RGB332[3] = CAM_IN[1];
				pixel_data_RGB332[2] = CAM_IN[0];

				// enable writing
				W_EN = 1'b0;		
			end	
		// change counter
		cam_counter = ~cam_counter;
		end
		else begin 
			W_EN = 1'b0;
			cam_counter = 0;
			if ( ~HREF && HREF_P ) begin
				X_ADDR = 15'd0;
				Y_ADDR = Y_ADDR + 15'd1;
				// cam_counter = 0;
			end	
			else begin
				X_ADDR = X_ADDR;
				Y_ADDR = Y_ADDR;
			end
		end
	end
	// increment X after a write
	HREF_P = HREF;
	VSYNC_P = VSYNC;
end

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
	
endmodule 