#ifndef _MATH_H
#define _MATH_H

#define HUGE_VAL	__builtin_huge_val()
#define isinf(x)	__builtin_isinf(x)
#define isnan(x)	__builtin_isnan(x)

double ceil(double x);
float ceilf(float x);
double exp(double x);
float expf(float x);
double fabs(double x);
float fabsf(float x);
double floor(double x);
float floorf(float x);
double fmod(double x, double y);
float fmodf(float x, float y);
double frexp(double x, int* exp);
float frexpf(float x, int* exp);
double ldexp(double x, int exp);
float ldexpf(float x, int exp);
long double ldexpl(long double x, int exp);
double log(double x);
float logf(float x);
double log10(double x);
float log10f(float x);
double log2(double x);
float log2f(float x);
double pow(double x, double y);
float powf(float x, float y);
double sin(double x);
float sinf(float x);
double sqrt(double x);
float sqrtf(float x);
double cos(double x);
float cosf(float x);
double tan(double x);
float tanf(float x);
double asin(double x);
float asinf(float x);
double acos(double x);
float acosf(float x);
double atan(double x);
float atanf(float x);
double atan2(double y, double x);
float atan2f(float y, float x);
double sinh(double x);
float sinhf(float x);
double cosh(double x);
float coshf(float x);
double tanh(double x);
float tanhf(float x);

#endif //_MATH_H
