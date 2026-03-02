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

D = logger.debug
I = logger.info
W = logger.warning
E = logger.error
C = logger.critical
