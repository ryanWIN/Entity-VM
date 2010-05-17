#include <iostream>
#include <time.h>
#include "entity.h"

void thefunction(void *owner)
{
	THREAD *thread = (THREAD *) owner;
	ENTITY *MASTER = (ENTITY *) thread->owner;

	std::cout << "Hello, my name is " << MASTER->name << std::endl;
}

int main(int argc, char *argv[])
{
	
	if(argc==1)
	{
		std::cout << "Usage: entity file.bin" << std::endl;
		return -1;
	}
	
	ENTITY shippy("Shippy TEST OBJECT",argv[1]);
	shippy.api.install("thefunction",thefunction);
	shippy.Execute();
	
	std::cout << (float)clock()/CLOCKS_PER_SEC << " seconds!" << std::endl;
	return 0;
}
