import pytest
import subprocess

from match_recovery.source_replica import source_replica, source_tree, replica_snapshot


@pytest.fixture
def tree(tmp_path):
    (tmp_path / "code/sub").mkdir(parents=True)
    (tmp_path / "code/unit.cc").write_bytes(b"original\r\n\xff")
    (tmp_path / "code/sub/include_without_suffix").write_bytes(b"header")
    return tmp_path


def test_replica_copies_all_inputs_without_linking_or_editing_originals(tree):
    (tree / "code/empty").mkdir()
    with source_replica(tree, ("code", "code/sub"), {"code/unit.cc": b"candidate"}) as replica:
        assert not replica.is_relative_to(tree)
        assert (replica / "code/unit.cc").read_bytes() == b"candidate"
        assert (replica / "code/empty").is_dir()
        header = replica / "code/sub/include_without_suffix"
        assert header.read_bytes() == b"header"
        assert header.stat().st_ino != (tree / "code/sub/include_without_suffix").stat().st_ino
        header.write_bytes(b"private edit")
    assert not replica.exists()
    assert (tree / "code/unit.cc").read_bytes() == b"original\r\n\xff"
    assert (tree / "code/sub/include_without_suffix").read_bytes() == b"header"


@pytest.mark.parametrize("damage", ["changed", "added", "removed"])
def test_replica_refuses_changed_dependencies_without_restoring_them(tree, damage):
    with pytest.raises(ValueError, match="inputs changed"):
        with source_replica(tree, ("code",), {}) as replica:
            header = tree / "code/sub/include_without_suffix"
            if damage == "changed":
                header.write_bytes(b"newer content")
            elif damage == "removed":
                header.unlink()
            else:
                (tree / "code/new.h").write_text("#error damaged")
    assert not replica.exists()
    if damage == "changed":
        assert header.read_bytes() == b"newer content"


def test_exception_cleans_private_tree(tree):
    with pytest.raises(RuntimeError, match="compiler failed"):
        with source_replica(tree, ("code",), {}) as replica:
            raise RuntimeError("compiler failed")
    assert not replica.exists()


@pytest.mark.parametrize("path", ["code/link.h", "code/sub/link", "alias"])
def test_symlink_files_directories_and_roots_are_refused(tree, path):
    (tree / path).symlink_to(tree / "code/sub", target_is_directory=True)
    with pytest.raises(ValueError, match="symlink"):
        with source_replica(tree, ("alias",) if path == "alias" else ("code",), {}):
            pytest.fail("must not materialize ambiguous inputs")


@pytest.mark.parametrize("roots, replacements", [((), {}), (("missing",), {}), ((".",), {}),
                                                (("code",), {"../escape": b"bad"}),
                                                (("code",), {"code/new.cc": b"new"}),
                                                (("code",), {"code/sub": b"not a file"}),
                                                (("code",), {"code/unit.cc": "text"})])
def test_invalid_scopes_and_replacements_are_refused(tree, roots, replacements):
    with pytest.raises(ValueError):
        with source_replica(tree, roots, replacements):
            pytest.fail("invalid replica")


def test_configured_artifacts_are_verified_ignored_and_cleaned(tree):
    subprocess.run(["git", "init", "-q", str(tree)], check=True)
    (tree / ".gitignore").write_text("private/\n")
    with source_replica(tree, ("code",), {}, artifacts="private/variants") as replica:
        assert replica.is_relative_to(tree / "private/variants")
    assert not replica.exists() and not list((tree / "private/variants").iterdir())


@pytest.mark.parametrize("damage", ["unignored", "tracked", "source-root", "symlink"])
def test_unsafe_artifact_destinations_are_refused(tree, damage):
    subprocess.run(["git", "init", "-q", str(tree)], check=True)
    (tree / "private").mkdir()
    if damage != "unignored":
        (tree / ".gitignore").write_text("private/\ncode/cache/\n")
    if damage == "tracked":
        (tree / "private/tracked").write_text("keep")
        subprocess.run(["git", "add", "-f", "private/tracked"], cwd=tree, check=True)
    if damage == "symlink":
        (tree / "private/link").symlink_to(tree / "private", target_is_directory=True)
    location = "code/cache" if damage == "source-root" else "private/link" if damage == "symlink" else "private"
    with pytest.raises(ValueError):
        with source_replica(tree, ("code",), {}, artifacts=location):
            pytest.fail("unsafe destination")


def test_complete_tree_materializes_historical_files_without_current_fallback(tree):
    before = replica_snapshot(tree, ("code",))
    files = {"code/old/unit.cc": b"historical\r\n\xff", "code/old/header": b"original header"}
    with source_tree(tree, ("code",), files) as replica:
        assert {name: data for name, data in replica_snapshot(replica, ("code",)).items()
                if data is not None} == files
        assert not (replica / "code/unit.cc").exists()
        assert not (replica / "code/sub/include_without_suffix").exists()
    assert not replica.exists()
    assert replica_snapshot(tree, ("code",)) == before


@pytest.mark.parametrize("files", [{"other/header": b"outside"}, {"code": b"root"},
                                 {"code/old": b"file", "code/old/header": b"child"},
                                 {"code/../outside": b"escape"}, {"code/header": "text"}])
def test_invalid_complete_trees_are_refused(tree, files):
    with pytest.raises(ValueError):
        with source_tree(tree, ("code",), files):
            pytest.fail("invalid complete tree")


@pytest.mark.parametrize("damage", ["added", "removed", "changed"])
def test_historical_tree_detects_newer_product_work_without_reverting(tree, damage):
    with pytest.raises(ValueError, match="inputs changed"):
        with source_tree(tree, ("code",), {"code/old.cc": b"old"}) as replica:
            path = tree / "code/unit.cc"
            if damage == "added": (tree / "code/new.h").write_bytes(b"newer")
            elif damage == "removed": path.unlink()
            else: path.write_bytes(b"newer")
    assert not replica.exists()
    if damage == "changed": assert path.read_bytes() == b"newer"
