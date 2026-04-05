#include <iostream>
#include <string>
#include "MBR/MBRTable.h"
#include "GPT/GPTTable.h"

// Функция для выбора типа таблицы
std::unique_ptr<PartitionTable> getTable(const std::string& type) {
    if (type == "mbr") {
        return std::make_unique<MBRTable>();
    } else if (type == "gpt") {
        return std::make_unique<GPTTable>();
    }
    return nullptr;
}

// Основная функция с принятием значений
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: parser <mbr|gpt> <device>\n";
        return 1;
    }

    std::string type = argv[1];
    std::string device = argv[2];

    auto table = getTable(type);
    if (!table) {
        std::cerr << "Unknown type. Use mbr or gpt\n";
        return 1;
    }

    table->parse(device);
    return 0;
}
