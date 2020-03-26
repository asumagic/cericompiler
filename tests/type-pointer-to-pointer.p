VAR p1 : ^INTEGER;
VAR p2 : ^^INTEGER;
VAR v : INTEGER;

BEGIN
    v := 123;
    p2 := @p1;
    p1 := @v;
    DISPLAY p2^^;
    p2^^ := 321;
    DISPLAY v
END.
