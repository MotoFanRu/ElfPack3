from pathlib import Path

class AlephConsts:
	P2K_SDK_ROOT: Path = Path(__file__).resolve().parent.parent

	P2K_BUILD_CONFIG: Path = P2K_SDK_ROOT / 'config.toml'

	P2K_SDK_API: Path = P2K_SDK_ROOT / 'api'
	P2K_SDK_ASM: Path = P2K_SDK_ROOT / 'asm'
	P2K_SDK_RES: Path = P2K_SDK_ROOT / 'res'
	P2K_SDK_INC: Path = P2K_SDK_ROOT / 'inc'
	P2K_SDK_LDS: Path = P2K_SDK_ROOT / 'lds'
	P2K_SDK_SRC: Path = P2K_SDK_ROOT / 'src'
	P2K_SDK_GCC: Path = P2K_SDK_ROOT / 'gcc'
	P2K_SDK_BUILD: Path = P2K_SDK_ROOT / 'build'
	P2K_SDK_RELEASE: Path = P2K_SDK_ROOT / 'release'

	EP3_BIN_LDR: Path = Path('ep3.bin')
	EP3_BIN_FPA: Path = Path('ep3.fpa')
	EP3_ELF_LDR: Path = Path('ep3.ldr')
	EP3_ELF_LDR_LIB: Path = Path('ep3.so')
	EP3_ELF_LDR_CFG: Path = Path('ep3.ini')
	EP3_ELF_LDR_LOG: Path = Path('ep3.log')
