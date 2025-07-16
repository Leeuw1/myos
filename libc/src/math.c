#include <math.h>
#include <stdint.h>
#include <assert.h>

// TEMP
#include <stdio.h>
#define UNIMP()			printf("[libc] Warning: %s is not implemented.\n" ,__FUNCTION__);

//#define LOG2_OF_10			3.3219280948873623
// TODO: we can probably get more digits
#define LOG_OF_10			2.302585092994045684
#define LOG_OF_2			0.6931471805599453
#define DOUBLE_EXP_BIAS		1023

double ceil(double x) {
	asm volatile (
		"frintp	d0, d0"
	);
	return x;
}

float ceilf(float x) {
	asm volatile (
		"frintp	s0, s0"
	);
	return x;
}

double exp(double x) {
	UNIMP();
}

float expf(float x) {
	UNIMP();
}

double fabs(double x) {
	return __builtin_fabs(x);
}

float fabsf(float x) {
	return __builtin_fabsf(x);
}

double floor(double x) {
	asm volatile (
		"frintm	d0, d0"
	);
	return x;
}

float floorf(float x) {
	asm volatile (
		"frintm	s0, s0"
	);
	return x;
}

double fmod(double x, double y) {
	return x - (floor(x / y) * y);
}

float fmodf(float x, float y) {
	return x - (floorf(x / y) * y);
}

static uint64_t _double_bits(double x) {
	(void)x;
	uint64_t bits;
	asm volatile (
		"fmov	%0, d0"
		: "=r" (bits)
	);
	return bits;
}

// Set the exponent to 0
static double _fractional(uint64_t bits) {
	double result;
	const uint64_t exponent = DOUBLE_EXP_BIAS;
	asm volatile (
		"bfi	%0, %2, #52, #11\n"
		"fmov	d0, %0"
		: "+r" (bits), "=r" (result)
		: "r" (exponent)
	);
	return result;
}

double frexp(double x, int* exp) {
	const uint64_t bits = _double_bits(x);
	*exp = (int)((bits & (0x7ffull << 52)) >> 52) - DOUBLE_EXP_BIAS;
	return _fractional(bits);
}

float frexpf(float x, int* exp) {
	UNIMP();
}

double ldexp(double x, int exp) {
	UNIMP();
}

float ldexpf(float x, int exp) {
	UNIMP();
}

double _log_taylor_poly(double x) {
	assert(x > -1.0 && x <= 1.0);
	x -= 1.0;
	const int k = 7;
	double term = x;
	double result = term;
	for (int i = 1; i <= k; ++i) {
		term *= -x / (double)i;
		result += term;
	}
	return result;
}

double log(double x) {
	// log(2^exp * mant)
	// exp*log(2) + log(mant)
	int exponent;
	const double mantissa = frexp(x, &exponent);
	return (double)exponent * LOG_OF_2 + _log_taylor_poly(mantissa);
}

double log10(double x) {
	return log(x) / LOG_OF_10;
}

double log2(double x) {
	// log2(2^exp * mant)
	// exp + log2(mant)
	// exp + log(mant) / log(2)
#if 0
	int exponent;
	const double mantissa = frexp(x, &exponent);
	return (double)exponent + _log_taylor_poly(mantissa) / LOG_OF_2;
#endif
	return log(x) / LOG_OF_2;
}

float logf(float x) {
	UNIMP();
}

float log10f(float x) {
	UNIMP();
}

float log2f(float x) {
	UNIMP();
}

double pow(double x, double y) {
	UNIMP();
}

float powf(float x, float y) {
	UNIMP();
}

double sin(double x) {
	UNIMP();
}

float sinf(float x) {
	UNIMP();
}

double sqrt(double x) {
	asm volatile (
		"fsqrt	d0, d0"
	);
	return x;
}

float sqrtf(float x) {
	asm volatile (
		"fsqrt	s0, s0"
	);
	return x;
}

double cos(double x) {

	UNIMP();
}

float cosf(float x) {

	UNIMP();
}

double tan(double x) {

	UNIMP();
}

float tanf(float x) {
	UNIMP();

}

double asin(double x) {
	UNIMP();

}

float asinf(float x) {
	UNIMP();

}

double acos(double x) {

	UNIMP();
}

float acosf(float x) {
	UNIMP();

}

double atan2(double y, double x) {
	UNIMP();

}

float atan2f(float y, float x) {

	UNIMP();
}
