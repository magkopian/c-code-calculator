#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*error report table*/
char error_log[10000] = "No Errors.\n";
int e_i = 0; //error counter

/*the struct for the instruction token*/
struct instruction {
	char op; //operator
	int val; //value
};

typedef struct instruction instruction;

/*put's instructions in a buffer, separated with semicolons*/
void get_instructions(char *buffer);

/*extracts instructions from buffer*/
int extract_instructions(char *buffer, instruction *inst);

/*return length of instruction*/
inline int llen(char *str);

/*parse line and extract instruction tokens*/
instruction parse_line(char *line, int line_len, int k);

/*execute an instruction*/
int do_operation(char op, int val);

/*optimize the instructions and return number of instructions after optimization*/
int optimize(instruction *inst, int int_num);

/*check if an integer is power of 2*/
inline int is_power_of_2(int x);

/*return shift equiv for power of 2 div or mul*/
int shift_times(int x);

/*generate the c code based on the instruction tokens*/
void generate_code(instruction *inst, char *code, int int_num);

/*generate c file with the code*/
void save_code(char *code);

int main(void) {
	char buffer[10000];
	int int_num;
	int i;
	int num;
	instruction inst[1000];
	char code[2000];
	
	get_instructions(buffer);
	int_num = extract_instructions(buffer, inst);
	
	if (int_num == -1) {
		puts("No input. Nothing to do.");
	}
	else {
		puts(error_log); //print error log
		int_num = optimize(inst, int_num); //instruction opimization
		generate_code(inst, code, int_num); //generate c code
		save_code(code);
		
		/*print tokens (for debugging)
		putchar('\n');
		for (i = 0; i < int_num; ++i) {
			printf("%c %d\n", inst[i].op, inst[i].val);
		}
		putchar('\n');
		putchar('\n');
		*/
		
		//for (i = 0; i < int_num; ++i) {
		//	if (inst[i].op != '!') {
		//		num = do_operation(inst[i].op, inst[i].val);
		//	}
		//}
		
		//printf("Result: %d\n", num);
	}
	
	return 0;
}

void get_instructions(char *buffer) {
	int i = 0;
	int line_len;
	
	
	for(i = 0; ; i += line_len) {
		fgets(&buffer[i], 1000, stdin); //max 1000 chars each line
		line_len = strlen(&buffer[i]);
		
		//discard null lines
		if (line_len == 1) {
			line_len = 0;
			continue;
		}
		
		//detect end char
		if (buffer[i + line_len - 2] == '=' && line_len == 2) {
			buffer[i + line_len - 2] = '\0';
			break;
		}
		
		//put delimiter at the end of line
		buffer[i + line_len - 1] = ';';
	}
}

int extract_instructions(char *buffer, instruction *inst) {
	int i = 0;
	int k = 0;
	int line_len = 0;
	
	if (strlen(buffer) == 0) return -1; //error NULL buffer
	
	for (i = 0, k = 0; buffer[i] != '\0'; i += line_len + 1, ++k) {
		line_len = llen(&buffer[i]);
		
		inst[k] = parse_line(&buffer[i], line_len, k);
	}
	
	return k; //return num of instructions
}

inline int llen(char *str) {
	int i;
	for (i = 0; str[i] != ';'; ++i);
	return i;
}

instruction parse_line(char *line, int line_len, int k) {
	instruction inst;
	int i;
	char num[100];
	char *msg;
	char str[20];
	
	inst.op = '+'; //int operator
	
	if (line[0] != '+' && line[0] != '-' && line[0] != '*' && line[0] != '/') {
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
	
	//if operator = '!' then the line contains a syndax error
	if (inst.op != '!') {
		inst.op = line[0];
		
		for (i = 2; line[i] != ';'; ++i) {
			num[i-2] = line[i];
		}
		num[i-2] = '\0';
		
		inst.val = atoi(num);
	}
	
	return inst;
}

int do_operation(char op, int val) {
	int static num = 0;
	
	switch(op) {
		case '+':
			num += val;
			break;
		case '-':
			num -= val;
			break;
		case '*':
			num *= val;
			break;
		case '/':
			num /= val;
			break;
	}
	
	return num;
}

int optimize(instruction *inst, int int_num) {
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


	for (i = 0; i < int_num; ++i) { //optimize res + 1 and res - 1 to ++res and --res
		if (buff[i].op == '+' && buff[i].val == 1) {
			buff[i].op = '['; // '[' -> ++res
		}
		else if (buff[i].op == '-' && buff[i].val == 1) {
			buff[i].op = ']'; // ']' -> --res
		}
		else if (buff[i].op == '*' && is_power_of_2(buff[i].val)) {
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

void generate_code(instruction *inst, char *code, int int_num) {
	int i, k, z;
	char operation[1000];
	char tmp[1000];
	char prebuff[1000];
	
	for (i = 0, k = 0; i < int_num; ++i) {//put int_num brackets on the left
		if (inst[i].op != '[' && inst[i].op != ']') {
			operation[k++] = '(';
		}
	}
	operation[k++] = '0';
	
	for (i = 0, z = 0; i < int_num; ++i) {
		
		switch(inst[i].op) {
			case '<':
				operation[k++] = '<';
				operation[k++] = '<';
				break;
			case '>':
				operation[k++] = '>';
				operation[k++] = '>';
				break;
			case '[':
				strcpy(&prebuff[z], "\n\t++result;");
				z = strlen(prebuff);
				break;
			case ']':
				strcpy(&prebuff[z], "\n\t--result;");
				z = strlen(prebuff);
				break;
			default:
				operation[k++] = inst[i].op;
				break;
		}
		
		if (inst[i].op != ']' && inst[i].op != '[') {
			sprintf(tmp, "%d", inst[i].val); //convert int to string
			strcpy(&operation[k++], tmp);
			k += strlen(tmp) - 1;
				
			operation[k++] = ')';
		}
	}
	prebuff[z] = '\0';
	operation[k] = '\0';

	//generate the code
	strcpy(code, "#include <stdio.h>\n#include <stdlib.h>\nint main(void) {\n\tint result = ");
	i = strlen(code);
	strcpy(&code[i], operation);
	i = strlen(code);
	strcpy(&code[i], ";");
	i = strlen(code);
	strcpy(&code[i], prebuff);
	i = strlen(code);
	strcpy(&code[i], "\n\tprintf(\"Result = %d\\n\", result);\n}\n");
}


void save_code(char *code) {
	FILE *fp;
	int i;
	char fname[256];
	
	//ask user for a filename
	puts("Give name for the output file:");
	fgets(fname, 256, stdin);
	i = strlen(fname);
	fname[i-1] = '\0'; //remove \n char

	if (!(fp = fopen(fname, "w"))) {
		printf("Can't create file with name %s.\n", fname);
		exit(-1);
	}
	
	fputs(code, fp); //write the code to the file
	printf("File %s has been created.\n", fname);
	fclose(fp);
}

















