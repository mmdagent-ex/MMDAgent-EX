// Copyright 2009 The RE2 Authors.  All Rights Reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

/* ----------------------------------------------------------------- */
/*           The Toolkit for Building Voice Interaction Systems      */
/*           "MMDAgent" developed by MMDAgent Project Team           */
/*           http://www.mmdagent.jp/                                 */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2015  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAgent project team nor the names of  */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef RE2_UTIL_UTIL_H__
#define RE2_UTIL_UTIL_H__

// C
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>     // For size_t
#include <assert.h>
#include <stdarg.h>
#include <time.h>       // For clock_gettime, CLOCK_REALTIME
#include <ctype.h>      // For isdigit, isalpha

#if !defined(_WIN32)
#include <sys/time.h>   // For gettimeofday
#endif

// C++
#include <ctime>
#include <vector>
#include <string>
#include <algorithm>
#include <iosfwd>
#include <map>
#include <stack>
#include <ostream>
#include <utility>
#include <set>

// Use std names.
using std::set;
using std::pair;
using std::vector;
using std::string;
using std::min;
using std::max;
using std::ostream;
using std::map;
using std::stack;
using std::sort;
using std::swap;
using std::make_pair;

#if defined(__GNUC__) && !defined(USE_CXX0X) && !defined(_LIBCPP_ABI_VERSION)

#include <tr1/unordered_set>
using std::tr1::unordered_set;

#else

#include <unordered_set>
#if defined(_WIN32)// || defined(__ANDROID__) // for android ndk
using std::tr1::unordered_set;
#else
using std::unordered_set;
#endif

#endif

#ifdef _WIN32

#define snprintf _snprintf_s
#define sprintf sprintf_s
#define stricmp _stricmp
#define strtof strtod /* not really correct but best we can do */
#define strtoll _strtoi64
#define strtoull _strtoui64
#define vsnprintf vsnprintf_s

#pragma warning(disable: 4018) // signed/unsigned mismatch
#pragma warning(disable: 4800) // conversion from int to bool

#endif

namespace re2 {

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned short ushort;

// Prevent the compiler from complaining about or optimizing away variables
// that appear unused.
#undef ATTRIBUTE_UNUSED
#if defined(__GNUC__)
#define ATTRIBUTE_UNUSED __attribute__ ((unused))
#else
#define ATTRIBUTE_UNUSED
#endif

// COMPILE_ASSERT causes a compile error about msg if expr is not true.
#if __cplusplus >= 201103L
#define COMPILE_ASSERT(expr, msg) static_assert(expr, #msg)
#else
template<bool> struct CompileAssert {};
#define COMPILE_ASSERT(expr, msg) \
  typedef CompileAssert<(bool(expr))> msg[bool(expr) ? 1 : -1] ATTRIBUTE_UNUSED
#endif

// DISALLOW_COPY_AND_ASSIGN disallows the copy and operator= functions.
// It goes in the private: declarations in a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);                 \
  void operator=(const TypeName&)

#define arraysize(array) (int)(sizeof(array)/sizeof((array)[0]))

class StringPiece;

string CEscape(const StringPiece& src);
int CEscapeString(const char* src, int src_len, char* dest, int dest_len);

extern string StringPrintf(const char* format, ...);
extern void SStringPrintf(string* dst, const char* format, ...);
extern void StringAppendF(string* dst, const char* format, ...);
extern string PrefixSuccessor(const StringPiece& prefix);

uint32 hashword(const uint32*, size_t, uint32);
void hashword2(const uint32*, size_t, uint32*, uint32*);

static inline uint32 Hash32StringWithSeed(const char* s, int len, uint32 seed) {
  return hashword((uint32*)s, len/4, seed);
}

static inline uint64 Hash64StringWithSeed(const char* s, int len, uint32 seed) {
  uint32 x, y;
  x = seed;
  y = 0;
  hashword2((uint32*)s, len/4, &x, &y);
  return ((uint64)x << 32) | y;
}

int RunningOnValgrind();

}  // namespace re2

#include "util/logging.h"
#include "util/mutex.h"
#include "util/utf.h"

#ifdef _WIN32
#ifndef va_copy
#define va_copy(dest, src) ((dest) = (src))
#endif /* va_copy */
#if defined(_MSC_VER) && (_MSC_VER <= 1600)
#define COMPILER_MSVC
#endif /* _MSC_VER <= 1600 */
#endif /* _WIN32 */

#endif // RE2_UTIL_UTIL_H__
