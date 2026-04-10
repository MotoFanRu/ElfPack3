from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_model_def import AlephModelDef, ModelDef, HeaderDef, SymbolDef
from .aleph_filesystem import AlephFileSystem
from .aleph_stringer import AlephStringer
from .aleph_hexer import AlephHexer
from .aleph_patcher import AlephPatcher
from .aleph_exception import AlephParserException

class AlephFormatDef:
	DEF_HEADER_TEMPLATE = """\
# Aleph-generated definitions
# https://github.com/MotoFanRu/P2K-ELF-SDK
#
# PFW %pfw%
# VER %ver%
# CPU %cpu%

""".lstrip()

	log: AlephLogger
	chk: AlephChecker
	mdef: AlephModelDef
	fs: AlephFileSystem
	strs: AlephStringer
	hex: AlephHexer
	patch: AlephPatcher

	def __init__(
		self,
		logger: AlephLogger,
		checker: AlephChecker,
		mdef: AlephModelDef,
		filesystem: AlephFileSystem,
		stringer: AlephStringer,
		hexer: AlephHexer,
		patcher: AlephPatcher
	) -> None:
		self.log = logger
		self.chk = checker
		self.mdef = mdef
		self.fs = filesystem
		self.strs = stringer
		self.hex = hexer
		self.patch = patcher

	def find_header_lines(self, lines_c: list[str], required: list[tuple[str, str]]) -> list[str]:
		self.chk.is_ok_list(required, CheckerMode.EXCEPTION)

		results = []
		for prefix, err_msg in required:
			line = self.strs.find_line(lines_c, prefix)
			if not line or not self.chk.is_ok_str(line):
				raise AlephParserException(err_msg)
			results.append(line.replace(prefix, '').strip())

		self.chk.is_ok_list(results, CheckerMode.EXCEPTION)

		return results

	def parse_validate_header(self, lines_c: list[str]) -> HeaderDef:
		lines_t = [line.strip() for line in self.DEF_HEADER_TEMPLATE.splitlines()]

		if not self.strs.is_equal_strings(lines_t, lines_c, (0, 3)):
			self.log.D('No proper header comment')

		if not self.strs.is_equal_strings(lines_t, lines_c, (6, 6)):
			self.log.D('No blank line separator of header and body found')

		required = [
			('# PFW ', 'Cannot find PFW line (phone and firmware) in the header'),
			('# VER ', 'Cannot find VER line (version) in the header'),
			('# CPU ', 'Cannot find CPU line (architecture) in the header')
		]
		lines = self.find_header_lines(lines_c, required)
		if len(lines) != 3:
			raise AlephParserException(f'Unknown header props: {lines} ({len(lines)} items)')

		pfw, ver, cpu = lines
		header = HeaderDef(pfw, ver, cpu)
		if not self.mdef.is_header(header):
			raise AlephParserException(f'Header is not valid: {pfw} {ver} {cpu}')

		return header

	def parse_validate_line(self, line: str) -> SymbolDef:
		self.chk.is_ok_str(line, CheckerMode.EXCEPTION)

		parts = line.split()

		if len(parts) != 3:
			raise AlephParserException(f'Too many (more than 3) parts inside line: {line}')

		def_addr_str, def_type, def_name = parts
		def_addr = self.hex.u32i(def_addr_str)

		symbol = SymbolDef(def_addr, def_type, def_name)
		if not self.mdef.is_symbol(symbol, line):
			raise AlephParserException(f'Symbol is not valid: {symbol}')

		return symbol

	def parse_validate_body(self, lines_c: list[str]) -> list[SymbolDef]:
		symbols = []

		for line in lines_c:
			# Skip blank lines and comments.
			if self.chk.is_ok_str(line) and (not line.startswith('#')):
				symbols.append(self.parse_validate_line(line))
		self.chk.is_ok_list(symbols, CheckerMode.EXCEPTION)

		return symbols

	def read_def(self, p_i: Path, warn_on_duplicates: bool = False, sort_by_addrs: bool = False) -> ModelDef:
		self.chk.check_ext(p_i, ['.def'], CheckerMode.EXCEPTION)
		lines_c = self.fs.read_text_lines(p_i)
		self.chk.is_ok_list(lines_c, CheckerMode.EXCEPTION)

		header = self.parse_validate_header(lines_c)
		body = self.parse_validate_body(lines_c)

		if not self.mdef.check_addrs_names(body, warn_on_duplicates):
			raise AlephParserException(f'File did not pass validation: {p_i.name}')

		body = self.mdef.sort_symbols(body, sort_by_addrs)

		model = ModelDef(head=header, syms=body)

		if not self.mdef.is_def_model(model):
			raise AlephParserException(f'Model of file did not pass validation: {p_i.name}')

		return model

	def gen_def(self, model: ModelDef) -> str:
		if not self.mdef.is_def_model(model):
			raise AlephParserException('Model did not pass validation!')

		header  = ''
		markers = ['%pfw%',        '%ver%',        '%cpu%']
		patches = [model.head.pfw, model.head.ver, model.head.cpu]
		header += self.patch.patch_text(markers, patches, self.DEF_HEADER_TEMPLATE)

		content = '\n'.join(self.mdef.gen_def_line(s) for s in model.syms)
		self.chk.is_ok_str(content, CheckerMode.EXCEPTION)

		return header + content + '\n'

	def write_def(self, p_o: Path, model: ModelDef, update_version: bool = True) -> bool:
		if not self.mdef.is_def_model(model):
			return False

		if update_version:
			model.head.ver = self.mdef.gen_ver()

		return self.fs.write_text_file(p_o, self.gen_def(model))

	def format_def(self, p_i: Path, p_o: Path, warn_on_duplicates: bool = False, sort_by_addrs: bool = False) -> bool:
		self.chk.check_ext(p_i, ['.def'], CheckerMode.EXCEPTION)
		return self.write_def(p_o, self.read_def(p_i, warn_on_duplicates, sort_by_addrs))

	def del_def(self, p_io: Path, by_name: bool = False) -> bool:
		model = self.read_def(p_io)

		for d in model.syms:
			if d.name == by_name:
				self.log.I(f'Delete: {by_name} in {p_io}')
				model.syms.remove(d)
				self.write_def(p_io, model)
				return True

		self.log.I(f'Not found, cannot delete: {by_name} in {p_io}')
		return False

	def update_def(self, p_io: Path, sym_name: str, sym_type: str, sym_addr: int) -> bool:
		model = self.read_def(p_io)
		if not self.mdef.is_def_model(model):
			self.log.E(f'Model of file did not pass validation: {p_io.name}')
			return False

		sym_to_update = None
		for s in model.syms:
			if s.name == sym_name:
				sym_to_update = s
				break

		# Update version if symbol is not found (will be added later) or type/addr is changed.
		update_version = (not sym_to_update) or ((sym_to_update.addr != sym_addr) or (sym_to_update.type != sym_type))

		# Add new definition if not found or update it.
		if sym_to_update:
			if update_version:
				self.log.I(f'Update in: {p_io.name}, update_version: {update_version}')
				self.log.I(f' Old: {self.mdef.gen_def_line(sym_to_update)}')
				self.log.I(f' New: {self.mdef.gen_def_line_args(sym_addr, sym_type, sym_name)}')
			sym_to_update.addr = sym_addr
			sym_to_update.type = sym_type
			sym_to_update.name = sym_name
		else:
			self.log.I(f'Add to: {p_io.name}, update_version: {update_version}')
			self.log.I(f' New: {self.mdef.gen_def_line_args(sym_addr, sym_type, sym_name)}')
			model.syms.append(SymbolDef(
				type=sym_type,
				addr=sym_addr,
				name=sym_name
			))

		model.syms = self.mdef.sort_symbols(model.syms)

		if not self.mdef.is_def_model(model):
			self.log.E(f'Updated model of file did not pass validation: {p_io.name}')
			return False

		return self.write_def(p_io, model, update_version)
