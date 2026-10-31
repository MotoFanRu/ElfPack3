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

	# EXL, 31-Oct-2026: The stub libep3.so is fake anyway, so provide fake addresses for
	#   the symbols the test ELFs import but the recipe ep3.def does not list (EP3_Logger,
	#   dl* API). Real addresses are resolved at runtime from the on-device ep3.def and
	#   the DEF library inserts, so these values are never executed.
	def_test_res = a.consts.P2K_SDK_BUILD / 'ep3_test.def'
	def_lines = def_res.read_text().splitlines()
	def_fake = [
		'0x00000002 T EP3_Logger',
		'0x00000004 T dlopen',
		'0x00000006 T dlsym',
		'0x00000008 T dlclose',
		'0x0000000A T dlerror',
	]
	for fake in def_fake:
		sym_name = fake.split()[2]
		if not any(line.endswith(' ' + sym_name) for line in def_lines):
			def_lines.append(fake)
	def_test_res.write_text('\n'.join(def_lines) + '\n')

	defines = a.fdef.read_def(def_test_res)

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

def build_ep3_test_so(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build EP3 Test SO:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_lib_so, *recipe.flags.build, *recipe.flags.elf_ldr
	]

	def link_so(obj, name):
		lflags = [
			*recipe.toolchain.lflags_so_lib,
			f'-Wl,-soname,{name}',
			'-Wl,--hash-style=sysv',
			'-Wl,-zmax-page-size=4',
		]
		so_res = a.consts.P2K_SDK_BUILD / name
		if not a.toolchain.gcc_ld(recipe.toolchain, [obj], so_res, cflags, lflags):
			a.log.E(f'Cannot create: {so_res}')
			return False
		a.fs.copy_file(so_res, a.consts.P2K_SDK_RELEASE / recipe_name / name, True, True)
		return True

	# EXL, 31-Oct-2026: mylib.so for TestSO_1 and lib1.so (MyLib clone) for TestSO_2.
	so_src = a.consts.P2K_SDK_SRC / 'test' / 'MyLib_1.c'
	so_obj = a.consts.P2K_SDK_BUILD / 'MyLib_1.o'
	if not a.toolchain.gcc_cc(recipe.toolchain, so_src, so_obj, cflags):
		a.log.E(f'Cannot compile: {so_src.name} => {so_obj.name}')
		return False
	if not link_so(so_obj, 'mylib.so') or not link_so(so_obj, 'lib1.so'):
		return False

	# EXL, 31-Oct-2026: lib2.so — constructor + firmware data symbol GLOBAL_VARIABLE.
	so_src = a.consts.P2K_SDK_SRC / 'test' / 'MyLib_2.c'
	so_obj = a.consts.P2K_SDK_BUILD / 'MyLib_2.o'
	if not a.toolchain.gcc_cc(recipe.toolchain, so_src, so_obj, cflags):
		a.log.E(f'Cannot compile: {so_src.name} => {so_obj.name}')
		return False
	if not link_so(so_obj, 'lib2.so'):
		return False

	# EXL, 31-Oct-2026: lib3.so — SO -> SO call into lib1.so (DT_NEEDED via stub link).
	so_src = a.consts.P2K_SDK_SRC / 'test' / 'MyLib_3.c'
	so_obj = a.consts.P2K_SDK_BUILD / 'MyLib_3.o'
	if not a.toolchain.gcc_cc(recipe.toolchain, so_src, so_obj, cflags):
		a.log.E(f'Cannot compile: {so_src.name} => {so_obj.name}')
		return False
	lflags = [
		*recipe.toolchain.lflags_so_lib,
		'-Wl,-soname,lib3.so',
		'-Wl,--hash-style=sysv',
		'-Wl,-zmax-page-size=4',
		f'{a.consts.P2K_SDK_BUILD / "lib1.so"}',
	]
	so_res = a.consts.P2K_SDK_BUILD / 'lib3.so'
	if not a.toolchain.gcc_ld(recipe.toolchain, [so_obj], so_res, cflags, lflags):
		a.log.E(f'Cannot create: {so_res}')
		return False
	a.fs.copy_file(so_res, a.consts.P2K_SDK_RELEASE / recipe_name / 'lib3.so', True, True)

	return True

def build_ep3_elf_ui(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build ELF UI:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_elf_ldr, *recipe.flags.build, *recipe.flags.elf_ldr
	]
	src_obj = [
		(a.consts.P2K_SDK_SRC / 'test' / 'HelloUI_1.c', a.consts.P2K_SDK_BUILD / 'HelloUI_1.o'),
		(a.consts.P2K_SDK_SRC / 'test' / 'HelloUI_2.c', a.consts.P2K_SDK_BUILD / 'HelloUI_2.o'),
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
	elf1_res = a.consts.P2K_SDK_BUILD / 'HelloUI_1.elf'
	elf2_res = a.consts.P2K_SDK_BUILD / 'HelloUI_2.elf'
	# objs = [obj for _, obj in src_obj]
	if not a.toolchain.gcc_ld(recipe.toolchain, [a.consts.P2K_SDK_BUILD / 'HelloUI_1.o'], elf1_res, cflags, lflags):
		a.log.E(f'Cannot link: {elf1_res}')
		return False
	if not a.toolchain.gcc_ld(recipe.toolchain, [a.consts.P2K_SDK_BUILD / 'HelloUI_2.o'], elf2_res, cflags, lflags):
		a.log.E(f'Cannot link: {elf2_res}')
		return False

	a.fs.copy_file(elf1_res, a.consts.P2K_SDK_RELEASE / recipe_name / 'a.elf', True, True)
	a.fs.copy_file(elf2_res, a.consts.P2K_SDK_RELEASE / recipe_name / 'b.elf', True, True)

	return True

def build_ep3_elf_so(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build ELF SO Test:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_elf_ldr, *recipe.flags.build, *recipe.flags.elf_ldr
	]
	so_src = a.consts.P2K_SDK_SRC / 'test' / 'TestSO_1.c'
	so_obj = a.consts.P2K_SDK_BUILD / 'TestSO_1.o'
	if not a.toolchain.gcc_cc(recipe.toolchain, so_src, so_obj, cflags):
		a.log.E(f'Cannot compile: {so_src.name} => {so_obj.name}')
		return False

	lflags = [
		*recipe.toolchain.lflags_elf_ldr,
		'-Wl,-zmax-page-size=4',
		'-Wl,-eRegister',
		'-pie',
		# EXL, 31-Oct-2026: Export ui_notify (SO -> ELF call) into the dynamic symbol table.
		'-Wl,--export-dynamic',
		# EXL, 31-Oct-2026: The fake libep3.so stub supplies import symbols (APP_*, dl*, ...).
		f'-L{a.consts.P2K_SDK_BUILD}', '-lep3',
		# EXL, 31-Oct-2026: Link against the real mylib.so as a stub: records the DT_NEEDED
		#   dependency and import relocations, the code is resolved at runtime from mylib.so.
		f'{a.consts.P2K_SDK_BUILD / "mylib.so"}',
	]
	so_elf_res = a.consts.P2K_SDK_BUILD / 'TestSO_1.elf'
	if not a.toolchain.gcc_ld(recipe.toolchain, [so_obj], so_elf_res, cflags, lflags):
		a.log.E(f'Cannot link: {so_elf_res}')
		return False

	a.fs.copy_file(so_elf_res, a.consts.P2K_SDK_RELEASE / recipe_name / 'so.elf', True, True)

	return True

def build_ep3_elf_so2(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I(f' Build ELF SO Test 2:')

	cflags = [
		*recipe.soc.opt, *recipe.toolchain.cflags_elf_ldr, *recipe.flags.build, *recipe.flags.elf_ldr
	]
	so_src = a.consts.P2K_SDK_SRC / 'test' / 'TestSO_2.c'
	so_obj = a.consts.P2K_SDK_BUILD / 'TestSO_2.o'
	if not a.toolchain.gcc_cc(recipe.toolchain, so_src, so_obj, cflags):
		a.log.E(f'Cannot compile: {so_src.name} => {so_obj.name}')
		return False

	lflags = [
		*recipe.toolchain.lflags_elf_ldr,
		'-Wl,-zmax-page-size=4',
		'-Wl,-eRegister',
		'-pie',
		# EXL, 31-Oct-2026: Export ui_notify (SO -> ELF call) into the dynamic symbol table.
		'-Wl,--export-dynamic',
		# EXL, 31-Oct-2026: The fake libep3.so stub supplies import symbols (APP_*, ...).
		f'-L{a.consts.P2K_SDK_BUILD}', '-lep3',
		# EXL, 31-Oct-2026: DT_NEEDED lib1.so, lib2.so, lib3.so (stub links only).
		f'{a.consts.P2K_SDK_BUILD / "lib1.so"}',
		f'{a.consts.P2K_SDK_BUILD / "lib2.so"}',
		f'{a.consts.P2K_SDK_BUILD / "lib3.so"}',
	]
	so_elf_res = a.consts.P2K_SDK_BUILD / 'TestSO_2.elf'
	if not a.toolchain.gcc_ld(recipe.toolchain, [so_obj], so_elf_res, cflags, lflags):
		a.log.E(f'Cannot link: {so_elf_res}')
		return False

	a.fs.copy_file(so_elf_res, a.consts.P2K_SDK_RELEASE / recipe_name / 'so2.elf', True, True)

	return True

def cook_recipe(a: Aleph, recipe_name: str, recipe: Recipe) -> bool:
	a.log.I('')
	a.log.I(f'Cook recipe: {recipe_name}')

	a.fs.create_dir(a.consts.P2K_SDK_BUILD, True)

	build_steps = [
		(build_ep3_so_lib, [a, recipe_name, recipe], 'Cannot build EP3 Shared Library'),
		(build_ep3_test_so, [a, recipe_name, recipe], 'Cannot build EP3 Test SO'),
		(build_ep3_elf_ui, [a, recipe_name, recipe], 'Cannot build ELF UI'),
		(build_ep3_elf_so, [a, recipe_name, recipe], 'Cannot build ELF SO'),
		(build_ep3_elf_so2, [a, recipe_name, recipe], 'Cannot build ELF SO 2'),
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
