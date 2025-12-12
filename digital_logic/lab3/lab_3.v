/* PART 1 */
//module	lab_3 (Clk, R, S, Q);
	//input 	Clk,R,S;
	//output	Q;
	
	//wire R_g, S_g, Qa, Qb	/* synthesis keep */;
	
	//assign R_g  = R&Clk;
	//assign S_g  = S&Clk;
	//assign Qa 	= ~(R_g|Qb);
	//assign Qb	= ~(S_g|Qa);
	
	//assign Q = Qa;
	
//endmodule

/* PART 2 SUCCESS! ALSO PART 3 */
// create circuit for a gated D latch
module d_latch (Clk, D, Q);
	input Clk, D;
	output Q;
	
	wire R_g, S_g, Qa, Qb, S, R	/* synthesis keep */;
	
	assign	S		=	D;
	assign	R		= ~D;
	assign	R_g	= ~(R&Clk);
	assign	S_g	= ~(S&Clk);
	assign	Qa		= ~(S_g&Qb);
	assign	Qb		= ~(R_g&Qa);
	
	assign	Q		= Qa;
endmodule

/* PART 2 (gated D latch) 
module lab_3 (SW, LED);
	input		[1:0]			SW;
	output	[7:0]			LED;
	assign	LED[7:1]	=	1'b0;
	d_latch d1 (SW[1], SW[0], LED[0]);
endmodule 
*/

/* PART 3 (master-slave D flip-flop) 
module lab_3 (SW, LED);
	input		[1:0]			SW;
	output	[7:0]			LED;
	wire		Qm;
	assign	LED[7:1]	=	1'b0;
	d_latch master	(~SW[1], SW[0],	Qm);
	d_latch slave	(SW[1],	Qm,		LED[0]);
endmodule
*/

/* PART 4 (3 storage elements) 
module D_LATCH (D,Clk,Q);
	input 			D,Clk;
	output	reg 	Q;
	
	always @ (D,Clk)
		if (Clk)
			Q = D;
endmodule


module FLIP_FLOP_POS (D, Clk, Q);
	input				D,Clk;
	output	reg	Q;
	
	always @ (posedge Clk)
		Q = D;
endmodule

module FLIP_FLOP_NEG (D, Clk, Q);
	input				D,Clk;
	output	reg	Q;
	
	always @ (negedge Clk)
		Q = D;
endmodule


module lab_3 (Clk, d, Qa, Qb, Qc);
	input				Clk, d;
	output			Qa, Qb, Qc;
	D_LATCH			gated	(d, Clk, Qa);
	FLIP_FLOP_POS	pos_F (d, Clk, Qb);
	FLIP_FLOP_NEG	neg_F	(d, Clk, Qc);
endmodule
*/

module D_LATCH (D, Clk, Rst, Q);
	input		D,Clk,Rst;
	output	reg	Q;
	
	always @(*) begin
		if (Rst)
			Q = 1'b0;
		else if (Clk)
			Q = D;
		end
endmodule

module full_adder (ci, a, b, s, co);
	input		ci, a, b;
	output	s, co;
	assign	s = (a^b)^ci;
	assign	co = (b&ci)|(a&ci)|(a&b);
endmodule

/* PART 5 */
module	lab_3 (SW, LED, KEY);
	input		[3:0]	SW;
	output	[7:0]	LED;
	input		[1:0]	KEY;
	wire		[3:0]	A;
	wire		[2:0] C;
	D_LATCH	A1 (SW[0],~KEY[1],~KEY[0],A[0]);
	D_LATCH	A2 (SW[1],~KEY[1],~KEY[0],A[1]);
	D_LATCH	A3 (SW[2],~KEY[1],~KEY[0],A[2]);
	D_LATCH	A4 (SW[3],~KEY[1],~KEY[0],A[3]);
	full_adder F1 (1'b0,A[0],SW[0],LED[0],C[0]);
	full_adder F2 (C[0],A[1],SW[1],LED[1],C[1]);
	full_adder F3 (C[1],A[2],SW[2],LED[2],C[2]);
	full_adder F4 (C[2],A[3],SW[3],LED[3],LED[4]);
	
endmodule