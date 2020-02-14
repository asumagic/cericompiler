[a,b,i]
a := 0;
b := 0;

FOR i := 0 TO 5 DO
BEGIN
    a := a + i;
    i := i + 1
END;

i := 0;
WHILE i < 5 DO
BEGIN
    b := b + i;
    i := i + 1
END;

IF a == b THEN
    i := 1
ELSE
    i := 0
.