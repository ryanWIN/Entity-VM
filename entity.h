#define OPERATE 1
#define VMRAM 65536
#define STACKSIZE 16384

//32BIT PROCESSING DEFINES
//Stack
#define PUSH 0
#define POP 1
#define CLONE 2
#define CONVERT 3

//Base Math
#define ADD 4
#define SUB 5
#define MUL 6
#define DIV 7

//Bitwise Logic
#define AND 8
#define OR 9
#define XOR 10
#define NOT 11

//Extended Math and Bitshift
#define EXP 16
#define MOD 17
#define SHR 18
#define SHL 19

//Conditional Processing
//and branching
#define CMP 32
#define JMP 33
#define JME 34
#define JML 35
#define JMG 36
#define JLE 37
#define JGE 38
#define JNE 39
#define FUNCTION 42
#define FRETURN 43

#define DPUSH 44



//Memory routines

#define PRESERVE 46
#define RESTORE 47
#define LOAD 48
#define STORE 49
#define LOADB 50
#define STOREB 51

//Goodies
#define INCL 52
#define DECL 53
#define RSTORE 54


//API, SYS, and EXIT

#define API 61
#define SYS 62
#define EXIT 63


#define ENDPARAMS 0
#define DWORD 1
#define FLOATER 2
#define BYTE 3
#define RETURNADDRESS 4

//FLOATING POINT PROCESSING DEFINES

//Stack
#define PUSHF 64
#define POPF 65
#define CLONEF 66
#define CONVERTF 67

//Base Math
#define ADDF 68
#define SUBF 69
#define DIVF 70
#define MULF 71

//Extended Math
#define EXPF 72
#define MODF 73
#define SQRTF 74
#define COSF 75
#define SINF 76
#define TANF 77

//CMP
#define CMPF 96

//Memory Routines
#define LOADF 97
#define STOREF 98



//Stack swappers

//SYS defines

#define sys_write 1
#define sys_read 2
#define sys_putchar 3
#define sys_endl 4

#define sys_atoi 16
#define sys_itoa 17
#define sys_atof 18
#define sys_ftoa 19


#define sys_strlen 32
#define sys_sort 33


#define sys_szdouble 64
#define sys_szint 65

#define sys_thread 1337
#define sys_yield 1338



class THREAD
{

	public:
	
	union CONVERTDOUBLE
	{
		char bytes[sizeof(double)];
		double data;
	};
	
	struct ALLDATA
	{
		char type;
		int address;
		char byte;
		int dword;
		double floater;
		
	};

	void *owner;
	THREAD *next;
	THREAD *parent;
	CONVERTDOUBLE writedouble;
	unsigned int writeiterate;
	
	//Preserve,Restore
	int writes;
	unsigned int address;
	int functioniterate;
	int swapper;
	int preserve;

	char *ram;
	int *base; //Used for addressing in 32bit
	int sizeram;
	int operational;
	int yield;
		
	int stack[STACKSIZE];
	int stackpointer;
	double fstack[STACKSIZE];
	int fstackpointer;
	ALLDATA pstack[STACKSIZE];
	int pstackpointer;
	int newadd;
	int codepointer;
	int workingloc;
	int done;
	
	

	union evaluation
	{
		
		struct BITEVAL
		{
		
			char equal : 1;
			char notequal : 1;
			char lessthan : 1;
			char greaterthan : 1;
			char greaterthanequal : 1;
			char lessthanequal : 1;
		} cmpbit;
		int cmpint;
	};

	evaluation bitflags;

	THREAD(char *ram, int sizeram, int startaddress, void *mymaster);
	int ExecObj(int cycles);
	void AddThread(int startpoint);
	~THREAD();
		
};

class VMOBJ
{
	public:

	struct BINHEADER
	{
		char endian;
		char version[8];
		char szint;
		char szdouble;
		int numops;
		int	startaddress;
	};

	
	
	void *owner;
	int sizeram;
	char *ram;
	int operational;
	THREAD *processes;
	
	VMOBJ();
	void setram(char *newram, int newsizeram, void *mymaster);
	void loadbinary(const char *file);
	void RemoveProcess(THREAD *pskill);
	void KillEntries(THREAD *root);
	int Execute(int cycles);
	
};





typedef void (*APIFUNCTION)(void *);

struct APIBLOCK
{
	APIBLOCK *next;
	char name[128];
	APIFUNCTION entrypoint;
};

class APIHANDLER
{

	public:

	

	APIBLOCK *root;
	APIHANDLER();
	~APIHANDLER();
	void exec(const char *functionname, void *owner);
	void install(const char *functionname,APIFUNCTION function);
	
};


class ENTITY
{

	//Don't mess with unions, for they are teh powerful.
	public:

#ifdef _MSC_VER
	#pragma pack(1)
#endif

	struct ATTRIBUTES
	{
				
		//Stick the stuff you want in here!
		int x;
		int y;
		double z;
		
#ifdef __GNUC__
	} __attribute__((packed));
#else
	};
#endif
	
#ifdef _MSC_VER
	#pragma pack()	
#endif

	char ram[65536];
	
	ATTRIBUTES *shared;
	VMOBJ vm;
	APIHANDLER api;
	char name[1337];
	int seconds;
	int execrate;
	int operational;
	void Execute();
	void SetExecutionRate(int cyclespersecond, int tickrate);
	ENTITY(const char *myname, const char *binary);
	ENTITY();
	
};

