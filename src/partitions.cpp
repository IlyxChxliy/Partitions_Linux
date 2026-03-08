#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#pragma pack(push, 1)

struct MBRPartitionEntry {
    uint8_t bootFlag;
    uint8_t startCHS[3];
    uint8_t type;
    uint8_t endCHS[3];
    uint32_t startLBA;
    uint32_t sectors;
};

struct GPTHeader {
    char signature[8];
    uint32_t revision;
    uint32_t headerSize;
    uint32_t crc32;
    uint32_t reserved;
    uint64_t currentLBA;
    uint64_t backupLBA;
    uint64_t firstUsableLBA;
    uint64_t lastUsableLBA;
    uint8_t diskGUID[16];
    uint64_t partitionEntryLBA;
    uint32_t numPartitionEntries;
    uint32_t sizeOfPartitionEntry;
    uint32_t partitionArrayCRC32;
};

struct GPTEntry {
    uint8_t typeGUID[16];
    uint8_t uniqueGUID[16];
    uint64_t firstLBA;
    uint64_t lastLBA;
    uint64_t attributes;
    uint16_t name[36];
};

#pragma pack(pop)

class PartitionTable {
protected:
    std::string deviceName;
    int partitionCount = 0;

    void printCommonHeader(const std::string& tableType) const {
        std::cout << "Device: " << deviceName << "\n";
        std::cout << "Table: " << tableType << "\n\n";
    }

    static double sectorsToMiB(uint64_t sectors) {
        return (sectors * 512.0) / (1024.0 * 1024.0);
    }

public:
    virtual void parse(const std::string& device) = 0;
    virtual ~PartitionTable() = default;
};

class MBRTable : public PartitionTable {
private:
    static bool isExtendedType(uint8_t type) {
        return type == 0x05  type == 0x0F  type == 0x85;
    }

    static std::string typeToString(uint8_t type) {
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

    void printTableHeader() const {
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

    void printRow(const std::string& name,
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

public:
    void parse(const std::string& device) override {
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
[08.03.2026 21:33] 𝕴𝖑𝖞𝖝 𝕮𝖍𝖝𝖑𝖎𝖞: if (isExtendedType(entries[i].type)) {
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
};

class GPTTable : public PartitionTable {
private:
    static std::string guidToString(const uint8_t guid[16]) {
        char buffer[37];
        std::snprintf(
            buffer, sizeof(buffer),
            "%02X%02X%02X%02X-"
            "%02X%02X-"
            "%02X%02X-"
            "%02X%02X-"
            "%02X%02X%02X%02X%02X%02X",
            guid[3], guid[2], guid[1], guid[0],
            guid[5], guid[4],
            guid[7], guid[6],
            guid[8], guid[9],
            guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]
        );
        return std::string(buffer);
    }

    static std::string utf16NameToString(const uint16_t name[36]) {
        std::string result;
        for (int i = 0; i < 36; i++) {
            if (name[i] == 0) {
                break;
            }

            if (name[i] < 128) {
                result.push_back(static_cast<char>(name[i]));
            } else {
                result.push_back('?');
            }
        }
        return result;
    }

    void printTableHeader() const {
        std::cout << std::left
                  << std::setw(10) << "Partition"
                  << std::setw(12) << "StartLBA"
                  << std::setw(12) << "EndLBA"
                  << std::setw(12) << "Sectors"
                  << std::setw(12) << "Size(MiB)"
                  << '\n';

        std::cout << std::string(58, '-') << '\n';
    }

    void printRow(const std::string& name,
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
                  << std::setw(12) << std::fixed << std::setprecision(2)
                  << sectorsToMiB(sectors)
                  << '\n';

        std::cout << "  Type GUID:   " << typeGuid << "\n";
        std::cout << "  Unique GUID: " << uniqueGuid << "\n";
        std::cout << "  Name:        " << (partName.empty() ? "(empty)" : partName) << "\n\n";

        partitionCount++;
    }

public:
    void parse(const std::string& device) override {
        deviceName = device;
        partitionCount = 0;
[08.03.2026 21:33] 𝕴𝖑𝖞𝖝 𝕮𝖍𝖝𝖑𝖎𝖞: std::ifstream disk(device, std::ios::binary);
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

        disk.seekg(static_cast<std::streamoff>(header.partitionEntryLBA) * 512);

        for (uint32_t i = 0; i < header.numPartitionEntries; i++) {
            GPTEntry entry{};
            disk.read(reinterpret_cast<char*>(&entry), sizeof(entry));

            if (entry.firstLBA == 0 && entry.lastLBA == 0) {
                continue;
            }

            uint64_t sectors = entry.lastLBA - entry.firstLBA + 1;
            std::string typeGuid = guidToString(entry.typeGUID);
            std::string uniqueGuid = guidToString(entry.uniqueGUID);
            std::string partName = utf16NameToString(entry.name);

            printRow("P" + std::to_string(i + 1),
                     entry.firstLBA,
                     entry.lastLBA,
                     sectors,
                     typeGuid,
                     uniqueGuid,
                     partName);
        }

        std::cout << "\nTotal partitions: " << partitionCount << "\n";
    }
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: parser <mbr|gpt> <device>\n";
        return 1;
    }

    std::string type = argv[1];
    std::string device = argv[2];

    PartitionTable* table = nullptr;

    if (type == "mbr") {
        table = new MBRTable();
    } else if (type == "gpt") {
        table = new GPTTable();
    } else {
        std::cout << "Unknown type. Use mbr or gpt\n";
        return 1;
    }

    table->parse(device);
    delete table;

    return 0;
}
