:main


:lewp
 
 ;fib(1)
 
  ;1 param

   dpush &i &fib.frame
   load
   function
  
  dpush &buffer sys_itoa
  sys
  dpush &buffer sys_write
  sys
  push sys_endl
  sys

 

  
;  push &heh
;  push sys_write
;  sys
;  push &i
;  load
;  push &buffer
;  push sys_itoa
;  sys
;  push &buffer
;  push sys_write
;  sys
;  push sys_endl
;  sys


  push &i
  incl
  push 31
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
:fib

push &fib.n 
load
push 3
cmp
push &fib.lessthan1
jml




;do the maths
 push &fib.n
 decl
 push &fib.frame
 function

 push &fib.n
 clone
 decl
 pop
 decl
 push &fib.frame
 function

 add
 
return




;return less than 1
:fib.lessthan1
push 1
return 

 



