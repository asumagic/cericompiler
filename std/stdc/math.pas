(*
    sdelang's CERIcompiler bindings for C99 <math.h>
    https://en.cppreference.com/w/c/numeric/math
*)

(* TODO: some of those are incorrect because we use INTEGER where int is used.
   That might have consequences on the ABI. *)

FFI llabs(INTEGER): INTEGER;
(* FFI lldiv() *) (* This will require record support and record support in the FFI *)

(* Basic operations *)
FFI fabs(DOUBLE): DOUBLE;
FFI fmod(DOUBLE, DOUBLE): DOUBLE;
FFI remainder(DOUBLE, DOUBLE): DOUBLE;
(* FFI remquo(DOUBLE, DOUBLE): DOUBLE; *)
FFI fma(DOUBLE, DOUBLE, DOUBLE): DOUBLE;
FFI fmax(DOUBLE, DOUBLE): DOUBLE;
FFI fmin(DOUBLE, DOUBLE): DOUBLE;
FFI fdim(DOUBLE, DOUBLE): DOUBLE;
FFI nan(DOUBLE, DOUBLE): DOUBLE;

(* Exponential functions *)
FFI exp(DOUBLE): DOUBLE;
FFI exp2(DOUBLE): DOUBLE;
FFI expm1(DOUBLE): DOUBLE;
FFI log(DOUBLE): DOUBLE;
FFI log10(DOUBLE): DOUBLE;
FFI log2(DOUBLE): DOUBLE;
FFI log1p(DOUBLE): DOUBLE;

(* Power functions *)
FFI pow(DOUBLE, DOUBLE): DOUBLE;
FFI sqrt(DOUBLE): DOUBLE;
FFI cbrt(DOUBLE): DOUBLE;
FFI hypot(DOUBLE, DOUBLE): DOUBLE;

(* Trigonometric functions *)
FFI sin(DOUBLE): DOUBLE;
FFI cos(DOUBLE): DOUBLE;
FFI tan(DOUBLE): DOUBLE;
FFI asin(DOUBLE): DOUBLE;
FFI acos(DOUBLE): DOUBLE;
FFI atan(DOUBLE): DOUBLE;
FFI atan2(DOUBLE, DOUBLE): DOUBLE;

(* Hyperbolic functions *)
FFI sinh(DOUBLE): DOUBLE;
FFI cosh(DOUBLE): DOUBLE;
FFI tanh(DOUBLE): DOUBLE;
FFI asinh(DOUBLE): DOUBLE;
FFI acosh(DOUBLE): DOUBLE;
FFI atanh(DOUBLE): DOUBLE;

(* Error functions *)
FFI erf(DOUBLE): DOUBLE;
FFI erfc(DOUBLE): DOUBLE;
FFI tgamma(DOUBLE): DOUBLE;
FFI lgamma(DOUBLE): DOUBLE;

(* Nearest integer floating-point operations *)
FFI ceil(DOUBLE): DOUBLE;
FFI floor(DOUBLE): DOUBLE;
FFI trunc(DOUBLE): DOUBLE;
FFI round(DOUBLE): DOUBLE;
(* FFI nearbyint(DOUBLE): DOUBLE; *) (* meaningless, we have no fenv.h support *)
FFI llrint(DOUBLE): INTEGER;

(* Floating-point manipulation functions *)
(* FFI frexp *)
FFI ldexp(DOUBLE, INTEGER): DOUBLE;
(* FFI modf *)
FFI scalbn(DOUBLE, INTEGER): DOUBLE;
FFI ilogb(DOUBLE): DOUBLE;
FFI logb(DOUBLE): DOUBLE;
FFI nextafter(DOUBLE): DOUBLE;
(* FFI nexttoward(DOUBLE): DOUBLE; *) (* meaningless, we have no >64-bit double *)
FFI copysign(DOUBLE, DOUBLE): DOUBLE;

(* TODO: classification and comparison are mostly macros *)

(* Types. ABI dependent. *)
TYPE float_t = DOUBLE;
TYPE double_t = DOUBLE;

(* TODO: macro constants *)
