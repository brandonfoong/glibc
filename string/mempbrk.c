/* Copyright (C) 1991-2022 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <https://www.gnu.org/licenses/>.  */

#include <string.h>
#include <stdint.h>

#undef mempbrk

#ifndef MEMPBRK
# define MEMPBRK mempbrk
#endif

/* Find the first occurrence of any character in ACCEPT
   in the first N bytes of S. */
void *
MEMPBRK (const void *s, const char *accept, size_t n)
{
  const unsigned char *t;

  if (__glibc_unlikely (accept[0] == '\0'))
    return NULL;

  if (__glibc_unlikely (accept[1] == '\0'))
    return memchr (s, accept[0], n);

  /* Use multiple small memsets to enable inlining on most targets.  */
  unsigned char table[256];
  unsigned char *p = memset (table, 0, 64);
  memset (p + 64, 0, 64);
  memset (p + 128, 0, 64);
  memset (p + 192, 0, 64);

  for (t = (unsigned char*) accept; *t; ++t)
    p[*t] = 1;

  for (t = (unsigned char*) s; n > 0; --n, ++t)
    if (p[*t])
      return (void *) t;

  return NULL;
}
libc_hidden_builtin_def (mempbrk)
