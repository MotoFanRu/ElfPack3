#!/usr/bin/env python3

"""
EP3 Build Tool

A general tool for building ELF Loader, BIN Loader, and libraries for a selected platform.
"""

from pathlib import Path
from datetime import datetime
from argparse import Namespace
from argparse import RawDescriptionHelpFormatter

from config import *

def build_lib(recipe_name: str, recipe: Recipe, lib_name: str) -> bool:
	I(f' Building "{recipe_name}" {lib_name} library.')

	lib_name = lib_name.replace(' ', '_')
	asm_tpl = recipe.soc.asm
	asm_src = P2K_SDK_BUILD / asm_tpl.name.replace('.tpl', f'_{lib_name}')
	asm_obj = P2K_SDK_BUILD / asm_src.name.replace('.S', '.o')
	def_res = P2K_SDK_RES / recipe_name / 'ep3.def'

	if lib_name == 'STUB_BIN_LDR':
		defines = get_api_from_definitions(def_res, P2K_SDK_API / 'EP3_BIN_Loader.api')
		defines_reactor = get_api_from_definitions(def_res, P2K_SDK_API / 'EP3_Task_Reactor.api')
		if recipe.tasks and defines and defines_reactor:
			defines.defs = defines.defs + defines_reactor.defs
	elif lib_name == 'STUB_ELF_LDR':
		defines = get_api_from_definitions(def_res, P2K_SDK_API / 'EP3_ELF_Loader.api')
	else:
		defines = read_definitions(def_res)

	if not write_assembler_listing(asm_tpl, asm_src, defines):
		E(f'Cannot create assembly listing: "{def_res.name}" => "{asm_src}"')
		return False

	cflags = [
		*recipe.toolchain.cflags_lib_so
	]
	if not gcc_compile(recipe, asm_src, asm_obj, cflags):
		E(f'Cannot compile assembly listing: "{asm_src.name} => {asm_obj.name}"')
		return False

	# TODO: Links library to .a and .so here?
	# TODO: Copy library to Release?

	return True

def build_bin_ldr(recipe_name: str, recipe: Recipe) -> bool:
	I(f' Building "{recipe_name}" BIN loader.')

	cflags = [
		*recipe.toolchain.cflags_bin_ldr, *recipe.flags.bin_ldr,
		f'-DFTR_LOAD_TO_ADDR={format_32bit_addr(recipe.addresses.loader)}',
	]
	src_obj = [
		(P2K_SDK_SRC / 'P2K_EP3_Logger.c',      P2K_SDK_BUILD / 'P2K_EP3_Logger.o'),
		(P2K_SDK_SRC / 'P2K_EP3_Memory.c',      P2K_SDK_BUILD / 'P2K_EP3_Memory.o'),
		(P2K_SDK_SRC / 'P2K_EP3_File_System.c', P2K_SDK_BUILD / 'P2K_EP3_File_System.o'),
		(P2K_SDK_SRC / 'P2K_EP3_BIN_Loader.c',  P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.o'),
	]
	recipe.tasks and src_obj.append((P2K_SDK_SRC / 'P2K_EP3_Task_Reactor.c', P2K_SDK_BUILD / 'P2K_EP3_Task_Reactor.o'))
	for src, obj in src_obj:
		if not gcc_compile(recipe, src, obj, cflags):
			E(f'Cannot compile source file: "{src.name}" => "{obj.name}"')
			return False

	lflags = [
		*cflags, *recipe.toolchain.lflags_bin_ldr,
		f'-Wl,-L,{recipe.soc.lds.parent}',
		f'-Wl,-T,{recipe.soc.lds.name}',
		f'-Wl,-Ttext={format_32bit_addr(recipe.addresses.inject)}',
	]
	elf_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.elf'
	objs = [obj for _, obj in src_obj]
	objs.append(P2K_SDK_BUILD / recipe.soc.asm.name.replace('.tpl.S', '_STUB_BIN_LDR.o'))
	if not gcc_link(recipe, objs, elf_res, lflags):
		E(f'Cannot link ELF executable file: "{elf_res}"')
		return False

	map_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.map'
	if not gcc_nm(recipe, elf_res, map_res):
		E(f'Cannot extract symbols: "{map_res}"')
		return False

	bin_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.bin'
	if not gcc_bin(recipe, elf_res, bin_res):
		E(f'Cannot create BIN file: "{elf_res}" => "{bin_res}"')
		return False

	copy_file(bin_res, P2K_SDK_RELEASE / recipe_name / EP3_BIN_LDR, True, True)
	# TODO: Create and combine patches instead of copying loader file?

	return True

def build_elf_ldr(recipe_name: str, recipe: Recipe) -> bool:
	I(f' Building "{recipe_name}" ELF loader.')

	cflags = [
		*recipe.toolchain.cflags_elf_ldr, *recipe.flags.elf_ldr,
		f'-DFTR_LOAD_TO_ADDR={format_32bit_addr(recipe.addresses.loader)}',
	]
	src_obj = [
		(P2K_SDK_SRC / 'P2K_EP3_ELF_Loader.c',  P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.o'),
	]
	for src, obj in src_obj:
		if not gcc_compile(recipe, src, obj, cflags):
			E(f'Cannot compile source file: "{src.name}" => "{obj.name}"')
			return False

	lflags = [
		*cflags, *recipe.toolchain.lflags_elf_ldr,
		f'-Wl,-L,{recipe.soc.lds.parent}',
		f'-Wl,-T,{recipe.soc.lds.name}',
		f'-Wl,-Ttext={format_32bit_addr(recipe.addresses.loader)}',
	]
	elf_res = P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.elf'
	objs = [obj for _, obj in src_obj]
	objs.append(P2K_SDK_BUILD / recipe.soc.asm.name.replace('.tpl.S', '_STUB_ELF_LDR.o'))
	objs.append(P2K_SDK_BUILD / 'P2K_EP3_Logger.o')
	objs.append(P2K_SDK_BUILD / 'P2K_EP3_Memory.o')
	objs.append(P2K_SDK_BUILD / 'P2K_EP3_File_System.o')
	if not gcc_link(recipe, objs, elf_res, lflags):
		E(f'Cannot link ELF executable file: "{elf_res}"')
		return False

	map_res = P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.map'
	if not gcc_nm(recipe, elf_res, map_res):
		E(f'Cannot extract symbols: "{map_res}"')
		return False

	bin_res = P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.bin'
	if not gcc_bin(recipe, elf_res, bin_res):
		E(f'Cannot create BIN file: "{elf_res}" => "{bin_res}"')
		return False

	if not copy_file(bin_res, P2K_SDK_RELEASE / recipe_name / EP3_ELF_LDR, True, True):
		return False

	return True

def cook_recipe(recipe_name: str, recipe: Recipe) -> bool:
	I('')
	I(f'Cooking "{recipe_name}" recipe.')

	if not create_clean_dir(P2K_SDK_BUILD):
		E(f'Cannot create and clean build directory: "{P2K_SDK_BUILD}"')
		return False

	build_steps = [
		(build_lib, [recipe_name, recipe, 'STUB_BIN_LDR'], 'Cannot build STUB BIN LDR Library'),
		(build_bin_ldr, [recipe_name, recipe], 'Cannot build BIN Loader'),
		(build_lib, [recipe_name, recipe, 'STUB_ELF_LDR'], 'Cannot build STUB ELF LDR Library'),
		(build_elf_ldr, [recipe_name, recipe], 'Cannot build ELF Loader'),
	]

	for func, args, error_msg in build_steps:
		if not func(*args):
			E(f'{error_msg} for "{recipe_name}".')
			return False

	return True

def do_work(args: Namespace) -> bool:
	if args.recipe == 'all':
		I('Cooking all recipes.')
		for recipe in RECIPES.keys():
			if not cook_recipe(recipe, RECIPES[recipe]):
				return False
		return True
	return cook_recipe(args.recipe, RECIPES[args.recipe])

def args_and_help() -> Namespace:
	class Args(Arguments):
		def check_args(self, args: Namespace) -> None:
			if (args.recipe not in RECIPES) and (args.recipe != 'all'):
				self.error(f'Recipe "{args.recipe}" not supported!')

	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name} all\n'
	for recipe in RECIPES.keys():
		epl += f'  python {Path(__file__).name} {recipe}\n'
	hlp = {
		'D': 'EP3 Build Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'R': 'phone and firmware recipe to build',
		'v': 'enable verbose output'
	}

	parser = Args(description=hlp['D'], epilog=epl, formatter_class=RawDescriptionHelpFormatter)
	parser.add_argument('recipe', type=str, help=hlp['R'])
	parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])
	return parser.parse_and_check_args()

def main() -> None:
	args = args_and_help()
	set_logger(args.verbose)

	start_time = datetime.now()
	D(f'Start building on "{start_time}".')

	do_work(args)

	D(f'End building on "{datetime.now()}".')
	I(f'Time elapsed: "{elapsed_format(datetime.now() - start_time)}".')

if __name__ == '__main__':
	main()
