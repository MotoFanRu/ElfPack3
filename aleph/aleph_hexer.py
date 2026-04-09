from .aleph_logger import AlephLogger
from .aleph_checker import AlephChecker
from .aleph_exception import AlephHexerException

class AlephHexer:
	log: AlephLogger
	chk: AlephChecker

	def __init__(self, logger: AlephLogger, chk: AlephChecker) -> None:
		self.log = logger
		self.chk = chk

	def is_u32(self, val: str | int) -> bool:
		if isinstance(val, str) and self.chk.is_ok_str(val):
			try:
				val = int(val, 0)
			except ValueError as e:
				self.log.E(f'Invalid 32-bit value: {val}, 0x{val:08X}, {e}')
				return False

		if not (0 <= val <= 0xFFFFFFFF):
			self.log.E(f'Beyond 32-bit value limits: {val}, 0x{val:08X}')
			return False

		return True

	def u32i(self, val: str) -> int:
		if not self.is_u32(val):
			raise AlephHexerException(f'Invalid 32-bit value: {val}, 0x{val:08X}')
		return int(val, 0)

	def u32s(self, val: int) -> str:
		if not self.is_u32(val):
			raise AlephHexerException(f'Invalid 32-bit value: {val}, 0x{val:08X}')
		return f'0x{val:08X}'

	def is_align(self, val: str | int, align: int) -> bool:
		if align == 0:
			return True

		if isinstance(val, str) and self.is_u32(val):
			val = int(val, 0)

		if not (val % align == 0):
			self.log.W(f'Value: {val}, 0x{val:08X} is not align by: {align}, 0x{align:08X}')
			return False

		return True
