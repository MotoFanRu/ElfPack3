class AlephHashFnv1a32:
	@staticmethod
	def hash(data: bytes) -> int:
		hash_val = 0x811C9DC5
		for byte in data:
			hash_val ^= byte
			hash_val = (hash_val * 0x01000193) & 0xFFFFFFFF
		return hash_val
