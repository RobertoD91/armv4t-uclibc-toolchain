#!/usr/bin/env bash
# Compila un sorgente C per l'dispositivo ARMv4T (ARMv4T, statico).
#   build.sh <sorgente.c> [output] [--bare]
#   --bare = freestanding (no libc, syscall diretti), per utility minime.
# Il toolchain buildroot deve essere nel PATH (output/host/bin) o dentro l'immagine docker
# `armv4t-uclibc`. Override: GCC=/percorso/arm-linux-gcc build.sh ...
set -e
SRC=${1:?"uso: build.sh <sorgente.c> [output] [--bare]"}
OUT=${2:-$(basename "${SRC%.c}")}
: "${GCC:=arm-linux-gcc}"
command -v "$GCC" >/dev/null || { echo "toolchain '$GCC' non nel PATH — vedi ../README.md (docker build -t armv4t-uclibc .)"; exit 1; }
COMMON="-O2 -march=armv4t -marm -mfloat-abi=soft -static"
case " $* " in
  *" --bare "*) "$GCC" $COMMON -nostdlib -o "$OUT" "$SRC" ;;
  *)            "$GCC" $COMMON          -o "$OUT" "$SRC" ;;
esac
file "$OUT"
echo "ok: $OUT (ARMv4T statico) — test: scripts/run-qemu.sh $OUT"
