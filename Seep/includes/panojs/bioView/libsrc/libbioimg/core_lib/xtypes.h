/*****************************************************************************
 Base typing and type conversion definitions

 DEFINITION

 Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>
 Copyright (c) 2006 Vision Research Lab, UCSB <http://vision.ece.ucsb.edu>

 History:
   04/19/2006 16:20 - First creation

 Ver : 1
*****************************************************************************/

#ifndef DIM_XTYPES
#define DIM_XTYPES
//#pragma message(">>>>>  xtypes: included types and type conversion utils")

#include <cmath>
#include <vector>

//------------------------------------------------------------------------------
// first define type macros, you can define them as needed, OS specific
//------------------------------------------------------------------------------
/*
Datatype  LP64  ILP64 LLP64 ILP32 LP32
char       8     8     8      8      8
short     16    16    16     16     16
_int32          32      
int       32    64    32     32     16
long      64    64    32     32     32
long long       64    
pointer   64    64    64     32     32
*/ 

#ifndef _DIM_TYPEDEFS_
#define _DIM_TYPEDEFS_

// system types
#ifndef uchar
typedef	unsigned char uchar;
#endif
#ifndef uint
typedef	unsigned int uint;
#endif

// sized types
typedef	signed char int8;
typedef	unsigned char uint8;
typedef	short int16;
typedef	unsigned short uint16;	// sizeof (uint16) must == 2
#if defined(__alpha) || (defined(_MIPS_SZLONG) && _MIPS_SZLONG == 64) || defined(__LP64__) || defined(__arch64__)
typedef	int int32;
typedef	unsigned int uint32;	// sizeof (uint32) must == 4
#else
typedef	long int32;
typedef	unsigned long uint32;	// sizeof (uint32) must == 4
#endif

#endif // _DIM_TYPEDEFS_

namespace dim {

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

//------------------------------------------------------------------------------
// BIG_ENDIAN is for SPARC, Motorola, IBM and LITTLE_ENDIAN for intel type
//------------------------------------------------------------------------------
static int dimOne = 1;
static int bigendian = (*(char *)&dimOne == 0);
static double Pi = 3.14159265358979323846264338327950288419716939937510;


template<typename T>
inline bool isnan(T value) { return value != value; }

//------------------------------------------------------------------------------
// SWAP types for big/small endian conversions
//------------------------------------------------------------------------------

inline void swapShort(uint16* wp) {
  register uchar* cp = (uchar*) wp;
  uchar t;
  t = cp[1]; cp[1] = cp[0]; cp[0] = t;
}

inline void swapLong(uint32* lp) {
  register uchar* cp = (uchar*) lp;
  uchar t;
  t = cp[3]; cp[3] = cp[0]; cp[0] = t;
  t = cp[2]; cp[2] = cp[1]; cp[1] = t;
}

void swapArrayOfShort(uint16* wp, register uint n);
void swapArrayOfLong(register uint32* lp, register uint n);

inline void swapFloat(float* lp) {
  register uchar* cp = (uchar*) lp;
  uchar t;
  t = cp[3]; cp[3] = cp[0]; cp[0] = t;
  t = cp[2]; cp[2] = cp[1]; cp[1] = t;
}

void swapDouble(double *dp);
void swapArrayOfFloat(float* dp, register uint n);
void swapArrayOfDouble(double* dp, register uint n);

//------------------------------------------------------------------------------
// min/max
//------------------------------------------------------------------------------

template<typename T> 
inline const T& min(const T& a, const T& b) {
  return (a < b) ? a : b;
}

template<typename T> 
inline const T& max(const T& a, const T& b) {
  return (a > b) ? a : b;
}

//------------------------------------------------------------------------------
// min/max for arrays
//------------------------------------------------------------------------------

template<typename T> 
const T max(const T *a, unsigned int size) {
  T val = a[0];
  for (unsigned int i=0; i<size; ++i)
    if (val < a[i]) val = a[i];
  return val;
}

template<typename T> 
const T max(const std::vector<T> &a) {
  T val = a[0];
  for (unsigned int i=0; i<a.size(); ++i)
    if (val < a[i]) val = a[i];
  return val;
}

template<typename T> 
unsigned int maxix(const T *a, unsigned int size) {
  unsigned int i,ix;
  T val = a[0];
  ix = 0;
  for (i=0; i<size; ++i)
    if (val < a[i]) {
      val = a[i];
      ix = i;
    }
  return ix;
}

template<typename T> 
const T min(const T *a, unsigned int size) {
  T val = a[0];
  for (unsigned int i=0; i<size; ++i)
    if (val > a[i]) val = a[i];
  return val;
}

template<typename T> 
const T min(const std::vector<T> &a) {
  T val = a[0];
  for (unsigned int i=0; i<a.size(); ++i)
    if (val > a[i]) val = a[i];
  return val;
}

template<typename T> 
unsigned int minix(const T *a, unsigned int size) {
  unsigned int i,ix;
  T val = a[0];
  ix = 0;
  for (i=0; i<size; ++i)
    if (val > a[i]) {
      val = a[i];
      ix = i;
    }
  return ix;
}

//------------------------------------------------------------------------------
// utils
//------------------------------------------------------------------------------

template <typename T>
inline T trim(T v, T min_v, T max_v) {
  if (v < min_v) return min_v;
  if (v > max_v) return max_v;
  return v;
}

template <typename To, typename Ti>
inline To trim( Ti val, To min, To max ) {
  if (val < min) return min;
  if (val > max) return max;
  return (To) val;
}

//------------------------------------------------------------------------------
// little math
//------------------------------------------------------------------------------

template <typename T>
inline T round( double x ) {
  return (T) floor(x + 0.5);
}

template <typename T>
inline T round( float x ) {
  return (T) floor(x + 0.5f);
}

// round number n to d decimal points
template <typename T>
inline T round(double x, int d) {
  return (T) floor(x * pow(10., d) + .5) / pow(10., d);
}

template <typename T>
T power(T base, int index) {

  if (index < 0) { 
    return 1.0/pow( base, -index );
  }
  else
  return pow( base, index );
}

template <typename T>
inline T log2(T n) {
  return (T) (log((double)n)/log(2.0));
}

/*
template <typename T>
inline T ln(T n) {
  return (T) (log((double)n)/log(E));
}
*/

/*
float invSqrt (float x) {
  float xhalf = 0.5f*x;
  int i = *(int*)&x;
  i = 0x5f3759df - (i >> 1);
  x = *(float*)&i;
  x = x*(1.5f - xhalf*x*x);
  return x;
}
*/

} // namespace dim

#endif // DIM_XTYPES
