#!/usr/bin/env python3

"""
EP3 Build Test Tool

A general tool for building testing ELFs for a selected platform.
"""

from pathlib import Path

from aleph import Aleph
from aleph import Recipe

def build_ep3_so_lib(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build EP3 Shared Library:')

	asm_tpl = recipe.soc.asm
	asm_src = a.consts.P2K_SDK_BUILD / asm_tpl.name.replace('.tpl', '')
	asm_obj = a.consts.P2K_SDK_BUILD / asm_src.name.replace('.S', '.o')
	def_res = a.consts.P2K_SDK_RES / recipe_name / 'ep3.def'

	defines = a.fdef.read_def(def_res)

	# TODO: Add EP1 API defs here?

	if not a.fasm.write_asm(asm_tpl, asm_src, defines):
		a.log.E(f'Cannot create: {def_res.name} => {asm_src}')
		return False

	cflags = [ *recipe.soc.opt, *recipe.flags.build, *recipe.toolchain.cflags_lib_so ]
	if not a.toolchain.gcc_cc(recipe.toolchain, asm_src, asm_obj, cflags):
		a.log.E(f'Cannot compile: {asm_src.name} => {asm_obj.name}')
		return False

	so_lib = a.consts.P2K_SDK_BUILD / 'libep3.so'
	objs = [ asm_obj ]
	if not a.toolchain.gcc_ld(recipe.toolchain, objs, so_lib, cflags, recipe.toolchain.lflags_so_lib):
		a.log.E(f'Cannot create: {so_lib}')
		return False

	return True

def build_ep3_elf_ui(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build ELF UI:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_elf_ldr, *recipe.flags.build, *recipe.flags.elf_ldr
	]
	src_obj = [
		(a.consts.P2K_SDK_SRC / 'test' / 'HelloUI.c', a.consts.P2K_SDK_BUILD / 'HelloUI.o'),
	]
	for src, obj in src_obj:
		if not a.toolchain.gcc_cc(recipe.toolchain, src, obj, cflags):
			a.log.E(f'Cannot compile: {src.name} => {obj.name}')
			return False

	lflags = [
		*recipe.toolchain.lflags_elf_ldr,
		'-Wl,-zmax-page-size=4',
		'-Wl,-eRegister',
		'-pie',
		f'-L{a.consts.P2K_SDK_BUILD}', '-lep3',
	]
	elf_res = a.consts.P2K_SDK_BUILD / 'HelloUI.elf'
	objs = [obj for _, obj in src_obj]
	if not a.toolchain.gcc_ld(recipe.toolchain, objs, elf_res, cflags, lflags):
		a.log.E(f'Cannot link: {elf_res}')
		return False

	a.fs.copy_file(elf_res, a.consts.P2K_SDK_RELEASE / recipe_name / 'h.elf', True, True)

	return True

def cook_recipe(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I('')
	a.log.I(f'Cook recipe: {recipe_name}')

	a.fs.create_dir(a.consts.P2K_SDK_BUILD, True)

	build_steps = [
		(build_ep3_so_lib, [a, recipe_name, recipe], 'Cannot build EP3 Shared Library'),
		(build_ep3_elf_ui, [a, recipe_name, recipe], 'Cannot build ELF UI'),
		#(build_ep3_elf_so, [a, recipe_name, recipe], 'Cannot build ELF SO'),
	]

	for func, args, error_msg in build_steps:
		if not func(*args):
			a.log.E(f'{error_msg}: {recipe_name}')
			return False

	return True

def do_build_work(a: Aleph) -> bool:
	if a.args.recipe == 'all':
		a.log.I('Cook all recipes.')
		for recipe in a.recipe.recipes.keys():
			if not cook_recipe(a, recipe, a.recipe.recipes[recipe]):
				return False
		return True
	return cook_recipe(a, a.args.recipe, a.recipe.recipes[a.args.recipe])

def fill_build_args(a: Aleph) -> None:
	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name} all\n'
	for recipe in a.recipe.recipes.keys():
		epl += f'  python {Path(__file__).name} {recipe}\n'

	hlp = {
		'D': 'EP3 Build Tool for Motorola phones on P2K platform, 01-Mar-2026',
		'R': 'phone and firmware recipe to build',
		'v': 'enable verbose output'
	}

	a.args_parser.add_epilog_description(epl, hlp['D'])
	a.args_parser.add_argument('recipe', type=str, help=hlp['R'])
	a.args_parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])

def check_build_args(a: Aleph) -> None:
	if (a.args.recipe not in a.recipe.recipes) and (a.args.recipe != 'all'):
		a.args_parser.error(f'Recipe not supported: {a.args.recipe}')

if __name__ == '__main__':
	aleph = Aleph()
	aleph.set_args(fill_build_args, check_build_args)
	aleph.main(do_build_work)
