#include <stdio.h>
#include <string.h>
#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "entity.h"



APIHANDLER::APIHANDLER()
{
	root=NULL;
}

APIHANDLER::~APIHANDLER()
{
	APIBLOCK *working = root;
	while(working!=NULL)
	{
		APIBLOCK *huhu = working->next;
		delete working;
		working = huhu;
	}

}

void APIHANDLER::exec(const char *functionname, void *owner)
{
	//std::cout << "Executing function named " << functionname << std::endl;
	APIBLOCK *working = root;
	while(working!=NULL)
	{
		//std::cout << "Comparing " << functionname << " with " << working->name << std::endl;
		//std::cout << "Comparing result: " << strcmp(functionname,working->name) << std::endl;
		if(strcmp(functionname,working->name)==0)
		{
			working->entrypoint(owner);
			return;
		}
		working=working->next;
	}
	//std::cout << "FUNCTION NOT FOUND! " << std::endl;

}

void APIHANDLER::install(const char *functionname,APIFUNCTION function)
{
	if(root == NULL)
	{
		root=new APIBLOCK;
		root->next=NULL;
		strcpy(root->name,functionname);
	//	std::cout << "Installed function named " << root->name << std::endl;
	//	std::cout << "What is root? " << (root==NULL) << std::endl;
		root->entrypoint = function;
		return;
	}
	
	
	
	APIBLOCK *working = root;
	while(working->next!=NULL)
	{
		working=working->next;
		
	}
		
	working->next=new APIBLOCK;
	working->next->next=NULL;
	strcpy(working->next->name,functionname);
	//std::cout << "Installed function named " << working->next->name << std::endl;
	//std::cout << "What is root? " << (root==NULL) << std::endl;
	working->next->entrypoint = function;
	
}








void ENTITY::SetExecutionRate(int cyclespersecond, int tickrate)
{
	if(tickrate==0 || cyclespersecond==-1) execrate=-1;
	else execrate=cyclespersecond/tickrate;

}

void ENTITY::Execute()
{
	if(execrate==-1)
	{
		while(vm.operational) vm.Execute(100000);
		
	}
	else vm.Execute(execrate);
	operational=vm.operational;
}

ENTITY::ENTITY(const char *myname, const char *binary)
{
	execrate=-1;
	operational=1;
	vm.setram(ram,65536,this);
	vm.loadbinary(binary);
	shared = (ATTRIBUTES *)ram;
	strcpy(name,myname);
}

ENTITY::ENTITY()
{
	seconds = 0;
	vm.setram(ram,65536,this);
	shared = (ATTRIBUTES *)ram;
}










static int threads=0;

THREAD::THREAD(char *ram, int sizeram, int startaddress, void *mymaster)
{
	owner=mymaster;
	++threads;
	yield=0;
	next=NULL;
	parent=NULL;
	THREAD::ram=ram;
	THREAD::sizeram=sizeram;
	operational=1;
	fstackpointer=0;
	stackpointer=0;
	pstackpointer=0;
	codepointer=startaddress;
}




int THREAD::ExecObj(int cycles)
{
	static ENTITY *MASTER=(ENTITY *)owner;
	
	//Execute cycles and return how many cycles are left
	//if something like a device poll takes place or whatnot
	if(operational==0) return cycles;
			//Loop until loop is broken by a rule condition
		while(cycles>0)
		{
			
			--cycles;
			#if defined DEBUG
				if(codepointer>=sizeram && codepointer<0)
				{
					std::cout << "ERR: INVALID CODE POINTER, DISABLING" << std::endl;
					operational=0;
					return cycles;
				}
			#endif			



			if(yield>0)
			{
				--yield;
				return cycles;
			}
			//Debug teh pointers
			
			//end of Debug




			switch (*(int *)(ram+codepointer))
			{
				
				case PUSH:
					codepointer+=sizeof(int);
					stack[stackpointer]=*(int *)(ram+codepointer);
					codepointer+=sizeof(int);
					++stackpointer;
					
					#if defined DEBUG
						if(stackpointer>=STACKSIZE)
						{
							std::cout << "ERR: STACK OVERFLOW" << std::endl;
							operational=0;
							return cycles;
						}
					#endif
					
					break;
			

				case FUNCTION:
		
					pstack[pstackpointer].type=RETURNADDRESS;
					pstack[pstackpointer].address=codepointer+sizeof(int);
					++pstackpointer;															
					
					--stackpointer;
					address=stack[stackpointer];
					
					#if defined DEBUG
						if(address<0 || address>(int)(sizeram-sizeof(int)))
						{
							std::cout << "INVALID FUNCTION FRAME POINTER: " << address << std::endl;
							operational=0;
							return cycles;
						}
					#endif
					
					//swapper is set to first byte of the frame
					swapper=address;
					
					//Read in Function Pointer
					address=*(int *)(ram+address);
					//address is set to the code pointer of the function
					
					#if defined DEBUG
						if(address<0 || address>(int)(sizeram-sizeof(int)))
						{
							std::cout << "INVALID FUNCTION POINTER IN FUNCTION FRAME: " << address << std::endl;
							operational=0;
							return cycles;
						}
					#endif
					
					//set code pointer to address
					codepointer=address;
					
					//Start preserving the frame
					preserve=swapper+sizeof(int);
					while(*(int *)(ram+preserve)!=ENDPARAMS)
					{
						preserve+=sizeof(int);	
					}

					address=preserve+sizeof(int);

					done=0;
					workingloc=swapper+sizeof(int);
					while(done==0)
					{
						switch(*(int *)(ram + workingloc))
						{
							case DWORD:
								pstack[pstackpointer].type=DWORD;
								pstack[pstackpointer].dword=*(int *)(ram+address);
								pstack[pstackpointer].address=address;
								++pstackpointer;
								--stackpointer;
								*(int *)(ram+address)=stack[stackpointer];
								address+=sizeof(int);
								break;
							case FLOATER:
								pstack[pstackpointer].type=FLOATER;
								pstack[pstackpointer].floater=*(double *)(ram+address);
								pstack[pstackpointer].address=address;
								++pstackpointer;
								--fstackpointer;
								*(double *)(ram+address)=fstack[fstackpointer];
								address+=sizeof(double);
								break;
							
							case ENDPARAMS:
								done=1;
								break;

							default:
								std::cout << "INVALID DATA TYPE IN FRAME PROTOTYPE" << std::endl;
								operational=0;
								return cycles;
								break;

						}
						workingloc+=sizeof(int);
						

					}
					
					break;
				

				case FRETURN:
					
					done=0;
					while(done==0)
					{
						--pstackpointer;
#if defined DEBUG						
						if(pstackpointer<0)
						{
							std::cout << "FUNCTION FRAME PRESERVER STACK UNDERFLOW!" << std::endl;
							operational=0;
							return cycles;
						}
#endif
						switch(pstack[pstackpointer].type)
						{
							case DWORD:
								
								address=pstack[pstackpointer].address;

								*(int *)(ram+address) = pstack[pstackpointer].dword;
								
								break;
							case FLOATER:
								writedouble.data=pstack[pstackpointer].floater;
								address=pstack[pstackpointer].address;
								for(writeiterate=0;writeiterate<sizeof(double);++writeiterate)
								{
									ram[address+writeiterate]=writedouble.bytes[writeiterate];
								}
								break;
							case BYTE:
								address=pstack[pstackpointer].address;
								ram[address]=pstack[pstackpointer].byte;
								break;

							case RETURNADDRESS:
								codepointer=pstack[pstackpointer].address;
								done=1;
								break;
							default:
								std::cout << "INVALID PRESERVE TYPE: YOU PROBLY SHOULDN'T SEE THIS" << std::endl;
								operational=0;
								return cycles;
								break;

						}

					}
					
					break;
	

				case PRESERVE:
					codepointer+=sizeof(int);
					stackpointer-=2;
					switch(stack[stackpointer+1])
					{
						case DWORD:
							pstack[pstackpointer].dword = *(int *)(ram+stack[stackpointer]);
							pstack[pstackpointer].type=DWORD;
							pstack[pstackpointer].address=stack[stackpointer];
							++pstackpointer;
							break;
						
						default:
							std::cout << "Preserve debug address: " << stack[stackpointer] << std::endl;
							std::cout << "Preserve type " << stack[stackpointer+1] << " not handled yet!" << std::endl;
							operational=0;
							return cycles;
					}

					break;
				
				case RESTORE:
					codepointer+=sizeof(int);
					--pstackpointer;					
					switch(pstack[pstackpointer].type)
					{
						case DWORD:
							*(int *)(ram+pstack[pstackpointer].address) = pstack[pstackpointer].dword;
							break;
						default:
							std::cout << "Restore type " << pstack[pstackpointer].type << " not handled yet!" << std::endl;
							operational=0;
							return cycles;
					}

					break;

				
				case LOAD:
					codepointer+=sizeof(int);
					--stackpointer;
					address=stack[stackpointer];
#if defined DEBUG
					if(address<0 || address>(sizeram-sizeof(int)))
					{
						std::cout << "INVALID LOAD POINTER: " << address << std::endl;
						operational=0;
						return cycles;
					}
#endif
					stack[stackpointer]=*(int *)(ram+address);
					++stackpointer;
					break;
				
				case STORE:
					codepointer+=sizeof(int);
					stackpointer-=2;
					address=stack[stackpointer+1];
#if defined DEBUG
					
					if(address<0 || address>(sizeram-sizeof(int)))
					{
						std::cout << "INVALID STORE POINTER: " << address << std::endl;
						operational=0;
						return cycles;
					}
#endif
					*(int *)(ram+address) = stack[stackpointer];

					break;

				
				
				case CLONE:
					codepointer+=sizeof(int);
					stack[stackpointer]=stack[stackpointer-1];
					++stackpointer;
					break;

				case CONVERT:
					codepointer+=sizeof(int);
					--stackpointer;
					fstack[fstackpointer]=(double)stack[stackpointer];
					++fstackpointer;
					break;
				
				case ADD:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] += stack[stackpointer+1];
					++stackpointer;
					break;
				
				case SUB:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] -= stack[stackpointer+1];
					++stackpointer;
					break;
				
				case MUL:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] *=stack[stackpointer+1];
					++stackpointer;
					break;

				case DIV:
					codepointer+=sizeof(int);
					stackpointer-=2;
					if(stack[stackpointer+1]==0)
					{
						std::cout << "ERR: DIVISION BY 0" << std::endl;
						operational=0;
						return cycles;
					}
					stack[stackpointer] = stack[stackpointer] / stack[stackpointer+1];
					++stackpointer;
					break;
				case AND:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = stack[stackpointer] & stack[stackpointer+1];
					++stackpointer;
					break;
					
				case OR:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = stack[stackpointer] | stack[stackpointer+1];
					++stackpointer;
					break;
				
				case XOR:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = stack[stackpointer] ^ stack[stackpointer+1];
					++stackpointer;
					break;

				case NOT:
					codepointer+=sizeof(int);
					--stackpointer;
					stack[stackpointer]=~stack[stackpointer];
					++stackpointer;
					break;


				case EXP:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = (int) pow(stack[stackpointer], stack[stackpointer+1]);
					++stackpointer;
					break;
				
				case MOD:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = stack[stackpointer] % stack[stackpointer+1];
					++stackpointer;
					break;
				
				case SHR:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = stack[stackpointer] >> stack[stackpointer+1];
					++stackpointer;
					break;

				case SHL:
					codepointer+=sizeof(int);
					stackpointer-=2;
					stack[stackpointer] = stack[stackpointer] << stack[stackpointer+1];
					++stackpointer;
					break;
				


				case CMP:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=0;
					bitflags.cmpbit.equal = (stack[stackpointer] == stack[stackpointer+1]);
					bitflags.cmpbit.greaterthan = (stack[stackpointer] > stack[stackpointer+1]) ;
					bitflags.cmpbit.notequal = (stack[stackpointer] != stack[stackpointer+1]);
					bitflags.cmpbit.lessthan = (stack[stackpointer] < stack[stackpointer+1]);
					bitflags.cmpbit.greaterthan = (stack[stackpointer] > stack[stackpointer+1]);
					bitflags.cmpbit.greaterthanequal = (stack[stackpointer] >= stack[stackpointer+1]);
					bitflags.cmpbit.lessthanequal = (stack[stackpointer] <= stack[stackpointer+1]);
					stack[stackpointer]=bitflags.cmpint;
					++stackpointer;
					break;
				
				case JMP:
					--stackpointer;
#if defined DEBUG
					if(stack[stackpointer]<0 || stack[stackpointer]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer] << std::endl;
						operational=0;
						return cycles;
					}
#endif					
					codepointer=stack[stackpointer];
					break;


				
				case JME:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=stack[stackpointer];
#if defined DEBUG					
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
#endif
					if(bitflags.cmpbit.equal!=0) codepointer=stack[stackpointer+1];
					break;
				
				case JML:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=stack[stackpointer];
#if defined DEBUG	
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
#endif					
					if(bitflags.cmpbit.lessthan!=0) codepointer=stack[stackpointer+1];
					break;


				
				case JMG:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=stack[stackpointer];
#if defined DEBUG					
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
#endif					
					if(bitflags.cmpbit.greaterthan!=0) codepointer=stack[stackpointer+1];
					break;
				
				
				case JLE:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=stack[stackpointer];
#if defined DEBUG					
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
#endif					
					if(bitflags.cmpbit.lessthanequal!=0) codepointer=stack[stackpointer+1];
					break;

				case JGE:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=stack[stackpointer];
#if defined DEBUG					
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
#endif					
					if(bitflags.cmpbit.greaterthanequal!=0) codepointer=stack[stackpointer+1];
					break;
				
				case JNE:
					codepointer+=sizeof(int);
					stackpointer-=2;
					bitflags.cmpint=stack[stackpointer];
#if defined DEBUG
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID JMP POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
#endif					
					if(bitflags.cmpbit.notequal!=0) codepointer=stack[stackpointer+1];
					break;


				case RSTORE:
					codepointer+=sizeof(int);
					stackpointer-=2;
					address=stack[stackpointer];
#if defined DEBUG
					
					if(address<0 || address>(sizeram-sizeof(int)))
					{
						std::cout << "INVALID STORE POINTER: " << address << std::endl;
						operational=0;
						return cycles;
					}
#endif
					*(int *)(ram+address) = stack[stackpointer+1];
					break;

				
				case STOREB:
					codepointer+=sizeof(int);
					stackpointer-=2;
					if(stack[stackpointer+1]<0 || stack[stackpointer+1]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID STOREB POINTER: " << stack[stackpointer+1] << std::endl;
						operational=0;
						return cycles;
					}
					ram[stack[stackpointer+1]] = (char)stack[stackpointer];
					break;
				

				case LOADB:
					codepointer+=sizeof(int);
					--stackpointer;
					if(stack[stackpointer]<0 || stack[stackpointer]>(int)(sizeram-sizeof(int)))
					{
						std::cout << "INVALID LOAD POINTER: " << stack[stackpointer] << std::endl;
						operational=0;
						return cycles;
					}
					stack[stackpointer]=(int)ram[stack[stackpointer]];
					++stackpointer;
					break;
				


				case INCL:
					codepointer+=sizeof(int);
					--stackpointer;
					address=stack[stackpointer];
#if defined DEBUG
					if(address<0 || address>(sizeram-sizeof(int)))
					{
						std::cout << "INVALID INCL POINTER: " << address << std::endl;
						operational=0;
						return cycles;
					}
#endif
					
					stack[stackpointer]=++*(int *)(ram+address);
					++stackpointer;
					break;

				case DECL:
				
					codepointer+=sizeof(int);
					--stackpointer;
					address=stack[stackpointer];
#if defined DEBUG
					if(address<0 || address>(sizeram-sizeof(int)))
					{
						std::cout << "INVALID DECL POINTER: " << address << std::endl;
						operational=0;
						return cycles;
					}
#endif
					stack[stackpointer]=--*(int *)(ram+address);
					++stackpointer;
					break;
									
				
				case DPUSH:
					codepointer+=sizeof(int);
					stack[stackpointer]=*(int *)(ram+codepointer);
					++stackpointer;
					codepointer+=sizeof(int);
					stack[stackpointer]=*(int *)(ram+codepointer);
					++stackpointer;
					codepointer+=sizeof(int);
					
					#if defined DEBUG
						if(stackpointer>=STACKSIZE)
						{
							std::cout << "ERR: STACK OVERFLOW" << std::endl;
							operational=0;
							return cycles;
						}
					#endif
					
					break;


				case POP:
					
					#if defined DEBUG
						
						if(stackpointer<0)
						{
							std::cout << "ERR: STACK UNDERFLOW" << std::endl;
							operational=0;
							return cycles;
						}
					#endif
					
					--stackpointer;
					codepointer+=sizeof(int);
					
					break;



				case API:
					//API PROCESSING
					codepointer+=sizeof(int);
					--stackpointer;
					MASTER->api.exec((char *)(ram + stack[stackpointer]) ,this);
					break;

				

				case SYS:
					//SYSTEM PROCESSING
					codepointer+=sizeof(int);
					--stackpointer;
					switch(stack[stackpointer])
					{


						//INSERT SYSTEM ROUTINES HERE :D


						case sys_write:
							--stackpointer;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_write: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							std::cout << (char *)(ram + stack[stackpointer]);
							break;
						case sys_read:
							stackpointer-=2;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_read: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							std::cin.getline((char *)(ram + stack[stackpointer]),stack[stackpointer+1]);
							break;
						case sys_sort:
							stackpointer-=2;
							base=(int *)&ram[stack[stackpointer]];  //Address of int array
							address=stack[stackpointer+1];  //Sort elements

							while(done>0)
							{
								done=0;
								for(writeiterate=1;writeiterate<address;++writeiterate)
								{
									if(base[writeiterate]<base[writeiterate-1])
									{
										writes=base[writeiterate];
										base[writeiterate]=base[writeiterate-1];
										base[writeiterate-1]=writes;
										++done;
									}
								}
							}

							break;
																			
						case sys_atoi:
							--stackpointer;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_atoi: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							stack[stackpointer]=atoi((char *)(ram + stack[stackpointer]));
							++stackpointer;
							break;

						case sys_itoa:
							stackpointer-=2;
							if(stack[stackpointer+1]<0 || stack[stackpointer+1]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_itoa: " << stack[stackpointer+1] << std::endl;
								operational=0;
								return cycles;
							}
							sprintf((char *)(ram + stack[stackpointer+1]),"%i",stack[stackpointer]);
							break;
						
						case sys_atof:
							--stackpointer;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_atof: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							fstack[fstackpointer]=atof((char *)(ram + stack[stackpointer]));
							++fstackpointer;
							break;

						case sys_ftoa:
							--stackpointer;
							--fstackpointer;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_ftoa: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							sprintf((char *)(ram + stack[stackpointer]),"%f",fstack[fstackpointer]);
							break;
						
						case sys_strlen:
							--stackpointer;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID POINTER FOR sys_write: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							stack[stackpointer]=strlen((char *)(ram + stack[stackpointer]));
							break;

						case sys_putchar:
							--stackpointer;
							std::cout << (char)stack[stackpointer];
							break;
											
						case sys_endl:
							std::cout << std::endl;
							break;
						case sys_szdouble:
							stack[stackpointer]=sizeof(double);
							++stackpointer;
							break;
						case sys_szint:
							stack[stackpointer]=sizeof(int);
							++stackpointer;
							break;

						case sys_thread:
							--stackpointer;
							if(stack[stackpointer]<0 || stack[stackpointer]>=sizeram)
							{
								std::cout << "INVALID CODE POINTER FOR sys_thread: " << stack[stackpointer] << std::endl;
								operational=0;
								return cycles;
							}
							
							AddThread(stack[stackpointer]);
							stack[stackpointer]=threads;
							++stackpointer;
							break;


						case sys_yield:
							--stackpointer;
							yield=stack[stackpointer];
							break;
						
	
					}

					break;


				case EXIT:
#ifdef DEBUG
					std::cout << "Stackpointer: " << stackpointer << std::endl;
#endif
					operational=0;
					return cycles;
					break;
					
					
					
					
					
		//ENTER THE FLOATING POINT MATH :boing:

				case PUSHF:
					
					if(fstackpointer>=STACKSIZE)
					{
						std::cout << "ERR: FSTACK OVERFLOW" << std::endl;
						operational=0;
						return cycles;
					}
				
					codepointer+=sizeof(int);
					
					for(writeiterate=0;writeiterate<sizeof(double);++writeiterate)
					{
						writedouble.bytes[writeiterate]=ram[codepointer+writeiterate];
					}
					
					fstack[fstackpointer]=writedouble.data;
					codepointer+=sizeof(double);
					++fstackpointer;
					break;
								
				case POPF:
					codepointer+=sizeof(int);
					if(fstackpointer<0)
					{
						std::cout << "ERR: FSTACK UNDERFLOW" << std::endl;
						operational=0;
						return cycles;
					}

					
					--fstackpointer;
					break;
				
				case CLONEF:
					codepointer+=sizeof(int);
					fstack[fstackpointer]=fstack[fstackpointer-1];
					++fstackpointer;
					break;
				
				case CONVERTF:
					codepointer+=sizeof(int);
					--fstackpointer;
					stack[stackpointer]=(int)fstack[fstackpointer];
					++stackpointer;
					break;
						


				case ADDF:
					codepointer+=sizeof(int);
					fstackpointer-=2;
					fstack[fstackpointer] = fstack[fstackpointer] + fstack[fstackpointer+1];
					++fstackpointer;
					break;
								
				case SUBF:
					codepointer+=sizeof(int);
					fstackpointer-=2;
					fstack[fstackpointer] = fstack[fstackpointer] - fstack[fstackpointer+1];
					++fstackpointer;
					break;
				
							
				case DIVF:
					codepointer+=sizeof(int);
					fstackpointer-=2;
					if(fstack[fstackpointer+1]==0)
					{
						std::cout << "ERR: DIVISION BY 0" << std::endl;
						operational=0;
						return cycles;
					}
					fstack[fstackpointer] = fstack[fstackpointer] / fstack[fstackpointer+1];
					++fstackpointer;
					break;


				case MULF:
					codepointer+=sizeof(int);
					fstackpointer-=2;
					fstack[fstackpointer] = fstack[fstackpointer] * fstack[fstackpointer+1];
					++fstackpointer;
					break;


				case EXPF:
					codepointer+=sizeof(int);
					fstackpointer-=2;
					fstack[fstackpointer] = pow(fstack[fstackpointer],fstack[fstackpointer+1]);
					++fstackpointer;
					break;
					
				case MODF:
					codepointer+=sizeof(int);
					fstackpointer-=2;
					fstack[fstackpointer] = fmod(fstack[fstackpointer],fstack[fstackpointer+1]);
					++fstackpointer;
					break;

				case SQRTF:
					codepointer+=sizeof(int);
					--fstackpointer;
					fstack[fstackpointer]=sqrt(fstack[fstackpointer]);
					++fstackpointer;
					break;

				case COSF:
					codepointer+=sizeof(int);
					--fstackpointer;
					fstack[fstackpointer]=cos(fstack[fstackpointer]);
					++fstackpointer;
					break;

				case SINF:
					codepointer+=sizeof(int);
					--fstackpointer;
					fstack[fstackpointer]=sin(fstack[fstackpointer]);
					++fstackpointer;
					break;

				case TANF:
					codepointer+=sizeof(int);
					--fstackpointer;
					fstack[fstackpointer]=tan(fstack[fstackpointer]);
					++fstackpointer;
					break;

				
				case CMPF:
					bitflags.cmpint=0;
					codepointer+=sizeof(int);
					fstackpointer-=2;
					bitflags.cmpbit.equal = (fstack[fstackpointer] == fstack[fstackpointer+1]);
					bitflags.cmpbit.greaterthan = (fstack[fstackpointer] > fstack[fstackpointer+1]);
					bitflags.cmpbit.notequal = (fstack[fstackpointer] != fstack[fstackpointer+1]);
					bitflags.cmpbit.lessthan = (fstack[fstackpointer] < fstack[fstackpointer+1]);
					bitflags.cmpbit.greaterthan = (fstack[fstackpointer] > fstack[fstackpointer+1]);
					bitflags.cmpbit.greaterthanequal = (fstack[fstackpointer] >= fstack[fstackpointer+1]);
					bitflags.cmpbit.lessthanequal = (fstack[fstackpointer] <= fstack[fstackpointer+1]);
					stack[stackpointer]=bitflags.cmpint;
					++stackpointer;
					break;
				
				
							
				case LOADF:
					codepointer+=sizeof(int);
					--stackpointer;
					if(stack[stackpointer]<0 || stack[stackpointer]>(int)(sizeram-sizeof(double)))
					{
						std::cout << "INVALID LOAD POINTER: " << stack[stackpointer] << std::endl;
						operational=0;
						return cycles;
					}
					for(writeiterate=0;writeiterate<sizeof(double);++writeiterate)
					{
						writedouble.bytes[writeiterate]=ram[stack[stackpointer]+writeiterate];
					}
					fstack[fstackpointer]=writedouble.data;
					
					
					++fstackpointer;
					break;

				case STOREF:
					codepointer+=sizeof(int);
					--stackpointer;
					if(stack[stackpointer]<0 || stack[stackpointer]>(int)(sizeram-sizeof(double)))
					{
						std::cout << "INVALID STOREF POINTER: " << stack[stackpointer] << std::endl;
						operational=0;
						return cycles;
					}
					--fstackpointer;
					writedouble.data=fstack[fstackpointer];
					for(writeiterate=0;writeiterate<sizeof(double);++writeiterate)
					{
						ram[stack[stackpointer]+writeiterate]=writedouble.bytes[writeiterate];
					}
					break;

				
				
								
				default:
					std::cout << "ERR: INVALID OPCODE (" << (int)ram[codepointer] << ") AT :" << codepointer << std::endl;
					operational=0;
					return cycles;
					break;

			}
			
		}

		return cycles;



	}



void THREAD::AddThread(int startpoint)
	{
		
		THREAD *crawler=this;
		while(1)
		{
			if(crawler->next==NULL)
			{
				crawler->next=new THREAD(ram,sizeram,startpoint,owner);
				if(crawler==NULL)
				{
					std::cout << "UNABLE TO START NEW THREAD, DISABLING" << std::endl;
					return;
				}
				crawler->next->parent=crawler;
				operational=1;
				return;
			}
			crawler=crawler->next;
		}

	}


THREAD::~THREAD()
	{
		--threads;
	}
	


VMOBJ::VMOBJ()
	{
		owner=NULL;
		processes=NULL;
		operational=0;
		sizeram=0;
		ram = NULL;
		
	}

void VMOBJ::setram(char *newram, int newsizeram, void *mymaster)
{
	owner=mymaster;
	ram=newram;
	sizeram=newsizeram;
}


		
void VMOBJ::loadbinary(const char *file)
{
        operational = 0;
		if(file==NULL) return;
		if(ram==NULL) return;
		std::cout << "BOOT " << file;
		FILE *loader=fopen(file,"rb");
		if(loader==NULL)
		{
			std::cout << " MISSING, DISABLING" << std::endl;
			return;
		}
		
       	#define BENDIAN 0
    	#define LENDIAN 1
    	
    	int mendian = BENDIAN;
        union endian {int huhu; char a[sizeof(int)];}; endian bleh;	bleh.huhu=1;
        if(bleh.a[0]==1) mendian = LENDIAN;

        char fendian;
        char szdouble;
        char szint;
        char version[8];
        int numops;
        int startaddress;

        int test;
        test = fread(&fendian,1,1,loader);
        test = fread(&szdouble,1,1,loader);
        test = fread(&szint,1,1,loader);
        test = fread(version,1,8,loader);
        
        if(szdouble!=sizeof(double) || szint!=sizeof(int))
        {
            std::cout << "Data size mismatch, disabling!" << std::endl;
            operational = 0;
        	fclose(loader);
			return;
        }
                

        if(fendian==BENDIAN && mendian==LENDIAN)
        {
            std::cout << "Endian mismatch.  Attempting to fix.  :gulp:" << std::endl;
            std::cout << "Endian in file is big.  This machine is little." << std::endl;
   	        int *endianconvert;
            unsigned char *fakeram;
            
            fakeram = (unsigned char*)&numops;
            endianconvert = (int *)&numops;
            test = fread(&numops,1,sizeof(int),loader);
            *endianconvert = fakeram[3] | (fakeram[2] << 8) | (fakeram[1] << 16) | (fakeram[0] << 24);

            fakeram = (unsigned char*)&startaddress;
            endianconvert = (int *)&startaddress;
            test = fread(&startaddress,1,sizeof(int),loader);
            *endianconvert = fakeram[3] | (fakeram[2] << 8) | (fakeram[1] << 16) | (fakeram[0] << 24);
            std::cout << "Size: " << numops << "  Start: " << startaddress << std::endl;
            if(numops>sizeram)
    		{
	      		std::cout << " TOO LARGE FOR RAM, DISABLING" << std::endl;
    			fclose(loader);
	       		return;
	       	}
	        unsigned int left = numops;
	        int adder = 0;
            fakeram = (unsigned char*)ram;
	        while(left>=sizeof(int))
	        {
	           test = fread(&fakeram[adder],1,numops,loader);
	           endianconvert = (int *)&fakeram[adder];
	           *endianconvert = fakeram[adder+3] | (fakeram[adder+2] << 8) | (fakeram[adder+1] << 16) | (fakeram[adder] << 24);
               adder+=sizeof(int);
	           left-=sizeof(int);
            }
            if(left>0)
            {
                std::cout << "Uhoh, extra garbage." << std::endl;
            }
            
       
        }
        else if(fendian==LENDIAN && mendian==BENDIAN)
        {
            std::cout << "Endian mismatch.  Attempting to fix.  :gulp:" << std::endl;
            std::cout << "Endian in file is little.  This machine is big." << std::endl;
   	        int *endianconvert;
            unsigned char *fakeram;
            
            fakeram = (unsigned char*)&numops;
            endianconvert = (int *)&numops;
            test = fread(&numops,1,sizeof(int),loader);
            *endianconvert = fakeram[0] | (fakeram[1] << 8) | (fakeram[2] << 16) | (fakeram[3] << 24);

            fakeram = (unsigned char*)&startaddress;
            endianconvert = (int *)&startaddress;
            test = fread(&startaddress,1,sizeof(int),loader);
            *endianconvert = fakeram[0] | (fakeram[1] << 8) | (fakeram[2] << 16) | (fakeram[3] << 24);
            std::cout << "Size: " << numops << "  Start: " << startaddress << std::endl;
            if(numops>sizeram)
    		{
	      		std::cout << " TOO LARGE FOR RAM, DISABLING" << std::endl;
    			fclose(loader);
	       		return;
	       	}
	        unsigned int left = numops;
	        int adder = 0;
            fakeram = (unsigned char*)ram;
	        while(left>=sizeof(int))
	        {
	           test = fread(&fakeram[adder],1,numops,loader);
	           endianconvert = (int *)&fakeram[adder];
	           *endianconvert = fakeram[adder] | (fakeram[adder+1] << 8) | (fakeram[adder+2] << 16) | (fakeram[adder+3] << 24);
               adder+=sizeof(int);
	           left-=sizeof(int);
            }
            if(left>0)
            {
                std::cout << "Uhoh, extra garbage." << std::endl;
            }

        
        }
        else if(fendian == mendian)
        {
                      

            test = fread(&numops,1,sizeof(int),loader);
            test = fread(&startaddress,1,sizeof(int),loader);
            if(numops>sizeram)
    		{
	      		std::cout << " TOO LARGE FOR RAM, DISABLING" << std::endl;
    			fclose(loader);
	       		return;
	       	}


            std::cout << "Size: " << numops << "  Start: " << startaddress << std::endl;

	        test = fread(ram,1,numops,loader);
    		if(test!=numops)
	        {
                std::cout << " UNEXPECTED EOF, DISABLING" << std::endl;
    			fclose(loader);
    			return;
	       	}
        
        
        
        }
        else
        {
            std::cout << "Broken endian!  Disabling!" << std::endl;
            operational = 0;
       		fclose(loader);
   			return;
           
        }
             





        int codepointer=startaddress;
		if(codepointer<0 || codepointer>=sizeram)
		{
			std::cout << "INVALID START ADDRESS, DISABLING" << std::endl;
			fclose(loader);
			return;
		}
         

		std::cout << " OK" << std::endl;
		fclose(loader);
		std::cout << "READY" << std::endl;
		processes=new THREAD(ram,sizeram,codepointer, owner);
		if(processes) operational=OPERATE;			
		else std::cout << "UNABLE TO ALLOCATE THREAD, DISABLING" << std::endl;

	}
	
void VMOBJ::RemoveProcess(THREAD *pskill)
	{
		if(pskill==NULL) return;
		if(pskill->parent!=NULL)
		{
			pskill->parent->next=pskill->next;
			if(pskill->next!=NULL)
			{
				pskill->next->parent=pskill->parent;
			}
		}
		else if(pskill->next!=NULL)
		{
			pskill->next->parent=pskill->parent;
		}
		std::cout << "huhu oh no" << std::endl;
		delete pskill;
		

	}


void VMOBJ::KillEntries(THREAD *root)
	{
		
		if(root->next!=NULL) KillEntries(root->next);
		delete root;

	}

int VMOBJ::Execute(int cycles)
	{
		if(operational==0)
		{
			return cycles;
		}
		if(processes==NULL ||processes->operational == 0) 
		{
			operational=0;
			KillEntries(processes);
			return cycles;
		}
		
		
		THREAD *crawler = processes;
		while(1)
		{
			if(crawler==NULL) return cycles;
			
			if(cycles<=0) return 0;
			if(crawler->operational) cycles -= cycles - crawler->ExecObj(cycles);
			crawler=crawler->next;
			
			
		}

		return cycles;
	}
	
















