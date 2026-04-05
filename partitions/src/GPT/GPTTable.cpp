#include "GPTTable.h"
#include <cstdio>

// Преобразование GUID в строку
std::string GPTTable::guidToString(const uint8_t guid[16]) {
    char buffer[37];

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%02X%02X%02X%02X-"
        "%02X%02X-"
        "%02X%02X-"
        "%02X%02X-"
        "%02X%02X%02X%02X%02X%02X",
        guid[3], guid[2], guid[1], guid[0],
        guid[5], guid[4],
        guid[7], guid[6],
        guid[8], guid[9],
        guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]);

    return std::string(buffer);
}

// Преобразование UTF16 имени
std::string GPTTable::utf16NameToString(const uint16_t name[36]) {
    std::string result;

    for (int i = 0; i < 36; i++) {
        if (name[i] == 0)
            break;

        if (name[i] < 128)
            result.push_back(static_cast<char>(name[i]));
        else
            result.push_back('?');
    }

    return result;
}

// Заголовок таблицы
void GPTTable::printTableHeader() const {
    std::cout << std::left
              << std::setw(10) << "Partition"
              << std::setw(12) << "StartLBA"
              << std::setw(12) << "EndLBA"
              << std::setw(12) << "Sectors"
              << std::setw(12) << "Size(MiB)"
              << "\n";

    std::cout << std::string(58, '-') << "\n";
}

// Печать строки
void GPTTable::printRow(const std::string& name,
                        uint64_t startLBA,
                        uint64_t endLBA,
                        uint64_t sectors,
                        const std::string& typeGuid,
                        const std::string& uniqueGuid,
                        const std::string& partName) {

    std::cout << std::left
              << std::setw(10) << name
              << std::setw(12) << startLBA
              << std::setw(12) << endLBA
              << std::setw(12) << sectors
              << std::setw(12) << std::fixed
              << std::setprecision(2)
              << sectorsToMiB(sectors)
              << "\n";

    std::cout << "  Type GUID:   " << typeGuid << "\n";
    std::cout << "  Unique GUID: " << uniqueGuid << "\n";
    std::cout << "  Name:        "
              << (partName.empty() ? "(empty)" : partName)
              << "\n\n";

    partitionCount++;
}

// Разбор GPT
void GPTTable::parse(const std::string& device) {
    deviceName = device;
    partitionCount = 0;

    std::ifstream disk(device, std::ios::binary);

    if (!disk) {
        std::cerr << "Cannot open device\n";
        return;
    }

    GPTHeader header{};

    disk.seekg(512);
    disk.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (std::string(header.signature, 8) != "EFI PART") {
        std::cout << "Invalid GPT\n";
        return;
    }

    printCommonHeader("GPT");
    printTableHeader();

    disk.seekg(
        static_cast<std::streamoff>(header.partitionEntryLBA) * 512);

    for (uint32_t i = 0; i < header.numPartitionEntries; i++) {

        GPTEntry entry{};
        disk.read(reinterpret_cast<char*>(&entry), sizeof(entry));

        if (entry.firstLBA == 0 && entry.lastLBA == 0)
            continue;

        uint64_t sectors =
            entry.lastLBA - entry.firstLBA + 1;

        std::string typeGuid =
            guidToString(entry.typeGUID);

        std::string uniqueGuid =
            guidToString(entry.uniqueGUID);

        std::string partName =
            utf16NameToString(entry.name);

        printRow(
            "P" + std::to_string(i + 1),
            entry.firstLBA,
            entry.lastLBA,
            sectors,
            typeGuid,
            uniqueGuid,
            partName);
    }

    std::cout << "\nTotal partitions: "
              << partitionCount
              << "\n";
}
