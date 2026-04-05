cmake_minimum_required(VERSION 3.10)

# Устанавливаем название проекта
project(PartitionParser)

# Указываем стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Указываем директорию исходных файлов
include_directories(${CMAKE_SOURCE_DIR}/src)

# Указываем, какие исходники будут использоваться для сборки
add_executable(parser
    src/main.cpp
    src/MBR/MBRTable.cpp
    src/GPT/GPTTable.cpp
    src/PartitionTable/PartitionTable.h
    src/MBR/MBRTable.h
    src/GPT/GPTTable.h
    src/DiskStructures.h
)

