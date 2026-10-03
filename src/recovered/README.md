# Remaining recovery seeds

This directory and `src/unrecovered/` contain historical recovery inputs, not
application build inputs. Their names do not establish current correctness or
ownership. Some seeds contain historical matching obligations; their raw-address
or opaque-layout scaffolding must not become the application implementation.

Recover genuine C++ in its owning translation unit using the installed
`match_recovery` workflow described in `recovery/WORKFLOW.md`.

Delete a superseded seed after checking that the compiled owner preserves its
original identity, complete spans, historical claims and useful discoveries.
Transfer unique findings before deletion; Git retains the old source provenance.
An empty body, executable-address forwarder or incomplete translation is not a
replacement. Keep seeds that still contain unfinished or unmigrated work.

Use `uv run recover inventory` for current coverage and
`uv run recover check-claims --summary` for historical identity continuity.
Neither source presence nor claim continuity proves behavioral acceptance.
