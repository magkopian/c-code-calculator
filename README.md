<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	</head>
	<body>
		<h3>Description:</h3>
		<p>
		This is a translator for a simple language, that describes mathematical operations, to C. The translator has been developed
		in two different ways. At first, all the functions for lectical and syntax analysis were written in C. Then,
		the implementation of the lexical analyzer was done with Flex.
		</p>

		<h3>The Input Language:</h3>
		<p>
		First of all, I am not the creator of the language. This project has been given originally as an assignment for the compilers 
		course at my school. The input language is very simple. It supports the '+', '-', '*', '/' and '%' operators. Also, it supports 
		variables and it has an assignment operator '='. Every program must end with an END_OF_PROGRAM symbol (EOP), which is the same 
		symbol that is used for the assignment operator.
		</p>
		
		<h3>Input Program Example:</h3>
		<p>
		Next, it follows an example program of the input language. Note though, that currently the comments are not supported, 
		I only use them in this readme file to explain each program line.
		</p>
		
		<p>
			
			+ 9		/*result = 0 + 9*/
			- 7 	/*result = result - 7*/
			/ 4		/*result = result / 4*/
			= a		/*a = result*/
			* 2		/*result = 0 * 2*/
			- 7 	/*result = result - 7*/
			= b		/*b = result*/
			+ a		/*result = 0 + a*/
			- b 	/*result = result + b*/
			= c		/*result = c*/
			=		/*EOP*/
			
		</p>
		
		<h3>Output Program Example:</h3>
		
		<p>
		Next, it follows the output of the previous example code. Note also here, that the comments are not generated by the translator.
		</p>
		
		<p>
			
			#include <stdio.h>
			#include <stdlib.h>

			int main(void) {
				int a, b, c, result;

				a = ((+9)-7)>>2;					/*the translation of the first four operations*/
				b = (0<<1)-7;						/*the translation of the next three operations*/
				c = (+a)-b;							/*the translation of the next three operations*/
				result = c;							/*the last variable is assigned to the default variable*/
				printf("Result = %d\n", result);	/*print the default variable*/
				return 0;
			}
			
		</p>
		
		<p>
		The "result" is the translator's default variable. The translator, will assign to it the last variable 
		that has been used in an assignment operation. If no variables have been used in the whole program, then the "result" 
		variable will have the total result of all the operations.
		</p>
		
		<p>
		Also, you may noticed that instead of "a = ((+9)-7)/4", you get "a = ((+9)-7)>>2", that is because the translator 
		does also code optimization.
		</p>
		
		<h3>Compilation:</h3>
		
		<p>
		To compile the C-only version, you just run: <br>
		gcc -o code_calc code_calc.c<br>
		For this to work, the gcc compiler have to be installed.
		</p>
		
		<p>
		For the Flex version: <br>
		Flex -o Flex_code_calc.c Flex_code_calc.l<br>
		gcc -o Flex_code_calc Flex_code_calc.c -lfl<br>
		For this to work, the gcc compiler and the Flex Lexical Analyzer Generator have to be installed.
		</p>
		
		<h3>Usage:</h3>
		<p>
		In order to use the program, you just run: <br>
		./code_calc <input_file> -o <output_file.c><br>
		or<br>
		./code_calc <input_file><br>
		In the first case, a C file with a user specified name will be created. In the second case, the program will create
		an output file with the name out.c.
		</p>
		
		<p>
		If the translator encounter any errors, it will display the apropriate error messages but this will not stop the 
		translation process. Any lines that contain errors will just be ignored and the translation will be done without them.
		To get detailed information on how the translator processes the input code, you can enable the debugging mode by 
		changing the DEBUG_MODE define to 1.
		</p>
	</body>
</html>
