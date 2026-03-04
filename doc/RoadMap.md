# P2K-SDK-ELF / EP3 RoadMap

*15-Feb-2026...20-Feb-2026*

* ✅ All-in-One ent. file from ARM/M-CORE libraries.

  * ✅ Create a tool which merge all symbols to one file for diffing.
  * ✅ Create all symbols sets.

* ✅ All-in-One ent. file from SDK headers.

  * ✅ Create a migration report file.
  * 🟦 Create a tool which merge all SDK entities to one file for diffing.
  * ✅ Prepare a different SDK sources for comparing entities and their mutable and immutable (gold) versions.

    * ✅ EP1 General.
    * ✅ EM1 General.
    * ✅ EP2 General.
    * ✅ EM2 General.
    * ✅ P2K-ELF-SDK Intermediate.
    * ✅ Andy51 SDK.
    * ✅ om2804 SDK.
    * ✅ G-Xad SDK.
    * ✅ Cool-T SDK.
    * ✅ E1 Leak.
    * ✅ E1000 Leak.
    * ✅ V9 Leak.

* ✅ Basic SDK skeleton (typedefs, core header files).

  * ✅ Motorola type definitions.
  * ✅ STD C/C++ type definitions.
  * ✅ Rust type definitions.
  * ✅ Primitives like bool, RESULT_OK, etc.
  * ✅ Feature file and List of Features.

* ✅ EP1/EP2/EP3 ELF format differences.
* ✅ Prepare FWs and environment for further developing.

  * ✅ E1, R373_49R, +MOOSE, NoRSA, E398_R373 Patch.
  * ✅ V3i, R4441D_03R, +MOOSE, NoRSA, PDS SecVer Patch.
  * ✅ V3x, R252211LD_E6P, NoRSA.

* ✅ Python CLI tool skeleton.

  * ✅ Port migration tools to Aleph library.

* 🟨 General "ElfLoaderStrap" patch + binary.

  * 🟨 Research EM2 ElfLoaderStrap.
  * 🟨 Collect a general compiler flags.
  * 🟨 Research '0' pushing determining.
  * 🟨 Research allocator system.
  * 🟨 Create a Minimal SDK + compiler subset.

* 🟨 Rework and research EP3 Logging system.

  * 🟨 Check va_args and logging functions.
  * 🟨 JAL Logger.
  * 🟨 SUAPI Logger.
  * 🟨 File Logger.
  * 🟨 Some Old Loggers like E380_Logger, KOSI_Logger, etc?

* 🟨 Aleph Library
* 🟨 Create `build.py` and `config.py` scripts.

  * ✅ A minimal working skeleton of building and config scripts.

...

Legend:
* ✅ — Done
* 🟩 — Planned
* 🟨 — In Progress
* 🟥 — Failed
* 🟦 — Out of interest


## Plans

### Plan #1

```
https://t.me/motofan_ru/1100646/1291612
Вот ещё нашёл мысли в записях:

EP3 ideas:
Полный отказ от ADS
Полный отказ от постлинкеров
Современный GCC 12+
Загрузка эльфов или кусочков эльфов в IRAM для оптимизации
Консоль выкинуть?
Релоад либы?
SO-либы как в EP2
Таски из коробки без патчей
Таск— ELF-менеджер?
+++ 
Классы/методы для чтения INI-like
Использование функций из IROM и IRAM повсеместное
Лоадер в виде файла с бутстрапом бинлоадером
Отображение ошибок в UI посредством UIS
API для процессов и пр.

EP3 pitfalls:
ARM7TDMI relocs
Отличия эльфов
```

### Plan #2

```
https://t.me/motofan_ru/1100646/1291598
Да в плане всего. Взять всё лучшее, выкинуть ненужное...

1. Выявить отличия эльфов ADS, GCC EP2 postlink, GCC EP1 pure.

2. Изучить работу EP2 как мы изучили работу EP1 и позаимствовать оттуда полезные фичи вроде SO-либ и констант, ненужные вроде консоли и постлинкера — не брать.

3. Изучить работу EM2 как мы изучили работу EP1 и позаимствовать оттуда полезные фичи вроде бинарного лоадера который можно менять не прошивая телефон каждый раз., ненужные вроде лаунчера и заменителя выкинуть.

4. Более серьёзно изучить работу тасок, попытаться добиться их работы без всяких патчей как у Andy51 было, а чисто на ванильных прошивках.

5. Перенести наши наработки с EP1 в EP3 по IRAM'у и по прочему. Добиться работы сегментов...

6. Объединённый SDK.

7. Первентирование повторного запуска ELF'а.
```

### Plan #3

```
https://t.me/motofan_ru/1100646/1339783
По EP3 среди фитч хотелось бы иметь:
- поддержку внешних либ как в EP2 на всех платформах;
- загружаемое в рантайме ядро эльфпака как на EM2;
- поддержка тасков на уровне эльфпака без патчей, отнимающих хип браузера;
- встроенный диспетчер запущенных эльфов, позволяющих полноценно сворачивать разворачивать приложение, возможно функцию регистрации в диспетчере задач стоит встроить в точку входа;
- возможность грузить эльф в IRAM;
- поддержка собственного формата инсталлятора в виде отдельного эльфа.
```
