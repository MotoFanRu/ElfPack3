import sys
from argparse import ArgumentParser

class AlephArgs(ArgumentParser):
	def add_epilog(self, epilog: str) -> None:
		self.epilog = epilog

	def add_description(self, description: str) -> None:
		self.description = description

	def add_epilog_description(self, epilog: str, description: str) -> None:
		self.add_epilog(epilog)
		self.add_description(description)

	def error(self, message: str | None = None) -> None:
		self.print_help(sys.stderr)
		if message:
			self.exit(2, f'\n{self.prog}: error: {message}\n')
		else:
			self.exit(2)
