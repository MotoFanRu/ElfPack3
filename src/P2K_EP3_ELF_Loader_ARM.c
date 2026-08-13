#include <P2K_SDK_Base.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>
#include <P2K_UIS_Ustring.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_ELF.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_DEF_Library.h>
#include <P2K_UIS_Ustring_Portable.h>

typedef struct tagEP3_ELF {

} EP3_ELF;

static STATUS EP3_ELF_Flavor(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header) {
	UNUSED(file_handle);

#if defined(FTR_ARM)
	if (elf_header->e_flags == 0x0200001E) {
		D("[EP3 ELF]: %s\n", "EP1 ELF loading not implemented yet.");
		return RESULT_FAIL;
	} else if (elf_header->e_flags == 0x05000002) {
		D("[EP3 ELF]: %s\n", "EP2 ELF loading not implemented yet.");
		return RESULT_FAIL;
	} else if (elf_header->e_flags == 0x05000200) {
		D("[EP3 ELF]: %s\n", "EP3 ELF loading is OK!");
		return RESULT_OK;
	}
#endif
	// TODO: Check M-CORE ELFs.

	return RESULT_OK;
}

static STATUS EP3_ELF_Read_Header(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header) {
	DL_FS_COUNT_T elements_read;

	if (DL_FsReadFile(elf_header, sizeof(Elf32_Ehdr), 1, file_handle, &elements_read) != DL_FS_RESULT_SUCCESS) {
		D("[EP3 ELF]: %s\n", "Cannot Read ELF header.");
		return RESULT_FAIL;
	}
	if (elements_read != 1) {
		D("[EP3 ELF]: elements_read=%d, should be 1.\n", elements_read);
		return RESULT_FAIL;
	}

	if (
		(elf_header->e_ident[EI_MAG0] != ELFMAG0) ||
		(elf_header->e_ident[EI_MAG1] != ELFMAG1) ||
		(elf_header->e_ident[EI_MAG2] != ELFMAG2) ||
		(elf_header->e_ident[EI_MAG3] != ELFMAG3)
	) {
		D("[EP3 ELF]: Wrong ELF header magic: 0x%08X\n", *((UINT32 *) elf_header->e_ident));
		return RESULT_FAIL;
	}

	if (elf_header->e_ident[EI_CLASS] != ELFCLASS32) {
		D("[EP3 ELF]: Wrong bitness: %d, should be 1 (32-bit).\n", elf_header->e_ident[EI_CLASS]);
		return RESULT_FAIL;
	}

#if defined(FTR_LITTLE_ENDIAN)
	if (elf_header->e_ident[EI_DATA] != ELFDATA2LSB) {
		D("[EP3 ELF]: Wrong endianness: %d, should be 1 (little-endian).\n", elf_header->e_ident[EI_DATA]);
		return RESULT_FAIL;
	}
#endif

#if defined(FTR_BIG_ENDIAN)
	if (elf_header->e_ident[EI_DATA] != ELFDATA2MSB) {
		D("[EP3 ELF]: Wrong endianness: %d, should be 2 (big-endian).\n", elf_header->e_ident[EI_DATA]);
		return RESULT_FAIL;
	}
#endif

	if (elf_header->e_ident[EI_VERSION] != EV_CURRENT) {
		D("[EP3 ELF]: Wrong version: %d, should be 1 (current).\n", elf_header->e_ident[EI_VERSION]);
		return RESULT_FAIL;
	}

	if (elf_header->e_ident[EI_OSABI] != ELFOSABI_SYSV) {
		D("[EP3 ELF]: Wrong ABI: %d, should be 0 (ELFOSABI_SYSV).\n", elf_header->e_ident[EI_OSABI]);
		return RESULT_FAIL;
	}

	if (elf_header->e_ident[EI_ABIVERSION] != ELFABIVERSION_NONE) {
		D("[EP3 ELF]: Wrong ABI version: %d, should be 0 (ELFABIVERSION_NONE).\n", elf_header->e_ident[EI_ABIVERSION]);
		return RESULT_FAIL;
	}

	if (
		(elf_header->e_type != ET_REL) &&
		(elf_header->e_type != ET_EXEC) &&
		(elf_header->e_type != ET_DYN)
	) {
		D("[EP3 ELF]: Wrong ELF type: %d, should be 1, 2, 3 (Relocatable, Executable, Shared).\n", elf_header->e_type);
		return RESULT_FAIL;
	}

#if defined(FTR_ARM)
	if (elf_header->e_machine != EM_ARM) {
		D("[EP3 ELF]: Wrong machine: %d, should be 40 (ARM).\n", elf_header->e_machine);
		return RESULT_FAIL;
	}
#endif

#if defined(FTR_MCORE)
	if (elf_header->e_machine != EM_MCORE || elf_header->e_machine != EM_CSKY) {
		D("[EP3 ELF]: Wrong machine: %d, should be 39, 252 (M-CORE, C-SKY).\n", elf_header->e_machine);
		return RESULT_FAIL;
	}
#endif

	if (elf_header->e_version != EV_CURRENT) {
		D("[EP3 ELF]: elements_read=%d, should be 1.\n", elf_header->e_version);
		return RESULT_FAIL;
	}

	if (elf_header->e_ehsize != sizeof(Elf32_Ehdr) || elf_header->e_phoff != sizeof(Elf32_Ehdr)) {
		D("[EP3 ELF]: ELF Header size mismatch: %d != %d\n", elf_header->e_ehsize, sizeof(Elf32_Ehdr));
		return RESULT_FAIL;
	}

	if (elf_header->e_ehsize != elf_header->e_phoff) {
		D("[EP3 ELF]: ELF Header size should equal to e_phoff: %d != %d\n", elf_header->e_ehsize, elf_header->e_phoff);
		return RESULT_FAIL;
	}

	if (elf_header->e_phentsize != sizeof(Elf32_Phdr)) {
		D("[EP3 ELF]: ELF Program Header size mismatch: %d != %d\n", elf_header->e_phentsize, sizeof(Elf32_Phdr));
		return RESULT_FAIL;
	}

	if (elf_header->e_shentsize != sizeof(Elf32_Shdr)) {
		D("[EP3 ELF]: ELF Section Header size mismatch: %d != %d\n", elf_header->e_shentsize, sizeof(Elf32_Shdr));
		return RESULT_FAIL;
	}

	return RESULT_OK;
}

static UINT32 EP3_ELF_Calc_Image(Elf32_Phdr *elf_program_headers, Elf32_Ehdr *elf_header) {
	// UINT32 mem_size = 0;

	Elf32_Addr upper_addr = 0;
	Elf32_Addr virtual_base = UINT32_MAX;

	for (UINT16 i = 0; i < elf_header->e_phnum; ++i) {
		Elf32_Phdr *elf_program_header = &elf_program_headers[i];

		D("Program Header #%d:\n", i);
		D("\tp_type=0x%08X\n", elf_program_header->p_type);
		D("\tp_vaddr=0x%08X\n", elf_program_header->p_vaddr);
		D("\tp_memsz=0x%08X\n", elf_program_header->p_memsz);

		if (elf_program_header->p_type == PT_LOAD) {
			if (elf_program_header->p_vaddr < virtual_base) {
				virtual_base = elf_program_header->p_vaddr;
			}
			if ((elf_program_header->p_vaddr + elf_program_header->p_memsz) > upper_addr) {
				upper_addr = elf_program_header->p_vaddr + elf_program_header->p_memsz;
			}
		}
	}

	return upper_addr - virtual_base;
}

static STATUS EP3_ELF_EP3_Load(DL_FS_HANDLE_T file_handle, DEF_LIB_Symbol *def_symbols, Elf32_Ehdr *elf_header) {
	DL_FS_COUNT_T elements_read;

	Elf32_Phdr *elf_program_headers = EP3_Memory_Alloc(sizeof(Elf32_Phdr) * elf_header->e_phnum);
	if (elf_program_headers == NULL) {
		D("[EP3 ELF]: Failed to allocate memory for ELF program headers.\n");
		return RESULT_FAIL;
	}

	D("[EP3 ELF]: Allocated %d bytes for ELF Program Headers.\n", sizeof(Elf32_Phdr) * elf_header->e_phnum);

	if (DL_FsFSeekFile(file_handle, elf_header->e_phoff, DL_FS_SEEK_SET) != DL_FS_RESULT_SUCCESS) {
		D("[EP3 ELF]: Cannot SEEK to: 0x08X\n", elf_header->e_phoff);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}

	if (
		DL_FsReadFile(
			elf_program_headers, sizeof(Elf32_Phdr), elf_header->e_phnum, file_handle, &elements_read
		) != DL_FS_RESULT_SUCCESS
	) {
		D("[EP3 ELF]: Failed to read from ELF program headers.\n");
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}
	if (elements_read != elf_header->e_phnum) {
		D("[EP3 ELF]: elements_read=%d, should be %d.\n", elements_read, elf_header->e_phnum);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}

	UINT32 mem_needed = EP3_ELF_Calc_Image(elf_program_headers, elf_header);
	if (mem_needed <= MEM_SIZE_TOO_SMALL) {
		D("[EP3 ELF]: elements_read=%d, should be %d.\n", mem_needed, MEM_SIZE_TOO_SMALL);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}

	UINT32 *mem_base = EP3_Memory_Alloc(mem_needed);
	if (mem_base == NULL) {
		D("[EP3 ELF]: Failed to allocate %d bytes.\n", mem_needed);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}

	D("[EP3 ELF]: %s\n", "Finished Analysis:");
	D("\n", "Finished Analysis:");

	return RESULT_OK;
}

static STATUS EP3_ELF_Load(const WCHAR *file_path, DEF_LIB_Symbol *def_symbols) {
	STATUS status = RESULT_OK;
	char elf_file_path[DL_FS_MAX_URI_NAME_LENGTH + 1];

	if ((file_path == NULL) || (file_path[0] == UNICODE_NULL)) {
		D("[EP3 ELF]: %s\n", "Argument file_path is NULL or empty.");
		return RESULT_FAIL;
	}

	// TODO: Check elf_name.ini
	DL_FS_SIZE_T file_size = DL_FsSGetFileSize(file_path, DL_FS_OWNER_RESERVED);
	if (file_size < FILE_SIZE_TOO_SMALL) {
		PORTABLE_u_utoa(file_path, elf_file_path);
		D("[EP3 ELF]: File '%s' is not exist or too small (%d bytes).\n", elf_file_path, file_size);
		return RESULT_FAIL;
	}

	DL_FS_HANDLE_T file_handle = DL_FsOpenFile(file_path, DL_FS_READ_MODE, DL_FS_OWNER_RESERVED);
	if (file_handle == DL_FS_HANDLE_INVALID) {
		PORTABLE_u_utoa(file_path, elf_file_path);
		D("[EP3 ELF]: Failed to open '%s' file.\n", elf_file_path);
		return RESULT_FAIL;
	}

	Elf32_Ehdr elf_header;
	status |= EP3_ELF_Read_Header(file_handle, &elf_header);
	if (status != RESULT_OK) {
		DL_FsCloseFile(file_handle);
		return status;
	}

	status |= EP3_ELF_Flavor(file_handle, &elf_header);
	if (status != RESULT_OK) {
		DL_FsCloseFile(file_handle);
		return status;
	}

	D("[EP3 ELF]: %s\n", "Loading ELF file.");
	D("%s\n", "ELF Header:");
	D("\te_type=0x%04X\n", elf_header.e_type);
	D("\te_entry=0x%08X\n", elf_header.e_entry);
	D("\te_phoff=0x%08X\n", elf_header.e_phoff);
	D("\te_phnum=0x%04X\n", elf_header.e_phnum);
	// TODO: Check them.
	D("\te_shoff=0x%04X\n", elf_header.e_shoff);
	D("\te_shnum=0x%04X\n", elf_header.e_shnum);

	status |= EP3_ELF_EP3_Load(file_handle, def_symbols, &elf_header);

	return status;
}
