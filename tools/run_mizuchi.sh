#!/usr/bin/env bash
# Run the full Mizuchi pipeline against this repository:
# the Atlas GUI (http://localhost:3000) and the matching pipeline together.
#
# Usage:
#   tools/run_mizuchi.sh [--personal] [--no-atlas]
#
#   --personal   use the personal Claude account
#                (CLAUDE_CONFIG_DIR=~/.claude-personal unless already set)
#   --no-atlas   run only the pipeline, without the Atlas GUI
#
# Environment:
#   MIZUCHI_DIR  mizuchi checkout (default: ~/code/mizuchi)
#   PORT         Atlas port (default: 3000)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MIZUCHI_DIR="${MIZUCHI_DIR:-$HOME/code/mizuchi}"
PORT="${PORT:-3000}"

PERSONAL=0
ATLAS=1
for arg in "$@"; do
    case "$arg" in
        --personal) PERSONAL=1 ;;
        --no-atlas) ATLAS=0 ;;
        *) echo "unknown option: $arg" >&2; exit 2 ;;
    esac
done

# Mizuchi needs Node >= 22; fall back to nvm if the default node is older.
if ! node -e 'process.exit(process.versions.node.split(".")[0] >= 22 ? 0 : 1)' 2>/dev/null; then
    if [ -s "${NVM_DIR:-$HOME/.nvm}/nvm.sh" ]; then
        . "${NVM_DIR:-$HOME/.nvm}/nvm.sh"
        nvm use 22 >/dev/null
    else
        echo "Node >= 22 required (current: $(node --version 2>/dev/null || echo none))" >&2
        exit 1
    fi
fi

if [ ! -d "$MIZUCHI_DIR" ]; then
    echo "mizuchi checkout not found at $MIZUCHI_DIR (set MIZUCHI_DIR)" >&2
    exit 1
fi

# Rebuild mizuchi (fast, idempotent) so dist/ matches its sources.
npm --prefix "$MIZUCHI_DIR" run build >/dev/null

if [ "$PERSONAL" = 1 ]; then
    export CLAUDE_CONFIG_DIR="${CLAUDE_CONFIG_DIR:-$HOME/.claude-personal}"
fi

CONFIG="$REPO_ROOT/mizuchi.yaml"
ATLAS_PID=""
cleanup() {
    if [ -n "$ATLAS_PID" ]; then
        kill "$ATLAS_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

if [ "$ATLAS" = 1 ]; then
    node "$MIZUCHI_DIR/dist/cli.js" atlas -c "$CONFIG" -p "$PORT" &
    ATLAS_PID=$!
    echo "Atlas GUI: http://localhost:$PORT"
fi

node "$MIZUCHI_DIR/dist/cli.js" run -c "$CONFIG"
