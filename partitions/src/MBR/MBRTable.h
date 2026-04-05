#pragma once

#include "../PartitionTable/PartitionTable.h"
#include "../DiskStructures.h"

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

// Класс для работы с таблицей разделов MBR
class MBRTable : public PartitionTable {
private:
    // Проверка, является ли тип расширенным разделом
    static bool isExtendedType(uint8_t type);

    // Преобразование кода типа раздела в строку
    static std::string typeToString(uint8_t type);

    // Вывод заголовка таблицы
    void printTableHeader() const;

    // Вывод одной строки таблицы
    void printRow(const std::string& name,
                  const MBRPartitionEntry& entry,
                  uint32_t absoluteStartLBA);

public:
    // Разбор таблицы разделов MBR
    void parse(const std::string& device) override;
};
