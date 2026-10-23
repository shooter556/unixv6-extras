echoall
cc -c -O text2.c ; as text1.s ; ld -r -x a.out text2.o -lc -la 
mv a.out text.o ; rm text2.o ; size text.o
