import ctypes
import os

try:
    dll_path = os.path.abspath("server/game_logic.dll")
    print(f"Loading {dll_path}")
    lib = ctypes.CDLL(dll_path)
    print("Loaded!")
    print(f"Result: {lib.add(1, 2)}")
except Exception as e:
    print(f"Error: {e}")
