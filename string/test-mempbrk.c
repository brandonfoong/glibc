/* Test and measure strpbrk functions.
   Copyright (C) 1999-2022 Free Software Foundation, Inc.
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

#ifndef WIDE
# define CHAR char
# define UCHAR unsigned char
# define STRLEN strlen
# define STRCHR strchr
# define BIG_CHAR CHAR_MAX
# define SMALL_CHAR 127
#else
# include <wchar.h>
# define CHAR wchar_t
# define UCHAR wchar_t
# define STRLEN wcslen
# define STRCHR wcschr
# define BIG_CHAR WCHAR_MAX
# define SMALL_CHAR 1273
#endif /* WIDE */

#ifndef MEMPBRK_RESULT
# define TEST_MAIN
# ifndef WIDE
#  define TEST_NAME "mempbrk"
# else
#  define TEST_NAME "wmempbrk"
# endif /* WIDE */
# include "test-string.h"

# ifndef WIDE
#  define MEMPBRK mempbrk
#  define SIMPLE_MEMPBRK simple_mempbrk
# else
#  include <wchar.h>
#  define MEMPBRK wmempbrk
#  define SIMPLE_MEMPBRK simple_wmempbrk
# endif /* WIDE */

typedef void *(*proto_t) (const void *, const CHAR *, size_t);

IMPL (MEMPBRK, 1)

/* Naive implementation to verify results.  */

CHAR *
SIMPLE_MEMPBRK (const void *s, const CHAR *rej, size_t n)
{
  const CHAR *r;
  CHAR c;

  while (c = *(char *) s++, n--)
    for (r = rej; *r != '\0'; ++r)
      if (*r == c)
       return (void *) s - 1;
  return NULL;
}

#endif /* !MEMPBRK_RESULT */

static void
do_one_test (impl_t *impl, const void *s, const CHAR *acc, size_t n, void * exp_res)
{
  void * res = CALL (impl, s, acc, n);
  if (res != exp_res)
    {
      error (0, 0, "Wrong result in function %s %p %p", impl->name,
	      res, exp_res);
      ret = 1;
      return;
    }
}

static void
do_test (size_t align, size_t pos, size_t len)
{
  size_t i;
  int c, byte_exists;
  void * result;
  CHAR *rej, *s;

  align &= 7;
  if ((align + pos + 10) * sizeof (CHAR) >= page_size || len > 240)
    return;

  rej = (CHAR *) (buf2) + (random () & 255);
  s = (CHAR *) (buf1) + align;

  for (i = 0; i < len; ++i)
    {
      rej[i] = random () & BIG_CHAR;
      if (!rej[i])
	rej[i] = random () & BIG_CHAR;
      if (!rej[i])
	rej[i] = 1 + (random () & SMALL_CHAR);
    }
  rej[len] = '\0';
  for (c = 1; c <= BIG_CHAR; ++c)
    if (STRCHR (rej, c) == NULL)
      break;

  for (i = 0; i < pos + 10; ++i)
    {
      s[i] = random () & BIG_CHAR;
      if (STRCHR (rej, s[i]))
	{
	  s[i] = random () & BIG_CHAR;
	  if (STRCHR (rej, s[i]))
	    s[i] = c;
	}
    }
  byte_exists = (random() & 1) && len;
  s[pos] = byte_exists ? rej[random () % len] : s[pos];
  result = byte_exists ? s + pos : NULL;

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, s, rej, pos + 10, result);
}

static void
do_random_tests (void)
{
  size_t i, j, n, align, pos, len, rlen;
  void * result;
  int c, byte_exists;
  UCHAR *p = (UCHAR *) (buf1 + page_size) - 512;
  UCHAR *rej;

  for (n = 0; n < ITERATIONS; n++)
    {
      align = random () & 15;
      pos = random () & 511;
      if (pos + align >= 511)
	pos = 510 - align - (random () & 7);
      len = random () & 511;
      if (pos >= len && (random () & 1))
	len = pos + 1 + (random () & 7);
      if (len + align >= 512)
	len = 511 - align - (random () & 7);
      if (random () & 1)
	rlen = random () & 63;
      else
	rlen = random () & 15;
      rej = (UCHAR *) (buf2 + page_size) - rlen - 1 - (random () & 7);
      for (i = 0; i < rlen; ++i)
	{
	  rej[i] = random () & BIG_CHAR;
	  if (!rej[i])
	    rej[i] = random () & BIG_CHAR;
	  if (!rej[i])
	    rej[i] = 1 + (random () & SMALL_CHAR);
	}
      rej[i] = '\0';
      for (c = 1; c <= BIG_CHAR; ++c)
	if (STRCHR ((CHAR *) rej, c) == NULL)
	  break;
      j = (pos > len ? pos : len) + align + 64;
      if (j > 512)
	j = 512;

      byte_exists = (random() & 1) && rlen;
      for (i = 0; i < j; i++)
	{
	  if (i == pos + align)
	    p[i] = byte_exists ? rej[random () % rlen] : '\0';
	  else if (i < align || i > len + align)
	    p[i] = random () & BIG_CHAR;
	  else
	    {
	      p[i] = random () & BIG_CHAR;
	      if (STRCHR ((CHAR *) rej, p[i]))
		{
		  p[i] = random () & BIG_CHAR;
		  if (STRCHR ((CHAR *) rej, p[i]))
		    p[i] = c;
		}
	    }
	}

      result = pos < len && byte_exists ? (CHAR *) p + align + pos : NULL;

      FOR_EACH_IMPL (impl, 1)
	if (CALL (impl, (CHAR *) p + align, (CHAR *) rej, len) != result)
	  {
	    error (0, 0, "Iteration %zd - wrong result in function %s (%zd, %p, %zd, %zd, %zd) %p != %p",
		   n, impl->name, align, rej, rlen, pos, len,
		   (void *) CALL (impl, (CHAR *) (p + align), (CHAR *) rej, len),
		   (void *) result);
	    ret = 1;
	  }
    }
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%32s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 0; i < 32; ++i)
    {
      do_test (0, 512, i);
      do_test (i, 512, i);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 16 << i, 4);
      do_test (i, 16 << i, 4);
    }

  for (i = 1; i < 8; ++i)
    do_test (i, 64, 10);

  for (i = 0; i < 64; ++i)
    do_test (0, i, 6);

  do_random_tests ();
  return ret;
}

#include <support/test-driver.c>
