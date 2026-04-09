import logging
from typing import Callable

class AlephLogger:
	D: Callable
	I: Callable
	W: Callable
	E: Callable
	C: Callable

	def __init__(self) -> None:
		root = logging.getLogger()

		self.D = root.debug
		self.I = root.info
		self.W = root.warning
		self.E = root.error
		self.C = root.critical

	def configure(self, verbose: bool) -> None:
		fmt_slim = '{levelname} {message}'
		fmt_full = '{asctime}.{msecs:03.0f} | {levelname:<8} | {filename:<25}{lineno:>5} | {funcName} \n{message}\n'
		fmt_date = '%Y-%m-%d %H:%M:%S'

		root = logging.getLogger()
		if not root.handlers:
			logging.basicConfig(
				level=logging.DEBUG if verbose else logging.INFO,
				format=fmt_full if verbose else fmt_slim,
				datefmt=fmt_date,
				style='{',
			)
			self.D(f'Logger initialized, verbose: {verbose}')
