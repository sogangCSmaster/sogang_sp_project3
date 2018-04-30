#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <math.h>

#define OPTXTDIR	"opcode.txt"
#define OPHASHSIZE	20
#define INPUTSIZE	80
#define COMMANDOPTION	4
#define MEMORYSIZE	16*65536
#define MEMORYLINE	16
#define ASCIISET	256	
#define NUMSYS		16	
#define SYMHASHSIZE	100
#define OBJLINEMAX	60	
#define SYMLENSIZE	6 
#define REGNUM		10 

/***
  History 저장 구조체
	his : 단어셋
 ***/
typedef struct __HISTORY{
	char his[50];
	struct __HISTORY *next;
} __history;

/***
  OPCODE 저장 구조체
	mn : mnemonic
	cd : code
  	fm
 ***/
typedef struct __OPCODE{
	int cd;
	char mn[10];
	char fm[10];
	struct __OPCODE *next;
} __opcode;

/***
  ASSEMBLY CODE 저장 구조체
  	line : 줄
	inputOrig : 라인 원본 입력
	input : 라인을 나눌 입력
	lbf : label 앞
	lbr : label 뒤
	op : 명령어 셋
	fmt : format구분
	adr : addressing mode
	loc : loc 값
	obc : object code 값
 */
typedef struct __ASMCODE {
	int line;
	char *inputOrig;
	char *input;
	char *lbf;
	char *lbr;
	char *op;
	int fmt;
	int adr;
	unsigned int loc;
	char* obc;
	struct __ASMCODE *next;
} __asmcode;

/***
  SYMBOL TABLE 저장 구조체
  	symbol : symbol의 이름
	loc : loc 값
 ***/
typedef struct __SYMTAB {
	char *symbol;
	unsigned int loc;
	char type; // proj 3
	unsigned int len; // prog 3
	struct __SYMTAB *next;
} __symtab;

typedef struct __OBJCODE {
	int line;
	char type;
	unsigned int adr;
	unsigned int len;
	char *code;
	struct __OBJCODE *next;
} __objcode;

/***
  DIRECTIVE 저장 구조체
  	dr : directive 명령어
	size : 명령어에 따른 BYTE 수
 ***/
struct __DIRECTIVES {
	char dr[6];
	int size;
};

// 미리 설정한 directive 명령어
struct __DIRECTIVES directives[] = {
	{"RESB", 1}, {"RESW", 3}, {"RESD", 6}, {"RESQ", 12}, 
	{"BYTE", 1}, {"WORD", 3}, {"DWORD", 6}, {"QWORD", 12}
};


/***
  REGISTERS 저장 구조체
  	mn : mnemonic
	value : 저장 값
 ***/
struct __REGISTERS {
	char mn[3];
	unsigned int value;
};

// 미리 설정한 register 셋
struct __REGISTERS registers[REGNUM] = {
	{"A"}, {"X"}, {"L"}, {"B"}, {"S"},
	{"T"}, {"F"}, {""}, {"PC"}, {"SW"}
};


/***
  BREAKPOINT 저장 구조체
  	adr : 주소값
 ***/
typedef struct __BREAKPOINT {
	unsigned long adr;
	struct __BREAKPOINT *next;
} __breakpoint;

void str_lwr(char *);
void str_upr(char *);
void help_print(void);
int str_icmp(char *, char *);
void dir_print(char *);
void history_print(__history *);
void history_free(__history *);
void memory_dump(unsigned char *, long, long);
int opcode_hashFunction(char *);
void memory_edit(unsigned char *, long, unsigned char);
void opcode_read(__opcode **);
void history_input(__history **, char *);
void opcode_print(__opcode **);
void memory_reset(unsigned char *);
unsigned int assemble_pass1(__asmcode *, __symtab **, __opcode **);
void memory_fill(unsigned char *, long, long, unsigned char);
void opcode_free(__opcode **);
__symtab* symtab_search(__symtab **, char *);
int type_print(char *);
__opcode* opcode_mnemonio(__opcode **, char *);
int assemble_objPrint(__asmcode *, __symtab **, char *, unsigned int);
void assemble_free(__asmcode *);
unsigned int symtab_hashFunction(char *);
int symbol_printCmp(const void *, const void *);
int assemble_read(__asmcode *, char *);
int assemble_pass2(__asmcode *, __symtab **, __opcode **);
void symtab_input(__symtab **, char *, unsigned int);
void symbol_print(__symtab **);
int progaddr_set(char*, long*);
int assemble_lstPrint(__asmcode *, char *);
int loader_loadmapPrintCmp(const void *, const void *);
int loader_read(__objcode**, char*, int);
void symtab_free(__symtab **);
int loader_pass1(__objcode**, __symtab **, long, int);
void loader_free(__objcode**, int);
long loader_loadmapPrint(__symtab**);
unsigned char memory_find(unsigned char*, long);
int loader_pass2(__objcode**, __symtab **, long, int, unsigned char*);
int run_process(unsigned char *, __breakpoint *, __opcode **, long, long *, long, int);
void run_register_store(char* , unsigned char*, int);
void run_register_input(char *, unsigned int);
unsigned int run_register_value(char *);
void run_register_print(void);
__opcode* opcode_operation(__opcode **, int);
void bp_input(__breakpoint *, unsigned long);
void run_register_reset(void);
void bp_print(__breakpoint *);
void bp_clear(__breakpoint *);
