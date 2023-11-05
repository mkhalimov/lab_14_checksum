#include "SHA256.hpp"

void SHA256::processBlock(const uint32_t *M, uint32_t *H) {
  uint32_t W[64];

  for (uint32_t t = 0; t < 16; ++t) {
    W[t] = M[t];
  }

  for (uint32_t t = 16; t < 64; ++t) {
    W[t] = ssig1(W[t - 2]) + W[t - 7] + ssig0(W[t - 15]) + W[t - 16];
  }

  uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
  uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

  for (uint32_t t = 0; t < 64; ++t) {
    uint32_t T1 = h + bsig1(e) + ch(e, f, g) + K[t] + W[t];
    uint32_t T2 = bsig0(a) + maj(a, b, c);
    h = g;
    g = f;
    f = e;
    e = d + T1;
    d = c;
    c = b;
    b = a;
    a = T1 + T2;
  }

  H[0] += a;
  H[1] += b;
  H[2] += c;
  H[3] += d;
  H[4] += e;
  H[5] += f;
  H[6] += g;
  H[7] += h;
}

std::string SHA256::hash(const void *data, size_t length) {
  uint32_t H[] = {
      0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
      0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
  };

  uint64_t bitLength = length * 8;
  const uint8_t *p = reinterpret_cast<const uint8_t *>(data);
  while (length >= 64) {
    uint32_t M[16];
    for (uint32_t i = 0; i < 16; ++i, p += 4) {
      M[i] = static_cast<uint32_t>(p[0]) << 24 |
             static_cast<uint32_t>(p[1]) << 16 |
             static_cast<uint32_t>(p[2]) << 8 | static_cast<uint32_t>(p[3]);
    }
    length -= 64;

    processBlock(M, H);
  }

  uint8_t buffer[128] = {0};
  memcpy(buffer, p, length);
  buffer[length] = 0x80;

  if (length >= 55) {
    uint32_t M[16] = {0};
    for (uint32_t i = 0; i < 15; ++i) {
      M[i] = static_cast<uint32_t>(buffer[i * 4]) << 24 |
             static_cast<uint32_t>(buffer[i * 4 + 1]) << 16 |
             static_cast<uint32_t>(buffer[i * 4 + 2]) << 8 |
             static_cast<uint32_t>(buffer[i * 4 + 3]);
    }

    processBlock(M, H);
    memset(buffer, 0, 128);
  }

  uint32_t *M = reinterpret_cast<uint32_t *>(buffer);
  for (uint32_t i = 0, k = 0; i < 14; ++i, k += 4) {
    M[i] = static_cast<uint32_t>(buffer[k]) << 24 |
           static_cast<uint32_t>(buffer[k + 1]) << 16 |
           static_cast<uint32_t>(buffer[k + 2]) << 8 |
           static_cast<uint32_t>(buffer[k + 3]);
  }
  M[14] = static_cast<uint32_t>(bitLength >> 32);
  M[15] = static_cast<uint32_t>(bitLength);

  processBlock(M, H);

  std::stringstream ss;
  ss << std::hex << std::setfill('0');
  for (uint32_t h : H) {
    ss << std::setw(8) << h;
  }
  return ss.str();
}
