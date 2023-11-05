#include "MD5.hpp"


std::string MD5::hash(const uint8_t* data, size_t len) {
    uint32_t a0 = 0x67452301;
    uint32_t b0 = 0xefcdab89;
    uint32_t c0 = 0x98badcfe;
    uint32_t d0 = 0x10325476;

    size_t padded_len = ((len + 8) / 64 + 1) * 64;
    uint8_t* padded_data = new uint8_t[padded_len];
    memcpy(padded_data, data, len);
    padded_data[len] = 0x80;

    for (size_t i = len + 1; i < padded_len - 8; ++i) {
        padded_data[i] = 0;
    }

    uint64_t bit_len = len * 8;
    memcpy(padded_data + padded_len - 8, &bit_len, 8);

    uint32_t A, B, C, D, F, g, temp;
    for (size_t i = 0; i < padded_len; i += 64) {
        A = a0;
        B = b0;
        C = c0;
        D = d0;

        for (uint32_t j = 0; j < 64; ++j) {
            if (j <= 15) {
                F = (B & C) | ((~B) & D);
                g = j;
            } else if (j <= 31) {
                F = (D & B) | ((~D) & C);
                g = (5*j + 1) % 16;
            } else if (j <= 47) {
                F = B ^ C ^ D;
                g = (3*j + 5) % 16;
            } else {
                F = C ^ (B | (~D));
                g = (7*j) % 16;
            }

            uint32_t m = i + g * 4;
            F += A + K[j] + (padded_data[m+3]<<24 | padded_data[m+2]<<16 | padded_data[m+1]<<8 | padded_data[m+0]);
            A = D;
            D = C;
            C = B;
            B += leftRotate(F, s[j]);
        }

        a0 += A;
        b0 += B;
        c0 += C;
        d0 += D;
    }

    delete[] padded_data;

    uint8_t hash[16];
    memcpy(hash, &a0, 4);
    memcpy(hash + 4, &b0, 4);
    memcpy(hash + 8, &c0, 4);
    memcpy(hash + 12, &d0, 4);

    char result[33];
    for (int i = 0; i < 16; ++i) {
        sprintf(result + i*2, "%02x", hash[i]);
    }
    result[32] = '\0';

    return std::string(result);
}
