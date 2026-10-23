.	\"OK - "other keywords" for cover sheet.
.de OK
.br
.di
.di OD
.nf
.ft 3
..
.	\"CS - spew out cover sheet
.de CS
.br
.di
.if t .pl 15i
.if t .po 0.5i
.if t .sp |.5i
.if t .lg
.if t .ll 7.0i
.nf
.if t \h'7n'\s24\(bs\s12\f3  Bell Laboratories        \s16\f1Cover Sheet for Technical Memorandum
.if n Bell Laboratories        Cover Sheet for Technical Memorandum
.sp
.if t \s6\l'7i'
.fi
.ps 9
.ft 2
.if t .sp 2p
The information contained herein is for the use of employees of Bell Laboratories and is not for publication.  (See GEI 13.9-3)\p
.QQ...Warning: studying this document may cause drowsiness.  Do not read before driving or operating machinery.  (See GEI 13.9-3)\p
.ft 1
.nf
.if t \s6\l'7i'
.sp
.if t .ll 6.0i
.if n .ll 30
.if t .po 1.0i
.ps 9
.if t .ta 0.5i 4.5i 4.9i
.if n .ta 9 39 46
.mk
Title- \f3\s10
.rt
.if t .in 0.5i
.if n .in 10
.hy 0
.WT
.hy 14
.rt
.ll \\n(LLu
.if t .in 4.5i
.if n .in 40
\s9\f1Date- \s10\f3
.rt
.if t .in 4.9i
.if n .in 47
\\*(DY
.sp
.if t .ti 4.5i
.if n .ti 40
.mk
\s9\f1TM- \f3\s10
.br
.rt
.CT \\*(MN
.in 0
.sp |\\n(.hu+1v
.mk
\s9\f1Other Keywords- \f3
.rt
.if t .in 1.1i
.if n .in 20
.OD
.rm OK
.rm OD
.in 0
.if t .sp .7i
.if n .sp 3
.ps 9
.if t .ta 1.8i 3.3i 4.3i
.if n .ta 19 34 44
.ft 1
Author	Location	Extension	Charging Case- \s10\f3\\*(CA
.ps 10
.nf
.mk
.A1
.rt
	\\*(R1	\\*(E1	\f1\s9Filing Case- \s10\f3\\*(CC
.if \\n(NA-1 .CB 2
.if \\n(NA-2 .CB 3
.if \\n(NA-3 .CB 4
.if \\n(NA-4 .CB 5
.ft 1
.if t .sp .4i
.if n .sp 2
.WB
.br
.if 9.3i-\\n(nlu-\\n(FPu>0 .sp 9.3i-\\n(nlu-\\n(FPu
.FG
.if t .sp |9.3i
.if n .sp |50
.if t .po 0.5i
.if t .ll 7i
.nf
.if t \l'7i'
.if t .ta 1.4i 2.8i 4.2i
.if n .ta 14 29 44
.if t .sp 8p
Pages Text   \\$1	Other   \\$2	Total   \\$3
.if t .sp 8p
.if n .sp 
.if t No. Figures   \\$4	No. Tables   \\$5	No. Refs.   \\$6	\b'|||||\ \ '
.if n No. Figures   \\$4	No. Tables   \\$5	No. Refs.   \\$6
.if t \l'7i'
.ps 7
.if n .sp
.if t .tl 'E-1932-U (6-73)'SEE REVERSE SIDE FOR DISTRIBUTION LIST''
.if n .tl 'E-1932-U (6-73)  SEE REVERSE SIDE FOR DISTRIBUTION LIST'''
.if t .po 0.5i
.ll \\n(LLu
.fi
.ps 10
.if t .pl
.bp 1
.if t .po \\n(POu
..
.	\"CB - help with cover sheet
.de CB
.br
.mk
.A\\$1
.rt
.if \\$1-1 .if !\\$1-2 \&	\\*(R2	\\*(E2
.if \\$1-2 .if !\\$1-3 \&	\\*(R3	\\*(E3
.if \\$1-3 .if !\\$1-4 \&	\\*(R4	\\*(E4
.if \\$1-4 .if !\\$1-5 \&	\\*(R5	\\*(E5
.br
..
.de CT
\\$1
\\$2
\\$3
\\$4
..
.	\"S1 - tm style 
.de S1
.br
.if t .ta 4.75i
.if n .ta 46
.ll 80
.ps 36
.br
.if t \t\(bs
.if t .vs .25i
.if n .sp 24p
.ps 12
.br
.ft 3
.nf
\tBell Laboratories
.ft 1
.fi
.na
.ps 8
.vs 12p
.sp .5i
.mk
.fi
.ll
.ll 4.0i
Subject:
.ft 3
.if n 'in +9
.ps
.hy 0
.WT
.hy 14
.ft 1
.br
.if n 'in -9
.if \\n(IM .nr MM 0
.if \\n(MM-1 \\s8Case- \\*(CA\\s10
.if \\n(MM-2 --\s8 File- \\*(CC\\s10
.br
.rt
.if n .ll 70
.if t .ll 6.5i
.if t .in 5.10i
.if n .in 52
.ps 8
.mk
.ti -6n
date:
.ps \\n(PS
.ft 3
.br
.rt
\\*(DY
.sp
.ft 1
.ps 8
.mk
.ti -6n
from:
.ps
.br
.rt
.ft 3
.A1
.br
.if \\n(NA>1 .A2
.br
.if \\n(NA>2 .A3
.br
.if \\n(NA>3 .A4
.br
.if \\n(NA>4 .A5
.ft 1
.sp
.mk
.ps 8
.ti -6n
.ft 1
.if \\n(MM .if \\n(MN=0  TM:
.if \\n(MM .if \\n(MN=1  MF:
.if \\n(MM .if \\n(MN=2  MR:
.br
.rt
.ps
.ft 3
.hy 0
.ll 10u*\w'0'u+\\n(.iu
.if \\n(MM \\*(MN
.hy 14
.ll
.ft 1
.ll \\n(LLu
.in
.if \\n(IM=0 .sp
.ad
.ce
.ft 2
.if \\n(IM=0 \\*(QF
.ft 1
.ce 0
.if \\n(IM=0 .sp
.ad
..
