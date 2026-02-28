from pathlib import Path
from dataclasses import dataclass

P2K_SDK_ROOT: Path = Path(__file__).resolve().parent.parent

P2K_SDK_BUILD = P2K_SDK_ROOT / 'build'
P2K_SDK_RES = P2K_SDK_ROOT / 'res'
P2K_SDK_INC = P2K_SDK_ROOT / 'inc'
P2K_SDK_SRC = P2K_SDK_ROOT / 'src'
P2K_SDK_TOOL = P2K_SDK_ROOT / 'tool'
P2K_SDK_GCC = P2K_SDK_TOOL / 'gcc'

@dataclass
class Toolchain:
	gcc: Path

@dataclass
class Flags:
	cpu: list[str]
	gen: list[str]
	build: list[str]

@dataclass
class Addresses:
	register: int
	inject: int

@dataclass
class Recipe:
	toolchain: Toolchain
	flags: Flags
	addresses: Addresses
