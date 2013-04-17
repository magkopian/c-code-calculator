#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char error_log[10000] = "No Errors.\n";
int e_i = 0;

struct instruction {
	char op; //operator
	int val; //value
};

typedef struct instruction instruction;

/*put's instructions in a buffer, separated with semicolons*/
void get_instructions(char *buffer);

/*extracts instructions from buffer*/
int extract_instructions(char *buffer, instruction *inst);

int llen(char *str);

instruction parse_line(char *line, int line_len, int k);

int do_operation(char op, int val);

int main(void) {
	char buffer[10000];
	int int_num;
	int i;
	int num;
	instruction inst[1000];
	
	get_instructions(buffer);
	int_num = extract_instructions(buffer, inst);
	
	if (int_num == -1) {
		puts("No input. Nothing to do.");
	}
	else {
		puts(error_log);
	
		for (i = 0; i < int_num; ++i) {
			if (inst[i].op != '!') {
				num = do_operation(inst[i].op, inst[i].val);
			}
		}
		
		printf("Result: %d\n", num);
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

int llen(char *str) {
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

