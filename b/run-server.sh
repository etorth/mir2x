#!/bin/bash
# mir2x server launcher (macOS)
# usage:
#   ./run-server.sh                              -> default --auto-launch=1, client-port=7100
#   ./run-server.sh [extra args...]              -> pass extra args to server
#
# notes:
# - macOS reserves port 7000 for AirPlay Receiver, so we default to 7100.
#   if you must use 7000, disable: System Settings -> General -> AirDrop & Handoff -> AirPlay Receiver
# - server is FLTK GUI; it will pop windows. with --auto-launch=1 it starts listening immediately.
# - sqlite db file 'mir2x.db3' is created in the cwd on first launch.
#   default accounts: test/123456, good/123456, id_1..id_4/123456

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_BIN="$SCRIPT_DIR/server/src/server"
RUN_DIR="$SCRIPT_DIR/server/run"
RES_DIR="$SCRIPT_DIR/3rdparty/mir2x_data/res"
SCRIPT_SRC_DIR="$SCRIPT_DIR/../server/script"

if [ ! -x "$SERVER_BIN" ]; then
    echo "server binary not found at $SERVER_BIN" >&2
    exit 1
fi
if [ ! -d "$RES_DIR/map" ]; then
    echo "map data dir not found at $RES_DIR/map" >&2
    exit 1
fi
if [ ! -d "$SCRIPT_SRC_DIR" ]; then
    echo "server script dir not found at $SCRIPT_SRC_DIR" >&2
    exit 1
fi

mkdir -p "$RUN_DIR"
ln -sfn "$RES_DIR/map" "$RUN_DIR/map"
ln -sfn "$SCRIPT_SRC_DIR" "$RUN_DIR/script"

cd "$RUN_DIR"
exec "$SERVER_BIN" --auto-launch=1 --client-port=7100 "$@"
