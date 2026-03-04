#!/usr/bin/env python3

"""
EP3 Build Tool

A general tool for building ELF Loader, BIN Loader, and libraries for a selected platform.
"""

import sys

from datetime import datetime

from argparse import Namespace
from argparse import ArgumentParser
from argparse import RawDescriptionHelpFormatter

from aleph import D
from aleph import I
from aleph import E
from aleph import Recipe
from aleph import gcc_compile
from aleph import gcc_link
from aleph import gcc_bin
from aleph import set_logger
from aleph import P2K_SDK_SRC
from aleph import P2K_SDK_BUILD
from aleph import elapsed_format
from aleph import create_clean_dir
from aleph import patch_text_file

from config import RECIPES
from config import BIN_LDR_ENTRY_POINT_FUNC

def patch_linker_script(recipe: Recipe) -> bool:
	I(f'Patching "{recipe.soc.lds.name}" linker script.')
	markers = ['%addr_entry%',               '%entry_point_function%']
	patches = [str(recipe.addresses.inject), BIN_LDR_ENTRY_POINT_FUNC]
	return patch_text_file(markers, patches, recipe.soc.lds, P2K_SDK_BUILD / recipe.soc.lds.name)

def build_bin_ldr(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Building {recipe_name} BIN loader.')

	lds_res = P2K_SDK_BUILD / recipe.soc.lds.name
	elf_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.elf'
	bin_res = P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.bin'

	sources = [P2K_SDK_SRC / f for f in (
		'P2K_EP3_LIB_Stubs.S',
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

	if not gcc_link(recipe, objs, elf_res, ['-T', lds_res]):
		return False

	if not gcc_bin(recipe, elf_res, bin_res):
		return False

	return True

def build_elf_ldr(recipe_name: str, recipe: Recipe) -> bool:
	return True

def build_so_lib(recipe_name: str, recipe: Recipe) -> bool:
	return True

def build_patches(recipe_name: str, recipe: Recipe) -> bool:
	return True

def cook_recipe(recipe_name: str, recipe: Recipe) -> bool:
	I(f'Cooking {recipe_name} recipe.')

	build_steps = [
		(create_clean_dir, [P2K_SDK_BUILD], 'Cannot create and clean build directory'),
		(patch_linker_script, [recipe], 'Cannot patch linker script'),
		(build_bin_ldr, [recipe_name, recipe], 'Cannot build BIN Loader'),
		(build_elf_ldr, [recipe_name, recipe], 'Cannot build ELF Loader'),
		(build_so_lib, [recipe_name, recipe], 'Cannot build SO Library'),
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
	class Args(ArgumentParser):
		def error(self, message: str) -> None:
			self.print_help(sys.stderr)
			self.exit(2, f'\n{self.prog}: error: {message}\n')
		def parse_and_check_args(self) -> Namespace:
			args = self.parse_args()
			if (args.recipe not in RECIPES) and (args.recipe != 'all'):
				self.error(f'Recipe "{args.recipe}" not supported!')
			return args

	epl = 'examples:\n'
	epl += '  python build.py all\n'
	for recipe in RECIPES.keys():
		epl += f'  python build.py {recipe}\n'
	hlp = {
		'D': 'EP3 Build Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'R': 'Phone and firmware recipe to build',
		'v': 'Enable verbose output'
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
