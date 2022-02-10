/* 
 * Copyright (C) 1999-2002 Free Software Foundation, Inc. 
 * This file is part of the GNU LIBICONV Library. 
 * 
 * The GNU LIBICONV Library is free software; you can redistribute it 
 * and/or modify it under the terms of the GNU Library General Public 
 * License as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version. 
 * 
 * The GNU LIBICONV Library is distributed in the hope that it will be 
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Library General Public License for more details. 
 * 
 * You should have received a copy of the GNU Library General Public 
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB. 
 * If not, write to the Free Software Foundation, Inc., 51 Franklin Street, 
 * Fifth Floor, Boston, MA 02110-1301, USA. 
 */ 
 
/* 
 * KOI8-T 
 */ 
 
static const unsigned short koi8_t_2uni[128] = { 
  /* 0x80 */ 
  0x049b, 0x0493, 0x201a, 0x0492, 0x201e, 0x2026, 0x2020, 0x2021, 
  0xfffd, 0x2030, 0x04b3, 0x2039, 0x04b2, 0x04b7, 0x04b6, 0xfffd, 
  /* 0x90 */ 
  0x049a, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 
  0xfffd, 0x2122, 0xfffd, 0x203a, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 
  /* 0xa0 */ 
  0xfffd, 0x04ef, 0x04ee, 0x0451, 0x00a4, 0x04e3, 0x00a6, 0x00a7, 
  0xfffd, 0xfffd, 0xfffd, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0xfffd, 
  /* 0xb0 */ 
  0x00b0, 0x00b1, 0x00b2, 0x0401, 0xfffd, 0x04e2, 0x00b6, 0x00b7, 
  0xfffd, 0x2116, 0xfffd, 0x00bb, 0xfffd, 0xfffd, 0xfffd, 0x00a9, 
  /* 0xc0 */ 
  0x044e, 0x0430, 0x0431, 0x0446, 0x0434, 0x0435, 0x0444, 0x0433, 
  0x0445, 0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 
  /* 0xd0 */ 
  0x043f, 0x044f, 0x0440, 0x0441, 0x0442, 0x0443, 0x0436, 0x0432, 
  0x044c, 0x044b, 0x0437, 0x0448, 0x044d, 0x0449, 0x0447, 0x044a, 
  /* 0xe0 */ 
  0x042e, 0x0410, 0x0411, 0x0426, 0x0414, 0x0415, 0x0424, 0x0413, 
  0x0425, 0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 
  /* 0xf0 */ 
  0x041f, 0x042f, 0x0420, 0x0421, 0x0422, 0x0423, 0x0416, 0x0412, 
  0x042c, 0x042b, 0x0417, 0x0428, 0x042d, 0x0429, 0x0427, 0x042a, 
}; 
 
static int 
koi8_t_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *s, int n) 
{ 
  unsigned char c = *s; 
  if (c < 0x80) { 
    *pwc = (ucs4_t) c; 
    return 1; 
  } 
  else { 
    unsigned short wc = koi8_t_2uni[c-0x80]; 
    if (wc != 0xfffd) { 
      *pwc = (ucs4_t) wc; 
      return 1; 
    } 
  } 
  return RET_ILSEQ; 
} 
 
static const unsigned char koi8_t_page00[32] = { 
  0x00, 0x00, 0x00, 0x00, 0xa4, 0x00, 0xa6, 0xa7, /* 0xa0-0xa7 */ 
  0x00, 0xbf, 0x00, 0xab, 0xac, 0xad, 0xae, 0x00, /* 0xa8-0xaf */ 
  0xb0, 0xb1, 0xb2, 0x00, 0x00, 0x00, 0xb6, 0xb7, /* 0xb0-0xb7 */ 
  0x00, 0x00, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */ 
}; 
static const unsigned char koi8_t_page04[240] = { 
  0x00, 0xb3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x00-0x07 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x08-0x0f */ 
  0xe1, 0xe2, 0xf7, 0xe7, 0xe4, 0xe5, 0xf6, 0xfa, /* 0x10-0x17 */ 
  0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, /* 0x18-0x1f */ 
  0xf2, 0xf3, 0xf4, 0xf5, 0xe6, 0xe8, 0xe3, 0xfe, /* 0x20-0x27 */ 
  0xfb, 0xfd, 0xff, 0xf9, 0xf8, 0xfc, 0xe0, 0xf1, /* 0x28-0x2f */ 
  0xc1, 0xc2, 0xd7, 0xc7, 0xc4, 0xc5, 0xd6, 0xda, /* 0x30-0x37 */ 
  0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, /* 0x38-0x3f */ 
  0xd2, 0xd3, 0xd4, 0xd5, 0xc6, 0xc8, 0xc3, 0xde, /* 0x40-0x47 */ 
  0xdb, 0xdd, 0xdf, 0xd9, 0xd8, 0xdc, 0xc0, 0xd1, /* 0x48-0x4f */ 
  0x00, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50-0x57 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x58-0x5f */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x60-0x67 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x68-0x6f */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x78-0x7f */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80-0x87 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */ 
  0x00, 0x00, 0x83, 0x81, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */ 
  0x00, 0x00, 0x90, 0x80, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa0-0xa7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa8-0xaf */ 
  0x00, 0x00, 0x8c, 0x8a, 0x00, 0x00, 0x8e, 0x8d, /* 0xb0-0xb7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xb8-0xbf */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd0-0xd7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */ 
  0x00, 0x00, 0xb5, 0xa5, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa2, 0xa1, /* 0xe8-0xef */ 
}; 
static const unsigned char koi8_t_page20[48] = { 
  0x00, 0x00, 0x00, 0x96, 0x97, 0x00, 0x00, 0x00, /* 0x10-0x17 */ 
  0x91, 0x92, 0x82, 0x00, 0x93, 0x94, 0x84, 0x00, /* 0x18-0x1f */ 
  0x86, 0x87, 0x95, 0x00, 0x00, 0x00, 0x85, 0x00, /* 0x20-0x27 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x28-0x2f */ 
  0x89, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30-0x37 */ 
  0x00, 0x8b, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x38-0x3f */ 
}; 
static const unsigned char koi8_t_page21[24] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb9, 0x00, /* 0x10-0x17 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x18-0x1f */ 
  0x00, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x20-0x27 */ 
}; 
 
static int 
koi8_t_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n) 
{ 
  unsigned char c = 0; 
  if (wc < 0x0080) { 
    *r = wc; 
    return 1; 
  } 
  else if (wc >= 0x00a0 && wc < 0x00c0) 
    c = koi8_t_page00[wc-0x00a0]; 
  else if (wc >= 0x0400 && wc < 0x04f0) 
    c = koi8_t_page04[wc-0x0400]; 
  else if (wc >= 0x2010 && wc < 0x2040) 
    c = koi8_t_page20[wc-0x2010]; 
  else if (wc >= 0x2110 && wc < 0x2128) 
    c = koi8_t_page21[wc-0x2110]; 
  if (c != 0) { 
    *r = c; 
    return 1; 
  } 
  return RET_ILUNI; 
} 
