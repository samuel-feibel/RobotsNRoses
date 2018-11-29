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
input 		    [33:0]		GPIO_1_D;
input 		     [1:0]		KEY;

///// PIXEL DATA /////
reg [7:0]	pixel_data_RGB332 = 8'd0;

///// READ/WRITE ADDRESS /////
reg [14:0] X_ADDR;
reg [14:0] Y_ADDR;
wire [14:0] WRITE_ADDRESS;
reg [14:0] READ_ADDRESS; 
reg hflag = 0; //horizontal flag

assign WRITE_ADDRESS = X_ADDR + Y_ADDR*(`SCREEN_WIDTH);

//wire			C0_to_GPIO_02;
wire			PLL_24;
wire			PLL_50;
wire			PLL_25;

wire			PCLK;
wire			HREF;
wire			VSYNC;

// Camera stuff
assign HREF = GPIO_1_D[29];
assign VSYNC = GPIO_1_D[30];
assign PCLK = GPIO_1_D[32];

reg  [7:0]  color_temp;
reg  [2:0]	temp_counter;


///// VGA INPUTS/OUTPUTS /////
wire 			VGA_RESET;
wire [7:0]	VGA_COLOR_IN;
wire [9:0]	VGA_PIXEL_X;
wire [9:0]	VGA_PIXEL_Y;
wire [7:0]	MEM_OUTPUT;
wire			VGA_VSYNC_NEG;
wire			VGA_HSYNC_NEG;
reg			VGA_READ_MEM_EN;

assign GPIO_0_D[5] = VGA_VSYNC_NEG;
assign VGA_RESET = ~KEY[0];

assign GPIO_0_D[33] = PLL_24;

///// Variables for Img Proc /////
reg [14:0] blue_px;
reg [14:0] red_px;
reg[14:0] green_px;

wire [1:0] b_blue_threshold;
wire [2:0] b_red_threshold;
wire [2:0] b_green_threshold;

wire [1:0] r_blue_threshold;
wire [2:0] r_red_threshold;
wire [2:0] r_green_threshold;

wire [14:0] px_threshold;

reg [2:0] shape_n_color;

reg [8:0] RESULT;

reg [7:0] background = 8'b0;

assign b_blue_threshold = 2'b01;
assign b_red_threshold = 3'b010;
assign b_green_threshold = 3'b010;

assign r_blue_threshold = 2'b10;
assign r_red_threshold = 3'b101;
assign r_green_threshold = 3'b100;

assign px_threshold = 15'd750;

// I/O for img processor
assign GPIO_0_D[28] = shape_n_color[2];
assign GPIO_0_D[30] = shape_n_color[1];
assign GPIO_0_D[32] = shape_n_color[0];

/* WRITE ENABLE */
reg W_EN;

///////* CREATE ANY LOCAL WIRES YOU NEED FOR YOUR PLL *///////

///////* INSTANTIATE YOUR PLL HERE *///////
sweetPLL	sweetPLL_inst (
	.inclk0 ( CLOCK_50 ),
	.c0 ( PLL_24 ),
	.c1 ( PLL_25 ),
	.c2 ( PLL_50 )
	);

///////* M9K Module *///////
Dual_Port_RAM_M9K mem(
	.input_data(pixel_data_RGB332),
	.w_addr(WRITE_ADDRESS),
	.r_addr(READ_ADDRESS),
	.w_en(W_EN),
	.clk_W(PLL_50),
	.clk_R(PLL_25), // DO WE NEED TO READ SLOWER THAN WRITE??
	.output_data(MEM_OUTPUT)
);

///////* VGA Module *///////
VGA_DRIVER driver (
	.RESET(VGA_RESET),
	.CLOCK(PLL_25),
	.PIXEL_COLOR_IN(VGA_READ_MEM_EN ? MEM_OUTPUT : background),
	.PIXEL_X(VGA_PIXEL_X),
	.PIXEL_Y(VGA_PIXEL_Y),
	.PIXEL_COLOR_OUT({GPIO_0_D[9],GPIO_0_D[11],GPIO_0_D[13],GPIO_0_D[15],GPIO_0_D[17],GPIO_0_D[19],GPIO_0_D[21],GPIO_0_D[23]}),
   .H_SYNC_NEG(GPIO_0_D[7]),
   .V_SYNC_NEG(VGA_VSYNC_NEG)
);

/////////* Image Processor *///////
//IMAGE_PROCESSOR proc(
//	.PIXEL_IN(MEM_OUTPUT),
//	.CLK(PLL_25),
//	.VGA_PIXEL_X(VGA_PIXEL_X),
//	.VGA_PIXEL_Y(VGA_PIXEL_Y),
//	.VGA_VSYNC_NEG(VGA_VSYNC_NEG),
//	.RESULT(RESULT)
//);


/////* Update Read Address *///////
always @ (VGA_PIXEL_X, VGA_PIXEL_Y) begin
	READ_ADDRESS = (VGA_PIXEL_X + VGA_PIXEL_Y*`SCREEN_WIDTH);
	if(VGA_PIXEL_X>(`SCREEN_WIDTH-1) || VGA_PIXEL_Y>(`SCREEN_HEIGHT-1))begin
			VGA_READ_MEM_EN = 1'b0;
	end
	else begin
			VGA_READ_MEM_EN = 1'b1;
	end
end

///* Camera Reading *///////
reg flag = 1'b0;
reg VSYNCprev;
reg HREFprev;

always @ (posedge PCLK) begin

	if(VSYNC && !VSYNCprev) begin
		Y_ADDR <= 0;
		X_ADDR <= 0;
		flag <= 0;
		W_EN <= 0;
		pixel_data_RGB332[7:0] <= 0;
		
		// Image processor
		if ((blue_px > red_px) && (blue_px > px_threshold)) begin
			background <= BLUE;
			shape_n_color <= 3'b101;
		end
		else if ((blue_px < red_px) && (red_px > px_threshold)) begin
			background <= RED;
			shape_n_color <= 3'b001;
		end
		else begin
			background <= GREEN;
			shape_n_color <= 3'b000;
		end
		red_px <= 0;
		blue_px <= 0;
		
	end
	
	else if (!HREF && HREFprev) begin
		X_ADDR <= 0;
		Y_ADDR <= Y_ADDR + 1;
		flag <= 0;
		W_EN <= 0;
		pixel_data_RGB332[7:0] <= 0;
	end
	else begin
		Y_ADDR = Y_ADDR;			
		if (!flag && HREF) begin
			flag = 1'b1;
			W_EN = 0;
			X_ADDR = X_ADDR;
			pixel_data_RGB332[1:0] = {GPIO_1_D[12], GPIO_1_D[11]}; //something is wrong with the cycles, blue being output before red/green, but this code works
		end
		else if (flag && HREF) begin
			pixel_data_RGB332[7:5] = {GPIO_1_D[15], GPIO_1_D[14], GPIO_1_D[13]};
			pixel_data_RGB332[4:2] = {GPIO_1_D[10], GPIO_1_D[9], GPIO_1_D[8]};
			flag = 1'b0;
			W_EN = 1;
			X_ADDR = X_ADDR + 1'b1;	
			
			// Pixel color counters
			if ((pixel_data_RGB332[1:0] >= b_blue_threshold) && (pixel_data_RGB332[7:5] < b_red_threshold) && (pixel_data_RGB332[4:2] < b_green_threshold)) begin
				blue_px <= blue_px + 1;
			end
			else if ((pixel_data_RGB332[1:0] < r_blue_threshold) && (pixel_data_RGB332[7:5] >= r_red_threshold) && (pixel_data_RGB332[4:2] < r_green_threshold)) begin
				red_px <= red_px + 1;
			end
			
		end
		else begin
			X_ADDR = 0;
		end
	end
	VSYNCprev = VSYNC;
	HREFprev = HREF;	
end

endmodule 
