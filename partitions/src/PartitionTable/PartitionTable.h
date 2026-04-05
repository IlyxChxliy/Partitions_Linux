#pragma once

#include <cstdint>
#include <iostream>
#include <string>

// Базовый абстрактный класс для таблиц разделов
class PartitionTable {
protected:
    std::string deviceName;
    int partitionCount = 0;

    // Вывод общей информации об устройстве и типе таблицы
    void printCommonHeader(const std::string& tableType) const {
        std::cout << "Device: " << deviceName << "\n";
        std::cout << "Table: " << tableType << "\n\n";
    }

    // Перевод количества секторов в MiB
    static double sectorsToMiB(uint64_t sectors) {
        return (sectors * 512.0) / (1024.0 * 1024.0);
    }

public:
    // Виртуальный метод разбора таблицы разделов
    virtual void parse(const std::string& device) = 0;

    // Виртуальный деструктор
    virtual ~PartitionTable() = default;
};
