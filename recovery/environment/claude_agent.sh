#!/bin/sh
# Provider transport helper for recovery/agents.json: run a headless claude
# command and print only the bare JSON object of its final message.
#
# The agent bridge feeds the prompt on stdin and strictly parses stdout as one
# JSON object. Models wrap or narrate their final answer (markdown fences,
# leading prose); this script strips transport formatting only — the bridge's
# schema validation and every verdict remain with the framework. Claude's exit
# status, stdout and stderr are passed through unchanged on failure.

set -u

CLAUDE_CONFIG_DIR="${CLAUDE_CONFIG_DIR:-$HOME/.claude-z}"
export CLAUDE_CONFIG_DIR

out=$(mktemp "${TMPDIR:-/tmp}/claude-agent-out.XXXXXX")
err=$(mktemp "${TMPDIR:-/tmp}/claude-agent-err.XXXXXX")
trap 'rm -f "$out" "$err"' EXIT

status=0
claude "$@" >"$out" 2>"$err" || status=$?
if [ "$status" -ne 0 ]; then
    cat "$out"
    cat "$err" >&2
    exit "$status"
fi

python3 - "$out" <<'PY'
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
    sys.stderr.write("claude agent answer contains no JSON object\n")
    sys.exit(1)
print(json.dumps(last))
PY
