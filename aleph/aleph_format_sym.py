from pathlib import Path

from .aleph_format_def import AlephFormatDef
from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_model_def import AlephModelDef, ModelDef, HeaderDef
from .aleph_filesystem import AlephFileSystem
from .aleph_patcher import AlephPatcher
from .aleph_stringer import AlephStringer
from .aleph_hexer import AlephHexer
from .aleph_exception import AlephParserException

class AlephFormatSym(AlephFormatDef):
	SYM_HEADER_TEMPLATE = """\
#<SYMDEFS>#symdef-file
# SYMDEFS ADS HEADER

""".lstrip()

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
		super().__init__(logger, checker, mdef, filesystem, stringer, hexer, patcher)

	def parse_validate_header(self, lines_c: list[str]) -> HeaderDef:
		lines_t = [line.strip() for line in self.SYM_HEADER_TEMPLATE.splitlines()]

		if not self.strs.is_equal_strings(lines_t, lines_c, (0, 3)):
			self.log.D('No proper header comment')

		required = [
			('# Phone: ', 'Cannot find phone line in the header'),
			('# Firmware: ', 'Cannot find firmware line in the header'),
		]
		lines = self.find_header_lines(lines_c, required)
		if len(lines) != 2:
			self.log.W(f'Cannot find phone firmware in header: {lines} ({len(lines)} items)')
			lines = 'PHONE', 'FIRMWARE'

		header = self.mdef.gen_header()
		header.pfw = '_'.join(lines)

		if not self.mdef.is_header(header):
			raise AlephParserException(f'Header is not valid: {header.pfw} {header.ver} {header.cpu}')

		return header

	def read_sym(self, p_i: Path, warn_on_duplicates: bool = False, sort_by_addrs: bool = False) -> ModelDef:
		self.chk.check_ext(p_i, ['.sym'], CheckerMode.EXCEPTION)
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

	def def_to_sym(self, model: ModelDef) -> ModelDef:
		self.chk.is_ok_list(model.syms, CheckerMode.EXCEPTION)
		# C => D
		# M => A
		for sym in model.syms:
			if sym.type == 'C':
				sym.type = 'D'
			elif sym.type == 'M':
				sym.type = 'A'
		return model

	def gen_sym(self, model: ModelDef) -> str:
		return self.SYM_HEADER_TEMPLATE + self.gen_def(model)

	def write_sym(self, p_o: Path, model: ModelDef) -> bool:
		if not self.mdef.is_def_model(model):
			return False
		model = self.def_to_sym(model)
		return self.fs.write_text_file(p_o, self.gen_sym(model))

	def format_sym(self, p_i: Path, p_o: Path, warn_on_duplicates: bool = False, sort_by_addrs: bool = False) -> bool:
		self.chk.check_ext(p_i, ['.sym'], CheckerMode.EXCEPTION)
		return self.write_sym(p_o, self.read_sym(p_i, warn_on_duplicates, sort_by_addrs))
