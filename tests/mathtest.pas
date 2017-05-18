VAR L,U,SQ,TP,ITER : INTEGER;
BEGIN
    SQ := 230;
    L  := 0;
    U  := SQ;    
    FOR ITER:=1 TO 20 DO
    BEGIN
        TP:= (L+U)/2;
        IF ((TP*TP) > SQ) THEN
            U:=TP;
        IF ((TP*TP) < SQ) THEN
            L:=TP;
        WRITE(TP);
    END
END
