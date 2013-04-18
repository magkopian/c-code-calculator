#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*error report table*/
char error_log[10000] = "No Errors.\n";
int e_i = 0; //error counter

/*the struct for the instruction token*/
struct instruction {
	char op; //operator
	int val; //value
};

/*the struct for the variable token*/
struct symbol {
	char op; //operator
	char var; //variable name;
};


typedef struct instruction instruction;
typedef struct symbol symbol;

/*put's instructions in a buffer, separated with semicolons*/
void get_instructions(char *buffer, FILE *fp);

/*extracts instructions from buffer*/
int extract_instructions(char *buffer, instruction *inst, symbol *syms);

/*return length of instruction*/
inline int llen(char *str);

/*parse line and extract instruction tokens*/
instruction parse_line(char *line, int line_len, int k, symbol *syms);

/*execute an instruction*/
int do_operation(char op, int val);

/*optimize the instructions and return number of instructions after optimization*/
int optimize(instruction *inst, int int_num, symbol *syms);

/*check if an integer is power of 2*/
inline int is_power_of_2(int x);

/*return shift equiv for power of 2 div or mul*/
int shift_times(int x);

/*generate the c code based on the instruction tokens*/
void generate_code(instruction *inst, char *code, int int_num, symbol *syms);

/*generate c file with the code*/
void save_code(char *code, char *fout);


int main(int argc, char **argv) {
	char buffer[10000]; //10000 characters max length of source code
	int int_num;
	int i;
	int num;
	instruction inst[1000]; //1000 instuctions max
	symbol syms[100]; //100 variables max
	char code[2000]; //2000 characters max length of C code
	FILE *fp;


	if (argc < 2) { //check if the number of arguments is correct
		puts("No input file has been specified.");
		exit(-1);
	}
	else if ((fp = fopen(argv[1], "r")) == NULL) { //try to open the input file
		printf("%s\n", strerror(errno));
		exit(-2);
	}
	else {
		get_instructions(buffer, fp); //put instuctions from the input file to the buffer
		close(fp);
	}

	
	int_num = extract_instructions(buffer, inst, syms); //extract instuctions from the buffer
	
	if (int_num == -1) {
		puts("No instructions found in the file.");
		exit(-3);
	}
	else { //if everything is ok
		puts(error_log); //print error log (or "No Errors" message if there isn't any)
		
		int_num = optimize(inst, int_num, syms); //do instruction opimization
		generate_code(inst, code, int_num, syms); //generate c code
		
		
		//check if output file has been specified
		if (argc == 4 && strcmp(argv[2], "-o") == 0) {
			save_code(code, argv[3]);
		}
		else {
			save_code(code, "out.c");
		}
		
		/*print tokens and symbols (for debugging)*/
		////////////////////////////////////////////
		for (i = 0; syms[i].var != '!'; ++i) {
			printf("%c %c\n", syms[i].var, syms[i].op);
		}
		putchar('\n');
		putchar('\n');
		for (i = 0; i < int_num; ++i) {
			printf("%c %d\n", inst[i].op, inst[i].val);
		}
		putchar('\n');
		putchar('\n');
		////////////////////////////////////////////
		
	}
	return 0;
}

void get_instructions(char *buffer, FILE *fp) {
	int i = 0;
	int line_len;
	
	for(i = 0; !feof(fp); i += line_len) {
		fgets(&buffer[i], 1000, fp); //max 1000 chars each line
		line_len = strlen(&buffer[i]);
		
		//discard null lines
		if (line_len == 1) {
			line_len = 0;
			continue;
		}
		
		
		//detect end char
		if (line_len == 2 && buffer[i] == '=') {
			buffer[i] = '\0';
			break;
		}
		else {
			//put ';' delimiter at the end of line
			buffer[i + line_len - 1] = ';';
		}
	}
}

int extract_instructions(char *buffer, instruction *inst, symbol *syms) {
	int i, k;
	int line_len = 0;
	
	if (strlen(buffer) == 0) return -1; //error NULL buffer

	for (i = 0, k = 0; buffer[i] != '\0'; i += line_len + 1, ++k) {
		line_len = llen(&buffer[i]);
		
		inst[k] = parse_line(&buffer[i], line_len, k, syms);
	}
	inst[k].op = '&';
	inst[k++].val = -2;
	
	return k; //return num of instructions
}

inline int llen(char *str) {
	int i;
	for (i = 0; str[i] != ';'; ++i);
	return i;
}

instruction parse_line(char *line, int line_len, int k, symbol *syms) {
	instruction inst;
	int i;
	char num[100];
	char *msg;
	char str[20];
	int isvar = 0;
	int isopvar = 0;
	static int varc = 0;
	
	inst.op = '+'; //int operator
	
	if (line[0] == '=' && line[1] == ' ' && line[2] >= 'a' && line[2] <= 'z' && line[3] == ';') { //it's a var!
		isvar = 1;
	}
	else if (line[0] != '+' && line[0] != '-' && line[0] != '*' && line[0] != '/') {
		inst.op = '!'; //error in instruction
		inst.val = 0;
		
		sprintf(str, "%d", k+1); //convert int to string
		msg = strcpy(&error_log[e_i], str);
		e_i += strlen(msg);
		msg = strcpy(&error_log[e_i], ": No operator specified!\n\0"); //only the \0 of the last error will not been overwritten
		e_i += strlen(msg);
	}
	else if (line[1] != ' ') {
		inst.op = '!'; //error in instruction
		inst.val = 0;
				
		sprintf(str, "%d", k+1); //convert int to string
		msg = strcpy(&error_log[e_i], str);
		e_i += strlen(msg);
		msg = strcpy(&error_log[e_i], ": No delimiter after operator!\n\0"); //only the \0 of the last error will not been overwritten
		e_i += strlen(msg);
	}
	else {
		if (line[2] >= 'a' && line[2] <= 'z' && line[3] == ';') { //it's operation with var
			isopvar = 1;
		}
		else { //it's operation with constant
			for (i = 2; i < line_len; ++i) {
				if (line[i] < '0' || line[i] > '9' ) {
					inst.op = '!'; //error in instruction
					inst.val = 0;
						
					sprintf(str, "%d", k+1); //convert int to string
					msg = strcpy(&error_log[e_i], str);
					e_i += strlen(msg);
					msg = strcpy(&error_log[e_i], ": Not an integer value!\n\0"); //only the \0 of the last error will not been overwritten
					e_i += strlen(msg);
					break;
				}
			}
		}
	}
	
	//if operator = '!' then the line contains a syndax error
	if (inst.op != '!' && !isvar && !isopvar) {
		inst.op = line[0];
		
		for (i = 2; line[i] != ';'; ++i) {
			num[i-2] = line[i];
		}
		num[i-2] = '\0';
		
		inst.val = atoi(num);
	}
	else if (inst.op != '!' && isvar && !isopvar) { //if isvar init the variable
		inst.op = '&';
		inst.val = varc; //this acts as a pointer to the var inside the source program flow
		
		syms[varc+2].var = '!'; //define the end of the symbols table
		syms[varc+2].op = '!';
		
		syms[varc+1].var = '$'; //the result variable
		syms[varc+1].op = '&';
		
		syms[varc].var = line[2]; //log the variable
		syms[varc++].op = '&';
	}
	else if (inst.op != '!' && isopvar && !isvar) { //if isvar init the variable
		inst.op = '%';
		
		switch (line[0]) {
			case '+':
				inst.val = 1;
				break;
			case '-':
				inst.val = 2;
				break;
			case '*':
				inst.val = 3;
				break;
			case '/':
				inst.val = 4;
				break;
		}
		
		syms[varc+2].var = '!'; //define the end of the symbols table
		syms[varc+2].op = '!';
		
		syms[varc+1].var = '$'; //the result variable
		syms[varc+1].op = '&';
		
		syms[varc].op = line[0];
		syms[varc++].var = line[2]; //log the variable
	}
	
	return inst;
}

int optimize(instruction *inst, int int_num, symbol *syms) {
	int i, k, z;
	instruction buff[int_num];

	for (i = 0, z = -1; i < int_num; ++i) { //find the last result *= 0
		if (inst[i].op == '*' && inst[i].val == 0) {
			z = i;
		}
	}

	for (i = z + 1, k = 0; i < int_num; ++i) { //start after the last *= 0
		if (inst[i].op != '!' && !((inst[i].op == '+' || inst[i].op == '-') && inst[i].val == 0) &&
				!((inst[i].op == '*' || inst[i].op == '/') && inst[i].val == 1) ) { //drop dead operations
			buff[k].op = inst[i].op;
			buff[k++].val = inst[i].val;
		}
	}

	for (i = 0; i < int_num; ++i) { //optimize for powers of two
		if (buff[i].op == '*' && is_power_of_2(buff[i].val)) {
			buff[i].op = '<'; // '<' -> res <<= x
			buff[i].val = shift_times(buff[i].val);
		}
		else if (buff[i].op == '/' && is_power_of_2(buff[i].val)) {
			buff[i].op = '>'; // '>' -> res >>= x
			buff[i].val = shift_times(buff[i].val);
		}
	}

	for (i = 0; i < k; ++i) {
		inst[i] = buff[i];
	}
	
	return k;
}

inline int is_power_of_2(int x) {
	if (x == 1 || x == 2 || x == 4 || x == 8 || x == 16 || x == 32 || x == 64 || x == 128 || x == 256 || x == 512 || x == 1024) {
		return 1;
	}
	return 0;
}

int shift_times(int x) {
	int i, k;
	for (i = 1, k = 0; i < x; i *= 2, ++k);
	
	if (i == x) { //if x was a valid power of 2, after the end of the for loop
		return k; //the i must be equal with x, in that case return k
	}
	else {
		return -1; //else return -1 to indicate wrong input
	}
}

void generate_code(instruction *inst, char *code, int int_num, symbol *syms) {
	int i, k, z, j, o = 0; //counters
	int def = 0;
	
	//its operation is line of c code with a var as the lvalue and mathematical proposition as the rvaule 
	char operation[100][1000]; //max 100 operations, 1000 char each
	char tmp[1000];
	
	for (i = 0, z = 0, j = 0; i < int_num; ++z) {
		
		for (k = 0; j < int_num && inst[j].op != '&'; ++j, ++k) {//put int_num start brackets on the left of the operation
			operation[z][k] = '(';
		}
		
		if (syms[z+o].var == '$') { //if only result var is left, put all data in it
			operation[z][k++] = syms[z-1+o].var;
		}
		else { 
			operation[z][k++] = '0';
		}
		++j;
		
		while (k != 0) {
			switch(inst[i].op) {
				case '<':
					operation[z][k++] = '<';
					operation[z][k++] = '<';
					break;
				case '>':
					operation[z][k++] = '>';
					operation[z][k++] = '>';
					break;
				case '&':
					break;
				case '%':
					break;
				default:
					operation[z][k++] = inst[i].op;
					break;
			}
		
			if (inst[i].op != '&' && inst[i].op != '%') {
				sprintf(tmp, "%d", inst[i].val); //convert int to string
				strcpy(&operation[z][k++], tmp);
				k += strlen(tmp) - 1;
				
				operation[z][k++] = ')';
			}
			else if (inst[i].op == '%') {
				switch (inst[i].val) {
					case 1:
						operation[z][k++] = '+';
						break;
					case 2:
						operation[z][k++] = '-';
						break;
					case 3:
						operation[z][k++] = '*';
						break;
					case 4:
						operation[z][k++] = '/';
						break;
				}
				operation[z][k++] = syms[z+o].var; //when we have operation with a var 
				operation[z][k++] = ')';
				
				++o;//and not assingment, we skip the next var in the table
			}
			else {
				operation[z][k] = '\0';
				k = 0;
			}
			++i; //instruction counter
		}
	}
	

	//generate the code
	strcpy(code, "#include <stdio.h>\n#include <stdlib.h>\nint main(void) {\n\tint ");
	i = strlen(code);
	
	def = 0;
	code[i++] = syms[0].var;
	for (j = 1; syms[j].var != '$'; ++j) {
	
		for (k = 0; k < j; ++k) { //check if var already defined
			if (syms[k].var == syms[j].var) {
				def = 1;
				break;
			}
		}
		if (def) { //if so
			def = 0;
			continue;
		}
	
		code[i++] = ',';
		code[i++] = ' ';
		if (syms[j].var != '$') {
			code[i++] = syms[j].var;
		}
		else {
			strcpy(&code[i], "result");
			i = strlen(code);
		}
	}
	code[i++] = ';';
	
	for (j=0,k=0; k < z; ++j) {
		if (syms[j].op != '&') {
			continue;
		}
	
		code[i++] = '\n';
		code[i++] = '\t';
		if (syms[j].var != '$') {
			code[i++] = syms[j].var;
		}
		else {
			strcpy(&code[i], "result");
			i = strlen(code);
		}
		
		code[i++] = ' ';
		code[i++] = '=';
		code[i++] = ' ';
		
		strcpy(&code[i], operation[k++]);
		i = strlen(code);
		
		code[i++] = ';';
	}
	strcpy(&code[i], "\n\tprintf(\"Result = %d\\n\", result);\n}\n");
}


void save_code(char *code, char *fout) {
	FILE *fp;

	if (!(fp = fopen(fout, "w"))) {
		printf("%s\n", strerror(errno));
		exit(-1);
	}
	
	fputs(code, fp); //write the code to the file
	printf("File %s has been created.\n", fout);
	fclose(fp);
}
















