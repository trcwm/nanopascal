;
; This is a comment
;

jmp @entry
jmp $1234

@entry:
halt

CAL 2
JMP 2       ; ignored
INT 0 5
LOD 1 3
STO 0 3
LOD 1 4
STO 0 4
LIT 0 0
STO 1 5
LOD 0 4
LIT 0 0
OPR 0 12
JPC 29
LOD 0 4
OPR 0 6
JPC 20
LOD 1 5
LOD 0 3
OPR 0 2
STO 1 5
LIT 0 2
LOD 0 3
OPR 0 4
STO 0 3
LOD 0 4
LIT 0 2
OPR 0 5
STO 0 4
JMP 9
OPR 0 0
