/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2022 Joseph Coffland <joseph@cauldrondevelopment.com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

\******************************************************************************/

#if defined(__GNUC__) && !defined(__clang__) && !defined(_WIN32)

#include <string.h>
#include <math.h>

#if defined(__aarch64__)
#define GLIBC_SYMVER "2.17"
#else
#define GLIBC_SYMVER "2.2.5"
#endif

__asm__(".symver log,log@GLIBC_"       GLIBC_SYMVER);
__asm__(".symver logf,logf@GLIBC_"     GLIBC_SYMVER);
__asm__(".symver exp,exp@GLIBC_"       GLIBC_SYMVER);
__asm__(".symver expf,expf@GLIBC_"     GLIBC_SYMVER);
__asm__(".symver pow,pow@GLIBC_"       GLIBC_SYMVER);
__asm__(".symver powf,powf@GLIBC_"     GLIBC_SYMVER);
__asm__(".symver memcpy,memcpy@GLIBC_" GLIBC_SYMVER);

double __wrap_log(double x)           {return log (x);}
float  __wrap_logf(float x)           {return logf(x);}
double __wrap_exp(double x)           {return exp (x);}
float  __wrap_expf(float x)           {return expf(x);}
double __wrap_pow(double x, double y) {return pow (x, y);}
float  __wrap_powf(float x, float  y) {return powf(x, y);}


void *__wrap_memcpy(void *dst, const void *src, size_t n) {
  return memcpy(dst, src, n);
}

#endif
