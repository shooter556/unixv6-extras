cc -c -O rkdfc.c
as rkdfs.s
ld -s a.out rkdfc.o -lc -l
rm -f *.o /unix.rkdf
mv a.out /unix.rkdf
chmod 0 /unix.rkdf
