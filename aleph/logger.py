import logging

def set_logger(verbose: bool) -> None:
	fmt_slim = '{levelname} {message}'
	fmt_full = '{asctime}.{msecs:03.0f} | {levelname:<8} | {filename:<15}{lineno:>5} | {funcName:<20} | {message}'
	fmt_date = '%Y-%m-%d %H:%M:%S'

	logging.basicConfig(
		level=logging.DEBUG if verbose else logging.INFO,
		format=fmt_full if verbose else fmt_slim,
		datefmt=fmt_date,
		style='{',
	)

logger = logging.getLogger(__name__)

def log_wrapper(level_func):
	def wrapper(msg, indent=0, *args, **kwargs):
		prefix = f'{"=" * indent}> ' if indent > 0 else ''
		return level_func(f'{prefix}{msg}', *args, **kwargs)
	return wrapper

D = log_wrapper(logger.debug)
I = log_wrapper(logger.info)
W = log_wrapper(logger.warning)
E = log_wrapper(logger.error)
C = log_wrapper(logger.critical)
