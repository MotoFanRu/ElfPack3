cmake_minimum_required(VERSION 3.16)

project(ElfPack3 VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(SOURCES
	../P2K_EP3_APP_Entry_Main.c
#	../P2K_EP3_APP_Viewer.c
	../P2K_EP3_BIN_Loader.c
	../P2K_EP3_Config.c
	../P2K_EP3_DEF_Library.c
	../P2K_EP3_ELF_Loader.c
	../P2K_EP3_Environment.c
	../P2K_EP3_Events.c
	../P2K_EP3_File_System.c
	../P2K_EP3_Image_Args.c
	../P2K_EP3_Image_Manager.c
	../P2K_EP3_Logger.c
	../P2K_EP3_Memory.c
	../P2K_EP3_SO_Loader.c
#	../P2K_EP3_Task_Reactor.c
	P2K_EP3_Host_Stubs.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../sdk
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
	__P2K__
	P2K
	FTR_ARM
	FTR_THUMB_MODE
	FTR_LITTLE_ENDIAN
	FTR_ARM7
	FTR_NEPTUNE
	FTR_NEPTUNE_LT_LCA
	FTR_ELF_LDR
	FTR_C330
	FTR_DEBUG
	FTR_LOGGER
	FTR_LOGGER_HOST
	FTR_ALLOC_SUAPI
	FTR_DONT_DEFINE_SDK_CLIKE_TYPES
	FTR_DONT_DEFINE_SDK_NULL
	FTR_DONT_DEFINE_LIMITS
	FTR_DONT_DEFINE_VA_ARGS
	FTR_DONT_DEFINE_STDLIB_FUNCTIONS
	FTR_HOST
	FTR_EP3_PHONE="C330"
	FTR_EP3_FIRMWARE="TA02_G_06.03.25R_elf"
	FTR_EP3_VERSION="v1.0 10-Nov-2025"
)

target_compile_options(${PROJECT_NAME} PRIVATE
	-Wall
	-Wextra
	-Wpedantic
	-Wno-builtin-declaration-mismatch
	-funsigned-char
	-fshort-enums
	-fshort-wchar
	-fpack-struct=4
)
