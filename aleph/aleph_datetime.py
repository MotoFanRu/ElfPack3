from datetime import date
from datetime import timedelta

class AlephDateTime:
	@staticmethod
	def elapsed_format(td: timedelta) -> str:
		total_seconds = int(td.total_seconds())
		hours, remainder = divmod(total_seconds, 3600)
		minutes, seconds = divmod(remainder, 60)
		milliseconds = td.microseconds // 1000
		return f'{hours:02d}:{minutes:02d}:{seconds:02d}.{milliseconds:03d}'

	@staticmethod
	def date_today() -> str:
		return date.today().strftime("%d-%b-%Y")
