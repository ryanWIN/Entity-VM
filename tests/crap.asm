:crap.frame &crap int end
:crap.n 0

:main
 push 5
 push &crap.frame
 function
exit

:crap
 push &crap.n
 load
 push &buffer
 push sys_itoa
 sys
 push &buffer
 push sys_write
 sys
return



:buffer "                     "