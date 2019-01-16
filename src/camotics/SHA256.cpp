/******************************************************************************\

  CAMotics is an Open-Source simulation and CAM software.
  Copyright (C) 2011-2019 Joseph Coffland <joseph@cauldrondevelopment.com>

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

/*********************************************************************
 Original code from https://github.com/B-Con/crypto-algorithms

 * Author:     Brad Conte (brad AT bradconte.com)
 * Copyright:
 * Disclaimer: This code is presented "as is" without any guarantees.
 * Details:    Implementation of the SHA-256 hashing algorithm.
               SHA-256 is one of the three algorithms in the SHA2
               specification. The others, SHA-384 and SHA-512, are not
               offered in this implementation.
               Algorithm specification can be found here:
               * http://csrc.nist.gov/publications/fips/fips180-2/
                    fips180-2withchangenotice.pdf
               This implementation uses little endian byte order.
*********************************************************************/

#include "SHA256.h"

#include <cbang/String.h>

#include <string.h>

using namespace CAMotics;
using namespace std;


#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32 - (b))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x, 2) ^ ROTRIGHT(x, 13) ^ ROTRIGHT(x, 22))
#define EP1(x) (ROTRIGHT(x, 6) ^ ROTRIGHT(x, 11) ^ ROTRIGHT(x, 25))
#define SIG0(x) (ROTRIGHT(x, 7) ^ ROTRIGHT(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x, 17) ^ ROTRIGHT(x, 19) ^ ((x) >> 10))


void SHA256::init() {
  datalen = bitlen = 0;
  state[0] = 0x6a09e667;
  state[1] = 0xbb67ae85;
  state[2] = 0x3c6ef372;
  state[3] = 0xa54ff53a;
  state[4] = 0x510e527f;
  state[5] = 0x9b05688c;
  state[6] = 0x1f83d9ab;
  state[7] = 0x5be0cd19;
}


void SHA256::update(const uint8_t data[], size_t len) {
  for (uint32_t i = 0; i < len; i++) {
    this->data[datalen++] = data[i];

    if (datalen == 64) {
      transform();
      bitlen += 512;
      datalen = 0;
    }
  }
}


void SHA256::update(const char *data, streamsize len) {
  update((const uint8_t *)data, (size_t)len);
}


void SHA256::update(const string &data) {
  update((const uint8_t *)CBANG_CPP_TO_C_STR(data), data.length());
}


void SHA256::finalize(uint8_t hash[32]) {
  uint32_t i = datalen;

  // Pad whatever data is left in the buffer.
  if (datalen < 56) {
    data[i++] = 0x80;
    while (i < 56) data[i++] = 0;

  } else {
    data[i++] = 0x80;
    while (i < 64) data[i++] = 0;
    transform();
    memset(data, 0, 56);
  }

  // Append to the padding the total message's length in bits and transform.
  bitlen += datalen * 8;
  data[63] = bitlen;
  data[62] = bitlen >> 8;
  data[61] = bitlen >> 16;
  data[60] = bitlen >> 24;
  data[59] = bitlen >> 32;
  data[58] = bitlen >> 40;
  data[57] = bitlen >> 48;
  data[56] = bitlen >> 56;

  transform();

  // Since this implementation uses little endian byte ordering and SHA uses
  // big endian, reverse all the bytes when copying the final state to the
  // output hash.
  for (uint32_t i = 0; i < 4; i++) {
    hash[i]      = (state[0] >> (24 - i * 8)) & 0xff;
    hash[i + 4]  = (state[1] >> (24 - i * 8)) & 0xff;
    hash[i + 8]  = (state[2] >> (24 - i * 8)) & 0xff;
    hash[i + 12] = (state[3] >> (24 - i * 8)) & 0xff;
    hash[i + 16] = (state[4] >> (24 - i * 8)) & 0xff;
    hash[i + 20] = (state[5] >> (24 - i * 8)) & 0xff;
    hash[i + 24] = (state[6] >> (24 - i * 8)) & 0xff;
    hash[i + 28] = (state[7] >> (24 - i * 8)) & 0xff;
  }
}


string SHA256::finalize() {
  uint8_t hash[32];
  finalize(hash);
  return string((char *)hash, 32);
}


void SHA256::transform() {
  static const uint32_t k[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,
    0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,
    0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,
    0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,
    0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,
    0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,
    0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,
    0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,
    0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
  };

  uint32_t m[64];
  uint32_t i;
  uint32_t j;

  for (i = 0, j = 0; i < 16; i++, j += 4)
    m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) |
      (data[j + 3]);

  for (; i < 64; i++)
    m[i] = SIG1(m[i - 2]) + m[i - 7] + SIG0(m[i - 15]) + m[i - 16];

  uint32_t a = state[0];
  uint32_t b = state[1];
  uint32_t c = state[2];
  uint32_t d = state[3];
  uint32_t e = state[4];
  uint32_t f = state[5];
  uint32_t g = state[6];
  uint32_t h = state[7];

  for (int i = 0; i < 64; i++) {
    uint32_t t1 = h + EP1(e) + CH(e, f, g) + k[i] + m[i];
    uint32_t t2 = EP0(a) + MAJ(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + t1;
    d = c;
    c = b;
    b = a;
    a = t1 + t2;
  }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  state[5] += f;
  state[6] += g;
  state[7] += h;
}
