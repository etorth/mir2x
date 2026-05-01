#!/bin/bash
# mir2x client launcher (macOS)
# usage:
#   ./run-client.sh                              -> connect to 127.0.0.1
#   ./run-client.sh <server-ip>                  -> connect to <server-ip>
#   ./run-client.sh <server-ip> [extra args...]  -> pass extra args to client

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLIENT_BIN="$SCRIPT_DIR/client/src/client"
RES_DIR="$SCRIPT_DIR/3rdparty/mir2x_data/res"

if [ ! -x "$CLIENT_BIN" ]; then
    echo "client binary not found at $CLIENT_BIN" >&2
    exit 1
fi
if [ ! -d "$RES_DIR" ]; then
    echo "res directory not found at $RES_DIR" >&2
    exit 1
fi

SERVER_IP="${1:-127.0.0.1}"
shift || true

cd "$(dirname "$RES_DIR")"
exec "$CLIENT_BIN" --server-ip="$SERVER_IP" "$@"
