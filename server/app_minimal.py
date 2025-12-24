import ctypes
import os

print("__file__:", __file__)
dll_path = os.path.abspath(os.path.join(os.path.dirname(__file__), "game_logic.dll"))
print(f"Loading {dll_path}")
try:
    ctypes.CDLL(dll_path)
    print("Success")
except Exception as e:
    print(f"Error: {e}")
