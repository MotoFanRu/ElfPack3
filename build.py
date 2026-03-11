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

def build_so_lib(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Building {recipe_name} Library.')

	asm_tpl = recipe.soc.asm
	asm_src = P2K_SDK_BUILD / asm_tpl.name.replace('.tpl', '')
	asm_obj = P2K_SDK_BUILD / asm_src.name.replace('.S', '.o')
	def_res = P2K_SDK_RES / recipe_name / 'ep3.def'

	if not write_assembler_listing(asm_tpl, asm_src, read_definitions(def_res)):
		return False

	if not gcc_compile(recipe, asm_src, asm_obj):
		return False

	# TODO: Links library to .a and .so here?

	return True

def build_bin_ldr(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Building {recipe_name} BIN loader.')

	lds = recipe.soc.lds
	addr = format_32bit_addr(recipe.addresses.inject)
	elf_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.elf'
	bin_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.bin'

	sources = [P2K_SDK_SRC / f for f in (
		'P2K_EP3_Logger.c',
		'P2K_EP3_File_System.c',
		'P2K_EP3_Memory.c',
		'P2K_EP3_BIN_Loader.c'
	)]

	objs = []
	for src in sources:
		obj = P2K_SDK_BUILD / src.with_suffix('.o').name
		if not gcc_compile(recipe, src, obj, recipe.flags.bin_ldr):
			return False
		objs.append(obj)

	objs.append(P2K_SDK_BUILD / recipe.soc.asm.name.replace('.tpl.S', '.o'))
	if not gcc_link(recipe, objs, elf_res, [f'-Wl,-Ttext={addr}', f'-Wl,-T,{lds}']):
		return False

	if not gcc_bin(recipe, elf_res, bin_res):
		return False

	return True

def build_elf_ldr(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Building {recipe_name} BIN loader.')

	lds = recipe.soc.lds
	addr = format_32bit_addr(recipe.addresses.inject)
	elf_res = P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.elf'
	bin_res = P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.bin'

	sources = [P2K_SDK_SRC / f for f in (
		'P2K_EP3_Memory.c',
		'P2K_EP3_ELF_Loader.c'
	)]

	objs = []
	for src in sources:
		obj = P2K_SDK_BUILD / src.with_suffix('.o').name
		if not gcc_compile(recipe, src, obj, recipe.flags.elf_ldr):
			return False
		objs.append(obj)

	objs.append(P2K_SDK_BUILD / recipe.soc.asm.name.replace('.tpl.S', '.o'))
	if not gcc_link(recipe, objs, elf_res, [f'-Wl,-T,{lds}']):
		return False

	if not gcc_bin(recipe, elf_res, bin_res):
		return False

	return True

def build_patches(recipe_name: str, recipe: Recipe) -> bool:
	return True

def cook_recipe(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Cooking {recipe_name} recipe.')

	if not create_clean_dir(P2K_SDK_BUILD):
		E(f'Cannot create and clean build directory: "{P2K_SDK_BUILD}"')
		return False

	build_steps = [
		(build_so_lib, [recipe_name, recipe], 'Cannot build SO Library'),
		(build_bin_ldr, [recipe_name, recipe], 'Cannot build BIN Loader'),
		(build_elf_ldr, [recipe_name, recipe], 'Cannot build ELF Loader'),
		(build_patches, [recipe_name, recipe], 'Cannot build Final Patches'),
	]

	for func, args, error_msg in build_steps:
		if not func(*args):
			E(f'{error_msg} for "{recipe_name}".')
			return False

	return True

def do_work(args: Namespace) -> bool:
	if args.recipe == 'all':
		I('Cooking all recipes.\n')
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
