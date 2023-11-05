#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace CRC32 {
void generate_crc32_table();
std::string hash(const char *data, size_t length);
} // namespace CRC32
