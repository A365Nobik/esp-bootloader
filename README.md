# ESP-Bootloader

[English](#english) | [Русский](#русский)

---

# English

## Overview

**ESP-Bootloader** is a bootloader application for ESP32-based devices. It provides a graphical interface for selecting and launching firmware applications.

The project supports multiple applications stored in ESP flash memory and optionally on external storage.

## Features

- Graphical boot menu.
- Application selection and navigation.
- Multiple applications in flash memory.
- Optional external storage support.
- Application information display.
- Optional touchscreen controls.
- Adaptive interface layout based on screen orientation.
- OTA partition support.
- Python flashing utility.

## Controls

When touchscreen support is enabled:

| Button | Action |
|---|---|
| UP | Select previous application |
| DW | Select next application |
| OK | Launch selected application |
| T | Switch between application sources |

Physical buttons can also be used if supported by the board configuration.

## Configuration

Board-specific configuration is located in:

```text
components/config/
```

Before building the project, select and configure the appropriate board settings.

Check the following parameters:

- ESP32 target.
- Flash size.
- Partition table.
- Display configuration.
- Touchscreen configuration.
- External storage support.
- Other board-specific hardware settings.

Make sure that the partition table matches the actual flash layout and provides enough space for the bootloader and applications.

### Partition Table Configuration

Before building the project, select the appropriate partition table file for your board and flash configuration.

1. Choose the required partition table file from the project's partition table directory.
2. Copy the selected file to the project root directory.
3. Rename the application names if necessary.
4. Rename the copied file to `partitions.csv`.

Make sure that the selected partition table matches your flash size and provides enough space for all applications.

### LGFX Configuration

Before building the project, verify the **LovyanGFX (LGFX)** configuration.

Make sure that the configuration matches your hardware:

- Display driver and controller.
- Display resolution.
- SPI pins (SCK, MOSI, MISO, CS).
- DC, RST and backlight pins.
- Display rotation and color settings.
- Touchscreen controller and pin configuration (if enabled).

Incorrect LGFX settings may cause display or touchscreen initialization problems.

## Python Flashing Utility

The project includes a Python script called `flash.py` for flashing the bootloader and applications.

The script uses ESP-IDF build information from:

```text
build/flasher_args.json
```

This allows it to obtain flash addresses automatically instead of requiring manual offset configuration.

### Usage

```bash
python flash.py board TARGET flash SIZE port PORT image FILE...
```

Arguments:

| Argument | Description |
|---|---|
| `TARGET` | ESP-IDF target |
| `SIZE` | Flash size |
| `PORT` | Serial port |
| `FILE...` | Firmware image files |

The script uses the project's partition table (`partitions.csv`) to determine application OTA partition locations.

### Flashing Process

The utility handles the flashing process, including:

1. Validating the build and configuration.
2. Resolving bootloader and partition table offsets.
3. Determining application OTA partition offsets.
4. Flashing the bootloader and partition table.
5. Flashing the specified applications.

Make sure the project has been built before running the script.

## Build

Configure ESP-IDF for your target board:

```bash
idf.py set-target esp32s3
idf.py build
```

## Flash

You can flash the firmware using the Python utility:

```bash
python flash.py board esp32s3 flash 16MB port /dev/ttyACM0 image app.bin
```

Replace the target, flash size, serial port and image filename with the values appropriate for your setup.

You can also use the standard ESP-IDF commands:

```bash
idf.py flash
idf.py monitor
```

## Requirements

- ESP32-compatible development board.
- ESP-IDF.
- Python 3.
- Compatible display.
- External storage device (optional).
- Touchscreen (optional).

## License

This project is licensed under the GNU General Public License v3.0 or later (GPL-3.0-or-later).

---

# Русский

## Обзор

**ESP-Bootloader** — приложение-загрузчик для устройств на базе ESP32. Оно предоставляет графический интерфейс для выбора и запуска прошивок.

Проект поддерживает несколько приложений, расположенных во Flash-памяти ESP32 и, при необходимости, на внешнем накопителе.

## Возможности

- Графическое меню загрузчика.
- Выбор и переключение между приложениями.
- Поддержка нескольких приложений во Flash-памяти.
- Опциональная поддержка внешнего накопителя.
- Отображение информации о приложениях.
- Сенсорное управление (опционально).
- Адаптация интерфейса под ориентацию экрана.
- Поддержка OTA-разделов.
- Python-скрипт для прошивки устройства.

## Управление

При включённой поддержке сенсорного экрана:

| Кнопка | Действие |
|---|---|
| UP | Выбрать предыдущее приложение |
| DW | Выбрать следующее приложение |
| OK | Запустить выбранное приложение |
| T | Переключить источник приложений |

Также могут использоваться физические кнопки, если они поддерживаются конфигурацией платы.

## Настройка конфигурации

Конфигурация плат находится в директории:

```text
components/config/
```

Перед сборкой проекта необходимо выбрать и настроить параметры используемой платы.

Основные параметры:

- Целевая платформа ESP32.
- Размер Flash-памяти.
- Таблица разделов.
- Настройки дисплея.
- Настройки сенсорного экрана.
- Поддержка внешнего накопителя.
- Остальные аппаратные параметры платы.

Убедитесь, что таблица разделов соответствует реальной разметке Flash-памяти и содержит достаточно места для загрузчика и приложений.

### Настройка таблицы разделов

Перед сборкой проекта необходимо выбрать подходящий файл разметки разделов для используемой платы и конфигурации Flash-памяти.

1. Выберите нужный файл таблицы разделов в директории с разметками проекта.
2. Скопируйте выбранный файл в корневую директорию проекта.
3. По необходимости переименуйте название приложений
4. Переименуйте его в `partitions.csv`.

Убедитесь, что выбранная таблица разделов соответствует размеру Flash-памяти и содержит достаточно места для всех приложений.

### Настройка LGFX

Перед сборкой проекта обязательно проверьте конфигурацию **LovyanGFX (LGFX)**.

Убедитесь, что настройки соответствуют вашему оборудованию:

- Драйвер и контроллер дисплея.
- Разрешение дисплея.
- SPI-пины (SCK, MOSI, MISO, CS).
- Пины DC, RST и подсветки.
- Поворот экрана и настройки цвета.
- Контроллер сенсорного экрана и его пины (если сенсор включён).

Неправильная конфигурация LGFX может привести к проблемам с инициализацией дисплея или сенсорного экрана.

## Python-скрипт прошивки

В проекте предусмотрен Python-скрипт `flash.py` для прошивки загрузчика и приложений.

Скрипт использует информацию о сборке ESP-IDF из файла:

```text
build/flasher_args.json
```

Благодаря этому адреса разделов Flash определяются автоматически, без необходимости вручную указывать смещения.

### Использование

```bash
python flash.py board TARGET flash SIZE port PORT image FILE...
```

Аргументы:

| Аргумент | Описание |
|---|---|
| `TARGET` | Целевая платформа ESP-IDF |
| `SIZE` | Размер Flash-памяти |
| `PORT` | Последовательный порт |
| `FILE...` | Файлы прошивок |

Для определения расположения OTA-разделов приложений скрипт использует таблицу разделов проекта (`partitions.csv`).

### Процесс прошивки

Скрипт выполняет следующие действия:

1. Проверяет сборку и конфигурацию.
2. Определяет адреса загрузчика и таблицы разделов.
3. Определяет адреса OTA-разделов приложений.
4. Прошивает загрузчик и таблицу разделов.
5. Прошивает указанные приложения.

Перед запуском скрипта необходимо предварительно собрать проект.

## Сборка

Настройте ESP-IDF для используемой платы:

```bash
idf.py set-target esp32s3
idf.py build
```

## Прошивка

Пример использования Python-скрипта:

```bash
python flash.py board esp32s3 flash 16MB port /dev/ttyACM0 image app.bin
```

Замените целевую платформу, размер Flash, последовательный порт и имя файла прошивки на соответствующие вашей конфигурации.

Также можно использовать стандартные команды ESP-IDF:

```bash
idf.py flash
idf.py monitor
```

## Требования

- Совместимая плата на базе ESP32.
- ESP-IDF.
- Python 3.
- Совместимый дисплей.
- Внешний накопитель (опционально).
- Сенсорный экран (опционально).

## Лицензия

Проект распространяется под лицензией GNU General Public License версии 3 или более поздней (GPL-3.0-or-later).
