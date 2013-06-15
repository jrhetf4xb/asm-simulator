#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  File name of the source file to run  */
#define FILE_NAME "asm.txt"
/*  The error message displayed when the file could not be found/opened  */
#define ERR_FILE_OPEN "Error: could not open \"asm.txt\"!"
/*  Maximum number of registers  */
#define MAX_REGISTERS 8
/*  Length of each register's name in characters  */
#define MAX_REG_NAME_LENGTH 4
/*  Maximum source file length in line length and line count  */
#define MAX_LINE_LENGTH 20
#define MAX_PROG_LENGTH 100
/*	Maximum operations provided by the assembler  */
#define MAX_OPCODES 9
/*  Maximum operation name length in characters (i.e. JMP is 3 chars long)  */
#define MAX_OPCODE_NAME_LENGTH 3

/* 	Function-pointer prototype for all the operations used by our assembler  */
typedef int (*opcode_func)(char *, char *, char *);

int *name2regpt(char *name);

int load_program(void);
int execute_program(void);

int exec_instruction(char *);
int add(char *, char *, char *);
int sub(char *, char *, char *);
int shl(char *, char *, char *);
int shr(char *, char *, char *);
int set(char *, char *, char *);
int and(char *, char *, char *);
int or(char *, char *, char *);
int jump(char *, char *, char *);
int prt(char *, char *, char *);

/* 	A special register, the instruction pointer!  */
unsigned int INSP = 0;
/*  All the registers used by our assembler. Initially will contain the value of 0  */
int REG[MAX_REGISTERS] = {0}; /* The compiler will fill the rest of the array with zeros */
/*  Pointers to our registers for easy access  */
int *REG_PTR[] = { &REG[0], &REG[1], &REG[2], &REG[3], &REG[4], &REG[5], &REG[6], &REG[7] };
/*  Names of the registers as they would appear in the source file (asm.txt)  */
char *REG_NAME[] = { "REG0", "REG1", "REG2", "REG3", "REG4", "REG5", "REG6", "REG7" };

/* 	The whole program after being loaded  */
char SOURCE[MAX_PROG_LENGTH][MAX_LINE_LENGTH];
/*	The length of the program in lines  */
int  PROG_LENGTH = 0;

char *OPCODE_STR[] = { "ADD", "SUB", "SHL", "SHR", "SET", "AND", "OR", "J", "PRT" };
opcode_func OPCODE_FUNC[] = { &add, &sub, &shl, &shr, &set, &and, &or, &jump, &prt };

/*-----------------------------------------------------------------------------------------------*/

int main(int argc, char *argv[]) {
	/*  Firstly load our program...  */
	if (load_program() < 0) {
		printf(ERR_FILE_OPEN);
		exit(1);
	}
	/*	Then execute it!  */
	if (execute_program() < 0) {
		printf("Execution error!\n");
		exit(1);
	}

	return 0;
}

/*-----------------------------------------------------------------------------------------------*/

int load_program(void) {
	FILE *fp;
	
	fp = fopen(FILE_NAME, "r");
	if (fp == NULL) {
		/* 	Most likely the file is not in this directory  */
		return -1;
	}
	while(fgets(&SOURCE[PROG_LENGTH][0], MAX_LINE_LENGTH, fp) != NULL) {
		++PROG_LENGTH;
    }
	
	
	return 0;
}

int execute_program(void) {
	INSP = 0;
	
	while (INSP < PROG_LENGTH) {
		if (exec_instruction(&SOURCE[INSP][0]) < 0) {
			return -1;
		}
	}
	
	printf("Program finished executing successfully.\n");
	
	return 0;
}


int exec_instruction(char *instruction) {
	/* 	Variables used to extract operation and arguments  */
    char *opcode = NULL;
    char *arg1 = NULL;
    char *arg2 = NULL;
		
	/* 	Delimiters used when tokenizing the instruction  */
    char delimiters[] = " " "\n" "\r";  
    char *token = NULL;  
	
	int i = 0;
	int mark = 0;
	
	int bool_is_j = 0;
	int bool_is_prt = 0;

	/* 	Make a copy of the instruction, otherwise we would be modifying the source itself  */
	char *instr = calloc(sizeof(char) * MAX_LINE_LENGTH, sizeof(char));
	if (instr == NULL) {
		printf("Error: could not allocate memory for the assembler!\n");
		return -1;
	}
	strncpy(instr, instruction, MAX_LINE_LENGTH);
	
	/*	Tokenize the string  */
	token = strtok(instr, delimiters);
	
	for (i = 0; i < MAX_OPCODES; ++i) {
		if (!strncmp(token, OPCODE_STR[i], MAX_OPCODE_NAME_LENGTH)) {
			opcode = OPCODE_STR[i];
			break;
		}
	}
	
	if (opcode == NULL) {
		printf("Error: unknown operation: (LINE %d) %s\n", INSP, instr);
		mark = -1;
	} else if (!strncmp(opcode, OPCODE_STR[7], MAX_OPCODE_NAME_LENGTH)) {
		bool_is_j = 1;
	} else if (!strncmp(opcode, OPCODE_STR[8], MAX_OPCODE_NAME_LENGTH)) {
		bool_is_prt = 1;
	}
	
	arg1 = strtok(NULL, delimiters);
	if (bool_is_j) {
		mark = jump(NULL, arg1, NULL);
	} else if (bool_is_prt) {
		mark = prt(NULL, arg1, NULL);
	} else {
		arg2 = strtok(NULL, delimiters);
		mark = (*OPCODE_FUNC[i])(NULL, arg1, arg2);
	}
	
	free(instr);
	return mark;
}

/* 	The following function takes a string (presumably a valid register name) 
	and returns a corresponding pointer. Returns -1 if an invalid name was passed.  */
int *name2regpt(char *name) {
	int i = 0;

	while (i < MAX_REGISTERS) {
		if (!strncmp(name, REG_NAME[i], MAX_REG_NAME_LENGTH)) {
			return REG_PTR[i];
		}
		++i;
	}
	
	return (int *)-1;
}


int add(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = *reg1 + atoi(arg2);
	} else {
		*reg1 = *reg1 + *reg2;
	}
		
	++INSP;
	return 0;
}

int sub(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = *reg1 - atoi(arg2);
	} else {
		*reg1 = *reg1 - *reg2;
	}
		
	++INSP;
	return 0;
}

int shl(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = *reg1 << atoi(arg2);
	} else {
		*reg1 = *reg1 << *reg2;
	}
		
	++INSP;
	return 0;
}

int shr(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = *reg1 >> atoi(arg2);
	} else {
		*reg1 = *reg1 >> *reg2;
	}
		
	++INSP;
	return 0;
}

int set(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = atoi(arg2);
	} else {
		*reg1 = *reg2;
	}
		
	++INSP;
	return 0;
}

int and(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = *reg1 & atoi(arg2);
	} else {
		*reg1 = *reg1 & *reg2;
	}
		
	++INSP;
	return 0;
}

int or(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	int *reg2 = name2regpt(arg2); 
	
	if (reg1 == (int *)-1) {
		return -1;
	}
	
	if (reg2 == (int *)-1) {
		*reg1 = *reg1 | atoi(arg2);
	} else {
		*reg1 = *reg1 | *reg2;
	}
		
	++INSP;
	return 0;
}

int jump(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	
	if (reg1 == (int *)-1) {
		INSP = atoi(arg1);
	} else {
		return -1;
	}
	
	return 0;
}

int prt(char *opcode, char *arg1, char *arg2) {
	int *reg1 = name2regpt(arg1);
	
	if (reg1 == (int *)-1) {
		printf("%s\n", arg1);
	} else {
		printf("%d\n", *reg1);
	}
		
	++INSP;
	return 0;
}
