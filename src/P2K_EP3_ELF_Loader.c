#include <P2K_SDK_Base.h>
#include <P2K_C_Lib.h>
#include <P2K_C_Lib_Portable.h>
#include <P2K_Logger.h>
#include <P2K_DL_File_System.h>
#include <P2K_Hash_Portable.h>
#include <P2K_UIS_Ustring.h>
#include <P2K_UIS_Ustring_Portable.h>

#include <P2K_EP3_Base.h>
#include <P2K_EP3_Memory.h>
#include <P2K_EP3_ELF.h>
#include <P2K_EP3_DEF_Library.h>
#include <P2K_EP3_Config.h>
#include <P2K_EP3_Image_Manager.h>
#include <P2K_EP3_SO_Loader.h>
#include <P2K_EP3_ELF_Loader.h>

STATUS EP3_ELF_Flavor(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header) {
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

STATUS EP3_ELF_Read_Header(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header) {
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

#if defined(FTR_ARM)
	if ((elf_header->e_type != ET_REL) && (elf_header->e_type != ET_EXEC) && (elf_header->e_type != ET_DYN)) {
		D("[EP3 ELF]: Wrong ELF type: %d, should be 1, 2, 3 (Relocatable, Executable, Shared).\n", elf_header->e_type);
		return RESULT_FAIL;
	}
#endif

#if defined(FTR_ARM)
	if (elf_header->e_machine != EM_ARM) {
		D("[EP3 ELF]: Wrong machine: %d, should be 40 (ARM).\n", elf_header->e_machine);
		return RESULT_FAIL;
	}
#endif

#if defined(FTR_MCORE)
	if ((elf_header->e_machine != EM_RCE) && (elf_header->e_machine != EM_CSKY)) {
		D("[EP3 ELF]: Wrong machine: %d, should be 39, 252 (M-CORE, C-SKY).\n", elf_header->e_machine);
		return RESULT_FAIL;
	}
#endif

	if (elf_header->e_version != EV_CURRENT) {
		D("[EP3 ELF]: Wrong ELF version: %d, should be 1 (current).\n", elf_header->e_version);
		return RESULT_FAIL;
	}

	if (elf_header->e_ehsize != sizeof(Elf32_Ehdr)) {
		D("[EP3 ELF]: ELF Header size mismatch: %d != %d\n", elf_header->e_ehsize, sizeof(Elf32_Ehdr));
		return RESULT_FAIL;
	}

	// EXL, 30-Oct-2026: Program header table must follow the ELF header directly.
	if (elf_header->e_phoff != sizeof(Elf32_Ehdr)) {
		D("[EP3 ELF]: Wrong program header offset: %d, should be %d.\n", elf_header->e_phoff, sizeof(Elf32_Ehdr));
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

	// EXL, 30-Oct-2026: Guard against ELFs without PT_LOAD segments.
	if (virtual_base == UINT32_MAX) {
		return 0;
	}

	return upper_addr - virtual_base;
}

// EXL, 31-Oct-2026: Shared DYN core used by both ELF and .so loading: deploy PT_LOAD
//   segments, parse the PT_DYNAMIC tags, collect DT_NEEDED entries, load the dependencies
//   (refcounted .so images) and apply REL + JMPREL/PLT relocations.
STATUS EP3_ELF_DYN_Load(DL_FS_HANDLE_T file_handle, Elf32_Ehdr *elf_header, EP3_DYN_Context *ctx) {
	DL_FS_COUNT_T elements_read;

	memset(ctx, 0, sizeof(EP3_DYN_Context));

	Elf32_Phdr *elf_program_headers = EP3_Memory_Alloc(sizeof(Elf32_Phdr) * elf_header->e_phnum);
	if (elf_program_headers == NULL) {
		D("[EP3 ELF]: %s\n", "Failed to allocate memory for ELF program headers.");
		return RESULT_FAIL;
	}

	D("[EP3 ELF]: Allocated %d bytes for ELF Program Headers.\n", sizeof(Elf32_Phdr) * elf_header->e_phnum);

	if (DL_FsFSeekFile(file_handle, (DL_FS_SEEK_OFFSET_T) elf_header->e_phoff, DL_FS_SEEK_SET) != DL_FS_RESULT_SUCCESS) {
		D("[EP3 ELF]: Cannot SEEK to: 0x%08X\n", elf_header->e_phoff);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}

	if (
		DL_FsReadFile(
			elf_program_headers, sizeof(Elf32_Phdr), elf_header->e_phnum, file_handle, &elements_read
		) != DL_FS_RESULT_SUCCESS
	) {
		D("[EP3 ELF]: %s\n", "Failed to read ELF program headers.");
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
		D("[EP3 ELF]: Image size is too small: %d, should be > %d.\n", mem_needed, MEM_SIZE_TOO_SMALL);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}

	Elf32_Addr virtual_base = UINT32_MAX;
	for (UINT16 i = 0; i < elf_header->e_phnum; ++i) {
		if (
			(elf_program_headers[i].p_type == PT_LOAD) &&
			(elf_program_headers[i].p_vaddr < virtual_base)
		) {
			virtual_base = elf_program_headers[i].p_vaddr;
		}
	}

	UINT32 *mem_base = EP3_Memory_Alloc(mem_needed);
	if (mem_base == NULL) {
		D("[EP3 ELF]: Failed to allocate %d bytes.\n", mem_needed);
		EP3_Memory_Free(elf_program_headers);
		return RESULT_FAIL;
	}
	memset(mem_base, 0x00, mem_needed);

	Elf32_Addr phys_base = (Elf32_Addr) mem_base;

	D("[EP3 ELF]: %s\n", "Finished Analysis:");
	D("\tvirtual_base=0x%08X\n", virtual_base);
	D("\tmem_needed=0x%08X (%d)\n", mem_needed, mem_needed);
	D("\tphys_base=0x%08X\n", phys_base);

	// EXL, 30-Oct-2026: Read and deploy the PT_LOAD program segments from ELF to RAM.
	for (UINT16 i = 0; i < elf_header->e_phnum; ++i) {
		Elf32_Phdr *elf_program_header = &elf_program_headers[i];

		if (elf_program_header->p_type != PT_LOAD) {
			continue;
		}

		if (
			DL_FsFSeekFile(
				file_handle, (DL_FS_SEEK_OFFSET_T) elf_program_header->p_offset, DL_FS_SEEK_SET
			) != DL_FS_RESULT_SUCCESS
		) {
			D("[EP3 ELF]: Cannot SEEK to segment #%d offset 0x%08X.\n", i, elf_program_header->p_offset);
			goto fail_free_image;
		}

		D("[EP3 ELF]: Segment #%d loading 0x%08X 0x%08X\n", i, phys_base + elf_program_header->p_vaddr - virtual_base, elf_program_header->p_filesz);

		if (elf_program_header->p_filesz > 0) {
			if (
				DL_FsReadFile(
					(void *) (phys_base + elf_program_header->p_vaddr - virtual_base),
					elf_program_header->p_filesz, 1, file_handle, &elements_read
				) != DL_FS_RESULT_SUCCESS
			) {
				D("[EP3 ELF]: Failed to read segment #%d from ELF.\n", i);
				goto fail_free_image;
			}
			if (elements_read != 1) {
				D("[EP3 ELF]: Segment #%d: elements_read=%d, should be 1.\n", i, elements_read);
				goto fail_free_image;
			}
		}
	}

	// EXL, 30-Oct-2026: Process the PT_DYNAMIC segment: load dynamic tags then apply relocations.
	//   ET_DYN (GCC) ELFs only: DT_REL for base relocations, DT_JMPREL for PLT API calls.
	Elf32_Addr dyn_segment = 0;

	for (UINT16 i = 0; i < elf_header->e_phnum; ++i) {
		if (elf_program_headers[i].p_type != PT_DYNAMIC) {
			continue;
		}

		// EXL, 30-Oct-2026: For ET_DYN the dynamic segment is already deployed to RAM by PT_LOAD.
		dyn_segment = phys_base + elf_program_headers[i].p_vaddr - virtual_base;

		Elf32_Dyn tag;
		UINT32 j = 0;
		do {
			tag = ((Elf32_Dyn *) dyn_segment)[j++];
			// EXL, 31-Oct-2026: DT_NEEDED can repeat, collect every occurrence separately.
			if (tag.d_tag == DT_NEEDED) {
				if (ctx->needed_cnt < EP3_SO_MAX_NEEDED) {
					ctx->needed_off[ctx->needed_cnt++] = tag.d_val;
				} else {
					D("[EP3 ELF]: Too many DT_NEEDED entries, max %d.\n", EP3_SO_MAX_NEEDED);
				}
			} else if (tag.d_tag < EP3_DYN_TAGS_COUNT) {
				ctx->dyn_tags[tag.d_tag] = tag.d_val;
			}
		} while (tag.d_tag);

		break;
	}

	if (dyn_segment == 0) {
		D("[EP3 ELF]: %s\n", "No PT_DYNAMIC segment found.");
		goto fail_free_image;
	}

	ctx->mem_base = (void *) mem_base;
	ctx->mem_size = mem_needed;
	ctx->virtual_base = virtual_base;
	ctx->phys_base = phys_base;
	ctx->sym_table = (Elf32_Sym *) (phys_base + ctx->dyn_tags[DT_SYMTAB] - virtual_base);
	ctx->str_table = (char *) (phys_base + ctx->dyn_tags[DT_STRTAB] - virtual_base);

	EP3_Memory_Free(elf_program_headers);
	D("[EP3 ELF]: Freed %d bytes of ELF Program Headers.\n", sizeof(Elf32_Phdr) * elf_header->e_phnum);

	return RESULT_OK;

fail_free_image:
	EP3_Memory_Free(mem_base);
	EP3_Memory_Free(elf_program_headers);
	return RESULT_FAIL;
}

// EXL, 31-Oct-2026: Second pass of the DYN core: load DT_NEEDED dependencies (all
//   providers must be registered before this, see the two-pass scheme) and apply
//   REL + JMPREL/PLT relocations.
STATUS EP3_ELF_DYN_Link(EP3_DYN_Context *ctx, const char *dir) {
	// EXL, 31-Oct-2026: Load DT_NEEDED .so dependencies BEFORE relocations, so the
	//   newly loaded libraries are available as symbol providers.
	if (ctx->needed_cnt > 0) {
		if (EP3_SO_Process_Needed(ctx, dir) != RESULT_OK) {
			D("[EP3 ELF]: %s\n", "WARNING! Some DT_NEEDED libraries failed to load.");
		}
	}

	Elf32_Addr phys_base = ctx->phys_base;
	Elf32_Addr virtual_base = ctx->virtual_base;

	D("[EP3 ELF]: Relocation start\n DT_REL=0x%08X\n DT_RELSZ=%d\n", ctx->dyn_tags[DT_REL], ctx->dyn_tags[DT_RELSZ]);

	// EXL, 30-Oct-2026: Apply "R_ARM_RABS32"/"R_ARM_RELATIVE" base relocations and
	//   resolve "R_ARM_ABS32"/"R_ARM_GLOB_DAT" symbols via DEF lib and .so providers.
	if (ctx->dyn_tags[DT_REL] != 0) {
		Elf32_Rel *rel_table = (Elf32_Rel *) (phys_base + ctx->dyn_tags[DT_REL] - virtual_base);

		for (UINT32 j = 0; j * sizeof(Elf32_Rel) < ctx->dyn_tags[DT_RELSZ]; j++) {
			// EXL, 30-Oct-2026: Only last byte of UINT32 r_info is relType.
			Elf32_Word rel_type = ELF32_R_TYPE(rel_table[j].r_info);

			D(" Reloc #%d\n  Type  %d\n  Off  0x%08X\n", j, rel_type, rel_table[j].r_offset);

			if ((rel_type == R_ARM_RABS32) || (rel_type == R_ARM_RELATIVE)) {
				UINT32 *reloc_addr = (UINT32 *) (phys_base + rel_table[j].r_offset - virtual_base);
				D(" R_ARM_RABS32\n  Old  0x%08X\n  New  0x%08X\n", *reloc_addr, *reloc_addr + phys_base - virtual_base);
				*reloc_addr += phys_base - virtual_base;
			} else if ((rel_type == R_ARM_ABS32) || (rel_type == R_ARM_GLOB_DAT)) {
				UINT32 *reloc_addr = (UINT32 *) (phys_base + rel_table[j].r_offset - virtual_base);
				INT32 sym_idx = (INT32) ELF32_R_SYM(rel_table[j].r_info);
				const char *sym_str = &ctx->str_table[ctx->sym_table[sym_idx].st_name];

				// EXL, 30-Oct-2026: Point symbol to the module image base if it was not
				//   found in the DEF library or .so providers. Workaround is here.
				*reloc_addr = phys_base - virtual_base;

				UINT32 sym_addr = EP3_Resolve_Symbol(sym_str);
				if (sym_addr != 0) {
					*reloc_addr = sym_addr;
				}
			} else {
				D(" UNK Reloc #%d\n  Type  %d\n  Off  0x%08X\n", j, rel_type, rel_table[j].r_offset);
			}
		}
	}

	// EXL, 30-Oct-2026: DT_JMPREL symbols handling. The GCC uses DT_JMPREL + DT_SYMTAB/DT_STRTAB
	//   instead of SHT_SYMTAB/SHT_STRTAB sections. Iterate over "R_ARM_JUMP_SLOT" relocations
	//   in '.rel.plt' section and patch the PLT jump table at runtime.
	if (ctx->dyn_tags[DT_JMPREL] != 0) {
		Elf32_Rel *rel_table = (Elf32_Rel *) (phys_base + ctx->dyn_tags[DT_JMPREL] - virtual_base);

		// EXL, 30-Oct-2026: This is a pointer to the first jump in the PLT jump table:
		// E5 2D E0 04 E5 9F E0 04 E0 8F E0 0E E5 BE F0 08 00 00 00 D0 47 78 E7 FD E2 8F C6 00 E2 8C CA 00 E5 BC F0 C0
		// |    0     |     1     |     2     |     3     |     4     |  5  |  5  |^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		Elf32_Addr *plt_jump_tbl_ptr = (Elf32_Addr *) (
			phys_base + *((Elf32_Word *) (phys_base + rel_table[0].r_offset - virtual_base)) - virtual_base
		);
		// EXL, 30-Oct-2026: It has 4778E7FD opcode in Thumb after 4 .plt bytes? If yes, just skip it.
		plt_jump_tbl_ptr += ((*(plt_jump_tbl_ptr + 5)) == 0x4778E7FD) ? 6 : 5;

		for (UINT32 i = 0; i * sizeof(Elf32_Rel) < ctx->dyn_tags[DT_PLTRELSZ]; ++i) {
			INT32 sym_idx = (INT32) ELF32_R_SYM(rel_table[i].r_info);
			const char *sym_str = &ctx->str_table[ctx->sym_table[sym_idx].st_name];

			UINT32 ldr_st_addr = EP3_Resolve_Symbol(sym_str);
			if (ldr_st_addr == 0) {
				D("[EP3 ELF]: %s\n", "WARNING! Module can start/execute not correctly!");
				continue;
			}

			// EXL, 30-Oct-2026: We need to patch the PLT jump table at runtime.
			//   This will resolve the launch issues on real ARMv4T, ARM7TDMI-S, and Big-Endian CPUs/MCUs.
			// E2 8F C6 00 E2 8C CA 00 E5 BC F0 C0 => E5 9F C0 00 E1 2F FF 1C 10 00 00 00
			{
				Elf32_Word ARM_ADRL_1 = *(plt_jump_tbl_ptr + 0);
				Elf32_Word ARM_ADRL_2 = *(plt_jump_tbl_ptr + 1);
				Elf32_Word ARM_LDR    = *(plt_jump_tbl_ptr + 2);

				// add ip, pc, #0, #12    =>   ldr ip, [pc]
				// add ip, ip, #0, #20    =>   bx ip
				// ldr pc, [ip, #0xc0]!   =>   10 00 00 00 (function address)

				// EXL, 30-Oct-2026: It has 4778E7FD opcode in Thumb? If yes, just skip it.
				if (*plt_jump_tbl_ptr == 0x4778E7FD) {
					plt_jump_tbl_ptr += 1;
				}

				*(plt_jump_tbl_ptr + 0) = 0xE59FC000;
				*(plt_jump_tbl_ptr + 1) = 0xE12FFF1C;
				*(plt_jump_tbl_ptr + 2) = ldr_st_addr;

				D("=> PLT PATCH: %08X: %08X %08X %08X => %08X %08X %08X\n",
					plt_jump_tbl_ptr, ARM_ADRL_1, ARM_ADRL_2, ARM_LDR,
					*(plt_jump_tbl_ptr + 0), *(plt_jump_tbl_ptr + 1), *(plt_jump_tbl_ptr + 2)
				);

				plt_jump_tbl_ptr += 3;
			}

			*((Elf32_Word *) (phys_base + rel_table[i].r_offset - virtual_base)) = ldr_st_addr;
		}
	}

	return RESULT_OK;
}

// EXL, 31-Oct-2026: DT_INIT + DT_INIT_ARRAY constructors of a module.
void EP3_ELF_Run_Init(const EP3_DYN_Context *ctx) {
	if (ctx->dyn_tags[DT_INIT] != 0) {
		((void (*)(void)) (ctx->phys_base + ctx->dyn_tags[DT_INIT] - ctx->virtual_base))();
	}
	if ((ctx->dyn_tags[DT_INIT_ARRAY] != 0) && (ctx->dyn_tags[DT_INIT_ARRAYSZ] != 0)) {
		void (**init_array)(void) = (void (**)(void)) (
			ctx->phys_base + ctx->dyn_tags[DT_INIT_ARRAY] - ctx->virtual_base
		);
		UINT32 init_cnt = ctx->dyn_tags[DT_INIT_ARRAYSZ] / sizeof(void *);
		for (UINT32 i = 0; i < init_cnt; ++i) {
			init_array[i]();
		}
	}
}

// EXL, 31-Oct-2026: DT_FINI_ARRAY (reverse order) + DT_FINI destructors of a module.
void EP3_ELF_Run_Fini(const EP3_DYN_Context *ctx) {
	if ((ctx->dyn_tags[DT_FINI_ARRAY] != 0) && (ctx->dyn_tags[DT_FINI_ARRAYSZ] != 0)) {
		void (**fini_array)(void) = (void (**)(void)) (
			ctx->phys_base + ctx->dyn_tags[DT_FINI_ARRAY] - ctx->virtual_base
		);
		UINT32 fini_cnt = ctx->dyn_tags[DT_FINI_ARRAYSZ] / sizeof(void *);
		for (UINT32 i = fini_cnt; i > 0; --i) {
			fini_array[i - 1]();
		}
	}
	if (ctx->dyn_tags[DT_FINI] != 0) {
		((void (*)(void)) (ctx->phys_base + ctx->dyn_tags[DT_FINI] - ctx->virtual_base))();
	}
}

static STATUS EP3_ELF_EP3_Load(
	DL_FS_HANDLE_T file_handle,
	const WCHAR *file_path,
	const char *params,
	UINT32 event,
	Elf32_Ehdr *elf_header,
	void **out_mem_base
) {
	// EXL, 31-Oct-2026: ASCII copy of the module path, its directory is the first
	//   search location for DT_NEEDED .so dependencies.
	char module_dir[DL_FS_MAX_URI_NAME_LENGTH + 1];
	PORTABLE_u_utoa(file_path, module_dir);
	char *dir_sep = (char *) PORTABLE_strchr(module_dir, '/');
	if (dir_sep != NULL) {
		*(dir_sep + 1) = ASCII_NULL;
	} else {
		module_dir[0] = ASCII_NULL;
	}

	// EXL, 31-Oct-2026: Two-pass dynamic linking, pass 1: deploy the image.
	EP3_DYN_Context dyn_ctx;
	if (EP3_ELF_DYN_Load(file_handle, elf_header, &dyn_ctx) != RESULT_OK) {
		return RESULT_FAIL;
	}

	// EXL, 31-Oct-2026: Register the ELF as a symbol provider BEFORE the link pass, so
	//   the DT_NEEDED .so libraries can resolve the ELF exports during their relocation.
	//   Export addresses are valid right after deployment (relocations only fix
	//   intra-image pointers). The record also holds the DT_NEEDED refs for unload.
	EP3_SO *elf_provider = EP3_SO_Register_ELF(&dyn_ctx);

	// EXL, 31-Oct-2026: Pass 2: load DT_NEEDED dependencies, then relocate.
	EP3_ELF_DYN_Link(&dyn_ctx, module_dir);

	// EXL, 31-Oct-2026: The provider record keeps the loaded DT_NEEDED .so references
	//   so EP3_ELF_Unload can release them (a shared .so outlives one user).
	if (elf_provider != NULL) {
		EP3_SO_Attach_Needed(elf_provider, &dyn_ctx);
	}

	// EXL, 31-Oct-2026: Register the image BEFORE starting it, so the entry itself can
	//   see its own image in the registry (and the slot-full check trips before the run).
	EP3_Image *image = EP3_Image_Register(
		EP3_IMG_ELF, dyn_ctx.mem_base, dyn_ctx.mem_size, file_path, event, EP3_IMG_FLAG_FREE_MEM
	);
	if (image == NULL) {
		// EXL, 31-Oct-2026: Unregistering the provider also releases the DT_NEEDED refs.
		EP3_SO_Unregister_ELF(dyn_ctx.mem_base);
		EP3_Memory_Free(dyn_ctx.mem_base);
		return RESULT_FAIL;
	}

	// EXL, 31-Oct-2026: Start ELF from the "e_entry" address, "start" stub which calls
	//   "Register" ELF entry point. The event is passed through to the "reserve" slot.
	D("[EP3 ELF]: Starting ELF at 0x%08X\n", dyn_ctx.phys_base + elf_header->e_entry - dyn_ctx.virtual_base);

	((EP3_ELF_Entry_T) (dyn_ctx.phys_base + elf_header->e_entry - dyn_ctx.virtual_base))((const char *) NULL, (char *) params, event);

	D("[EP3 ELF]: %s\n", "ELF returned");

	*out_mem_base = dyn_ctx.mem_base;

	return RESULT_OK;
}

STATUS EP3_ELF_Load(const WCHAR *file_path, const char *params, UINT32 event, void **out_elf) {
	STATUS status = RESULT_FAIL;
	char elf_file_path[DL_FS_MAX_URI_NAME_LENGTH + 1];

	if ((file_path == NULL) || (file_path[0] == UNICODE_NULL)) {
		D("[EP3 ELF]: %s\n", "Argument file_path is NULL or empty.");
		return RESULT_FAIL;
	}
	if (out_elf != NULL) {
		*out_elf = NULL;
	}

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
	status = EP3_ELF_Read_Header(file_handle, &elf_header);
	if (status != RESULT_OK) {
		DL_FsCloseFile(file_handle);
		return status;
	}

	status = EP3_ELF_Flavor(file_handle, &elf_header);
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
	D("\te_shoff=0x%08X\n", elf_header.e_shoff);
	D("\te_shnum=0x%04X\n", elf_header.e_shnum);

	void *elf_mem_base = NULL;
	status = EP3_ELF_EP3_Load(file_handle, file_path, params, event, &elf_header, &elf_mem_base);

	DL_FsCloseFile(file_handle);

	if ((status == RESULT_OK) && (out_elf != NULL)) {
		*out_elf = elf_mem_base;
	}

	return status;
}

STATUS EP3_ELF_Unload(void *elf_ptr) {
	if (elf_ptr == NULL) {
		D("[EP3 ELF]: %s\n", "Argument elf_ptr is NULL.");
		return RESULT_FAIL;
	}

	EP3_Image *image = EP3_Image_Find(elf_ptr);
	if (image == NULL) {
		D("[EP3 ELF]: Unknown ELF pointer: 0x%08X\n", (UINTPTR) elf_ptr);
		return RESULT_FAIL;
	}

	D("[EP3 ELF]: Unloading ELF at 0x%08X (%d bytes).\n", (UINTPTR) image->mem_base, image->mem_size);

	// EXL, 31-Oct-2026: Drop the ELF symbol provider record and release the refcounts
	//   of its DT_NEEDED .so dependencies (a shared .so survives until the last user).
	EP3_SO_Unregister_ELF(elf_ptr);

	return EP3_Image_Unregister(image);
}

__attribute__((used, section(".text.bin.entry_point")))
void EP3_ELF_Loader_MainRegister(void) {
	L("%s\n", "Hello Moto!");

	// EXL, 31-Oct-2026: Parse ep3.cfg (settings + autorun) before anything else.
	EP3_Config_T config;
	if (EP3_Config_Load(&config) != RESULT_OK) {
		L("[EP3 ELF]: %s\n", "Failed to load config, using defaults.");
		EP3_Config_Defaults(&config);
	}

	// EXL, 31-Oct-2026: Init the image registry with the configured slot count.
	EP3_Image_Init(config.max_elfs);

	DEF_LIB_Symbol *def_library = (DEF_LIB_Symbol *) EP3_Memory_Alloc(sizeof(DEF_LIB_Symbol) * config.lib_capacity);
	if (def_library == NULL) {
		L("Cannot allocate bytes: %d\n", sizeof(DEF_LIB_Symbol) * config.lib_capacity);
		return;
	}

	UINT32 def_cnt = EP3_DEF_Library_Load(def_library, config.lib_capacity);
	// EP3_DEF_Library_Insert(def_library, &def_cnt, FNV1A32("EP3_API_Viewer"), (UINTPTR) EP3_APP_View);
	L("[EP3 ELF]: Symbols: %d\n", def_cnt);

	// EXL, 31-Oct-2026: Init the .so subsystem: resolver context (DEF lib) + search paths.
	EP3_SO_Init(config.max_so, def_library, def_cnt, config.libpaths, config.libpath_cnt);

	// EXL, 31-Oct-2026: Expose the dl API to ELFs through the DEF library table, so they
	//   call dlopen/dlsym/dlclose/dlerror like any other firmware function.
	EP3_DEF_Library_Insert(def_library, &def_cnt, FNV1A32("dlopen"), (UINTPTR) EP3_dlopen);
	EP3_DEF_Library_Insert(def_library, &def_cnt, FNV1A32("dlsym"), (UINTPTR) EP3_dlsym);
	EP3_DEF_Library_Insert(def_library, &def_cnt, FNV1A32("dlclose"), (UINTPTR) EP3_dlclose);
	EP3_DEF_Library_Insert(def_library, &def_cnt, FNV1A32("dlerror"), (UINTPTR) EP3_dlerror);

	// EXL, 31-Oct-2026: Smoke test load kept from the previous iteration, runs only
	//   when there is no autorun list to execute.
//	if (config.autorun_cnt == 0) {
//		EP3_ELF_Load(L"h.elf", NULL, 0, NULL);
//	}

	// EXL, 31-Oct-2026: Run autorun entries in config file order. The event is passed
	//   through to the ELF entry ("reserve" slot). One failed entry does not stop the rest.
	for (UINT32 i = 0; i < config.autorun_cnt; ++i) {
		EP3_Autorun_Entry_T *entry = &config.autorun[i];

		WCHAR uri[EP3_CFG_MAX_PATH];
		PORTABLE_u_atou(entry->uri, uri);

		L("[EP3 ELF]: Autorun #%d: '%s' event=0x%08X\n", i, entry->uri, entry->event);

		if (EP3_ELF_Load(uri, entry->args[0] ? entry->args : NULL, entry->event, NULL) != RESULT_OK) {
			L("[EP3 ELF]: Autorun #%d failed, continuing.\n", i);
		}
	}

	L("%s\n", "End!");
}
