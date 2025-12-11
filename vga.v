// A simple system-on-a-chip (SoC) for the MiST
// (c) 2015 Till Harbaum

// VGA controller generating 160x100 pixles. The VGA mode ised is 640x400
// combining every 4 row and column

// http://tinyvga.com/vga-timing/640x400@70Hz

`define X_BITS 8
`define Y_BITS 7
`define COLOR_BITS 24
`define WHITE 	24'b111111111111111111111111
`define RED 	24'b111111110000000000000000
`define GREEN	24'b000000001111111100000000
`define BLUE	24'b000000000000000011111111

module vga (
   // pixel clock
   input  pclk,

   // VGA output
   output reg	hs,
   output reg 	vs,
   output [7:0] r,
   output [7:0] g,
   output [7:0] b,
	output VGA_DE
);
					
// 640x400 70HZ VESA according to  http://tinyvga.com/vga-timing/640x400@70Hz
parameter H   = 640;    // width of visible area
parameter HFP = 16;     // unused time before hsync
parameter HS  = 96;     // width of hsync
parameter HBP = 48;     // unused time after hsync

parameter V   = 400;    // height of visible area
parameter VFP = 12;     // unused time before vsync
parameter VS  = 2;      // width of vsync
parameter VBP = 35;     // unused time after vsync

reg[9:0]  h_cnt;        // horizontal pixel counter
reg[9:0]  v_cnt;        // vertical pixel counter

reg hblank;
reg vblank;

// both counters count from the begin of the visibla area

// horizontal pixel counter
always@(posedge pclk) begin
	if(h_cnt==H+HFP+HS+HBP-1)   h_cnt <= 10'b0;
	else                        h_cnt <= h_cnt + 10'b1;

	// generate negative hsync signal
	if(h_cnt == H+HFP)    hs <= 1'b0;
	if(h_cnt == H+HFP+HS) hs <= 1'b1;
	//if(h_cnt == H+HFP+HS) hblank <= 1'b1; else hblank<=1'b0;

	end

// veritical pixel counter
always@(posedge pclk) begin
	// the vertical counter is processed at the begin of each hsync
	if(h_cnt == H+HFP) begin
		if(v_cnt==VS+VBP+V+VFP-1)  v_cnt <= 10'b0; 
		else							   v_cnt <= v_cnt + 10'b1;

	        // generate positive vsync signal
		if(v_cnt == V+VFP)    vs <= 1'b1;
		if(v_cnt == V+VFP+VS) vs <= 1'b0;
		//if(v_cnt == V+VFP+VS) vblank <= 1'b1; else vblank<=1'b0;
	end
end

// read VRAM
reg [13:0] video_counter;
reg [13:0] vmem_counter;
reg [23:0] pixel;
reg de;
// stuff for vectors in project!
reg signed [`COLOR_BITS + `X_BITS + `Y_BITS+1:0] curr_vec;
reg signed [`COLOR_BITS + `X_BITS + `Y_BITS+1:0] vec_A;
reg signed [`COLOR_BITS + `X_BITS + `Y_BITS+1:0] vec_B;
reg signed [`COLOR_BITS + `X_BITS + `Y_BITS+1:0] vec_C;
// stuff for area calculation for triangles (barycentric coords)
reg signed [32:0] area_ABC;	// lets do 16 bits for integer and 16 bits for decimal part
reg signed [32:0] area_APC;	// area for A,P,C (a-c) x (p-c) [beta]
reg signed [32:0] area_ABP;	// area for A,B,P (b-a) x (p-a) [lambda]
reg signed [32:0] area_PBC;	// area for P,B,C (p-b) x (c-b) [alpha]

reg signed [49:0] beta;
reg signed [49:0] gamma;
reg signed [49:0] alpha;

reg [31:0] r_acc, g_acc, b_acc;

reg [7:0] r_interp;
reg [7:0] g_interp;
reg [7:0] b_interp;

// helper functions for vectors
function signed [`X_BITS+`Y_BITS+1:0] create_vector;
	input signed	[`X_BITS:0] i_x;
	input signed	[`Y_BITS:0] i_y;
	begin
		create_vector = {i_x,i_y};
	end
endfunction

// unpack x and y vals for easier writing
function signed [`X_BITS:0] get_x;
	input signed	[`X_BITS+`Y_BITS+1:0] i_vec;
	begin
		get_x = i_vec[`X_BITS+`Y_BITS+1:`Y_BITS+1];
	end
endfunction

function signed [`Y_BITS:0] get_y;
	input signed	[`X_BITS+`Y_BITS+1:0] i_vec;
	begin
		get_y = i_vec[`Y_BITS:0];
	end
endfunction

function signed [`X_BITS+`Y_BITS+1:0] wedge_product;
	input signed	[`X_BITS+`Y_BITS+1:0] i_vec1;
	input signed	[`X_BITS+`Y_BITS+1:0] i_vec2;
	reg signed	[`X_BITS:0] v1x, v2x;
	reg signed	[`Y_BITS:0] v1y, v2y;
	begin
		v1x = get_x(i_vec1);
		v1y = get_y(i_vec1);
		v2x = get_x(i_vec2);
		v2y = get_y(i_vec2);
		wedge_product = (v1x*v2y)-(v2x*v1y);
	end
endfunction

function signed [`X_BITS+`Y_BITS+1:0] sub_vector;
	input signed	 [`X_BITS+`Y_BITS+1:0] i_vec1;
	input signed	 [`X_BITS+`Y_BITS+1:0] i_vec2;
	// note our v3x and v3y is meant to create our return vector
	reg signed	[`X_BITS:0] v1x, v2x, v3x;
	reg signed	[`Y_BITS:0] v1y, v2y, v3y;
	begin
		v1x = get_x(i_vec1);
		v1y = get_y(i_vec1);
		v2x = get_x(i_vec2);
		v2y = get_y(i_vec2);
		v3x = v2x-v1x;
		v3y = v2y-v1y;
		sub_vector = create_vector(v3x,v3y);
	end
endfunction

function inside_triangle;
	input signed	 [`X_BITS+`Y_BITS+1:0] i_pv0;
	input signed	 [`X_BITS+`Y_BITS+1:0] i_pv1;
	input signed	 [`X_BITS+`Y_BITS+1:0] i_pv2;
	input signed	 [`X_BITS+`Y_BITS+1:0] i_pv;
	// regs for direction vectors	
	reg signed	[`X_BITS + `Y_BITS+1:0] vec_d01;
	reg signed	[`X_BITS + `Y_BITS+1:0] vec_d12;
	reg signed	[`X_BITS + `Y_BITS+1:0] vec_d20;
	// regs for vectors derived from pv
	reg signed	[`X_BITS + `Y_BITS+1:0] vec_p0p;
	reg signed	[`X_BITS + `Y_BITS+1:0] vec_p1p;
	reg signed	[`X_BITS + `Y_BITS+1:0] vec_p2p;
	// regs for storage of wedge_product results
	reg signed	[`X_BITS + `Y_BITS+1:0] res_p0p;
	reg signed	[`X_BITS + `Y_BITS+1:0] res_p1p;
	reg signed	[`X_BITS + `Y_BITS+1:0] res_p2p;
	begin
		// calculate direction vectors
		vec_d01 = sub_vector(i_pv0, i_pv1);
		vec_d12 = sub_vector(i_pv1, i_pv2);
		vec_d20 = sub_vector(i_pv2, i_pv0);
		// calculate position vectors
		vec_p0p = sub_vector(i_pv0, i_pv);
		vec_p1p = sub_vector(i_pv1, i_pv);
		vec_p2p = sub_vector(i_pv2, i_pv);
		// calculate wedge products
		res_p0p = wedge_product(vec_d01, vec_p0p);
		res_p1p = wedge_product(vec_d12, vec_p1p);
		res_p2p = wedge_product(vec_d20, vec_p2p);
		// compare results
		if ((res_p0p > 0) && (res_p1p > 0) && (res_p2p > 0)) begin
			inside_triangle = 1;
		end
		else begin
			inside_triangle = 0;
		end
	end
endfunction

// 16000 bytes of internal video memory for 160x100 pixel at 8 Bit (RGB 332)
reg [23:0] vmem [160*100-1:0];

initial begin
	vec_A = {`RED, create_vector(60,20)};
	vec_B = {`GREEN, create_vector(100,50)};
	vec_C = {`BLUE, create_vector(70,80)};
	// get area for vectors in total
	area_ABC = wedge_product(sub_vector(vec_A[`X_BITS+`Y_BITS+1:0], vec_B[`X_BITS+`Y_BITS+1:0]), sub_vector(vec_A[`X_BITS+`Y_BITS+1:0], vec_C[`X_BITS+`Y_BITS+1:0])) >>> 1;
end

//reset to blocking if all doesnt work out

always@(posedge pclk) begin
	if(vmem_counter < 16000) begin
		curr_vec = {`RED, create_vector(vmem_counter % 160, vmem_counter / 160)};
		if (inside_triangle(vec_A[`X_BITS + `Y_BITS + 1:0], vec_B[`X_BITS + `Y_BITS + 1:0], 
		vec_C[`X_BITS + `Y_BITS + 1:0], curr_vec[`X_BITS + `Y_BITS + 1:0])) begin
			// start by getting areas
			area_APC = wedge_product(sub_vector(vec_A[`X_BITS+`Y_BITS+1:0], curr_vec[`X_BITS+`Y_BITS+1:0]), 
			sub_vector(vec_A[`X_BITS+`Y_BITS+1:0], vec_C[`X_BITS+`Y_BITS+1:0])) >>> 1;
			
			area_ABP = wedge_product(sub_vector(vec_A[`X_BITS+`Y_BITS+1:0], vec_B[`X_BITS+`Y_BITS+1:0]), 
			sub_vector(vec_A[`X_BITS+`Y_BITS+1:0], curr_vec[`X_BITS+`Y_BITS+1:0])) >>> 1;
			
			area_PBC = wedge_product(sub_vector(vec_B[`X_BITS+`Y_BITS+1:0], vec_C[`X_BITS+`Y_BITS+1:0]), 
			sub_vector(vec_B[`X_BITS+`Y_BITS+1:0], curr_vec[`X_BITS+`Y_BITS+1:0])) >>> 1;
			// get gamma, beta, alpha
			alpha = (area_PBC <<< 16) / area_ABC;
			beta = (area_APC <<< 16) / area_ABC;
			gamma = (area_ABP <<< 16) / area_ABC;
			// get interpolated values
			r_acc = alpha * vec_A[40:33] + beta * vec_B[40:33] + gamma * vec_C[40:33];
			g_acc = alpha * vec_A[32:25] + beta * vec_B[32:25] + gamma * vec_C[32:25];
			b_acc = alpha * vec_A[24:17] + beta * vec_B[24:17] + gamma * vec_C[24:17];
			
			r_interp = r_acc >>> 16;
			g_interp = g_acc >>> 16;
			b_interp = b_acc >>> 16;
			
			vmem[vmem_counter] = {r_interp, g_interp, b_interp};
		end
		else begin
			vmem[vmem_counter] = 0;
		end
		vmem_counter = vmem_counter + 1;
	end
end


always@(posedge pclk) begin
        // The video counter is being reset at the begin of each vsync.
        // Otherwise it's increased every fourth pixel in the visible area.
        // At the end of the first three of four lines the counter is
        // decreased by the total line length to display the same contents
        // for four lines so 100 different lines are displayed on the 400
        // VGA lines.

	// visible area?
	if((v_cnt < V) && (h_cnt < H)) begin
		if(h_cnt[1:0] == 2'b11)
			video_counter <= video_counter + 14'd1;
		
		pixel <= vmem[video_counter];               // read VRAM
		de<=1;
	end else begin
		if(h_cnt == H+HFP) begin
			if(v_cnt == V+VFP)
				video_counter <= 14'd0;
			else if((v_cnt < V) && (v_cnt[1:0] != 2'b11))
				video_counter <= video_counter - 14'd160;
		de<=0;
		end
			
		pixel <= 24'h00;   // black
	end
end

// seperate 8 bits into three colors (332)
assign r = pixel[23:16];
assign g = pixel[15:8];
assign b = pixel[7:0];

assign VGA_DE = de;


endmodule
