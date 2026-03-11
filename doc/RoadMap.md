# P2K-SDK-ELF / EP3 RoadMap

## Legend

* ✅ — Done
* 🟩 — Planned
* 🟨 — In Progress
* 🟥 — Failed
* 🟦 — Out of interest

## Mar, 2026

* ✅ Python CLI tool skeleton.

  * ✅ Port migration tools to Aleph library.

* 🟨 General "ElfLoaderStrap" patch + binary.

  * 🟨 Research EM1 and EM2 ElfLoaderStrap.
  * 🟨 Collect a general compiler flags.
  * 🟨 Research '0' pushing determining.
  * 🟨 Research allocator system.
  * 🟨 Create a Minimal SDK + compiler subset.

* ✅ Rework and research EP3 Logging system.

  * 🟨 Check va_args and logging functions.
  * ✅ NULL Logger.
  * ✅ JAL Logger.
  * ✅ SUAPI Logger.
  * ✅ File Logger.
  * 🟦 Some Old Loggers like E380_Logger, KOSI_Logger, etc?

* ✅ Aleph Library.
* ✅ Create `build.py` and `config.py` scripts.

  * ✅ A minimal working skeleton of building and config scripts.

* ✅ Symbol definitions.

  * ✅ Specification document.
  * ✅ Convert tool.
  * ✅ Validator & parser.
  * ✅ Assembler generator with veneers.
  * ✅ Compiler support.

* ✅ Linker LDS script/templates for general SoCs.

  * ✅ Wally
  * ✅ Patriot
  * ✅ Rainbow
  * ✅ Rainbow PoG
  * ✅ Neptune LT/LCA
  * ✅ Neptune ULS
  * ✅ Neptune LTS
  * ✅ Neptune LTE
  * ✅ Neptune LTE2
  * ✅ Neptune LTE2 ROM4
  * ✅ Argon+
  * ✅ ArgonLV

* 🟨 Setting developer environments.

  * 🟨 C330-like.
  * 🟨 A830-like.
  * ✅ E398/E1-like.

## Feb, 2026

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
