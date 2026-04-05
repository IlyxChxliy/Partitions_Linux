#pragma once

#include "../PartitionTable/PartitionTable.h"
#include "../DiskStructures.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

// Класс для работы с GPT таблицей разделов
class GPTTable : public PartitionTable {
private:
    // Преобразование GUID в строку
    static std::string guidToString(const uint8_t guid[16]);

    // Преобразование UTF16 имени в строку
    static std::string utf16NameToString(const uint16_t name[36]);

    // Печать заголовка таблицы
    void printTableHeader() const;

    // Печать строки таблицы
    void printRow(const std::string& name,
                  uint64_t startLBA,
                  uint64_t endLBA,
                  uint64_t sectors,
                  const std::string& typeGuid,
                  const std::string& uniqueGuid,
                  const std::string& partName);

public:
    // Разбор GPT таблицы
    void parse(const std::string& device) override;
};
