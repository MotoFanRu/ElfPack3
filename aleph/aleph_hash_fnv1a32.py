CONSTS_TAG_PREFIX: int = 0xC0000000
CONSTS_TAG_MASKED: int = 0x0FFFFFFF

class AlephHashFnv1a32:
	@staticmethod
	def hash(data: bytes) -> int:
		hash_val = 0x811C9DC5
		for byte in data:
			hash_val ^= byte
			hash_val = (hash_val * 0x01000193) & 0xFFFFFFFF
		return hash_val

	@staticmethod
	def hash_const(name: str) -> int:
		return CONSTS_TAG_PREFIX | (AlephHashFnv1a32.hash(name.encode('ascii')) & CONSTS_TAG_MASKED)
