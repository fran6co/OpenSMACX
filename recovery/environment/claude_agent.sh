#!/bin/sh
# Provider transport helper for recovery/agents.json: run a headless claude
# command and print only the bare JSON object of its final message.
#
# The agent bridge feeds the prompt on stdin and strictly parses stdout as one
# JSON object. Models wrap or narrate their final answer (markdown fences,
# leading prose), and gateways occasionally answer an empty final message
# under pacing. This script strips transport formatting and retries bounded
# times on an unusable answer; provider exit failures pass through unchanged.
# No verdict interpretation happens here — schema validation stays with the
# framework.

set -u

CLAUDE_CONFIG_DIR="${CLAUDE_CONFIG_DIR:-$HOME/.claude-z}"
export CLAUDE_CONFIG_DIR

ATTEMPTS="${ATTEMPTS:-3}"
BACKOFF="${BACKOFF:-120}"

prompt=$(mktemp "${TMPDIR:-/tmp}/claude-agent-prompt.XXXXXX")
out=$(mktemp "${TMPDIR:-/tmp}/claude-agent-out.XXXXXX")
err=$(mktemp "${TMPDIR:-/tmp}/claude-agent-err.XXXXXX")
trap 'rm -f "$prompt" "$out" "$err"' EXIT
cat >"$prompt"

# Transport observability: one line per call (size, mode, model, role flags).
printf '%s %s bytes\n' "$(date -u +%FT%TZ)" "$(wc -c <"$prompt")" \
    >>"${TMPDIR:-/tmp}/claude-agent-calls.log"
printf '  argv: %s\n' "$*" >>"${TMPDIR:-/tmp}/claude-agent-calls.log"

extract() {
    python3 - "$1" <<'PY'
import json
import re
import sys

raw = open(sys.argv[1], encoding="utf-8", errors="replace").read()
text = raw.strip()
if text.startswith("```"):
    for fence in re.finditer(r"```[a-zA-Z0-9_-]*\n(.*?)```", text, re.DOTALL):
        try:
            value = json.loads(fence.group(1))
        except ValueError:
            continue
        if isinstance(value, dict):
            print(json.dumps(value))
            sys.exit(0)
decoder = json.JSONDecoder()
last = None
for match in re.finditer(r"\{", text):
    try:
        value, _ = decoder.raw_decode(text[match.start():])
    except ValueError:
        continue
    if isinstance(value, dict):
        last = value
if last is None:
    sys.exit(1)
print(json.dumps(last))
PY
}

attempt=1
while :; do
    status=0
    claude "$@" <"$prompt" >"$out" 2>"$err" || status=$?
    if [ "$status" -ne 0 ]; then
        cat "$out"
        cat "$err" >&2
        exit "$status"
    fi
    if extract "$out"; then
        exit 0
    fi
    if [ "$attempt" -ge "$ATTEMPTS" ]; then
        echo "claude agent answer contained no JSON object after $ATTEMPTS attempts" >&2
        cat "$err" >&2
        exit 1
    fi
    echo "claude agent answer unusable (attempt $attempt/$ATTEMPTS); retrying in ${BACKOFF}s" >&2
    sleep "$BACKOFF"
    attempt=$((attempt + 1))
done
