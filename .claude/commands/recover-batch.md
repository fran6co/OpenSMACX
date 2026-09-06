---
description: Coordinate ownership-scoped recovery tasks using the match_recovery workflow
---

Read `AGENTS.md` and `recovery/WORKFLOW.md`. `$ARGUMENTS` may name a task or
ownership group. If none is supplied, inspect `uv run recover partition` and
choose a coherent application ownership group from evidence. Do not dispatch
address-only lists or run a separate class workflow.

Follow the workflow's task authorization, isolated worktree setup, context,
discovery expansion, independent review and restoration sequence. Worker and
reviewer guidance is in `.claude/agents/recovery-worker.md` and
`.claude/agents/recovery-reviewer.md`. Use configured executable adapters when
invoking `recover run`; those Markdown files do not implement the JSON protocol.

One ownership/header conflict domain has one active writer. Do not infer a fleet
size from obsolete Wine throughput measurements. The coordinator owns Git and
integration; workers do not auto-fast-forward, collect, commit or reap worktrees.

Report current checks and their coverage separately from source progress.
Preserve justified structural edits and restoration obligations on a mismatch.
This project's supplied migration and full acceptance adapters remain incomplete;
never call a parked migration, scoped pass or “0 REGRESSED” a green acceptance.
