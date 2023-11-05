#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <array>
#include <future>
#include <filesystem>

#include "CRC32.cpp"
#include "MD5.cpp"
#include "SHA256.cpp"

// Функция для вычисления контрольной суммы файла
std::string calculate_checksum(const std::string& file_path, const std::string& algorithm) {
    if (algorithm != "crc32" & algorithm != "md5" & algorithm != "sha256") {
        std::cerr << "Unknown algorithm: " << algorithm << std::endl;
        return "";
    }

    std::ifstream file(file_path);

    if (!file) {
        std::cerr << "Не удалось открыть файл." << std::endl;
        return "";
    }

    // Получаем размер файла
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Выделяем память для хранения содержимого файла
    std::vector<char> buffer(fileSize);

    // Считываем содержимое файла в буфер
    file.read(buffer.data(), fileSize);

    // Закрываем файл
    file.close();

    // Копируем содержимое буфера в массив типа char*
    char* data = new char[fileSize]; // +1 для завершающего нулевого символа
    std::copy(buffer.begin(), buffer.end(), data);
    // data[fileSize] = '\0'; // Завершающий нулевой символ
    std::string result;
    if (algorithm == "crc32")
    {
        result = CRC32::hash(data, fileSize);
    } else if (algorithm == "md5")
    {
        result = MD5::hash((uint8_t*) data, fileSize);
    } else if (algorithm == "sha256")
    {
        result = SHA256::hash(data, fileSize);
    }
    else
    {
        std::cerr << "WATAFAK bitch ya molodoy tupak";
        return "";
    }

    // Обработка вывода и получение контрольной суммы
    std::stringstream ss(result);
    std::string checksum;
    ss >> checksum;
    return checksum;
}

// Функция для чтения контрольных сумм из файла Checksum.ini
std::map<std::string, std::string> read_checksums() {
    std::map<std::string, std::string> checksums;
    std::ifstream ini_file("Checksum.ini");
    if (!ini_file.is_open()) {
        return checksums;
    }

    std::string line;
    std::string current_section;
    while (std::getline(ini_file, line)) {
        // Удаление комментариев
        line.erase(std::find(line.begin(), line.end(), ';'), line.end());
        line.erase(std::find(line.begin(), line.end(), '#'), line.end());

        // Обработка секций
        if (line.front() == '[') {
            int ind = line.find(']');
            current_section = line.substr(1, ind - 1);
            line = line.substr(ind + 2, line.length() - ind - 2);

        }

        // Обработка ключей и значений
        std::stringstream ss(line);
        std::string key;
        std::string value;
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            std::string file_path = key;
            std::string checksum = value;

            // Преобразование строки контрольной суммы
            if (checksum.size() >= 2 && checksum.substr(0, 2) == "0x") {
                checksum = checksum.substr(2);
            }

            // Добавление записи в карту контрольных сумм
            if (!current_section.empty()) {
                file_path = file_path;
            }

            checksums[file_path] = checksum;
        }
    }

    return checksums;
}

// Функция для асинхронного вычисления контрольной суммы файла
std::string async_calculate_checksum(const std::string& file_path, const std::string& algorithm) {
    std::string checksum;
    try {
        checksum = calculate_checksum(file_path, algorithm);
    } catch (const std::exception& e) {
        std::cerr << "Error calculating checksum for file: " << file_path << std::endl;
        std::cerr << "Error message: " << e.what() << std::endl;
    }

    return checksum;
}

int main(int argc, char* argv[]) {
    int state = 0;
    std::map<std::string, std::string> checksums;
    std::string algorithm = "md5";
    // Обработка параметров командной строки
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-a") {
            if (i + 1 < argc) {
                algorithm = argv[i + 1];
                state = 2;
                i++;
            } else {
                std::cerr << "Missing algorithm parameter" << std::endl;
                return 1;
            }
        } else if (arg.front() == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            return 1;
        } else {
            state = 3;
            checksums[arg] = "";
        }
    }

    // Чтение контрольных сумм из файла Checksum.ini
    
    if (argc == 1) {
        checksums = read_checksums();
        state = 1;

    }

    // Если файлы не указаны явно, читаем из stdin
    // if (file_paths.empty() && checksums.empty()) {
    if (state == 2) {
        std::string line;
        while (std::getline(std::cin, line)) {
            std::stringstream ss(line);
            std::string file_name;
            std::string checksum;
            if (std::getline(ss, file_name, '\t') && std::getline(ss, checksum)) {
                checksums[file_name] = checksum;
            }
        }
    }
    if (state == 1){}
    // Асинхронное вычисление контрольных сумм файлов
    std::vector<std::future<std::pair<std::string, std::string>>> futures;
    for (const auto& sum : checksums) {
        futures.push_back(std::async(std::launch::async, [](const std::string& path, const std::string& algo) {
            return std::make_pair(path, async_calculate_checksum(path, algo));
        }, sum.first, algorithm));
    }

    // Проверка контрольных сумм файлов
    bool is_valid = true;
    std::cout << futures.size() << std::endl;
    for (auto& future : futures) {
        auto result = future.get();
        const std::string& file_path = result.first;
        // 
        const std::string& expected_checksum = checksums[file_path];
        const std::string& actual_checksum = result.second;
        std::cout << expected_checksum << "     " << actual_checksum << '\n';

        if (expected_checksum.empty()) {
            std::cerr << "No checksum found for file: " << file_path << std::endl;
            is_valid = false;
            continue;
        }

        if (actual_checksum.empty()) {
            std::cerr << "Error calculating checksum for file: " << file_path << std::endl;
            is_valid = false;
            continue;
        }

        if (actual_checksum != expected_checksum) {
            std::cerr << "Checksum mismatch for file: " << file_path << std::endl;
            std::cerr << "Expected: " << expected_checksum << std::endl;
            std::cerr << "Actual:   " << actual_checksum << std::endl;
            is_valid = false;
        }
    }
    // Вывод результата
    if (is_valid) {
        std::cout << "All files are valid" << std::endl;
        return 0;
    } else {
        std::cerr << "Some files are invalid" << std::endl;
        return 1;
    }
}