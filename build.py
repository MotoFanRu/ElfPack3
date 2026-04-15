#!/usr/bin/env python3

"""
EP3 Build Tool

A general tool for building ELF Loader, BIN Loader, and libraries for a selected platform.
"""

from pathlib import Path

from aleph import Aleph
from aleph import Recipe

def build_ep3_lib(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build EP3 Library:')

	asm_tpl = recipe.soc.asm
	asm_src = a.consts.P2K_SDK_BUILD / asm_tpl.name.replace('.tpl', '')
	asm_obj = a.consts.P2K_SDK_BUILD / asm_src.name.replace('.S', '.o')
	def_res = a.consts.P2K_SDK_RES / recipe_name / 'ep3.def'

	defines = a.fdef.read_def(def_res)

	if not a.fasm.write_asm(asm_tpl, asm_src, defines):
		a.log.E(f'Cannot create: {def_res.name} => {asm_src}')
		return False

	cflags = [ *recipe.soc.opt, *recipe.flags.build, *recipe.toolchain.cflags_lib_so ]
	if not a.toolchain.gcc_cc(recipe.toolchain, asm_src, asm_obj, cflags):
		a.log.E(f'Cannot compile: {asm_src.name} => {asm_obj.name}')
		return False

	a_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_Library.a'
	objs = [ asm_obj ]
	if not a.toolchain.gcc_ar(recipe.toolchain, objs, a_res):
		a.log.E(f'Cannot archive: {a_res}')
		return False

	# TODO: Links library to .a and .so here?
	# TODO: Copy library to Release?

	return True

def build_bin_ldr(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build BIN Loader:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_bin_ldr, *recipe.flags.build, *recipe.flags.bin_ldr,
		f'-DFTR_LOAD_TO_ADDR={a.hex.u32s(recipe.addresses.loader)}',
	]
	src_obj = [
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_Logger.c',        a.consts.P2K_SDK_BUILD / 'P2K_EP3_Logger.o'),
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_Memory.c',        a.consts.P2K_SDK_BUILD / 'P2K_EP3_Memory.o'),
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_File_System.c',   a.consts.P2K_SDK_BUILD / 'P2K_EP3_File_System.o'),
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_Task_Reactor.c',  a.consts.P2K_SDK_BUILD / 'P2K_EP3_Task_Reactor.o'),
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_APP_Viewer.c',    a.consts.P2K_SDK_BUILD / 'P2K_EP3_APP_Viewer.o'),
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_BIN_Loader.c',    a.consts.P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.o'),
	]
	for src, obj in src_obj:
		if not a.toolchain.gcc_cc(recipe.toolchain, src, obj, cflags):
			a.log.E(f'Cannot compile: {src.name} => {obj.name}')
			return False

	lflags = [
		*recipe.toolchain.lflags_bin_ldr,
		f'-Wl,-L,{recipe.soc.lds.parent}',
		f'-Wl,-T,{recipe.soc.lds.name}',
		f'-Wl,-Ttext={a.hex.u32s(recipe.addresses.inject)}',
		f'{a.consts.P2K_SDK_BUILD / "P2K_EP3_Library.a"}',
	]
	elf_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.elf'
	objs = [obj for _, obj in src_obj]
	if not a.toolchain.gcc_ld(recipe.toolchain, objs, elf_res, cflags, lflags):
		a.log.E(f'Cannot link: {elf_res}')
		return False

	map_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.map'
	if not a.toolchain.gcc_nm(recipe.toolchain, elf_res, map_res):
		a.log.E(f'Cannot create: {elf_res} => {map_res}')
		return False

	bin_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_BIN_Loader.bin'
	if not a.toolchain.gcc_bin(recipe.toolchain, elf_res, bin_res):
		a.log.E(f'Cannot create: {elf_res} => {bin_res}')
		return False

	a.fs.copy_file(bin_res, a.consts.P2K_SDK_RELEASE / recipe_name / a.consts.EP3_BIN_LDR, True, True)
	# TODO: Create and combine patches instead of copying loader file?

	return True

def build_elf_ldr(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build ELF Loader:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_elf_ldr, *recipe.flags.build, *recipe.flags.elf_ldr,
		f'-DFTR_LOAD_TO_ADDR={a.hex.u32s(recipe.addresses.loader)}',
	]
	src_obj = [
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_DEF_Library.c',   a.consts.P2K_SDK_BUILD / 'P2K_EP3_DEF_Library.o'),
		(a.consts.P2K_SDK_SRC / 'P2K_EP3_ELF_Loader.c',   a.consts.P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.o'),
	]
	for src, obj in src_obj:
		if not a.toolchain.gcc_cc(recipe.toolchain, src, obj, cflags):
			a.log.E(f'Cannot compile: {src.name} => {obj.name}')
			return False

	lflags = [
		*recipe.toolchain.lflags_elf_ldr,
		f'-Wl,-L,{recipe.soc.lds.parent}',
		f'-Wl,-T,{recipe.soc.lds.name}',
		f'-Wl,-Ttext={a.hex.u32s(recipe.addresses.loader)}',
		f'{a.consts.P2K_SDK_BUILD / "P2K_EP3_Library.a"}',
	]
	elf_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.elf'
	objs = [obj for _, obj in src_obj]
	objs.append(a.consts.P2K_SDK_BUILD / 'P2K_EP3_Logger.o')
	objs.append(a.consts.P2K_SDK_BUILD / 'P2K_EP3_Memory.o')
	objs.append(a.consts.P2K_SDK_BUILD / 'P2K_EP3_File_System.o')
	if not a.toolchain.gcc_ld(recipe.toolchain, objs, elf_res, cflags, lflags):
		a.log.E(f'Cannot link: {elf_res}')
		return False

	map_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.map'
	if not a.toolchain.gcc_nm(recipe.toolchain, elf_res, map_res):
		a.log.E(f'Cannot create: {elf_res} => {map_res}')
		return False

	bin_res = a.consts.P2K_SDK_BUILD / 'P2K_EP3_ELF_Loader.bin'
	if not a.toolchain.gcc_bin(recipe.toolchain, elf_res, bin_res):
		a.log.E(f'Cannot create: {elf_res} => {bin_res}')
		return False

	a.fs.copy_file(bin_res, a.consts.P2K_SDK_RELEASE / recipe_name / a.consts.EP3_ELF_LDR, True, True)

	return True

def cook_recipe(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I('')
	a.log.I(f'Cook recipe: {recipe_name}')

	a.fs.create_dir(a.consts.P2K_SDK_BUILD, True)

	build_steps = [
		(build_ep3_lib, [a, recipe_name, recipe], 'Cannot build EP3 Library'),
		(build_bin_ldr, [a, recipe_name, recipe], 'Cannot build BIN Loader'),
		(build_elf_ldr, [a, recipe_name, recipe], 'Cannot build ELF Loader'),
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
