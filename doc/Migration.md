# SDK and EP3 Migration/Development Log

## 20-Feb-2026

* E1, V3i, V3x kitchens
* Functions:

  ```
  0472 A DL_FsOpenFile
  0473 A DL_FsReadFile
  0437 A DL_FsCloseFile
  0447 A DL_FsFSeekFile
  0455 A DL_FsGetFileSize
  0485 A DL_FsSGetFileSize

  0194 T APP_CALC_MainRegister

  0879 A suAllocMem
  0895 A suFreeMem

  0915 A u_atol
  0916 A u_atou
  0917 A u_isAlpha
  0918 A u_isDigit
  0919 A u_isLower
  0920 A u_isSpace
  0921 A u_isUpper
  0922 A u_itoa
  0923 A u_itoh
  0924 A u_ltou
  0926 A u_strcat
  0927 A u_strchr
  0928 A u_strcmp
  0929 A u_strcpy
  0930 A u_strcut
  0932 A u_stristr
  0933 A u_strlen
  0934 A u_strmakelower
  0935 A u_strmakeupper
  0936 A u_strncat
  0937 A u_strncmp
  0938 T u_strncmpi
  0939 A u_strncpy
  0940 A u_strrchr
  0942 A u_strstr
  0943 A u_uint64tou
  0944 T u_uitoa
  0945 A u_ultou
  ```

## 19-Feb-2026

* EM1, EM2 elfs will be dropped.
* ELF ARM Flags that can be used to distinguish ELFs:

  ```
  EP1 ELF     — 0x0200001E
  EP2 ELF, SO — 0x05000002
  EG1 ELF     — 0x05000200
  ```

## 18-Feb-2026

* Core Types and Platform Defines.
