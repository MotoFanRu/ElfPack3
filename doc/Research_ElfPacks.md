## ELF Format Differences

### ELF ARM

Can be distinguished by ARM flags:

```
EP1 ELF     - 0x0200001E
EP2 ELF, SO - 0x05000002
EG1 ELF     - 0x05000200
```

### ELF M-CORE

No difference between EM1 and EM2. It can probably be distinguished by a different entry function name:

```
_main      - EM1
ELF_Entry  - EM2
```

## Motorola EP1/EG1

First-generation ELF Loader project for Neptune LTE/LTE2 phones by Andy51, plus a port of EP1 to GCC and Argon phones by EXL and fkcoder (EG1).

### ELF Loader Strap

Embedded into the firmware patch and combined with the ELF Loader.

### ELF Loader App

1. Starts with the AutorunMain function.

   1.1. Registers a hook application using the APP_SyncML_MainRegister function.
   1.2. Runs the ElfLoaderStart function.
   1.3. Parses the `auto.run` text file and runs ELFs by the `LdrLoadELF((WCHAR *) uri, (WCHAR *) params)` function.

2. ElfLoaderStart registers the ElfLoaderApp application.

   2.1. Calls APP_Register to register the app in AFW.
   2.2. Uses the LdrStartApp wrapper.
   2.3. Calls AFW_CreateInternalQueuedEvAux to run the Start function immediately.
   2.4. The Start function uses APP_GetInstData/AFW_AddAppToRoutingStack functions (run app in the background).
   2.5. Loads the library by LoadLibrary function, the library is a binary file loaded just as a memory chunk.

3. LdrLoadELF is the start function to load ELFs, it emits an `AFW_CreateInternalQueuedEvAuxD(EVCODE_LOADELF, ...)` function with event.

   3.1. Handle_LoadELF starts by processing the event with data, etc., and sends everything to the loadELF function.
   3.2. loadELF is the magic ELF-loader function.

4. Managing ELFs by events, LdrUnloadELF, etc.

## Motorola EP2

Second-generation ELF Loader project for Neptune LTE/LTE2 phones by Andy51 & tim_apple.

### ELF Loader Strap

Embedded into the firmware patch and combined with the ELF Loader.

### ELF Loader App

1. Some patches to the ATI Driver as injections.
2. elfpackEntry_ven => elfpackEntry + UpdateDisplayInjection_ven => UpdateDisplayInjection

   2.1. Veneers for stack preserving, etc. Not sure if they are needed.

3. Starts with the elfpackEntry function.

   3.1. Registers a hook application using the FTR_REPLACED_REGISTER_FUNC function/macro.
   3.2. Calls ElfPackRegister and cbkDisplayInit functions.

4. ElfPackRegister calls APP_Register with ElfPackStart as the start function.

   4.1. ldrSendEvent calls AFW_CreateInternalQueuedEvAux with evcode.
   4.2. AFW_CreateInternalQueuedEvAux drops to ElfPackStart.

5. cbkDisplayInit just clears some arrays.
6. ElfPackStart calls ElfPackInit and APP_InitAppData/APP_Start + Timer.

   6.1. ElfPackInit => setDefaultConfig where it clears/copies config data.

7. Timer affects the TID_RUN_DELAY slot.

   7.1. ElfPackLoad is started by the timer.
   7.2. ldrLoadConfig() is loaded.
   7.3. loadDefaultLib() is loaded, checking integrity, the library is a binary file.
   7.4. Checks if some key like `0` is pressed. If so, then no autorun loading.
   7.5. loadAutorun() is called, which calls `loadElf(uri, params)`.
   7.6. Shows the Console Logo, etc.

8. loadElf do some huge magic to  map ELF and SO libraries to system memory.

## Motorola EM1

First-generation ELF Loader project for Rainbow PoG/M-CORE phones by theCor3 & flash.tato.

### ELF Loader Strap

Complex, with small usage of the AFW system.

1. The beginning function is the start point.

   1.1. Copies the font table to some free RAM region. Why? Looks like some rudimentary stuff.
   1.2. Calls APP_Register to register the app in AFW + AFW_CreateInternalQueuedEvAux to run the InfoStart start wrapper.
   1.3. The InfoStart function uses APP_InitAppData/APP_Start => InfoExit.

2. InfoStart is the start wrapper function.

   2.1. Initializes the app struct using APP_InitAppData.
   2.2. Starts the app using APP_Start with state machine structs; on MAIN_STATE, registers the APP_Exit wrapper.

3. MainStateEnter is the enter function.

   3.1. Opens the ElfLoader binary and jumps to its entry point.

4. InfoExit is just a standard exit stub using APP_Exit.

### ELF Loader App

1. The beginning function is the start point.

   1.1. Calls SetDRM to rename the InFusio item to Applications and set its new GIF icons.
   1.2. Calls APP_Register to register the app in AFW but with no AFW_CreateInternalQueuedEvAux function. I think the AppLoaderStart function should start on an event by pressing it in the menu.
   1.3. Calls LoadSymbolDB for library parsing; the library is a text file in def format `func_name=addr`.
   1.4. Calls DoAutorun for AutoRun => ElfLoad (path, ev_code).

2. AppLoaderStart is a GUI browser with a list of ELFs.

   2.1. ELFs can be found only on the /a/ disk.

## Motorola EM2

Second-generation ELF Loader project for Rainbow PoG/M-CORE phones by zeDDer and Chik_v.

### ELF Loader Strap

Simple loader program, just one function, like MainStateEnter in EM1. It just loads the ElfLoader binary from the file system and jumps to it using an inline asm `jsri` opcode with the fixed address.

### ELF Loader App

1. The beginning function is the start point.

   1.1. Calls SetDRM to rename the InFusio item to Applications; icons are probably from AOL.
   1.2. Calls APP_Register to register the app in AFW but with no AFW_CreateInternalQueuedEvAux function. I think the APPStart function should start on an event by pressing it in the menu.
   1.3. Checks if the `0` key is pressed. If so, then no library/autorun loading.
   1.4. Calls ldrLoadDefLibrary for library parsing; the library is a binary file.
   1.5. Calls ldrAutorunAction for AutoRun => ldrLoadElf (path, params/NULL, evcode_base); the autorun file is binary.

2. APPStart is a GUI browser with a list of ELFs.

   2.1. ELFs can be found only on the /a/ disk.
