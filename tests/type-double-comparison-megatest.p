(*
    This testbench tests most operators with floating point values.
    A correct implementation should only display 'o's.
*)


IF 1.0 == 1.0   THEN DISPLAY 'o' ELSE DISPLAY 'x';
IF 1.0 == 2.0   THEN DISPLAY 'x' ELSE DISPLAY 'o';

IF 1.0 <> 1.0   THEN DISPLAY 'x' ELSE DISPLAY 'o';
IF 1.0 <> 2.0   THEN DISPLAY 'o' ELSE DISPLAY 'x';

IF 16.0 >= 15.0 THEN DISPLAY 'o' ELSE DISPLAY 'x';
IF 15.0 >= 15.0 THEN DISPLAY 'o' ELSE DISPLAY 'x';
IF 14.0 >= 15.0 THEN DISPLAY 'x' ELSE DISPLAY 'o';

IF 16.0 > 15.0  THEN DISPLAY 'o' ELSE DISPLAY 'x';
IF 15.0 > 15.0  THEN DISPLAY 'x' ELSE DISPLAY 'o';
IF 14.0 > 15.0  THEN DISPLAY 'x' ELSE DISPLAY 'o';

IF 16.0 <= 15.0 THEN DISPLAY 'x' ELSE DISPLAY 'o';
IF 15.0 <= 15.0 THEN DISPLAY 'o' ELSE DISPLAY 'x';
IF 14.0 <= 15.0 THEN DISPLAY 'o' ELSE DISPLAY 'x';

IF 16.0 < 15.0  THEN DISPLAY 'x' ELSE DISPLAY 'o';
IF 15.0 < 15.0  THEN DISPLAY 'x' ELSE DISPLAY 'o';
IF 14.0 < 15.0  THEN DISPLAY 'o' ELSE DISPLAY 'x'.
