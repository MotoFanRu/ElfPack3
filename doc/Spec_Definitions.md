# Specification of Definitions

Aleph-generated (.def) definitions format specification document.

Comments should start with the `#` symbol, inline comments are forbidden.

## Definitions Header

```def
# Aleph-generated definitions file
# https://github.com/MotoFanRu/P2K-ELF-SDK
#
# PFW C650_R365_G_0B.D3.08R
# VER EP3_01-Feb-2025
# CPU ARM7
<BLANK LINE>
<BODY START>
```

1. Comment about the generator tool.
2. Phone firmware string comment.

   The `PFW` means "phone firmware". The first part is a short name of the phone, such as `C650`, `E398`, `V3xx`, `M702iG`, etc. The remaining parts indicate the firmware version.

3. Version string comment.

   - The `VER` means "version". The first part indicates that these definitions are for ElfPack 3 (`EP3`). The remaining part is the date in DD-MMM-YYYY format.

4. CPU architecture string comment.

   - `ARM7` - ARM7TDMI-S MCU Core.
   - `ARM9` - ARM926JF-S MCU Core.
   - `ARM11` - ARM1136JF-S MCU Core.
   - `MCORE210` - M-CORE M210 MCU Core.
   - `MCORE340` - M-CORE M340 MCU Core.
   - `NONE` - General definitions like global constants only.

5. Blank line.

## Definitions Body

The body is represented by (addr, type, name) tuple triplets; inline comments are forbidden.

```def
0x0000120B C EV_DATA_CHANGE
0x03FEB87C T __16__rt_memset
0x03FEB6FC A __rt_memclr_w
0x12190FF8 D display_source_buffer
0x1009D3B8 M DL_AudPlayTone
```

1. Address part must be a 32-bit UINT32 value in hexadecimal format, normalized to the `0x00000000` value with leading zeros.
2. Type part must be one of the following characters:

   - `T` - Thumb function address.

     - Note: The address of Thumb functions must be even, as with ARM functions. No `+ 1` or `| 1` is needed.

   - `A` - ARM function address.
   - `D` - Data address, 32-bit UINT32 type.
   - `C` - Constant value, 32-bit UINT32 type, similar to `D` and provided only for clarity.
   - `M` - M-CORE function address.

3. Entity name must not exceed 80 characters and cannot contain spaces.

The file must not contain duplicate names. Duplicate addresses are allowed but may generate a warnings.

## Sorting Modes

1. By names (default).
2. By address.

In both modes, `C` and `D` entities should come first, followed by `A`, `T`, and `M` entities in any order.

Sorting must be case-insensitive.
