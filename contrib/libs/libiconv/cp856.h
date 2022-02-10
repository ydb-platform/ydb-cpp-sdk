/* 
 * Copyright (C) 1999-2001 Free Software Foundation, Inc. 
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
 * CP856 
 */ 
 
static const unsigned short cp856_2uni[128] = { 
  /* 0x80 */ 
  0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7, 
  0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df, 
  /* 0x90 */ 
  0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7, 
  0x05e8, 0x05e9, 0x05ea, 0xfffd, 0x00a3, 0xfffd, 0x00d7, 0xfffd, 
  /* 0xa0 */ 
  0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 
  0xfffd, 0x00ae, 0x00ac, 0x00bd, 0x00bc, 0xfffd, 0x00ab, 0x00bb, 
  /* 0xb0 */ 
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0xfffd, 0xfffd, 0xfffd, 
  0x00a9, 0x2563, 0x2551, 0x2557, 0x255d, 0x00a2, 0x00a5, 0x2510, 
  /* 0xc0 */ 
  0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0xfffd, 0xfffd, 
  0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x00a4, 
  /* 0xd0 */ 
  0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 
  0xfffd, 0x2518, 0x250c, 0x2588, 0x2584, 0x00a6, 0xfffd, 0x2580, 
  /* 0xe0 */ 
  0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0x00b5, 0xfffd, 
  0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0x00af, 0x00b4, 
  /* 0xf0 */ 
  0x00ad, 0x00b1, 0x2017, 0x00be, 0x00b6, 0x00a7, 0x00f7, 0x00b8, 
  0x00b0, 0x00a8, 0x00b7, 0x00b9, 0x00b3, 0x00b2, 0x25a0, 0x00a0, 
}; 
 
static int 
cp856_mbtowc (conv_t conv, ucs4_t *pwc, const unsigned char *s, int n) 
{ 
  unsigned char c = *s; 
  if (c < 0x80) { 
    *pwc = (ucs4_t) c; 
    return 1; 
  } 
  else { 
    unsigned short wc = cp856_2uni[c-0x80]; 
    if (wc != 0xfffd) { 
      *pwc = (ucs4_t) wc; 
      return 1; 
    } 
  } 
  return RET_ILSEQ; 
} 
 
static const unsigned char cp856_page00[88] = { 
  0xff, 0x00, 0xbd, 0x9c, 0xcf, 0xbe, 0xdd, 0xf5, /* 0xa0-0xa7 */ 
  0xf9, 0xb8, 0x00, 0xae, 0xaa, 0xf0, 0xa9, 0xee, /* 0xa8-0xaf */ 
  0xf8, 0xf1, 0xfd, 0xfc, 0xef, 0xe6, 0xf4, 0xfa, /* 0xb0-0xb7 */ 
  0xf7, 0xfb, 0x00, 0xaf, 0xac, 0xab, 0xf3, 0x00, /* 0xb8-0xbf */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc0-0xc7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xc8-0xcf */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9e, /* 0xd0-0xd7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xd8-0xdf */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe0-0xe7 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf6, /* 0xf0-0xf7 */ 
}; 
static const unsigned char cp856_page05[32] = { 
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, /* 0xd0-0xd7 */ 
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f, /* 0xd8-0xdf */ 
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, /* 0xe0-0xe7 */ 
  0x98, 0x99, 0x9a, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xe8-0xef */ 
}; 
static const unsigned char cp856_page25[168] = { 
  0xc4, 0x00, 0xb3, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x00-0x07 */ 
  0x00, 0x00, 0x00, 0x00, 0xda, 0x00, 0x00, 0x00, /* 0x08-0x0f */ 
  0xbf, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, /* 0x10-0x17 */ 
  0xd9, 0x00, 0x00, 0x00, 0xc3, 0x00, 0x00, 0x00, /* 0x18-0x1f */ 
  0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, /* 0x20-0x27 */ 
  0x00, 0x00, 0x00, 0x00, 0xc2, 0x00, 0x00, 0x00, /* 0x28-0x2f */ 
  0x00, 0x00, 0x00, 0x00, 0xc1, 0x00, 0x00, 0x00, /* 0x30-0x37 */ 
  0x00, 0x00, 0x00, 0x00, 0xc5, 0x00, 0x00, 0x00, /* 0x38-0x3f */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40-0x47 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x48-0x4f */ 
  0xcd, 0xba, 0x00, 0x00, 0xc9, 0x00, 0x00, 0xbb, /* 0x50-0x57 */ 
  0x00, 0x00, 0xc8, 0x00, 0x00, 0xbc, 0x00, 0x00, /* 0x58-0x5f */ 
  0xcc, 0x00, 0x00, 0xb9, 0x00, 0x00, 0xcb, 0x00, /* 0x60-0x67 */ 
  0x00, 0xca, 0x00, 0x00, 0xce, 0x00, 0x00, 0x00, /* 0x68-0x6f */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70-0x77 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x78-0x7f */ 
  0xdf, 0x00, 0x00, 0x00, 0xdc, 0x00, 0x00, 0x00, /* 0x80-0x87 */ 
  0xdb, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x88-0x8f */ 
  0x00, 0xb0, 0xb1, 0xb2, 0x00, 0x00, 0x00, 0x00, /* 0x90-0x97 */ 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x98-0x9f */ 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xa0-0xa7 */ 
}; 
 
static int 
cp856_wctomb (conv_t conv, unsigned char *r, ucs4_t wc, int n) 
{ 
  unsigned char c = 0; 
  if (wc < 0x0080) { 
    *r = wc; 
    return 1; 
  } 
  else if (wc >= 0x00a0 && wc < 0x00f8) 
    c = cp856_page00[wc-0x00a0]; 
  else if (wc >= 0x05d0 && wc < 0x05f0) 
    c = cp856_page05[wc-0x05d0]; 
  else if (wc == 0x2017) 
    c = 0xf2; 
  else if (wc >= 0x2500 && wc < 0x25a8) 
    c = cp856_page25[wc-0x2500]; 
  if (c != 0) { 
    *r = c; 
    return 1; 
  } 
  return RET_ILUNI; 
} 
