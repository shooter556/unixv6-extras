: generate the gtl task
cc -c -O gtl?.c ld8.s
cc -s -o gtl gtl?.o ld8.o -lS
rm gtl?.o ld8.o
: cp gtl /usr/bin/gtl
