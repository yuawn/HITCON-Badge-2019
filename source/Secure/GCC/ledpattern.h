#ifndef _LEDPATTERN_H_
#define _LEDPATTERN_H_
#include "NuMicro.h"

void patternError();

void function_pattern(int32_t controlA,int32_t controlB,int degree,float b,float x0,float y0,float height_amp);

void pattern1(int32_t controlA,int32_t controlB);

void pattern2(int32_t controlA,int32_t controlB);

void pattern3(int32_t controlA,int32_t controlB);

void pattern4(int32_t controlA,int32_t controlB);

void pattern5(int32_t controlA,int32_t controlB);

void pattern6(int32_t controlA,int32_t controlB);

void pattern7(int32_t controlA,int32_t controlB);

void pattern8(int32_t controlA,int32_t controlB);

void pattern9(int32_t controlA,int32_t controlB);

void pattern10(int32_t controlA,int32_t controlB);

void pattern11(int32_t controlA,int32_t controlB);

void colorholeled();

void rainbowCycle();


#endif // _LEDPATTERN_H_
