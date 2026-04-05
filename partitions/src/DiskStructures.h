#pragma once

#include <cstdint>

#pragma pack(push, 1)

// Структура записи раздела MBR
struct MBRPartitionEntry {
    uint8_t bootFlag;      // Флаг загрузки
    uint8_t startCHS[3];   // Начальный CHS
    uint8_t type;          // Тип раздела
    uint8_t endCHS[3];     // Конечный CHS
    uint32_t startLBA;     // Начальный сектор
    uint32_t sectors;      // Количество секторов
};


// Заголовок GPT
struct GPTHeader {
    char signature[8];            // EFI PART
    uint32_t revision;            // Версия
    uint32_t headerSize;          // Размер заголовка
    uint32_t crc32;               // CRC
    uint32_t reserved;            // Резерв
    uint64_t currentLBA;          // Текущий LBA
    uint64_t backupLBA;           // Backup LBA
    uint64_t firstUsableLBA;      // Первый доступный сектор
    uint64_t lastUsableLBA;       // Последний доступный сектор
    uint8_t diskGUID[16];         // GUID диска
    uint64_t partitionEntryLBA;   // LBA массива разделов
    uint32_t numPartitionEntries; // Количество записей
    uint32_t sizeOfPartitionEntry;// Размер записи
    uint32_t partitionArrayCRC32; // CRC массива
};

// Запись раздела GPT
struct GPTEntry {
    uint8_t typeGUID[16];     // GUID типа
    uint8_t uniqueGUID[16];   // Уникальный GUID
    uint64_t firstLBA;        // Начальный сектор
    uint64_t lastLBA;         // Конечный сектор
    uint64_t attributes;      // Атрибуты
    uint16_t name[36];        // Имя раздела (UTF-16)
};

#pragma pack(pop)
