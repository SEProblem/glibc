/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.  */

/* This particular implementation was written by Eric Blake, 2008.  */

#ifndef _LIBC
# include <config.h>
#endif

/* Specification of memmem.  */
#include <string.h>

#ifndef _LIBC
# define __builtin_expect(expr, val)   (expr)
# define __memmem	memmem
#endif

#define RETURN_TYPE void *
#define AVAILABLE(h, h_l, j, n_l) ((j) <= (h_l) - (n_l))
#define FASTSEARCH(S,C,N) (void*) memchr ((void *)(S), (C), (N))
#include "str-two-way.h"

#undef memmem

/* Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
   if NEEDLE_LEN is 0, otherwise NULL if NEEDLE is not found in
   HAYSTACK.  */
void *
__memmem (const void *haystack_start, size_t haystack_len,
	  const void *needle_start, size_t needle_len)
{
  /* Abstract memory is considered to be an array of 'unsigned char' values,
     not an array of 'char' values.  See ISO C 99 section 6.2.6.1.  */
  const unsigned char *haystack = (const unsigned char *) haystack_start;
  const unsigned char *needle = (const unsigned char *) needle_start;

  if (needle_len == 0)
    /* The first occurrence of the empty string is deemed to occur at
       the beginning of the string.  */
    return (void *) haystack;

  /* Sanity check, otherwise the loop might search through the whole
     memory.  */
  if (__glibc_unlikely (haystack_len < needle_len))
    return NULL;

  /* Use optimizations in memchr when possible, to reduce the search
     size of haystack using a linear algorithm with a smaller
     coefficient.  However, avoid memchr for long needles, since we
     can often achieve sublinear performance.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    {
      haystack = memchr (haystack, *needle, haystack_len);
      if (!haystack || __builtin_expect (needle_len == 1, 0))
	return (void *) haystack;
      haystack_len -= haystack - (const unsigned char *) haystack_start;
      if (haystack_len < needle_len)
	return NULL;
      /* Check whether we have a match.  This improves performance since we
	 avoid the initialization overhead of the two-way algorithm.  */
      if (memcmp (haystack, needle, needle_len) == 0)
	return (void *) haystack;
      return two_way_short_needle (haystack, haystack_len, needle, needle_len);
    }
  else
    return two_way_long_needle (haystack, haystack_len, needle, needle_len);
}
libc_hidden_def (__memmem)
weak_alias (__memmem, memmem)
libc_hidden_weak (memmem)

#undef LONG_NEEDLE_THRESHOLD
