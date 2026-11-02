#!/usr/bin/env python3

"""
EP3 Const Generation Tool

Reads the constants header (#define NAME (0xVALUE) lines), computes the FNV1A32-based
tagged hash for every constant (0xC0000000 | (FNV1A32(name) & 0x0FFFFFFF) — the same
hash the ELF loader uses, see sdk/P2K_Hash_Portable.h and sdk/P2K_EP3_DEF_Library.h),
checks the whole list for collisions and writes the values back.

Line endings: the file is read through universal-newline translation, so both LF and
CR LF variants parse identically; it is written back through the platform text mode,
so the file has LF line endings on Linux and CR LF on Windows.
"""

from pathlib import Path

from aleph import Aleph
from aleph.aleph_hash_fnv1a32 import AlephHashFnv1a32
from aleph.aleph_exception import AlephException

def const_chunks(line: str) -> list[str] | None:
	chunks = line.split()
	if (len(chunks) != 3) or (chunks[0] != '#define') or not chunks[2].startswith('('):
		return None
	return chunks

def parse_constants(a: Aleph, text: str) -> dict[str, int]:
	constants: dict[str, int] = {}

	for raw_line in text.splitlines():
		line = raw_line.strip()
		if not line.startswith('#define'):
			continue

		chunks = const_chunks(line)
		if chunks is None:  # Header guard or a plain macro without a value.
			continue

		_, name, value_tok = chunks
		if name in constants:
			raise AlephException(f'Duplicate constant name: {name}')

		inner = value_tok.strip('()')
		if not name.isidentifier():
			raise AlephException(f'Bad constant name: {name}')
		if not inner.startswith('0x'):
			raise AlephException(f'Bad constant value (expected 0x-prefixed): {value_tok}')

		constants[name] = a.hex.u32i(inner)

	if len(constants) == 0:
		raise AlephException(f'No "#define NAME (0xVALUE)" constants found in {a.consts.P2K_SDK_CONSTS_HEADER.name}')

	a.log.I(f'Parsed constants: {len(constants)}')

	return constants

def check_collisions(a: Aleph, constants: dict[str, int], hashes: dict[str, int]) -> None:
	dup_names = a.strs.find_duplicates(list(constants.keys()))
	if dup_names:
		raise AlephException(f'Constant name collisions: {dup_names}')

	dup_tags = a.strs.find_duplicates(list(hashes.values()))
	if dup_tags:
		report = ', '.join(f'{name}=0x{hashes[name]:08X}' for name in constants if hashes[name] in dup_tags)
		raise AlephException(f'Constant tag collisions: {report}')

	a.log.I('Collision check: names OK, tags OK')

def update_constants(a: Aleph, text: str, hashes: dict[str, int]) -> str:
	fixed = 0
	updated = []

	for line in text.splitlines():
		stripped = line.strip()
		if const_chunks(stripped) is not None:
			_, name, value_tok = stripped.split(maxsplit=2)
			inner = value_tok.strip('()')
			new_value = a.hex.u32s(hashes[name])

			is_updated = inner != new_value
			if is_updated:
				fixed += 1

			state = 'UPDATED' if is_updated else 'KEEP'
			a.log.I(f'\t#define {name.ljust(40)} ({inner}) => ({new_value}) ({state})')
			line = line.replace(inner, new_value, 1)

		updated.append(line)

	a.log.I(f'Constants updated: {fixed} of {len(hashes)}')

	return '\n'.join(updated) + '\n'

def do_const_work(a: Aleph) -> bool:
	body = a.fs.read_text_file(a.consts.P2K_SDK_CONSTS_HEADER)
	if not a.chk.is_ok_str(body):
		a.log.C(f'File not found or empty: "{a.consts.P2K_SDK_CONSTS_HEADER.name}"')
		return False

	try:
		constants = parse_constants(a, body)
		hashes: dict[str, int] = {name: AlephHashFnv1a32.hash_const(name) for name in constants}
		check_collisions(a, constants, hashes)
		updated = update_constants(a, body, hashes)
	except AlephException as e:
		a.log.C(f'Exception: {e}')
		return False

	if not a.fs.write_text_file(a.consts.P2K_SDK_CONSTS_HEADER, updated):
		a.log.C(f'Cannot write: {a.consts.P2K_SDK_CONSTS_HEADER}')
		return False

	a.log.I(f'Constants written: {len(hashes)} => {a.consts.P2K_SDK_CONSTS_HEADER.name}')

	return True

def fill_const_args(a: Aleph) -> None:
	epl = 'examples:\n'
	epl += f'  python {Path(__file__).name}\n'

	hlp = {
		'D': 'EP3 Const Generation Tool for Motorola phones on P2K platform, 01-Nov-2026',
		'v': 'enable verbose output'
	}

	a.args_parser.add_epilog_description(epl, hlp['D'])
	a.args_parser.add_argument('-v', '--verbose', action='store_true', help=hlp['v'])

if __name__ == '__main__':
	aleph = Aleph()
	aleph.set_args(fill_const_args)
	aleph.main(do_const_work)
