CC       = g++
RM       = /bin/rm
CFLAGS   = -O3 -Wall

all:
	$(CC) -o gvma gvma.cpp $(CFLAGS)
	$(CC) -o gvm entity.cpp main.cpp $(CFLAGS)

clean:
	$(RM) -f *.o
	$(RM) -f gvma
	$(RM) -f demo
