#include "CRC32.hpp"

uint32_t crc32_table[256];

void CRC32::generate_crc32_table() {
  uint32_t crc;
  for (uint32_t i = 0; i < 256; i++) {
    crc = i;
    for (uint8_t j = 0; j < 8; j++) {
      crc = (crc & 1) ? (crc >> 1) ^ 0xEDB88320 : crc >> 1;
    }
    crc32_table[i] = crc;
  }
}

std::string CRC32::hash(const char *data, size_t length) {
  // Гарантируем, что таблица инициализирована перед началом вычислений
  static bool table_initialized = false;
  if (!table_initialized) {
    generate_crc32_table();
    table_initialized = true;
  }

  // Начинаем с начального значения 0xFFFFFFFF
  uint32_t crc = 0xFFFFFFFF;

  for (size_t i = 0; i < length; i++) {
    uint8_t index = (crc ^ data[i]) & 0xFF;
    crc = (crc >> 8) ^ crc32_table[index];
  }

  // Инвертируем результат перед возвращением
  crc ^= 0xFFFFFFFF;
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(8) << crc;

  return ss.str();
}
