import sys
from argparse import Namespace
from argparse import ArgumentParser

class Arguments(ArgumentParser):
	def error(self, message: str) -> None:
		self.print_help(sys.stderr)
		self.exit(2, f'\n{self.prog}: error: {message}\n')

	def parse_and_check_args(self) -> Namespace:
		args = self.parse_args()
		self.check_args(args)
		return args

	def check_args(self, args: Namespace) -> None:
		pass
