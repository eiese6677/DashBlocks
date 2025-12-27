#!/usr/bin/env bash
set -euo pipefail

SRC="server/game_logic.cpp"
OUTDIR="server"
TMP="$OUTDIR/game_logic.build.cpp"

if [ ! -f "$SRC" ]; then
  echo "Source file not found: $SRC" >&2
  exit 1
fi

echo "Creating temp copy without __declspec(dllexport)"
sed 's/__declspec(dllexport)//g' "$SRC" > "$TMP"

if ! command -v g++ >/dev/null 2>&1; then
  echo "g++ not found in PATH" >&2
  rm -f "$TMP"
  exit 2
fi

OUT="$OUTDIR/game_logic.so"
echo "Compiling to $OUT"
g++ -O2 -fPIC -shared -o "$OUT" "$TMP"

echo "Build succeeded: $OUT"
rm -f "$TMP"

exit 0
