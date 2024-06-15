
module FullAdder4( input[3:0] a, input[3:0] b, input c_in, output c_out,
		output[3:0] sum);
	assign {c_out, sum} = a + b + c_in;
endmodule

module tb_FullAdder4;
	reg[3:0]	a;
	reg[3:0]	b;
	reg			c_in;
	wire         c_out;
	wire[3:0]	sum;
	integer		i;

	FullAdder4 adder (.a(a), .b(b), .c_in(c_in), .c_out(c_out), .sum(sum));

	initial begin
		a <= 0;
		b <= 0;
		c_in <= 0;
		$monitor("a=%0h b=%0h c_in=%0h c_out=%0h sum=%0h", a, b, c_in, c_out, sum);

		for (i = 0; i <= 10; i = i + 1) begin
			#10 a <= $random;
			b <= $random;
			c_in <= $random;
		end
	end
endmodule
