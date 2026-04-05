#include "MBRTable.h"

// Проверка, является ли раздел расширенным
bool MBRTable::isExtendedType(uint8_t type) {
    return type == 0x05 || type == 0x0F || type == 0x85;
}

// Преобразование типа раздела в строку
std::string MBRTable::typeToString(uint8_t type) {
    switch (type) {
        case 0x00: return "Empty";
        case 0x05: return "Extended";
        case 0x0F: return "Extended (LBA)";
        case 0x07: return "NTFS/exFAT";
        case 0x82: return "Linux swap";
        case 0x83: return "Linux";
        case 0x85: return "Linux extended";
        case 0xEE: return "GPT protective";
        default:   return "Unknown";
    }
}

// Вывод заголовка таблицы
void MBRTable::printTableHeader() const {
    std::cout << std::left
              << std::setw(12) << "Partition"
              << std::setw(18) << "Type"
              << std::setw(12) << "StartLBA"
              << std::setw(12) << "EndLBA"
              << std::setw(12) << "Sectors"
              << std::setw(12) << "Size(MiB)"
              << '\n';

    std::cout << std::string(78, '-') << '\n';
}

// Вывод одной записи раздела
void MBRTable::printRow(const std::string& name,
                        const MBRPartitionEntry& entry,
                        uint32_t absoluteStartLBA) {
    uint32_t endLBA = absoluteStartLBA + entry.sectors - 1;

    std::cout << std::left
              << std::setw(12) << name
              << std::setw(18) << typeToString(entry.type)
              << std::setw(12) << absoluteStartLBA
              << std::setw(12) << endLBA
              << std::setw(12) << entry.sectors
              << std::setw(12) << std::fixed << std::setprecision(2)
              << sectorsToMiB(entry.sectors)
              << '\n';

    partitionCount++;
}

// Разбор таблицы разделов MBR
void MBRTable::parse(const std::string& device) {
    deviceName = device;
    partitionCount = 0;

    std::ifstream disk(device, std::ios::binary);
    if (!disk) {
        std::cerr << "Cannot open device\n";
        return;
    }

    char buffer[512];
    disk.read(buffer, 512);

    uint16_t signature = *reinterpret_cast<uint16_t*>(buffer + 510);
    if (signature != 0xAA55) {
        std::cout << "Invalid MBR\n";
        return;
    }

    printCommonHeader("MBR");
    printTableHeader();

    auto* entries = reinterpret_cast<MBRPartitionEntry*>(buffer + 446);
    uint32_t extendedBaseLBA = 0;

    for (int i = 0; i < 4; i++) {
        if (entries[i].type == 0 || entries[i].sectors == 0) {
            continue;
        }

        if (isExtendedType(entries[i].type)) {
            extendedBaseLBA = entries[i].startLBA;
            printRow("EXT", entries[i], entries[i].startLBA);
        } else {
            printRow("P" + std::to_string(i + 1), entries[i], entries[i].startLBA);
        }
    }

    if (extendedBaseLBA != 0) {
        uint32_t currentEBRLBA = extendedBaseLBA;
        int logicalIndex = 1;

        while (true) {
            disk.seekg(static_cast<std::streamoff>(currentEBRLBA) * 512);
            disk.read(buffer, 512);

            uint16_t ebrSignature = *reinterpret_cast<uint16_t*>(buffer + 510);
            if (ebrSignature != 0xAA55) {
                break;
            }

            auto* ebrEntries = reinterpret_cast<MBRPartitionEntry*>(buffer + 446);

            if (ebrEntries[0].type != 0 && ebrEntries[0].sectors != 0) {
                uint32_t logicalAbsoluteStart = currentEBRLBA + ebrEntries[0].startLBA;
                printRow("L" + std::to_string(logicalIndex), ebrEntries[0], logicalAbsoluteStart);
                logicalIndex++;
            } else {
                break;
            }

            if (ebrEntries[1].type != 0 && ebrEntries[1].sectors != 0) {
                currentEBRLBA = extendedBaseLBA + ebrEntries[1].startLBA;
            } else {
                break;
            }
        }
    }

    std::cout << "\nTotal partitions: " << partitionCount << "\n";
}
