VAR K,X,A,B,C : INTEGER;
BEGIN
    A := 10;
    B := 7;
    C := -100;
    A := A + (B - C) / 100;
    X := 0;
    FOR K:=1000 DOWNTO 1 DO
    BEGIN
       X := X + 1;
       WRITE(X);
    END
END
    