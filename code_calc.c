#include <regex.h>
#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <errno.h>

#define VALINE "^[ ]*\\(\\(\\([*]\\|[+]\\|[-]\\|[/]\\)\\([ ]\\+\\)\\(\\([0-9]\\+\\)\\|\\([a-z]\\)\\)\\)\\|\\([=][ ]\\+[a-z]\\)\\|\\([=]\\)\\)[ ]*$"
#define BRACKET 1
#define NO_BRACKET 0

char error_buffer[2000];
int error_cnt = 0;
int removed_lines = 1; //set it to 1 and not 0 because for user first line is 1

/*******************************************************
* Valid tokens operations:                             *
*                                                      *
* Type variable: t_plus, t_min, t_mul, t_div, t_assign *
* Type literal: t_plus, t_min, t_mul, t_div            *
* Type eop: t_end                                      *
********************************************************/

/*Valid token type and token operation declaration*/
typedef enum {variable = 100, literal, eop, invalid} token_type;
typedef enum {t_plus = 200, t_min, t_mul, t_div, t_shl, t_shr, t_assign, t_end} token_operation;

/*Token struct declaration*/
typedef struct {
	token_type type; //variable | literal | eop
	token_operation operation; //t_plus | t_min | t_mul | t_div | t_assign | t_end
	union {
		int value; //for literals only
		char name; //for variables only
	} data; //eop carries no data
} token;

/*Checks if a line of code is valid using a regex*/
int is_valid_line (char *line);

/*Gets the instuctions from the input file and serialezes them*/
int serialize_input (char *buffer, FILE *fp);

/*Validate the tokens in each line and make sure only valid tokens exist*/
int validate_tokens (char *buffer);

/*Extracts the tokens from a buffer with valid serialized instructions, using function scan_one_token*/
int extract_tokens (char *buffer, token *tokens);

/*Takes a line of valid code and extracts the token from it*/
token scan_one_token (char *line);

/*Print the tokens from a token array (for debugging usage)*/
void print_tokens (token *tokens, int t);

/*Analize the code to detect syndax errors, unreachable code etc*/
int analize_tokens (token *tokens, int t);

/*Do some basic optimization actions on the tokens before code generation*/
int optimize_tokens (token *tokens, int t);

/*Checks if an integer is power for two from 0 to 10*/
inline int is_power_of_2 (int x);

/*Return shift equiv for power of two div or mul*/
int shift_times (int x);

/*Generates C code based on a tokens array*/
int generate_code (char *buffer, token *tokens, int t);

/*Convert a token to code*/
void token_to_code(token tkn, char *code_token, int put_bracket);

int main (int argc, char *argv[]) {
	char buffer[10000];
	token tokens[500];
	int t; //total number of tokens
	int i; //general use counter
	int res;
	FILE *fp;
	
	if (argc < 2) { //check if the number of arguments is correct
		printf("Usage: %s <input_file> -o <output_file>\n", argv[0]);
		return 1;
	}
	else if ((fp = fopen(argv[1], "r")) == NULL) { //try to open the input file
		printf("%s\n", strerror(errno));
		return 2;
	}
	else {
		res = serialize_input(buffer, fp); //put instuctions from the input file to the buffer serialized
		close(fp);
		if (res == 0) {
			puts("Empty input file.");
			return 3;
		}
	}

	/*parse the code*/
	validate_tokens(buffer);
	t = extract_tokens(buffer, tokens);
	
	/*syntax analysis*/
	t = analize_tokens(tokens, t);
	
	/*optimization*/
	t = optimize_tokens(tokens, t);
	
	/*code generation*/
	generate_code(buffer, tokens, t);
	
/*	Print tokens and errors for debugging*/
	print_tokens(tokens, t);
	
	if (error_cnt != 0) {
		puts(error_buffer);
	}
	
	return 0;
}

/*Checks if a line of code is valid using a regex*/
int is_valid_line (char *line) {
	int regres; //the result of regex
	regex_t regex; //the regular expretion
	char *regerr; //regex error buffer
	
	regres = regcomp(&regex, VALINE, 0); //compile the regex
	regres = regexec(&regex, line, 0, NULL, 0); //execute regex
	
	if (!regres){
		regfree(&regex); //free the compiled regex
		return 1; //line is accepted
	}
	else if (regres == REG_NOMATCH){
		regfree(&regex); //free the compiled regex
		return 0; //line is not accepted
	}
	else{
		regfree(&regex); //free the compiled regex
		regerror(regres, &regex, regerr, sizeof(regerr));
		fputs(regerr, stderr);
		exit(-1);
	}
}

/*Gets the instuctions from the input file and validates them*/
int serialize_input (char *buffer, FILE *fp) {
	int i = 0; //lines counter
	int line_len;
	
	for(i = 0; !feof(fp); i += line_len) {
		fgets(&buffer[i], 1000, fp); //max 1000 chars each line
		line_len = strlen(&buffer[i]);
		
		//discard null lines
		if (line_len == 1) {
			line_len = 0;
			continue;
		}
		
		//put ';' delimiter at the end of line
		buffer[i + line_len - 1] = ';';
	}
	return i;
}

/*Validate the tokens in each line and make sure only valid tokens exist*/
int validate_tokens (char *buffer) {
	char tmp[10000];
	char line[500]; //max 500 characters each line
	int i; //buffer counter
	int j; //line pos counter
	int l = 0; //valid line counter
	int k = 0; //tmp char offset
	int ln = 0; //lines counter
	int noerror = 1;


	for (i = 0, j = 0; buffer[i] != '\0'; ++i) {
		if (j > 499) { //buffer overflow
			fputs("Error! Buffer overflow.", stderr);
			exit(-3);
		}
		else if (buffer[i] != ';') {
			line[j++] = buffer[i];
		}
		else {
			line[j] = '\0';
			j = 0;
			
			if (is_valid_line(line)) {
				++l;
				strcpy(&tmp[k], line);
				k += strlen(&tmp[k]);
				tmp[k++] = ';';
			}
			else {
				noerror = 0;
				//log the error to the global error buffer
				sprintf(&error_buffer[error_cnt], "%d: unrecognised token\n", ln + removed_lines);
				error_cnt += strlen(&error_buffer[error_cnt]);
			}
			++ln;
		}
	}
	
	removed_lines += ln - l;
	
	strcpy(buffer, tmp);
	return noerror;
}

/*Extracts the tokens from a buffer with valid serialized instructions, using function scan_one_token*/
int extract_tokens (char *buffer, token *tokens) {
	char line[500]; //max 500 characters each line
	int i; //buffer counter
	int j; //line characters counter
	int t = 0; //total scaned tokens

	for (i = 0, j = 0; buffer[i] != '\0'; ++i) {

		if (buffer[i] != ';') {
			line[j++] = buffer[i];
		}
		else {
			line[j] = '\0';
			j = 0;
			
			tokens[t++] = scan_one_token(line);
			if (tokens[t-1].type == invalid) { //this should never happen
				--t;
				++removed_lines;
			}
		}
		
	}
	return t;
}

/*Takes a line of valid code and extracts the token from it*/
token scan_one_token (char *line) {
	token tkn;
	char num[100];
	char op;
	int i;
	int s;
	
	if (!is_valid_line(line)) { //if validate_tokens function works as expected this should never happen, just in case
		tkn.type = invalid;
		return tkn;
	}

	for (s = 0; line[s] == ' '; ++s); //drop spaces on the start of line
	op = line[s]; //get the operator character
	
	switch (op) {
		case '+':
			tkn.operation = t_plus;
			break;
		case '-':
			tkn.operation = t_min;
			break;
		case '*':
			tkn.operation = t_mul;
			break;
		case '/':
			tkn.operation = t_div;
			break;
		case '=':
			tkn.operation = t_assign; //or t_end we will find out later
			break;
	}
	
	for (++s; line[s] == ' '; ++s); //drop spaces on the start of line
	
	if (line[s] == '\0') { //then we have a t_end, so...
		tkn.operation = t_end;
		tkn.type = eop;
		//eop has no data
	}
	else if (line[s] >= 'a' && line[s] <= 'z') { //then we have a type of variable
		//operation has been set in the switch
		tkn.type = variable;
		tkn.data.name = line[s];	
	}
	else if (line[s] >= '0' && line[s] <= '9') {
		for (i = 0; line[s] >= '0' && line[s] <= '9'; num[i] = line[s], ++i, ++s); //parse the number
		num[i] = '\0';
		
		//operation has been set in the switch
		tkn.type = literal;
		tkn.data.value = atoi(num);
	}
	else {
		//this should also never happen, just in case
		tkn.type = invalid;
	}
	
	return tkn;
}

/*Print the tokens from a token array (for debugging usage)*/
void print_tokens (token *tokens, int t) {
	int i;
	
	for (i = 0; i < t; ++i) {
		if (tokens[i].type == literal) {
			printf("type=%d operation=%d data=%d\n", tokens[i].type, tokens[i].operation, tokens[i].data.value);
		}
		else if (tokens[i].type == variable) {
			printf("type=%d operation=%d data=%c\n", tokens[i].type, tokens[i].operation, tokens[i].data.name);
		}
		else {
			printf("type=%d operation=%d\n", tokens[i].type, tokens[i].operation);
		}
	}
}

/*Analize the code to detect syndax errors, unreachable code etc*/
int analize_tokens (token *tokens, int t) {
	int i;
	int e = 0; //eop counter
	
	/*Detect unreachable code of if eop is missing*/
	for (i = 0; i < t; ++i) {
		if (tokens[i].type == eop) {
			++e;
			if (e == 1) {
				break;
			}
		}
	}
	
	if (e == 0) { //eop is missing
		//log the error to the global error buffer
		sprintf(&error_buffer[error_cnt], "%d: end_of_program token is missing. Autoassign it at line %d\n", i + removed_lines, i + removed_lines);
		error_cnt += strlen(&error_buffer[error_cnt]);
		
		/*Assign eop token at the end of the program*/
		tokens[i].type = eop;
		tokens[i].operation = t_end;
		
		t = i + 1; //update tokens counter
	}
	else if (t > i + 1) { //then we have unreachable code at position i + 1
		//log the error to the global error buffer
		sprintf(&error_buffer[error_cnt], "%d: unreachable code detected\n", i + removed_lines + 1);
		error_cnt += strlen(&error_buffer[error_cnt]);
		
		t = i + 1; //drop the unreachable code
	}


	/*Add result variable assignment at the end of the program*/
	tokens[++t-1].type = eop;
	tokens[t-1].operation = t_end;
	
	tokens[t-2].type = variable;
	tokens[t-2].operation = t_assign;
	tokens[t-2].data.name = '$'; //result variable is symbolized with the dollar sign
	
	return t;
}


int optimize_tokens (token *tokens, int t) {
	int i, z, k;
	token tokens_tmp[500];

	/*Drop all lines before last '* 0'*/
	for (i = 0, z = -1; i < t; ++i) {
		if (tokens[i].operation == t_mul && tokens[i].type == literal && tokens[i].data.value == 0) {
			z = i;
			break;
		}
	}

	/*Drop '+ 0', '- 0', '* 1', '/ 1' tokens*/
	for (i = z + 1, k = 0; i < t; ++i) { //and start after the last '* 0'
		if (!(tokens[i].type == literal && 
			(
				(tokens[i].data.value == 0 && (tokens[i].operation == t_plus || tokens[i].operation == t_min)) ||
				(tokens[i].data.value == 1 && (tokens[i].operation == t_mul || tokens[i].operation == t_div))
			)
		)) {
			tokens_tmp[k++] = tokens[i];
		}
	}
	t = k; // update tokens counter

	/*update tokens array*/
	for (i = 0; i < t; ++i) {
		tokens[i] = tokens_tmp[i];
	}
	
	/*Convert divs and muls with powers of two to shifts*/
	for (i = 0; i < t; ++i) {
		if (tokens[i].type == literal && tokens[i].operation == t_mul && is_power_of_2(tokens[i].data.value)) {
			tokens[i].operation = t_shl;
			tokens[i].data.value = shift_times(tokens[i].data.value);
		}
		
		else if (tokens[i].type == literal && tokens[i].operation == t_div && is_power_of_2(tokens[i].data.value)) {
			tokens[i].operation = t_shr;
			tokens[i].data.value = shift_times(tokens[i].data.value);
		}
	}

	return t;
}

/*Checks if an integer is power for two from 0 to 10*/
inline int is_power_of_2 (int x) {
	if (x == 1 || x == 2 || x == 4 || x == 8 || x == 16 || x == 32 || x == 64 || x == 128 || x == 256 || x == 512 || x == 1024) {
		return 1;
	}
	return 0;
}

/*Return shift equiv for power of two div or mul*/
int shift_times (int x) {
	int i, k;
	for (i = 1, k = 0; i < x; i *= 2, ++k);

	if (i == x) { //if x was a valid power of 2, after the end of the for loop
		return k; //the i must be equal with x, in that case return k
	}
	else {
		return -1; //else return -1 to indicate wrong input
	}
}

/*Generates C code based on a tokens array*/
int generate_code (char *buffer, token *tokens, int t) {
	int i, j;
	int last_assign = 0;
	int last_assign_with_data = -1;
	int a = 0; //assignment counter
	int k = 0; //assignment counter 
	char assignments[500][256];
	char code_token[50];
	
	/*Generate the assignments to the variables*/
	for (i = 0; i < t; ++i) {
		if (tokens[i].operation == t_assign) { //then assign it, all the above tokens
			
			/*Begin the assignment line with the variable*/
			if (tokens[i].data.name != '$') { //if we have an ordinary variable
				assignments[a][k++] = tokens[i].data.name;
			}
			else {
				strcpy(&assignments[a][k], "result"); //if we reached the default variable
				k += strlen(&assignments[a][k]);
			}
			
			/*Next we have the assign operator*/
			assignments[a][k++] = ' ';
			assignments[a][k++] = '=';
			assignments[a][k++] = ' ';
			
			/*Put as many brackets on the start as the total operations in the assignment - 1*/
			for (j = last_assign; j + 1 < i; ++j, assignments[a][k++] = '(');
			
			/*If we have don't have +- at the start of the assignment put a zero*/
			if (j != last_assign && tokens[last_assign].operation != t_plus && tokens[last_assign].operation != t_min) {
				assignments[a][k++] = '0';
			}
			
			/*Convert the tokens to code*/
			for (j = last_assign; j < i; ++j) {
				if (j != i - 1) {
					token_to_code(tokens[j], code_token, BRACKET); //if not the last operation put bracket
				}
				else {
					token_to_code(tokens[j], code_token, NO_BRACKET); //don't put bracket to the last operation
				}
				
				strcpy(&assignments[a][k], code_token);
				k += strlen(&assignments[a][k]);
			}

			/*If the assignment has no operation in it*/
			if (last_assign >= i) { //this happens when we have 2 '= variable' in a row inside the code
				assignments[a][k++] = '0';
			}
			else {
				last_assign_with_data = a; //detect the last assignment with data
			}
			
			/*Put the semicolon at the end of the assignment*/
			assignments[a++][k] = ';';
			
			k = 0;
			last_assign = i + 1;
		}
	}
	
	/*If we have assignments without data, put the last assignment with data in the result variable*/
	if (last_assign_with_data != -1 && last_assign_with_data != a - 1) {
		k = strlen(assignments[a-1]);
		assignments[a-1][k-2] = assignments[last_assign_with_data][0];
	}
	
	for (i = 0; i < a; ++i) {
		puts(assignments[i]);
	}

}

/*Convert a token to code*/
void token_to_code(token tkn, char *code_token, int put_bracket) {
	int i = 0;
	
	switch (tkn.operation) {
		case t_plus:
			code_token[i++] = '+';
			break;
		case t_min:
			code_token[i++] = '-';
			break;
		case t_mul:
			code_token[i++] = '*';
			break;
		case t_div:
			code_token[i++] = '/';
			break;
		case t_shl:
			code_token[i++] = '<';
			code_token[i++] = '<';
			break;
		case t_shr:
			code_token[i++] = '>';
			code_token[i++] = '>';
			break;
	}
		
	if (tkn.type == literal) {
		sprintf(&code_token[i++], "%d", tkn.data.value);
		i = strlen(code_token);
	}
	else {
		code_token[i++] = tkn.data.name;
	}
	
	if (put_bracket) {
		code_token[i++] = ')';
	}
	
	code_token[i++] = '\0';
}



























