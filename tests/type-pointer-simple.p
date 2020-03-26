TYPE pinteger_t = ^INTEGER;
VAR pinteger : pinteger_t;
VAR foo : INTEGER;

BEGIN
    pinteger := @foo;
    foo := 123;
    DISPLAY pinteger^
END.
