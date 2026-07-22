#!/usr/bin/env python3

import unittest

import classify_recovery_shapes as shapes


def wrap(body: str) -> str:
    return "void FUN_00400000(int param_1)\n\n{\n" + body + "\n}\n"


class CountCallsTest(unittest.TestCase):
    """Named imports are calls too. Counting only Ghidra's synthetic FUN_
    names reported Buffer::change_color - which calls IntersectRect - as a
    call-free loop of stores, and inflated every store category by roughly a
    third on this tool's first run."""

    def test_counts_named_imports(self):
        self.assertEqual(1, shapes.count_calls("  BVar3 = IntersectRect(a, b, c);"))
        self.assertEqual(1, shapes.count_calls("  sVar1 = _strlen(param_1);"))

    def test_counts_synthetic_and_indirect_calls(self):
        self.assertEqual(1, shapes.count_calls("  FUN_005de8f0(x);"))
        self.assertEqual(1, shapes.count_calls("  (**(code **)(*p + 0x44))(p, q);"))

    def test_ignores_control_keywords(self):
        self.assertEqual(0, shapes.count_calls("  if (param_1 == 0) {"))
        self.assertEqual(0, shapes.count_calls("  while (iVar2 != 0) {"))
        self.assertEqual(0, shapes.count_calls("  for (i = 0; i < 8; i++) {"))


class ClassifyTest(unittest.TestCase):
    def shape(self, body: str) -> str:
        return shapes.classify(wrap(body))[0]

    def test_empty_body(self):
        self.assertEqual("empty", self.shape("  return;"))

    def test_constant_return(self):
        self.assertEqual("constant-return", self.shape("  return 3;"))

    def test_field_store(self):
        self.assertEqual("field-store", self.shape(
            "  *(int *)(param_1 + 0x10) = 0;\n"
            "  *(int *)(param_1 + 0x14) = 1;"))

    def test_guarded_store(self):
        self.assertEqual("guarded-store", self.shape(
            "  if (param_1 == 0) {\n"
            "    return;\n"
            "  }\n"
            "  *(int *)(param_1 + 0x10) = 0;"))

    def test_single_call_is_a_delegate(self):
        self.assertEqual("delegate", self.shape("  FUN_00590c20();"))

    def test_loop_of_stores(self):
        self.assertEqual("loop-store", self.shape(
            "  do {\n"
            "    *puVar1 = 0;\n"
            "    puVar1 = puVar1 + 0x67;\n"
            "    iVar2 = iVar2 + -1;\n"
            "  } while (iVar2 != 0);"))

    def test_call_bearing_loop_is_complex(self):
        # A loop that calls anything is not mechanically generatable, and is
        # deliberately pushed to manual work rather than guessed at.
        self.assertEqual("complex", self.shape(
            "  do {\n"
            "    FUN_00400100(puVar1);\n"
            "    iVar2 = iVar2 + -1;\n"
            "  } while (iVar2 != 0);"))

    def test_import_bearing_body_is_not_a_store_shape(self):
        # The regression the call-detection fix addresses.
        self.assertEqual("complex", self.shape(
            "  local_18.right = *(int *)(param_1 + 0x80);\n"
            "  BVar3 = IntersectRect(&local_18, &local_20, &local_18);\n"
            "  local_18.left = 0;\n"
            "  local_18.top = 0;\n"
            "  local_20 = param_1;"))


class ConservatismTest(unittest.TestCase):
    """Validated against eight functions recovered by hand in this repository.
    Five classified exactly; the other three - a two-call measure-and-forward,
    a counted loop ending in a tail call, and a self-recursive walk - were
    reported `complex` when they were in fact recoverable.

    Every disagreement was in that direction, and that is the direction this
    tool must err in: a missed candidate costs an opportunity, while a body
    wrongly called simple would feed a generated implementation into the
    pipeline on the strength of a decompilation that has repeatedly been wrong
    about receivers and stack layout.
    """

    def shape(self, body: str) -> str:
        return shapes.classify(wrap(body))[0]

    def test_two_call_delegate_is_reported_complex(self):
        self.assertEqual("complex", self.shape(
            "  if (param_1 == 0) {\n"
            "    return 0;\n"
            "  }\n"
            "  sVar1 = _strlen(param_1);\n"
            "  uVar2 = FUN_005dc7c0(param_1, sVar1);\n"
            "  return uVar2;"))

    def test_loop_then_call_is_reported_complex(self):
        self.assertEqual("complex", self.shape(
            "  do {\n"
            "    *puVar1 = 0;\n"
            "    puVar1 = puVar1 + 0x67;\n"
            "    iVar2 = iVar2 + -1;\n"
            "  } while (iVar2 != 0);\n"
            "  *(int *)(param_1 + 0x768) = 0;\n"
            "  FUN_0062e010();"))


if __name__ == "__main__":
    unittest.main()
