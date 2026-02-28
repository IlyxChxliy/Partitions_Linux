# Автоматизация монтирования разделов
---

## Задача скрипта

Скрипт выполняет следующие действия для каждого раздела `/dev/sdb5` – `/dev/sdb54`:

1. Получает UUID раздела  
2. Создаёт точку монтирования  
3. Генерирует строку для `/etc/fstab`

---

## Почему используется UUID

UUID (уникальный идентификатор раздела) используется вместо `/dev/sdbX`, потому что:

- имена устройств могут меняться при загрузке
- UUID остаётся постоянным
- это стандартная практика в Linux

Пример записи:
UUID=29596aac-3e36-4373-b0fd-5c713f11f597 /mnt/data/sdb5 ext4 defaults,nofail 0 2

---

## Скрипт

Файл: `scripts/fstab_add.sh`

```bash
#!/bin/bash

# Генерация записей для /dev/sdb5 ... /dev/sdb54

for i in $(seq 5 54); do
    mp="/mnt/data/sdb$i"
    uuid=$(/sbin/blkid -s UUID -o value /dev/sdb$i)

    if [ -n "$uuid" ]; then
        mkdir -p "$mp"
        echo "UUID=$uuid $mp ext4 defaults,nofail 0 2"
    else
        echo "ERROR: no UUID for /dev/sdb$i" >&2
    fi
done
```
# Разбор работы скрипта
Цикл
```for i in $(seq 5 54)```

Перебирает номера разделов от 5 до 54.

Формирование пути монтирования
```mp="/mnt/data/sdb$i"```

Создаёт пути вида:

/mnt/data/sdb5

/mnt/data/sdb6

...

Получение UUID
```uuid=$(/sbin/blkid -s UUID -o value /dev/sdb$i)```

Команда:
- blkid — показывает информацию о блочных устройствах
- s UUID — взять только UUID
- o value — вывести только значение

Проверка UUID
```if [ -n "$uuid" ]```
Проверяет, что UUID получен (раздел существует и отформатирован).

Создание директории
```mkdir -p "$mp"```
Создаёт папку для монтирования.

Генерация строки fstab
```echo "UUID=$uuid $mp ext4 defaults,nofail 0 2"```

Поле	Значение
UUID	идентификатор раздела
/mnt/data/...	точка монтирования
ext4	файловая система
defaults,nofail	параметры
0	dump
2	fsck

## Параметры монтирования
defaults
Стандартные параметры:
-rw (чтение/запись)
-auto
-exec
-suid
nofail

Важно: система продолжит загрузку, даже если раздел не примонтируется

Применение скрипта
1. Создание резервной копии
```sudo cp /etc/fstab /etc/fstab.bak```
2. Генерация записей
```sudo bash scripts/fstab_add.sh >> /etc/fstab```
3. Применение конфигурации
```sudo mount -a```

Команда: монтирует всё из /etc/fstab, позволяет проверить ошибки без перезагрузки

## Проверка
Проверка разделов
```lsblk /dev/sdb```
Проверка монтирования
```mount | grep /mnt/data```
Проверка fstab
```grep /mnt/data /etc/fstab```

## Возможные ошибки
Ошибка: blkid not found
Решение: использовать полный путь: /sbin/blkid

Ошибка: parse error в /etc/fstab
Причина: неверный формат строки
Решение: проверить, что в строке ровно 6 полей

Ошибка: unknown parameter 'deafults'
Причина: опечатка (defaults)

Ошибка: UUID не найден
Причина: раздел не отформатирован

Проверка: lsblk -f
