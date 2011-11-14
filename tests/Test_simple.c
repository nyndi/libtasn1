/*
 * Copyright (C) 2011 Free Software Foundation, Inc.
 *
 * This file is part of LIBTASN1.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Written by Simon Josefsson
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libtasn1.h"

struct tv
{
  const char *name;
  size_t bitlen;
  const char *bitstr;
  size_t derlen;
  const char *der;
};

static const struct tv tv[] = {
  { "foo", 0, "", 2, "\x01\x00" },
  { "foo", 1, "\xFF", 3, "\x02\x07\x80" },
  { "foo", 2, "\xFF", 3, "\x02\x06\xc0" },
  { "foo", 3, "\xFF", 3, "\x02\x05\xe0" },
  { "foo", 4, "\xFF", 3, "\x02\x04\xf0" },
  { "foo", 5, "\xFF", 3, "\x02\x03\xf8" },
  { "foo", 6, "\xFF", 3, "\x02\x02\xfc" },
  { "foo", 7, "\xFF", 3, "\x02\x01\xfe" },
  { "foo", 8, "\xFF\xFF", 3, "\x02\x00\xff" },
  { "foo", 9, "\xFF\xFF", 4, "\x03\x07\xff\x80" },
  { "foo", 10, "\xFF\xFF", 4, "\x03\x06\xff\xc0" },
  { "foo", 11, "\xFF\xFF", 4, "\x03\x05\xff\xe0" },
  { "foo", 12, "\xFF\xFF", 4, "\x03\x04\xff\xf0" },
  { "foo", 13, "\xFF\xFF", 4, "\x03\x03\xff\xf8" },
  { "foo", 14, "\xFF\xFF", 4, "\x03\x02\xff\xfc" },
  { "foo", 15, "\xFF\xFF", 4, "\x03\x01\xff\xfe" },
  { "foo", 16, "\xFF\xFF", 4, "\x03\x00\xff\xff" },
  { "foo", 17, "\xFF\xFF\xFF", 5, "\x04\x07\xff\xff\x80" },
  { "foo", 18, "\xFF\xFF\xFF", 5, "\x04\x06\xff\xff\xc0" },
  { "foo", 19, "\xFF\xFF\xFF", 5, "\x04\x05\xff\xff\xe0" },
};

int
main (int argc, char *argv[])
{
  asn1_retCode result;
  unsigned char der[100];
  unsigned char str[100];
  int der_len = sizeof (der);
  int str_size = sizeof (str);
  int ret_len, bit_len;
  size_t i;

  /* Dummy test */

  asn1_bit_der (NULL, 0, der, &der_len);
  result = asn1_get_bit_der (der, 0, &ret_len, str, str_size, &bit_len);
  if (result != ASN1_GENERIC_ERROR)
    {
      fprintf (stderr, "asn1_get_bit_der zero\n");
      return 1;
    }

  /* Encode short strings with increasing bit lengths */

  for (i = 0; i < sizeof (tv) / sizeof (tv[0]); i++)
    {
      /* Encode */

      asn1_bit_der (tv[i].bitstr, tv[i].bitlen, der, &der_len);
      if (der_len != tv[i].derlen
	  || memcmp (der, tv[i].der, der_len) != 0)
	{
	  fprintf (stderr, "asn1_bit_der iter %d\n", i);
	  return 1;
	}

      /* Decode it */

      result = asn1_get_bit_der (der, der_len, &ret_len, str,
				 str_size, &bit_len);
      if (result != ASN1_SUCCESS || ret_len != tv[i].derlen
	  || bit_len != tv[i].bitlen)
	{
	  fprintf (stderr, "asn1_get_bit_der iter %d\n", i);
	  return 1;
	}
    }


  /* Decode sample from "A Layman's Guide to a Subset of ASN.1, BER,
     and DER" section 5.4 "BIT STRING": "The BER encoding of the BIT
     STRING value "011011100101110111" can be any of the following,
     among others, depending on the choice of padding bits, the form
     of length octets [...]".
  */

  /* 03 04 06 6e 5d c0  DER encoding */

  memcpy (der, "\x04\x06\x6e\x5d\xc0", 5);
  der_len = 5;

  result = asn1_get_bit_der (der, der_len, &ret_len, str, str_size, &bit_len);
  if (result != ASN1_SUCCESS || ret_len != 5
      || bit_len != 18 || memcmp (str, "\x6e\x5d\xc0", 3) != 0)
    {
      fprintf (stderr, "asn1_get_bit_der example\n");
      return 1;
    }

  der_len = sizeof (der);
  asn1_bit_der (str, bit_len, der, &der_len);
  if (der_len != 5 || memcmp (der, "\x04\x06\x6e\x5d\xc0", 5) != 0)
    {
      fprintf (stderr, "asn1_bit_der example roundtrip\n");
      return 1;
    }

  /* 03 04 06 6e 5d e0 padded with "100000" */

  memcpy (der, "\x04\x06\x6e\x5d\xe0", 5);
  der_len = 5;

  result = asn1_get_bit_der (der, der_len, &ret_len, str, str_size, &bit_len);
  if (result != ASN1_SUCCESS || ret_len != 5
      || bit_len != 18 || memcmp (str, "\x6e\x5d\xe0", 3) != 0)
    {
      fprintf (stderr, "asn1_get_bit_der example padded\n");
      return 1;
    }

  der_len = sizeof (der);
  asn1_bit_der (str, bit_len, der, &der_len);
  if (der_len != 5 || memcmp (der, "\x04\x06\x6e\x5d\xc0", 5) != 0)
    {
      fprintf (stderr, "asn1_bit_der example roundtrip\n");
      return 1;
    }

  /* 03 81 04 06 6e 5d c0 long form of length octets */

  memcpy (der, "\x81\x04\x06\x6e\x5d\xc0", 6);
  der_len = 6;

  result = asn1_get_bit_der (der, der_len, &ret_len, str, str_size, &bit_len);

  if (result != ASN1_SUCCESS || ret_len != 6
      || bit_len != 18 || memcmp (str, "\x6e\x5d\xc0", 3) != 0)
    {
      fprintf (stderr, "asn1_get_bit_der example long form\n");
      return 1;
    }

  der_len = sizeof (der);
  asn1_bit_der (str, bit_len, der, &der_len);
  if (der_len != 5 || memcmp (der, "\x04\x06\x6e\x5d\xc0", 5) != 0)
    {
      fprintf (stderr, "asn1_bit_der example roundtrip\n");
      return 1;
    }

  return 0;
}
