/* PART 1
module lab_1 (SW, LED);
	input		[3:0]	SW;		// slide switches
	output	[3:0]	LED;		// green LEDs
	assign LED = SW;
endmodule
*/

/* PART 2 
module lab_1 (SW, LED);
	input		[3:0]	SW;	// will make array of all SW due to imported assignment for DE-10 nano
	wire		[3:0]	x;
	wire		[3:0]	y;
	output	[7:0]	LED;	// ^^ will only use 1 led because the circuit is designed such that
	genvar	i;
	// assignments
	assign	LED[7:4]		= 4'b0000;
	// CONST TESTING
	//assign	x[3:0]		= 4'b0001;		// so, when SW[0] is OFF (0) far right LED should light up! 
	//assign	y[3:0]		= 4'b0010;		// and when SW[0] is ON	(1) LED next to far right should light up
	//assign	x[3:0]		= 4'b0011;		// ^^ SW[0] = 0 -> LED[1:0] lit up
	//assign	y[3:0]		= 4'b0100;		// ^^ SW[0] = 1 -> LED[2] lit up
	//assign	x[3:0]		= 4'b0101;		// ^^ SW[0] = 0 -> LED[2], LED[0] lit up
	//assign	y[3:0]	 	= 4'b0110;		// ^^ SW[0] = 1 -> LED[2:1] lit up
	assign	x[3:0]		= 4'b0111;
	assign	y[3:0]		= 4'b1000;
	//assign	x[3:0]		= 4'b1001;
	//assign	y[3:0]		= 4'b1010;
	//assign	x[3:0]		= 4'b1011;
	//assign	y[3:0]		= 4'b1100;
	//assign	x[3:0]		= 4'b1101;
	//assign	y[3:0]		= 4'b1110;
	//assign	x[3:0]		= 4'b1111;
	//assign	y[3:0]		= 4'b0000;
	// assume x and y are two 4 bit inputs and m (LED) is a 4 bit output
	generate
		for (i = 0; i < 4; i = i+1) begin : led_assign
			assign	LED[i]	= (~SW[0]&x[i])|(SW[0]&y[i]);
		end
	endgenerate
endmodule
*/

/* PART 3 
module lab_1 (SW, LED);
	input		[1:0]		SW;
	output 	[7:0]		LED;
	wire		[1:0]		u;
	wire		[1:0]		v;
	wire		[1:0]		w;
	wire		[1:0]		x;
	genvar	i;
	assign	LED[7:2]	= 6'b000000;
	// CONST TESTING
	assign	u[1:0]	= 2'b00;
	assign	v[1:0]	= 2'b01;
	assign	w[1:0]	= 2'b10;
	assign	x[1:0]	= 2'b11;
	generate
		for (i = 0; i < 2; i = i+1) begin : led_assign
			assign	LED[i]	= (~SW[1]&~SW[0]&u[i]|~SW[1]&SW[0]&v[i]|SW[1]&~SW[0]&w[i]|SW[1]&SW[0]&x[i]);
		end
	endgenerate
	
endmodule
*/

/* NOT AN OFFICIAL PART. lets try decoding into bytes WORKS! this is my "Part 4"
module lab_1 (SW, LED);
	input		[1:0]	SW;
	output	[7:0]	LED;
	wire		[7:0]	d;
	wire		[7:0]	e;
	wire		[7:0]	o;
	wire		[7:0]	z;
	genvar	i;
	assign	d[7:0]	= 8'b01100100;		// 'd' in binary
	assign	e[7:0]	= 8'b01000101;		// 'E' in binary
	assign	o[7:0]	= 8'b00110001;		// '1' in binary
	assign	z[7:0]	= 8'b00110000;		// '0' in binary
	generate
		for (i = 0; i < 8; i = i+1) begin : led_assign
			assign	LED[i]	= (~SW[1]&~SW[0]&d[i]|~SW[1]&SW[0]&e[i]|SW[1]&~SW[0]&o[i]|SW[1]&SW[0]&z[i]);
		end
	endgenerate
endmodule
*/

/* NOT AN OFFICIAL PART. lets try decoding the word "de10" into bytes. maybe come back to this ? 
my idea is to create a verilog file such that there is a "loop" based on the clock such that the binary
for 'd' then 'E' then '1' then '0' is flashed on the 8 LEDs, and based on certain sets of the switch the
order changes. I'm assuming I need to implement timing for this to be possible, because even though in
simulation its possible to accomplish this using # times, I want it on the board. come back when ready. */
	