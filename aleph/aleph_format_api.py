from pathlib import Path

from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker, CheckerMode
from .aleph_model_def import AlephModelDef, ModelDef, SymbolDef
from .aleph_filesystem import AlephFileSystem
from .aleph_exception import AlephParserException

class AlephFormatApi:
	log: AlephLogger
	chk: AlephChecker
	mdef: AlephModelDef
	fs: AlephFileSystem

	def __init__(
		self,
		logger: AlephLogger,
		checker: AlephChecker,
		mdef: AlephModelDef,
		filesystem: AlephFileSystem
	) -> None:
		self.log = logger
		self.chk = checker
		self.mdef = mdef
		self.fs = filesystem

	def parse_validate_line(self, line: str) -> SymbolDef:
		self.chk.is_ok_str(line, CheckerMode.EXCEPTION)

		parts = line.split()

		if len(parts) != 1:
			raise AlephParserException(f'Too many (more than 1) parts inside line: {line}')

		if len(line) > 80:
			raise AlephParserException(f'Too long name (more than 80 characters) in the line: {line}')

		return SymbolDef(0x00000000, 'A', line)

	def parse_validate_body(self, lines_c: list[str]) -> list[SymbolDef]:
		symbols = []

		for line in lines_c:
			# Skip blank lines and comments.
			if self.chk.is_ok_str(line) and (not line.startswith('#')):
				symbols.append(self.parse_validate_line(line))
		self.chk.is_ok_list(symbols, CheckerMode.EXCEPTION)

		return symbols

	def read_api(self, p_i: Path, warn_on_duplicates: bool = False) -> ModelDef:
		self.chk.check_ext(p_i, ['.api'], CheckerMode.EXCEPTION)
		lines_c = self.fs.read_text_lines(p_i)
		self.chk.is_ok_list(lines_c, CheckerMode.EXCEPTION)

		body = self.parse_validate_body(lines_c)

		if not self.mdef.check_addrs_names(body, warn_on_duplicates):
			raise AlephParserException(f'File did not pass validation: {p_i.name}')

		body = self.mdef.sort_symbols(body, False)

		model = self.mdef.gen_model()
		model.syms = body

		if not self.mdef.is_def_model(model):
			raise AlephParserException(f'Model of file did not pass validation: {p_i.name}')

		return model
