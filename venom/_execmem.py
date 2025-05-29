import ctypes
import mmap
import platform
import typing

from ctypes import wintypes

class ExecMemory():

    def __init__(self, size: int) -> None:
        self._size = size
        self._ptr = None
        self._platform = platform.system()

        if self._platform == "Windows":
            self._alloc_windows()
        else:
            self._alloc_unix()

    def _alloc_windows(self) -> None:
        MEM_COMMIT = 0x1000
        PAGE_READWRITE = 0x04

        ctypes.windll.kernel32.VirtualAlloc.restype = ctypes.c_void_p
        ctypes.windll.kernel32.VirtualAlloc.argtypes = (
            wintypes.LPVOID, # lpAddress
            ctypes.c_size_t, # dwSize
            wintypes.DWORD,  # flAllocationType
            wintypes.DWORD,  # flProtect
        )

        self._ptr = ctypes.windll.kernel32.VirtualAlloc(None, self._size, MEM_COMMIT, PAGE_READWRITE)

        if self._ptr is None:
            raise MemoryError("VirtualAlloc failed")

        self._buffer = (ctypes.c_char * self._size).from_address(self._ptr)

    def _alloc_unix(self) -> None:
        self._mmap_obj = mmap.mmap(-1, 
                                   self._size,
                                   prot=mmap.PROT_READ | mmap.PROT_WRITE,
                                   flags=mmap.MAP_PRIVATE  | mmap.MAP_ANONYMOUS)

        self._buffer = self._mmap_obj
        self._ptr = ctypes.addressof(ctypes.c_char.from_buffer(self._mmap_obj))

    def write(self, code: bytes) -> None:
        if len(code) > self._size:
            raise ValueError("Code too large for buffer")

        ctypes.memmove(self._ptr, code, len(code))

        self._protect_exec()

    def _protect_exec(self) -> None:
        if self._platform == "Windows":
            PAGE_EXECUTE_READ = 0x20
            old_protect = ctypes.c_ulong()

            ctypes.windll.kernel32.VirtualProtect.restype = wintypes.BOOL
            ctypes.windll.kernel32.VirtualProtect.argtypes = (
                wintypes.LPVOID, # lpAddress
                ctypes.c_size_t, # dwSize
                wintypes.DWORD,  # flNewProtect
                wintypes.PDWORD, # lpflOldProtect
            )

            res = ctypes.windll.kernel32.VirtualProtect(wintypes.LPVOID(self._ptr),
                                                        ctypes.c_size_t(self._size),
                                                        PAGE_EXECUTE_READ,
                                                        ctypes.byref(old_protect))

            if res == 0:
                raise OSError("VirtualProtect failed")
        else:
            PROT_READ = 1
            PROT_EXEC = 4

            libc = ctypes.CDLL(None)

            res = libc.mprotect(ctypes.c_void_p(self._ptr),
                                ctypes.c_size_t(self._size),
                                PROT_READ | PROT_EXEC)

            if res != 0:
                raise OSError("mprotect failed")

    def address(self) -> typing.Optional[ctypes.c_void_p]:
        return self._ptr

    def free(self) -> None:
        if self._platform == "Windows":
            ctypes.windll.kernel32.VirtualFree.restype = wintypes.BOOL
            ctypes.windll.kernel32.VirtualFree.argtypes = (
                wintypes.LPVOID, # lpAddress
                ctypes.c_size_t, # dwSize
                wintypes.DWORD,  # dwFreeType
            )

            MEM_DECOMMIT = 0x4000
            MEM_RELEASE = 0x8000
            
            if self._ptr:
                ctypes.windll.kernel32.VirtualFree(self._ptr, 0, MEM_RELEASE)
        else:
            if hasattr(self, "_mmap_obj"):
                self._mmap_obj.close()

        self._ptr = None

    def __del__(self):
        self.free()
