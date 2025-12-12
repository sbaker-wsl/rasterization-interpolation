/* PARTS 1 AND 2 N/A, PART 3 */
module full_adder (ci, a, b, s, co);
	input		ci;
	input		a;
	input		b;
	output	s;
	output	co;
	assign s		= (a^b)^ci;
	assign co	= (b&ci)|(a&ci)|(a&b);
endmodule

module lab_2 (SW, LED);
	input		[3:0]	SW;
	output	[7:0]	LED;
	wire		a1, a2, a3, a4;
	wire		b1, b2, b3, b4;
	wire		c1, c2, c3;
	//assign	a1		=	1'b0;	// 00
	//assign	b1		=	1'b0;
	//assign	a1		=	1'b0;	// 01
	//assign	b1		=	1'b1;
	//assign	a1		=	1'b1;	// 10
	//assign	b1		=	1'b0;
	assign	a1		=	1'b1;	// 11
	assign	b1		=	1'b1;
	//assign	a2		=	1'b0;	// 00
	//assign	b2		=	1'b0;
	//assign	a2		=	1'b0;	// 01
	//assign	b2		=	1'b1;
	assign	a2		=	1'b1;	// 10
	assign	b2		=	1'b0;
	//assign	a2		=	1'b1;	// 11
	//assign	b2		=	1'b1;
	assign	a3		=	1'b0;	// 00
	assign	b3		=	1'b0;
	//assign	a3		=	1'b0;	// 01
	//assign	b3		=	1'b1;
	//assign	a3		=	1'b1;	// 10
	//assign	b3		=	1'b0;
	//assign	a3		=	1'b1;	// 11
	//assign	b3		=	1'b1;
	//assign	a4		=	1'b0;	// 00
	//assign	b4		=	1'b0;
	assign	a4		=	1'b0;	// 01
	assign	b4		=	1'b1;
	//assign	a4		=	1'b1;	// 10
	//assign	b4		=	1'b0;
	//assign	a4		=	1'b1;	// 11
	//assign	b4		=	1'b1;
	full_adder	fa1	(.ci(SW[0]),	.a(a1),	.b(b1),	.s(LED[0]),	.co(c1));
	full_adder	fa2	(.ci(c1),		.a(a2),	.b(b2),	.s(LED[1]),	.co(c2));
	full_adder	fa3	(.ci(c2),		.a(a3),	.b(b3),	.s(LED[2]),	.co(c3));
	full_adder	fa4	(.ci(c3),		.a(a4),	.b(b4),	.s(LED[3]),	.co(LED[4]));
	
endmodule