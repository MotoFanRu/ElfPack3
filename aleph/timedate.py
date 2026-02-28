from datetime import timedelta

def elapsed_format(td: timedelta) -> str:
	total_seconds = int(td.total_seconds())
	hours, remainder = divmod(total_seconds, 3600)
	minutes, seconds = divmod(remainder, 60)
	milliseconds = td.microseconds // 1000
	return f'{hours:02d}:{minutes:02d}:{seconds:02d}.{milliseconds:03d}'
