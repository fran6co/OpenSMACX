#!/usr/bin/env python3

import unittest
from unittest import mock

import derive_class_layout as layout


class SettledTest(unittest.TestCase):
    """What may and may not settle a size.

    Every case here is a bug this tool actually had. Measuring Font by the gap
    to the next global gave 0x718 against its true 0x28, because globals with
    no dynamic initialiser sit in that gap unseen - so an upper bound must
    never be mistaken for an answer.
    """

    def make(self, *evidence):
        return layout.Layout("Example", ctor=0x00400000, evidence=list(evidence))

    def test_exact_evidence_settles(self):
        subject = self.make(layout.Evidence("virtual", 0x100, ""))
        self.assertEqual(0x100, subject.settled())

    def test_an_upper_bound_alone_settles_nothing(self):
        subject = self.make(layout.Evidence("global", 0x718, "", exact=False))
        self.assertIsNone(subject.settled())

    def test_an_upper_bound_does_not_contradict_a_smaller_exact_size(self):
        subject = self.make(
            layout.Evidence("virtual", 0x247A8, ""),
            layout.Evidence("global", 0x247D8, "", exact=False))
        self.assertEqual(0x247A8, subject.settled())

    def test_an_exact_size_exceeding_its_bound_is_refused(self):
        # Being larger than the space the object provably occupies means one
        # of the two readings is wrong; answering anyway would pick blindly.
        subject = self.make(
            layout.Evidence("virtual", 0x900, ""),
            layout.Evidence("global", 0x400, "", exact=False))
        self.assertIsNone(subject.settled())

    def test_disagreeing_exact_evidence_is_refused(self):
        subject = self.make(layout.Evidence("virtual", 0x100, ""),
                            layout.Evidence("allocation", 0x200, ""))
        self.assertIsNone(subject.settled())

    def test_no_evidence_settles_nothing(self):
        self.assertIsNone(self.make().settled())


class OwningFunctionTest(unittest.TestCase):
    TABLE = {
        0x00400000: {"name": "?free_function@@YAXXZ", "size": "64"},
        0x00400040: {"name": "??0Popup@@QAE@XZ", "size": "32"},
    }

    def test_locates_the_containing_function(self):
        found = layout.owning_function(self.TABLE, 0x00400050)
        self.assertEqual("??0Popup@@QAE@XZ", found["name"])

    def test_returns_nothing_for_an_address_past_every_function(self):
        # Falling in a gap must not silently attribute the site to whichever
        # function happens to start below it.
        self.assertIsNone(layout.owning_function(self.TABLE, 0x00400100))


class AllocationEvidenceTest(unittest.TestCase):
    """Allocation adjacency is only this class's size when the pointer being
    constructed is the allocation itself."""

    CTOR = 0x006051D0

    def run_against(self, owner_name, instructions):
        image = mock.Mock()
        table = {0x00404000: {"name": owner_name, "size": "256"}}

        def disasm(address, length):
            return instructions
        image.disasm.side_effect = disasm
        image.code_start = 0x00401000
        with mock.patch.object(layout, "scan_callers", return_value=[0x004040F0]):
            return layout.allocation_size(image, self.CTOR, table)

    def instruction(self, mnemonic, op_str, immediate=None):
        item = mock.Mock()
        item.mnemonic, item.op_str = mnemonic, op_str
        if immediate is None:
            item.operands = []
        else:
            operand = mock.Mock()
            operand.type = layout.X86_OP_IMM
            operand.imm = immediate
            item.operands = [operand]
        return item

    def allocation_then_construct(self, size):
        return [self.instruction("push", str(size), size),
                self.instruction("call", "operator new", layout.OPERATOR_NEW)]

    def test_accepts_a_direct_allocation_in_an_ordinary_function(self):
        found = self.run_against("?make_thing@@YAPAXXZ",
                                 self.allocation_then_construct(0x214C))
        self.assertIsNotNone(found)
        self.assertEqual(0x214C, found.size)

    def test_rejects_a_site_inside_another_constructor(self):
        # Scroll's every call site sits inside Popup, AlphaSave, or BaseWin,
        # where the adjacent operator new(0x537C) is the enclosing object's.
        self.assertIsNone(
            self.run_against("??0Popup@@QAE@XZ",
                             self.allocation_then_construct(0x537C)))

    def test_rejects_a_site_inside_an_allocator(self):
        self.assertIsNone(
            self.run_against("?alloc@Popup@@QAAHXZ",
                             self.allocation_then_construct(0x537C)))

    def test_rejects_an_offset_this_pointer(self):
        # `lea ecx, [eax+N]` means a member subobject, not the allocation.
        instructions = self.allocation_then_construct(0x1000) + [
            self.instruction("lea", "ecx, [eax + 0x40]")]
        self.assertIsNone(
            self.run_against("?make_thing@@YAPAXXZ", instructions))

    def test_rejects_an_allocation_another_constructor_claimed_first(self):
        table = {0x00404000: {"name": "?make_thing@@YAPAXXZ", "size": "256"},
                 0x00500000: {"name": "??0Other@@QAE@XZ", "size": "16"}}
        instructions = self.allocation_then_construct(0x1000) + [
            self.instruction("call", "Other", 0x00500000)]
        image = mock.Mock()
        image.disasm.side_effect = lambda address, length: instructions
        image.code_start = 0x00401000
        with mock.patch.object(layout, "scan_callers", return_value=[0x004040F0]):
            self.assertIsNone(layout.allocation_size(image, self.CTOR, table))


class VirtualBaseTest(unittest.TestCase):
    """Offset plus base size is the object's size only when the virtual base
    is genuinely last."""

    VBTABLE = 0x00670584

    def instruction(self, mnemonic, op_str, operands=()):
        item = mock.Mock()
        item.mnemonic, item.op_str, item.address = mnemonic, op_str, 0
        item.operands = list(operands)
        return item

    def memory_write_of(self, immediate):
        destination = mock.Mock()
        destination.type = layout.X86_OP_MEM
        destination.mem.disp = 0
        source = mock.Mock()
        source.type = layout.X86_OP_IMM
        source.imm = immediate
        return self.instruction("mov", f"dword ptr [esi], 0x{immediate:x}",
                                (destination, source))

    def derive(self, trailing):
        image = mock.Mock()
        instructions = [self.memory_write_of(self.VBTABLE),
                        self.instruction("call", "0x5d4cf0", ())]
        instructions += trailing
        image.disasm.side_effect = lambda address, length: list(instructions)
        # vbtable {0, 0x48}: a virtual base at 0x48.
        image.dword.side_effect = lambda address: {
            self.VBTABLE: 0, self.VBTABLE + 4: 0x48}.get(address, 0x99999999)
        table = {0x005D4CF0: {"name": "??0GraphicWin@@QAE@XZ", "size": "230"}}
        call = mock.Mock()
        call.type = layout.X86_OP_IMM
        call.imm = 0x005D4CF0
        instructions[1].operands = [call]
        return layout.virtual_base(image, 0x00609DB0, 299,
                                   {"GraphicWin": 0xA14}, table)

    def test_reports_a_size_when_nothing_follows_the_base(self):
        offset, evidence, base = self.derive([])
        self.assertEqual(0x48, offset)
        self.assertEqual(0xA5C, evidence.size)
        self.assertEqual("GraphicWin", base)

    def test_withholds_a_size_when_a_member_sits_past_the_base(self):
        # ListBox constructs a Dialog at 0xA60, past the 0xA5C the sum gives,
        # so for that class the sum is not a size at all.
        trailing = [self.instruction("lea", "ecx, [esi + 0xa60]")]
        offset, evidence, _ = self.derive(trailing)
        self.assertEqual(0x48, offset)
        self.assertIsNone(evidence)

    def test_ignores_references_inside_the_base(self):
        trailing = [self.instruction("mov", "dword ptr [esi + 0x444], eax")]
        _, evidence, _ = self.derive(trailing)
        self.assertIsNotNone(evidence)


class FirstConstructedTest(unittest.TestCase):
    """Telling a base class from a member.

    Four headers in this repository declared a base that is really a member,
    because the check read only which constructor ran first. A member built at
    a nonzero offset, declared as a base, lands at offset zero and moves every
    field after it.
    """

    TABLE = {0x005D4CF0: {"name": "??0GraphicWin@@QAE@XZ", "size": "230"},
             0x00617F00: {"name": "??0Caviar@@QAE@XZ", "size": "100"}}

    def instruction(self, mnemonic, op_str, immediate=None):
        item = mock.Mock()
        item.mnemonic, item.op_str = mnemonic, op_str
        if immediate is None:
            item.operands = []
        else:
            operand = mock.Mock()
            operand.type = layout.X86_OP_IMM
            operand.imm = immediate
            item.operands = [operand]
        return item

    def run_against(self, instructions):
        image = mock.Mock()
        image.disasm.side_effect = lambda address, length: instructions
        return layout.first_constructed(image, 0x00400000, 200, self.TABLE)

    def test_an_unadjusted_this_is_a_base(self):
        found = self.run_against([
            self.instruction("mov", "esi, ecx"),
            self.instruction("call", "0x5d4cf0", 0x005D4CF0)])
        self.assertEqual(("GraphicWin", 0, False), found)

    def test_an_offset_this_is_a_member(self):
        # StatusWin builds a Caviar at +0x30; it is not a Caviar.
        found = self.run_against([
            self.instruction("lea", "ecx, [esi + 0x30]"),
            self.instruction("call", "0x617f00", 0x00617F00)])
        self.assertEqual(("Caviar", 0x30, True), found)

    def test_reports_nothing_when_no_constructor_runs(self):
        self.assertIsNone(self.run_against([self.instruction("ret", "")]))

    def test_an_adjustment_after_the_call_does_not_apply(self):
        # The lea that matters is the one before the call, not after it.
        found = self.run_against([
            self.instruction("call", "0x5d4cf0", 0x005D4CF0),
            self.instruction("lea", "ecx, [esi + 0x660]")])
        self.assertEqual(("GraphicWin", 0, False), found)


class PinnedParsingTest(unittest.TestCase):
    def test_reads_hexadecimal_and_decimal_pins(self):
        source = ('static_assert(sizeof(Win) == 0x444, "x");\n'
                  'static_assert(sizeof(Random) == 4, "x");\n')
        header = mock.Mock()
        header.read_text.return_value = source
        with mock.patch.object(layout, "SRC_DIR") as directory:
            directory.glob.return_value = [header]
            pinned = layout.load_pinned()
        self.assertEqual({"Win": 0x444, "Random": 4}, pinned)


class VectorIteratorTest(unittest.TestCase):
    """sizeof read off an EH vector iterator call site.

    This reaches a population allocation evidence cannot: a class that is never
    heap-allocated but does appear as a global array. It is exact because the
    iterator's own signature takes the element size, so the image is stating
    sizeof rather than bounding it - which is why Font resolves to its true
    0x28 here, where the global-bound method reports 0x718.
    """

    ITERATOR = 0x006457C2      # ??_L, five arguments
    DTOR_ITERATOR = 0x006456E4  # ??_M, four arguments
    CTOR = 0x00500000
    DTOR = 0x00500100

    def push(self, value):
        item = mock.Mock()
        item.mnemonic, item.op_str = "push", hex(value)
        operand = mock.Mock()
        operand.type, operand.imm = layout.X86_OP_IMM, value
        item.operands = [operand]
        return item

    def push_register(self):
        # A NON-ZERO imm on purpose. Capstone leaves .imm meaningless for a
        # register operand, and a zero here would let the separate
        # zero-size guard catch the case - so the test would pass while the
        # type check it exists for was gone.
        item = mock.Mock()
        item.mnemonic, item.op_str = "push", "eax"
        operand = mock.Mock()
        operand.type, operand.imm = 0, 0x99
        item.operands = [operand]
        return item

    def call(self, target):
        item = mock.Mock()
        item.mnemonic, item.op_str = "call", hex(target)
        operand = mock.Mock()
        operand.type, operand.imm = layout.X86_OP_IMM, target
        item.operands = [operand]
        return item

    def scan(self, instructions, table=None):
        image = mock.Mock()
        image.disasm.side_effect = lambda address, length: instructions
        table = table or {0x00404000: {"name": "?holder@@YAXXZ", "size": "256"},
                          self.CTOR: {"name": "??0Widget@@QAE@XZ", "size": "32"},
                          self.DTOR: {"name": "??1Widget@@QAE@XZ", "size": "16"}}
        # Only the holder carries instructions; the others are name lookups.
        def disasm(address, length):
            return instructions if address == 0x00404000 else []
        image.disasm.side_effect = disasm
        return layout.vector_iterator_sizes(image, table)

    def site(self, size, count=4):
        # Right-to-left, so program order is dtor, ctor, count, size, ptr.
        return [self.push(self.DTOR), self.push(self.CTOR), self.push(count),
                self.push(size), self.push(0x00900000), self.call(self.ITERATOR)]

    def test_reads_the_element_size_from_a_constructor_iterator(self):
        self.assertEqual({"Widget": {0x28}}, self.scan(self.site(0x28)))

    def test_reads_it_from_a_destructor_iterator_too(self):
        # Four arguments, so program order is dtor, count, size, ptr - the size
        # is still second-to-last, which is the whole reason that is the rule.
        instructions = [self.push(self.DTOR), self.push(4), self.push(0x70),
                        self.push(0x00900000), self.call(self.DTOR_ITERATOR)]
        self.assertEqual({"Widget": {0x70}}, self.scan(instructions))

    def test_two_call_sites_that_disagree_are_both_kept(self):
        # Keeping both is what lets settled() refuse. Collapsing them here
        # would silently pick one, and a wrong layout compiles perfectly and
        # corrupts memory at runtime.
        found = self.scan(self.site(0x28) + self.site(0x30))
        self.assertEqual({0x28, 0x30}, found["Widget"])

    def test_a_register_argument_yields_nothing(self):
        # A count or size computed at runtime means the site states nothing.
        instructions = [self.push(self.DTOR), self.push(self.CTOR),
                        self.push(4), self.push_register(),
                        self.push(0x00900000), self.call(self.ITERATOR)]
        self.assertEqual({}, self.scan(instructions))

    def test_a_short_push_run_yields_nothing(self):
        instructions = [self.push(4), self.push(0x28),
                        self.push(0x00900000), self.call(self.ITERATOR)]
        self.assertEqual({}, self.scan(instructions))

    def test_a_zero_size_is_refused(self):
        self.assertEqual({}, self.scan(self.site(0)))

    def test_a_call_to_something_else_is_ignored(self):
        instructions = self.site(0x28)[:-1] + [self.call(0x00401234)]
        self.assertEqual({}, self.scan(instructions))


if __name__ == "__main__":
    unittest.main()
