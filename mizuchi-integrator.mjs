// Mizuchi integrator for OpenSMACX.
//
// Runs after Mizuchi's objdiff stage reports a match. objdiff compares
// against a SYNTHESISED target object; the ground truth is the pinned
// executable, so this gates the match through tools/byte_match.py and
// integrates only a BYTE_EXACT verdict - the project's recovery workflow
// advances only on BYTE_EXACT.
//
// What it does:
//   1. Rebuilds the full translation unit (scaffolding context + matched
//      code) and stores it at build/byte-match/<hex>/unit.cpp, the work
//      area tools/byte_match_fanout.py --collect re-verifies from.
//   2. Runs byte_match on the unit; refuses to integrate anything below
//      BYTE_EXACT.
//   3. Ratchets the ledger row in the worktree (same fields
//      byte_match_fanout.collect writes, same no-regression rule) so the
//      commit carries the evidence.

import { execFileSync } from 'node:child_process';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';

const PY = path.join('.opensmacx', 'venv', 'bin', 'python');

// Splicing the matched body into src/ is the step that makes a match a
// recovery. Set MIZUCHI_WRITE_BACK=0 to gate on byte_match and ratchet the
// ledger without editing the tree - useful when sweeping a batch to find out
// what matches before deciding what to land.
const WRITE_BACK_TO_SRC = process.env.MIZUCHI_WRITE_BACK !== '0';

// Runs inside the worktree; imports the project's own tools so the verdict
// and the ledger update use the canonical logic, not a copy of it.
const GATE_SCRIPT = `
import json
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, 'tools')

import pefile

import byte_match
import byte_match_fanout as fanout

address = int(sys.argv[1], 16)
unit_path = Path(sys.argv[2])

reason = byte_match.available()
if reason:
    print(json.dumps({"tier": "SKIP", "refusal_reason": reason}))
    sys.exit(3)

pe = pefile.PE(str(byte_match.DEFAULT_EXE))
catalogue = byte_match.load_rows()
shared = byte_match.shared_span_index(catalogue)
functions = fanout.emit.load_functions()

with tempfile.TemporaryDirectory() as tmp:
    outcome = byte_match.match_function(
        pe, catalogue, shared, address, unit_path.read_text(),
        Path(tmp), byte_match.MEASURED_FLAGS, f"v{address:08x}")

tier = outcome.get("tier", "NO_COMPILE")
key = f"0x{address:08X}"
ledger = fanout.read_ledger()
was = ledger.get(key, {}).get("tier", "")
if was in fanout.MATCHED and tier not in fanout.MATCHED:
    print(json.dumps({"tier": tier, "regression_from": was,
                      "note": outcome.get("note", ""),
                      "first_divergence": outcome.get("first_divergence", "")}))
    sys.exit(2)

row = functions.get(address, {})
entry = ledger.setdefault(key, {})
entry.update(
    address=key, name=row.get("name", ""),
    source_location="", generated=0,
    size=row.get("size", ""), tier=tier,
    span_classes=";".join(
        f"{k}={v}" for k, v in (outcome.get("span_classes") or {}).items()),
    original_bytes=outcome.get("original_bytes", ""),
    rebuilt_bytes=outcome.get("rebuilt_bytes", ""),
    original_mnemonics=outcome.get("original_mnemonics", ""),
    rebuilt_mnemonics=outcome.get("rebuilt_mnemonics", ""),
    first_divergence=outcome.get("first_divergence", ""),
    note=outcome.get("note", ""),
    refusal_reason=outcome.get("refusal_reason", ""))
fanout.write_ledger(ledger)

print(json.dumps({"tier": tier, "note": outcome.get("note", ""),
                  "original_bytes": outcome.get("original_bytes", ""),
                  "rebuilt_bytes": outcome.get("rebuilt_bytes", ""),
                  "first_divergence": outcome.get("first_divergence", "")}))
sys.exit(0 if tier in fanout.MATCHED else 1)
`;

function resolveAddress(projectRoot, functionName) {
  const csvPath = path.join(projectRoot, 'docs', 'recovery', 'functions.csv');
  const lines = fs.readFileSync(csvPath, 'utf-8').split('\n');
  const header = lines[0].split(',');
  const addressIdx = header.indexOf('address');
  const nameIdx = header.indexOf('name');
  for (const line of lines.slice(1)) {
    if (!line.includes(functionName)) {
      continue;
    }
    // The name field can contain commas inside quotes only when quoted; the
    // catalog never quotes it, so a plain split is safe here.
    const cols = line.split(',');
    if (cols[nameIdx] === functionName) {
      return parseInt(cols[addressIdx], 16);
    }
  }
  throw new Error(`Function ${functionName} not found in docs/recovery/functions.csv`);
}

export async function integrate({ functionName, generatedCode, worktreePath, projectRoot, helpers }) {
  const address = resolveAddress(projectRoot, functionName);
  const hex = address.toString(16).padStart(8, '0');

  // 1. Rebuild the full translation unit and store it in the canonical work
  //    area so byte_match_fanout --collect re-verifies it like any other unit.
  const context = execFileSync(PY, ['tools/mizuchi_context.py', `0x${hex}`], {
    cwd: projectRoot,
    encoding: 'utf-8',
  });
  const unit = `${context}\n${generatedCode.trim()}\n`;
  const unitDir = path.join(projectRoot, 'build', 'byte-match', hex);
  fs.mkdirSync(unitDir, { recursive: true });
  const unitPath = path.join(unitDir, 'unit.cpp');
  fs.writeFileSync(unitPath, unit);
  helpers.log(`Stored matched unit at build/byte-match/${hex}/unit.cpp`);

  // 2. Worktree setup. Worktrees are sparse: gitignored artifacts and
  //    untracked work-in-progress files are absent.
  //    - .opensmacx holds the pinned executable: symlink it.
  //    - The byte_match toolchain (and possibly the ledger) is untracked
  //      work-in-progress: COPY it. Symlinks would break it, because the
  //      tools compute the repo root from __file__ and would land back in
  //      the main tree, writing the ledger there instead of the worktree.
  //    Everything copied or overwritten here is restored after the gate so
  //    the commit carries ONLY the ratcheted ledger.
  const opensmacxSrc = path.join(projectRoot, '.opensmacx');
  const opensmacxDest = path.join(worktreePath, '.opensmacx');
  fs.rmSync(opensmacxDest, { recursive: true, force: true });
  fs.symlinkSync(opensmacxSrc, opensmacxDest);

  const copiedToolFiles = [];
  const overwrittenToolFiles = [];
  const toolsDest = path.join(worktreePath, 'tools');
  for (const entry of fs.readdirSync(path.join(projectRoot, 'tools'))) {
    if (!entry.endsWith('.py')) {
      continue;
    }
    const src = path.join(projectRoot, 'tools', entry);
    const dest = path.join(toolsDest, entry);
    if (!fs.existsSync(dest)) {
      fs.copyFileSync(src, dest);
      copiedToolFiles.push(dest);
    } else if (!fs.readFileSync(src).equals(fs.readFileSync(dest))) {
      fs.copyFileSync(src, dest);
      overwrittenToolFiles.push(path.join('tools', entry));
    }
  }

  const ledgerRel = path.join('docs', 'recovery', 'byte-match.csv');
  fs.copyFileSync(path.join(projectRoot, ledgerRel), path.join(worktreePath, ledgerRel));

  // 3. Gate through byte_match and ratchet the ledger, in the worktree.
  const gatePath = path.join(os.tmpdir(), `mizuchi-gate-${hex}.py`);
  const bodyPath = path.join(os.tmpdir(), `mizuchi-body-${hex}.cpp`);
  fs.writeFileSync(gatePath, GATE_SCRIPT);
  fs.writeFileSync(bodyPath, `${generatedCode.trim()}\n`);
  let stdout;
  let gateError = null;
  let writeback = null;
  let writebackError = null;
  try {
    stdout = execFileSync(PY, [gatePath, `0x${hex}`, unitPath], {
      cwd: worktreePath,
      encoding: 'utf-8',
    });

    // 4. The recovery only LANDS when the body is in src/. The gate proved
    //    the text is byte-exact but left it in build/, which is gitignored and
    //    wiped by a clean. The writeback splices it into the catalogued span
    //    and re-verifies from src/ - it reverts itself rather than leave a
    //    half-applied edit, so a failure here is reported and does not block
    //    the ledger commit the gate earned.
    if (WRITE_BACK_TO_SRC && JSON.parse(stdout.trim().split('\n').pop()).tier === 'BYTE_EXACT') {
      try {
        const out = execFileSync(
          PY,
          ['tools/mizuchi_writeback.py', `0x${hex}`, bodyPath, '--json'],
          { cwd: worktreePath, encoding: 'utf-8' },
        );
        writeback = JSON.parse(out.trim().split('\n').pop());
      } catch (error) {
        const detail = error.stdout ? error.stdout.toString().trim() : '';
        writebackError = detail || error.stderr?.toString().trim() || error.message;
      }
    }
  } catch (error) {
    gateError = error;
    stdout = error.stdout ? error.stdout.toString() : '';
  } finally {
    fs.rmSync(gatePath, { force: true });
    fs.rmSync(bodyPath, { force: true });

    // Restore the worktree so the commit carries only the ledger: revert
    // overwritten tracked tools, delete copied ones, drop the exe symlink.
    if (overwrittenToolFiles.length > 0) {
      execFileSync('git', ['checkout', '--', ...overwrittenToolFiles], { cwd: worktreePath });
    }
    for (const copied of copiedToolFiles) {
      fs.rmSync(copied, { force: true });
    }
    fs.rmSync(opensmacxDest, { force: true });
  }

  if (gateError) {
    const detail = stdout.trim() || gateError.message;
    throw new Error(`byte_match gate failed for 0x${hex} ${functionName}: ${detail}`);
  }

  const gate = JSON.parse(stdout.trim().split('\n').pop());
  if (gate.tier !== 'BYTE_EXACT') {
    throw new Error(
      `byte_match verdict ${gate.tier} (need BYTE_EXACT) for 0x${hex} ${functionName}: ` +
        `${gate.note || gate.refusal_reason || ''} ` +
        `first divergence at #${gate.first_divergence ?? '?'}`,
    );
  }

  helpers.log(
    `byte_match: BYTE_EXACT (${gate.original_bytes} B original, ${gate.rebuilt_bytes} B rebuilt)`,
  );

  const filesModified = [path.join(worktreePath, 'docs', 'recovery', 'byte-match.csv')];
  let summary = `byte_match BYTE_EXACT for 0x${hex} ${functionName}; ledger ratcheted`;

  if (writebackError) {
    // Reported, not thrown. The writeback restores every file it touched
    // before failing, so the tree is clean and the BYTE_EXACT verdict is
    // still worth banking - it is evidence that survives the next attempt.
    helpers.log(`writeback REFUSED, src/ unchanged: ${writebackError}`);
    summary += '; src/ writeback refused';
  } else if (writeback) {
    for (const file of writeback.files_modified) {
      filesModified.push(path.join(worktreePath, file));
    }
    helpers.log(
      `wrote body into ${writeback.source_location} ` +
        `(${writeback.lines_replaced} -> ${writeback.lines_written} lines), ` +
        `re-verified BYTE_EXACT from src/`,
    );
    if (writeback.line_delta !== 0) {
      // The catalogues this tool patches are the ones Mizuchi itself reads.
      // docs/recovery/summary.json and the metadata caches are regenerated
      // from the IDB, and saying so beats letting them drift unremarked.
      helpers.log(
        `catalogued line numbers below the edit shifted by ${writeback.line_delta}; ` +
          `run tools/verify_recovery_metadata.py to regenerate the derived catalogues`,
      );
    }
    summary += `; body written into ${writeback.source_location}`;
  } else if (!WRITE_BACK_TO_SRC) {
    helpers.log('MIZUCHI_WRITE_BACK=0: src/ left unchanged');
  }

  return { filesModified, summary };
}
