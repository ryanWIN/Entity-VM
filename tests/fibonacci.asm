:main


:lewp
 
 ;fib(1)
 
  ;1 param

   push &i
   load
   push &fib.frame
   function
  
  push &buffer
  push sys_itoa
  sys
  push &buffer
  push sys_write
  sys
  push sys_endl
  sys

  push &i
  load
  push 1
  add
  push &i
  store


  push &i
  load
  push 35
  cmp
  push &lewp
  jml  


exit

:buffer "                                                "
:i 1


;framename functionpointer ints floats bytes
:fib.frame &fib int end
:fib.n 0
:fib

push &fib.n
load
push 3
cmp
push &fib.lessthan1
jml

 push &fib.n
 load
 push 1
 sub
 push &fib.frame
 function

 push &fib.n
 load
 push 2
 sub
 push &fib.frame
 function
 add
 
return

:fib.lessthan1
push 1
return 

 



