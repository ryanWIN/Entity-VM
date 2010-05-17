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
  push 10
  cmp
  push &lewp
  jml  


exit

:buffer "                                                "
:heh "I is at : "
:i 1


;int fib(int n) { if(n < 3) return(1); return(fib(n-1) + fib(n-2)); }




;framename functionpointer ints floats bytes
:fib.frame &fib int end
:fib.n 0


:f.message "F is: "
:localvar.f 0

:fib

push &localvar.f
push int
preserve

 push &localvar.f
 incl
 pop

push &fib.n
load
push 3
cmp
push &fib.lessthan1
jml




;do the maths
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

 push &f.message
 push sys_write
 sys

 push &localvar.f
 load
 push &buffer
 push sys_itoa
 sys
 push &buffer
 push sys_write
 sys
 push sys_endl
 sys


restore

return




;return less than 1
:fib.lessthan1
push 1

 push &f.message
 push sys_write
 sys
 push &localvar.f
 load
 push &buffer
 push sys_itoa
 sys
 push &buffer
 push sys_write
 sys
 push sys_endl
 sys

restore
return 

 



