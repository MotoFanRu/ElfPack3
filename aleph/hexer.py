from .logger import E

def is_valid_32bit_addr(addr_str: str) -> bool:
	try:
		val = int(addr_str, 0)
		return 0 <= val <= 0xFFFFFFFF
	except ValueError:
		return False

def format_32bit_addr(addr: int) -> str | None:
	if not is_valid_32bit_addr(str(addr)):
		E(f'Unknown 32-bit address: {addr}')
		return None
	return f'0x{addr:08X}'
