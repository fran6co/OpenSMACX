"""Real synthetic build retention and native capability lifetime controls."""

import hashlib

import pytest

from match_recovery.application_context import application_build
from match_recovery.driver import working_state
from test_application_context import GRAPH, configured, files, project


@pytest.mark.parametrize("interrupted", [False, True])
def test_retained_replica_preserves_exact_outputs_but_expires_authority(configured, interrupted):
    root = configured
    before = working_state(root)
    current = files(root)
    application = None
    try:
        with application_build(root, ("code",), current,
                               versioned_files={GRAPH: (root / GRAPH).read_bytes()},
                               retain=True) as application:
            assert application.root != root
            assert application.directory.is_relative_to(application.root)
            assert files(application.root) == current
            application.check_current()
            report = application.report
            if interrupted:
                raise RuntimeError("interrupted after successful build")
    except RuntimeError as error:
        assert interrupted and str(error) == "interrupted after successful build"
    assert application is not None and application.root.is_dir()
    for row in report["outputs"]:
        raw = (application.directory / row["path"]).read_bytes()
        assert hashlib.sha256(raw).hexdigest() == row["sha256"]
    with pytest.raises(ValueError, match="expired|closed|active|ended"):
        application.check_current()
    assert working_state(root) == before


@pytest.mark.parametrize("retain", [1, None, "yes"])
def test_application_retention_is_not_truthy_coercion(configured, retain):
    with pytest.raises(ValueError, match="retention.*boolean"):
        with application_build(configured, ("code",), files(configured),
                               versioned_files={GRAPH: (configured / GRAPH).read_bytes()},
                               retain=retain):
            pytest.fail("unsupported retention value")
