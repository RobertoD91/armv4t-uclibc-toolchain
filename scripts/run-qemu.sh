#!/usr/bin/env bash
# Esegue un binario ARMv4T sotto qemu emulando una CPU ARMv4T (come l'ARM720T ).
# Se gira qui, gira sul device (stesso set di istruzioni).
#   run-qemu.sh <binario> [args...]
set -e
BIN=${1:?"uso: run-qemu.sh <binario> [args...]"}; shift || true
Q=$(command -v qemu-arm-static || command -v qemu-arm) || {
  echo "serve qemu-user-static:  sudo apt install qemu-user-static"; exit 1; }
echo "# $Q -cpu ti925t (ARMv4T)  $BIN $*"
exec "$Q" -cpu ti925t "$BIN" "$@"
