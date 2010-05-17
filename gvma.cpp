#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>

#define FILENOTFOUND 1
#define NOPARAMS 2
#define FILEEMPTY 3


#define EMPTY 0
#define TOKENNUM 1
#define TOKENALPH 2
#define TOKENLABEL 3
#define TOKENCOMMENT 4
#define TOKENADD 5
#define TOKENSTRING 6
#define TOKENFLOAT 7
#define TOKENCHAR 8

#define ERRALPHINNUM -1
#define ERRCOLON -2
#define ERRMINUS -3
#define ERRCOMMENT -4
#define ERRINVALIDCHAR -5
#define ERRSTRING -6
#define ERRPERIOD -7

#define KEYEMPTY 0
#define KEYINT 1
#define KEYBYTE 2
#define KEYFLOAT 3


int ERRMSG = 0;
int nkeywords=0;


union CONVERTDOUBLE
{
	char bytes[sizeof(double)];
	double data;
};

union CONVERTINT
{
	char bytes[sizeof(int)];
	int data;
};



class KEYWORD
{
public:
	char *token;
	int type;
	char bvalue;
	int value;
	double fvalue;

	KEYWORD()
	{
		++nkeywords;
		type=KEYEMPTY;
		
		token=NULL;
		value=0;
		fvalue=0;
		bvalue=0;
	}
	
	KEYWORD(char *token, int value)
	{
		++nkeywords;
		type=KEYINT;
		
		KEYWORD::token=token;
		KEYWORD::value=value;
		fvalue=0;
		bvalue=0;
	}

	KEYWORD(char *token, double fvalue)
	{
		
		++nkeywords;
		type=KEYFLOAT;

		KEYWORD::token=token;
		KEYWORD::fvalue=fvalue;
		value=0;
		bvalue=0;
	}
	
	KEYWORD(char *token, char bvalue)
	{
		++nkeywords;
		type=KEYBYTE;
		
		KEYWORD::token=token;
		KEYWORD::bvalue=bvalue;
		
		value=0;
		fvalue=0;
	}

};	

class LUSAGE
{
	public:
		int point;
		LUSAGE *next;

	LUSAGE(int point)
	{
		LUSAGE::point=point;
		next=NULL;
	}
	~LUSAGE()
	{
		if(next!=NULL) delete next;
	}
};


class LFIXUP
{

	public:
		LUSAGE *fixes;
		char token[256];
		int address;
		LFIXUP *next;

	LFIXUP(char *token,int address)
	{
		next=NULL;
		fixes=NULL;		
		LFIXUP::address=address;
		strcpy(LFIXUP::token,token);
	}
	~LFIXUP()
	{
		if(fixes!=NULL) delete fixes;
		if(next!=NULL) delete next;
	}
};


class LABELFIX
{
	public:
		LFIXUP *listfixup;
		
	LABELFIX()
	{
		listfixup=NULL;
	}

	
	void addpoint(LFIXUP *fix, int pointtoadd)
	{
		if(fix==NULL) return;
		
		if(fix->fixes==NULL)
		{
			fix->fixes=new LUSAGE(pointtoadd);
			return;
		}
		
		
		
		LUSAGE *working=fix->fixes;
		while(1)
		{
			if(working->next==NULL)
			{
				working->next=new LUSAGE(pointtoadd);
				return;
			}
			working=working->next;
		}
	}
	
	
	int fixup(unsigned char *code)
	{

		
		if(listfixup==NULL) return 0;

		CONVERTINT writeint;
		unsigned int writeiterate;
		LFIXUP *working=listfixup;
		LUSAGE *crawler=NULL;
		
		while(1)
		{

			if(working->fixes!=NULL)
			{
				crawler=working->fixes;
				while(1)
				{
					if(working->address==-1)
					{
						std::cout << "Address used without label : " << working->token << std::endl;
						return -1;
					}

					writeint.data=working->address;
					for(writeiterate=0;writeiterate<sizeof(int);++writeiterate)
					{
						code[crawler->point+writeiterate]=writeint.bytes[writeiterate];
					}
					

					if(crawler->next==NULL) break;
					crawler=crawler->next;
				}
			}

			if(working->next==NULL) return 0;
			working=working->next;

		}






	}
	
	int findaddress(char *label)
	{

		LFIXUP *working=listfixup;
		if(working==NULL) return -1;
		while(1)
		{
			if(strcmp(working->token,label)==0)
			{
				return working->address;

			}
			if(working->next==NULL) return -1;
			working=working->next;
		}


	}
	int addusage(char *label, int address)
	{

		//Get rid of the : and &
		char *token=label+1;

		//Search for a label that matches our address
		//If not found, create a new label with
		//address of -1 for use later :D


		//If we have no entries then
		//make one and stop :D
		if(listfixup==NULL)
		{
			listfixup=new LFIXUP(token,-1);
			addpoint(listfixup,address);
			return 0;
		}	
		
		//Search for an instance for fixup
		//If not found, add a new entry
		LFIXUP *working=listfixup;
		
		
		while(1)
		{
			if(strcmp(working->token,token)==0)
			{
				//Found our label, let's add a point
				if(working->address==-1)
				{
					addpoint(working,address);
					return 0;
				}
				else return working->address;

			}
			
			
			if(working->next==NULL)
			{
				//Make a new entry since one is not found
				working->next = new LFIXUP(token,-1);
				addpoint(working->next,address);
				return 0;

			}
			working=working->next;
		
		}

		
		

	}
	
	
	int addlabel(char *label,int address)
	{
		//Get rid of the : and *
		char *token=label+1;
		
		//If we have no entries then just make this label the first one
		//and exit addlabel :)
		if(listfixup==NULL)
		{
			listfixup =new LFIXUP(token, address);
			return 0;
		
		}	
		
		
		//Search for an instance for fixup
		//If not found, add a new entry
		LFIXUP *working=listfixup;
		while(1)
		{
			
			if(strcmp(working->token,token)==0)
			{
				//If our label has been referenced, but has no address
				//reference our label with this address
				if(working->address==-1)
				{
					working->address=address;
					return 0;
				}
				else return -1;
			}
			if(working->next==NULL) break;
			working=working->next;
		}

		working->next = new LFIXUP(token,address);
		return 0;

	}


	~LABELFIX()
	{
		if(listfixup!=NULL) delete listfixup;
	
	}
};








class TOKEN
{
	
	public:
		
		char *token;
		int type;
		int line;
		TOKEN *next;
	
	TOKEN(int line)
	{
		token = new char[256];
		if(token == NULL) std::cout << "Fatal oopsies, token unable to grab 256 bytes!" << std::endl;
		TOKEN::line=line;
		next=NULL;
		token[0]=0;
		type=EMPTY;
	}
	TOKEN()
	{
		token = new char[256];
		if(token == NULL) std::cout << "Fatal oopsies, token unable to grab 256 bytes!" << std::endl;
		line=0;		
		next=NULL;
		token[0]=0;
		type=EMPTY;
	}
	
	void getbigplease()
	{
		//This is only a request :D
		char *huhu = new char[65536];
		if(huhu != NULL)
		{
			
			memcpy(huhu,token,256);
			delete[] token;
			token=huhu;
		}
		
		else
		{
			std::cout << "Danger Will Robinson!  String can only be 256 chars :D" << std::endl;
		}



	}
	
	
	~TOKEN()
	{
		if(token!=NULL) delete[] token;
		if(next!=NULL) delete next;
	}
};


int findkeyword(char *word, KEYWORD words[], int nkeywords)
{
	int pos=0;
	while(pos<nkeywords)
	{
		if(strcmp(words[pos].token,word)==0) return pos;
		++pos;
	}
	return -1;
}


TOKEN *traverse(TOKEN *root)
{

	if(root->next==NULL) return root;
	else return traverse(root->next);

}

int findbytes(TOKEN *root,KEYWORD words[])
{
	TOKEN *working=root;
	int counter=0;
	int test;
	do
	{
		switch(working->type)
		{
			case TOKENCOMMENT:
			case TOKENLABEL:
			case EMPTY:
				break;
			
			case TOKENALPH:
				test=findkeyword(working->token,words,nkeywords);
				if(test!=-1)
				{
					
					switch (words[test].type)
					{
						case KEYEMPTY:
						break;
						
						case KEYBYTE:
						counter+=sizeof(int);
						break;
						
						case KEYINT:
						counter+=sizeof(int);
						break;
						
						case KEYFLOAT:
						counter+=sizeof(double);
						break;
					}
				}
						
				break;
			case TOKENCHAR:
				counter+=((strlen(working->token)+(sizeof(int)-1))/sizeof(int))*sizeof(int);
				break;
			case TOKENSTRING:
				counter+=((strlen(working->token)+sizeof(int))/sizeof(int))*sizeof(int);
				break;	
			case TOKENADD:
			case TOKENNUM:
				counter+=sizeof(int);
				break;
			case TOKENFLOAT:
				counter+=sizeof(double);
				break;
			
		}
		working=working->next;
	} while(working!=NULL);
	return counter;

}


void converttokens(TOKEN *root, KEYWORD words[], int nkeywords, char *outputfile, int endian)
{
	
	CONVERTINT writeint;
	CONVERTDOUBLE writedouble;
	unsigned int writeiterate;
	LABELFIX fixerupper;
	TOKEN *working=root;
	int test=findbytes(root,words);
	if(test<1)
	{
		std::cout << "No usable tokens found in file!" << std::endl;
		return;
	}
	unsigned char *ops = new unsigned char[test];
	if(ops==NULL)
	{
		std::cout << "OUT OF MEMORY" << std::endl;	
		return;

	}
	std::cout << "Found " << test << " bytes" << std::endl;
	int opspos=0;
	
	do
	{
		switch (working->type)
		{
			case EMPTY:
				break;
			case TOKENALPH:
				test=findkeyword(working->token,words,nkeywords);
				if(test!=-1)
				{
					
					switch (words[test].type)
					{
						case KEYEMPTY:
						break;
						
						case KEYBYTE:
						writeint.data=(int)words[test].bvalue;
						for(writeiterate=0;writeiterate<sizeof(int);++writeiterate)
						{
							ops[opspos+writeiterate]=writeint.bytes[writeiterate];
						}	
						opspos+=sizeof(int);
						break;
						
						case KEYINT:
						writeint.data=words[test].value;
						for(writeiterate=0;writeiterate<sizeof(int);++writeiterate)
						{
							ops[opspos+writeiterate]=writeint.bytes[writeiterate];
						}	
						opspos+=sizeof(int);
						break;

						case KEYFLOAT:
						writedouble.data=words[test].fvalue;
						for(writeiterate=0;writeiterate<sizeof(double);++writeiterate)
						{
							ops[opspos+writeiterate]=writedouble.bytes[writeiterate];
						}	
						opspos+=sizeof(double);
						break;

					}
					
					
				}
				else
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Keyword not found - " << working->token << std::endl;
					delete[] ops;
					return;
				}
				break;
			case TOKENNUM:
				if(strcmp(working->token,"-")==0)
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Number missing from '-'"<< std::endl;
					delete[] ops;
					return;

				}
				writeint.data=atoi(working->token);
				for(writeiterate=0;writeiterate<sizeof(int);++writeiterate)
				{
					ops[opspos+writeiterate]=writeint.bytes[writeiterate];
				}	
				opspos+=sizeof(int);
				break;
			case TOKENFLOAT:
				if(strcmp(working->token,"-")==0)
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Number missing from '-'"<< std::endl;
					delete[] ops;
					return;

				}
				if(strcmp(working->token,".")==0)
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Number missing from '.'"<< std::endl;
					delete[] ops;
					return;

				}
				
				writedouble.data=atof(working->token);
				for(writeiterate=0;writeiterate<sizeof(double);++writeiterate)
				{
					ops[opspos+writeiterate]=writedouble.bytes[writeiterate];
				}	
				opspos+=sizeof(double);
				break;
			
			case TOKENSTRING:
				if(strlen(working->token)>0)
				{
					strcpy((char *)(ops+opspos),working->token);
					opspos+=((strlen(working->token)+sizeof(int))/sizeof(int))*sizeof(int);
					
				}
				else
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Quotes contain no string!"<< std::endl;
					delete[] ops;
					return;
				}
				break;


			case TOKENCHAR:
				if(strlen(working->token)>0)
				{
					strcpy((char *)(ops+opspos),working->token);
					opspos+=((strlen(working->token)+(sizeof(int)-1))/sizeof(int))*sizeof(int);
					
				}
				else
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Single quotes contain no string!"<< std::endl;
					delete[] ops;
					return;
				}
				break;
			case TOKENLABEL:
				if(fixerupper.addlabel(working->token,opspos)==-1)
				{
					std::cout << std::endl << "Token Error(" << working->line << "): Duplicate label!" << std::endl;
					std::cout << "Duplicate " << working->token << std::endl;
					delete[] ops;
					return;
				}
				break;
			case TOKENADD:
				//Label fixup later
				if(strcmp(working->token,"&")==0)
				{
					std::cout << std::endl << "Token Error(" << working->line << "): token missing with address '&'"<< std::endl;
					delete[] ops;
					return;

				}
				
				writeint.data=fixerupper.addusage(working->token,opspos);
				for(writeiterate=0;writeiterate<sizeof(int);++writeiterate)
				{
					ops[opspos+writeiterate]=writeint.bytes[writeiterate];
				}	
				opspos+=sizeof(int);
				break;
			
			
		}
		working=working->next;	
	
	} while(working!=NULL);
	
	if(fixerupper.fixup(ops)==-1)
	{
		std::cout << "Unable to complete operation!" << std::endl;
		delete[] ops;
		return;
	}



        char fendian;
        char szdouble;
        char szint;
        char version[8];
        int numops;
        int startaddress;

	//write the endian
	fendian=(char) endian;
	strcpy(version,"gVM 1.0");
	szdouble=sizeof(double);
	szint=sizeof(int);
	numops=opspos;
	startaddress=fixerupper.findaddress((char *)"main");

	if(startaddress==-1)
	{
		std::cout << "Entry Point Missing (lack of :main in file)" << std::endl;
		delete[] ops;
		return;

	}
	std::cout << "Entry point: " << startaddress << std::endl;
	
	if(outputfile==NULL) outputfile=(char *)"output.bin";
	FILE *doing=fopen(outputfile,"wb");
	if(doing==NULL)
	{
		std::cout << "Cannot open output file!" << std::endl;
		delete[] ops;
		return;
	}

	
	//num of ops
        test = fwrite(&fendian,1,1,doing);
        test = fwrite(&szdouble,1,1,doing);
        test = fwrite(&szint,1,1,doing);
        test = fwrite(version,1,8,doing);
        test = fwrite(&numops,1,sizeof(int),doing);
        test = fwrite(&startaddress,1,sizeof(int),doing);


	//write the ops
	if(opspos>0) fwrite(ops,sizeof(unsigned char),opspos,doing);
	fclose(doing);
	delete[] ops;
	std::cout << "Done writing " << outputfile << "!" << std::endl;
}

TOKEN *gettokensfromline(char *buffer, TOKEN *root, int line)
{
	
	//First token in the line
	TOKEN *working=root;
	int foundlabels=0;
	int tokenworking=EMPTY;
	int tokenpos=0;
	int pos=0;
	
	while(buffer[pos]!=0)
	{
		
		if((buffer[pos]>='A' && buffer[pos]<='Z') || (buffer[pos]>='a' && buffer[pos]<='z') || buffer[pos]=='_')
		{
			switch(tokenworking)
			{
				case TOKENNUM:
					ERRMSG=ERRALPHINNUM;
					return NULL;
					break;
				case EMPTY:
					working->next = new TOKEN(line);
					if(working->next==NULL)
					{
						std::cout << "OUT OF MEMORY!" << std::endl;
						return NULL;
					}
					foundlabels=1;
					working=working->next;
					tokenworking=TOKENALPH;
					working->type=TOKENALPH;
					tokenpos=0;
				default:
					working->token[tokenpos]=buffer[pos];
					++tokenpos;
					break;
			}
			
		}

		else if(buffer[pos]>='0' && buffer[pos]<='9')
		{
			switch(tokenworking)
			{
				case EMPTY:
					working->next = new TOKEN(line);
					if(working->next==NULL)
					{
						std::cout << "OUT OF MEMORY!" << std::endl;
						return NULL;
					}
					foundlabels=1;
					working=working->next;
					tokenworking=TOKENNUM;
					working->type=TOKENNUM;
					tokenpos=0;
				default:
					working->token[tokenpos]=buffer[pos];
					++tokenpos;
					break;
			}
		}
		else
		{
			switch(buffer[pos])
			{
				
				case '.':
								
					switch(tokenworking)
					{
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenpos=0;
						//Switch a NUMBER into a float midstream!
						case TOKENNUM:
							tokenworking=TOKENFLOAT;
							working->type=TOKENFLOAT;
						case TOKENALPH:
						case TOKENLABEL:
						case TOKENCOMMENT:
						case TOKENADD:
						case TOKENCHAR:
						case TOKENSTRING:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						
						default:
							ERRMSG=ERRPERIOD;
							return NULL;
							break;
					}
				break;

				case '-':
					switch(tokenworking)
					{
						case TOKENFLOAT:
						case TOKENNUM:
							ERRMSG=ERRMINUS;
							return NULL;
							break;
					
					
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENNUM;
							working->type=TOKENNUM;
							tokenpos=0;
						default:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
					}
				break;
				
				case ':':
					switch(tokenworking)
					{
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENLABEL;
							working->type=TOKENLABEL;
							tokenpos=0;
						case TOKENCHAR:
						case TOKENSTRING:
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						default:
							ERRMSG=ERRCOLON;
							return NULL;
							break;
					}
				break;

				
				//String :D
				case 34:
				
					switch(tokenworking)
					{
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENSTRING;
							working->type=TOKENSTRING;
							//Increase our buffer size to 64k for teh stringage
							working->getbigplease();
							tokenpos=0;
							break;
						case TOKENSTRING:
							working->token[tokenpos]=0;
							tokenworking=EMPTY;
							break;
						
						case TOKENCHAR:
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						default:
							ERRMSG=ERRCOLON;
							return NULL;
							break;
					}

				break;
				
				case 39:
				
					switch(tokenworking)
					{
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENCHAR;
							working->type=TOKENCHAR;
							//Increase our buffer size to 64k for teh stringage
							working->getbigplease();
							tokenpos=0;
							break;
						case TOKENCHAR:
							working->token[tokenpos]=0;
							tokenworking=EMPTY;
							break;
						
						case TOKENSTRING:
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						default:
							ERRMSG=ERRCOLON;
							return NULL;
							break;
					}

				break;




				case '_':
					switch(tokenworking)
					{
						case TOKENNUM:
							ERRMSG=ERRALPHINNUM;
							return NULL;
							break;
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENALPH;
							working->type=TOKENALPH;
							tokenpos=0;
						default:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
					}
					
					
				break;
					
				
				case ';':
					switch(tokenworking)
					{
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							
							
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENCOMMENT;
							working->type=TOKENCOMMENT;
							tokenpos=0;
						case TOKENCHAR:
						case TOKENSTRING:
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						default:
							ERRMSG=ERRCOMMENT;
							return NULL;
							break;
						
					}
				break;

				case '&':
					switch(tokenworking)
					{
						case EMPTY:
							working->next = new TOKEN(line);
							if(working->next==NULL)
							{
								std::cout << "OUT OF MEMORY!" << std::endl;
								return NULL;
							}
							foundlabels=1;
							working=working->next;
							tokenworking=TOKENADD;
							working->type=TOKENADD;
							tokenpos=0;
						case TOKENCHAR:
						case TOKENSTRING:
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						default:
							ERRMSG=ERRINVALIDCHAR;
							return NULL;
							break;
					
					}
				break;
					
				case '\t':
				case ' ':
					switch(tokenworking)
					{
						case TOKENCHAR:
						case TOKENSTRING:	
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						case EMPTY:
							break;
						default:
							working->token[tokenpos]=0;
							tokenworking=EMPTY;
							break;
					}
				break;

				case 13:
					
					break;

				default:
					switch(tokenworking)
					{
						case TOKENCHAR:	
						case TOKENSTRING:	
						case TOKENCOMMENT:
							working->token[tokenpos]=buffer[pos];
							++tokenpos;
							break;
						default:
							ERRMSG=ERRINVALIDCHAR;
							return NULL;
							break;
					}
					
					break;
			}


			//switch chars
			//switch each token state for each set
			

		}
		
		++pos;
	}

	if(foundlabels==1)
	{
		if(tokenworking==TOKENSTRING || tokenworking==TOKENCHAR)
		{
			ERRMSG=ERRSTRING;
			return NULL;
		}
		
		working->token[tokenpos]=0;

	}
	return working;
}


int operate(char *file, char *fileout)
{
	
	KEYWORD words[]={ 
						//Opcodes
						
						//32bit processing
						KEYWORD((char *)"push",     0),
						KEYWORD((char *)"pop",      1),
						KEYWORD((char *)"clone",    2),
						KEYWORD((char *)"convert",  3),	
						
						KEYWORD((char *)"add",      4),
						KEYWORD((char *)"sub",      5),
						KEYWORD((char *)"mul",      6),
						KEYWORD((char *)"div",      7),
						
						KEYWORD((char *)"and",      8),
						KEYWORD((char *)"or",       9),
						KEYWORD((char *)"xor",     10),
						KEYWORD((char *)"not",     11),
						
						KEYWORD((char *)"exp",     16),
						KEYWORD((char *)"mod",     17),
						KEYWORD((char *)"shr",     18),
						KEYWORD((char *)"shl",     19),
						
						KEYWORD((char *)"cmp",     32),
						KEYWORD((char *)"jmp",     33),
						KEYWORD((char *)"jme",     34),
						KEYWORD((char *)"jml",     35),
						KEYWORD((char *)"jmg",     36),
						KEYWORD((char *)"jle",     37),
						KEYWORD((char *)"jge",     38),
						KEYWORD((char *)"jne",     39),
						KEYWORD((char *)"function",    42),
						KEYWORD((char *)"return",     43),
						KEYWORD((char *)"dpush",     44),

						KEYWORD((char *)"preserve",   46),
						KEYWORD((char *)"restore",    47),


						KEYWORD((char *)"load",    48),
						KEYWORD((char *)"store",   49),

						KEYWORD((char *)"loadb",    50),
						KEYWORD((char *)"storeb",   51),
						
						//Goodies
						KEYWORD((char *)"incl",    52),
						KEYWORD((char *)"decl",     53),
						KEYWORD((char *)"rstore",   54),


						KEYWORD((char *)"api",     61),
						KEYWORD((char *)"sys",     62),
						KEYWORD((char *)"exit",    63),
						
						
						//Floating Point Processing
						
						KEYWORD((char *)"pushf",    64),
						KEYWORD((char *)"popf",     65),
						KEYWORD((char *)"clonef",   66),
						KEYWORD((char *)"convertf", 67),
						

						KEYWORD((char *)"addf",     68),
						KEYWORD((char *)"subf",     69),
						KEYWORD((char *)"divf",     70),
						KEYWORD((char *)"mulf",     71),

						//Extras
						KEYWORD((char *)"expf",     72),
						KEYWORD((char *)"modf",     73),
						KEYWORD((char *)"sqrtf",    74),
						KEYWORD((char *)"cosf",     75),
						KEYWORD((char *)"sinf",     76),
						KEYWORD((char *)"tanf",     77),
						
					
						KEYWORD((char *)"cmpf",     96),
						
						
						KEYWORD((char *)"loadf",    97),
						KEYWORD((char *)"storef",   98),
						
						
						//function frame keywords
						KEYWORD((char *)"int",      1),
						KEYWORD((char *)"float",    2),
						KEYWORD((char *)"byte",     3),
						KEYWORD((char *)"end",      0),


						
						
						//sys_routines
						KEYWORD((char *)"sys_write",     1),
						KEYWORD((char *)"sys_read",      2),
						KEYWORD((char *)"sys_putchar",   3),
						KEYWORD((char *)"sys_endl",      4),
						
						KEYWORD((char *)"sys_atoi",      16),
						KEYWORD((char *)"sys_itoa",      17),
						KEYWORD((char *)"sys_atof",      18),
						KEYWORD((char *)"sys_ftoa",      19),
						
						KEYWORD((char *)"sys_strlen",    32),
						
						KEYWORD((char *)"sys_szdouble",  64),
						KEYWORD((char *)"sys_szint",     65),
						
						KEYWORD((char *)"sys_thread",  1337),
						KEYWORD((char *)"sys_yield",   1338),

						};
	
	
	int mendian;
	union endian {int huhu; char a[sizeof(int)];}; endian bleh;	bleh.huhu=1;
	
	std::cout << "Keywords: " << nkeywords << std::endl;

	if(bleh.a[0]==1)
	{
		std::cout << "Machine is Little Endian (intel)" << std::endl;
		mendian=1;
	}
	else
	{
		std::cout << "Machine is Big Endian (MIPS, Motorola, etc.)" << std::endl;
		mendian=0;
	}



	TOKEN *root=new TOKEN;
	if(root==NULL)
	{
		std::cout << "OUT OF MEMORY!" << std::endl;
		return -1;
	}

	TOKEN *index=root;
	char ftext[65536];
	std::ifstream asmfile;

	asmfile.open(file);
	int lines=1;
	if(asmfile)
	{
		while(asmfile.eof()==0)
		{
			asmfile.getline(ftext,65536);
			index=gettokensfromline(ftext,index,lines);
			if(index==NULL)
			{
				delete root;
				asmfile.close();
				switch (ERRMSG)
				{
					case ERRALPHINNUM:
						std::cout << "E1(" << lines << "): Found letter (a-z,A-Z) in numerical token" << std::endl << " " << ftext << std::endl;
						break;
					case ERRPERIOD:
						std::cout << "E2(" << lines << "): Found additional '.' (math says only one decimal in a float)" << std::endl << " " << ftext << std::endl;
						break;
					
					case ERRCOLON:
						std::cout << "E2(" << lines << "): Found ':' inside token (colons must be at start of token to be valid as labels)" << std::endl << " " << ftext << std::endl;
						break;
					case ERRMINUS:
						std::cout << "E3(" << lines << "): Found '-' in token.  Only valid as first char to negate a numerical token (ie: -1351)" << std::endl << " " << ftext << std::endl;
						break;
					case ERRCOMMENT:
						std::cout << "E4(" << lines << "): Found ';' in token.  Only valid as first char of a comment" << std::endl << " " << ftext << std::endl;
						break;
					case ERRINVALIDCHAR:
						std::cout << "E5(" << lines << "): Found invalid char in token" << std::endl << " " << ftext << std::endl;
						break;
					case ERRSTRING:
						std::cout << "E6(" << lines << "): String not closed (trailing " << (char)34 << " or ' missing) !" << std::endl << " " << ftext << std::endl;
						break;
					default:
						std::cout << "Unexpected error(" << lines << "): closing and quitting!" << std::endl;
				}
				return 1;
			}
			
			++lines;
		}
		asmfile.close();
		converttokens(root,words,nkeywords,fileout,mendian);
		delete root;
	}
	else
	{
		std::cout << "FILE NOT FOUND!" << std::endl;
	}
	
	return 0;
}

int main(int argc,char *argv[])
{
	std::cout << "gVM assembler (it's slow!) by Ryan Broomfield 2003" << std::endl;
	if(argv[1]==NULL) std::cout << "Usage: assembler asmfile.asm vmfile.bin" << std::endl;
	else 
	{
		return operate(argv[1],argv[2]);
	}
	return 0;
		
}
