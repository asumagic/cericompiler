INCLUDE "stdc/math.pas";

(* This is a crude way to check whether our math bindings work (no wrong name and no obvious crash) *)

BEGIN
    llabs(1);
    fabs(1.0);
    remainder(1.0, 1.0);
    fmod(1.0, 1.0);
    remainder(1.0, 1.0);
    fma(1.0, 1.0, 1.0);
    fmax(1.0, 1.0);
    fmin(1.0, 1.0);
    fdim(1.0, 1.0);
    exp(1.0);
    exp2(1.0);
    expm1(1.0);
    log(1.0);
    log10(1.0);
    log2(1.0);
    log1p(1.0);
    pow(1.0, 1.0);
    sqrt(1.0);
    cbrt(1.0);
    hypot(1.0, 1.0);
    sin(1.0);
    cos(1.0);
    tan(1.0);
    asin(1.0);
    acos(1.0);
    atan(1.0);
    atan2(1.0, 1.0);
    sinh(1.0);
    cosh(1.0);
    tanh(1.0);
    asinh(1.0);
    acosh(1.0);
    atanh(1.0);
    erf(1.0);
    erfc(1.0);
    tgamma(1.0);
    lgamma(1.0);
    ceil(1.0);
    floor(1.0);
    trunc(1.0);
    round(1.0);
    llrint(1.0);
    ldexp(1.0, 1);
    scalbn(1.0, 1);
    ilogb(1.0);
    logb(1.0);
    nextafter(1.0);
    copysign(1.0, 1.0);

    (* This will display if there was not a segmentation fault so far *)
    DISPLAY 'o'
END.
