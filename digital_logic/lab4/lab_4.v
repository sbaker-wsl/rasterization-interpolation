module T_FLIP_FLOP (T, Clk, Clr, Q);
	input				T, Clk, Clr;
	output	reg	Q;
	
	always @ (posedge Clk) begin
		if (Clr)
			Q <= 0;
		else if (T)
			Q <= ~Q;
	end
endmodule


/* PART 1 
module lab_4 (SW, LED, KEY);
	input		[1:0]	SW;
	output	[7:0]	LED;
	input		[1:0]	KEY;
	wire		[7:0] Q;

	T_FLIP_FLOP T1 (SW[1], 	~KEY[0], SW[0],Q[0]);
	T_FLIP_FLOP T2 (SW[1]&Q[0],~KEY[0],SW[0],Q[1]);
	T_FLIP_FLOP T3 (SW[1]&Q[0]&Q[1],~KEY[0],SW[0],Q[2]);
	T_FLIP_FLOP T4 (SW[1]&Q[0]&Q[1]&Q[2],~KEY[0],SW[0],Q[3]);
	T_FLIP_FLOP T5 (SW[1]&Q[0]&Q[1]&Q[2]&Q[3],~KEY[0],SW[0],Q[4]);
	T_FLIP_FLOP T6 (SW[1]&Q[0]&Q[1]&Q[2]&Q[3]&Q[4],~KEY[0],SW[0],Q[5]);
	T_FLIP_FLOP T7 (SW[1]&Q[0]&Q[1]&Q[2]&Q[3]&Q[4]&Q[5],~KEY[0],SW[0],Q[6]);
	T_FLIP_FLOP T8 (SW[1]&Q[0]&Q[1]&Q[2]&Q[3]&Q[4]&Q[5]&Q[6],~KEY[0],SW[0],Q[7]);
	assign LED[7:0] = Q[7:0];
endmodule
*/

/* PART 2: Specify a counter by using a register and adding 1 to its value 
module lab_4 (SW, LED, KEY);
	input		[1:0]	SW;
	output	[7:0]	LED;
	input		[1:0]	KEY;
	reg		[7:0] Q;
	always @(posedge ~KEY[0]) begin
		if (SW[0])
			Q <= 0;
		else if (SW[1])
			Q <= Q + 1;
	end
	assign LED[7:0] = Q[7:0];
endmodule
*/

/* PART 3: design a counter such that it will flash 0 through 8 (0000 - 1000) with 1 second intervals between it,
use 50 MHz clock signal */
module lab_4 (LED, CLOCK_50);
	input				CLOCK_50;
	output	[3:0] LED;
	reg		[26:0]	sec; // this is counter for seconds
	reg		[3:0]	Q; // use this to control the LEDs
	always @(posedge CLOCK_50) begin
		sec <= sec + 1;
		if (sec > 50000000 - 1) begin
			if (Q < 4'b1000)
				Q <= Q + 1;
			else
				Q <= 0;
			sec <= 0;
		end
	end
	assign LED[3:0] = Q[3:0];
endmodule
	