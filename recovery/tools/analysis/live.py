"""live.py: machine-code frame-slot liveness and webs for the image and for our compile.
import live; I=live.image(lo,hi); O=live.ours(unit,src,addr,pfx)
Each is a Func with .ins (list of dict: a, txt, succ, rd(set of slots), wr(set of slots)), .idx (addr->index).
Func.livein(slot) -> set of instruction indices where slot is live on entry.
Func.webs(slot) -> list of (defs:set(idx), uses:set(idx)).
Func.witness(i, slot) -> path of indexes from i to a read of slot with no write (or None)."""
import sys, os, re, struct, collections
sys.path.insert(0, 'recovery/tools')
import fe
import capstone
from capstone import x86 as X

md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
md.detail = True

WRITE_ONLY = {'mov', 'movzx', 'movsx', 'lea', 'setne', 'sete', 'setl', 'setg', 'setle', 'setge', 'seta', 'setb', 'setae', 'setbe', 'pop', 'fstp', 'fst', 'fistp'}
READ_ONLY = {'cmp', 'test', 'push', 'fild', 'fld', 'imul', 'idiv', 'div', 'mul', 'call', 'jmp', 'fcomp', 'fcom', 'fmul', 'fadd', 'fdiv', 'fsub'}


class Func:
    def __init__(self):
        self.ins = []
        self.idx = {}

    def add(self, a, ins, succ):
        rd = set()
        wr = set()
        for n, op in enumerate(ins.operands):
            if op.type == X.X86_OP_MEM and op.mem.base == X.X86_REG_EBP and op.mem.index == 0:
                s = op.mem.disp
                if ins.mnemonic == 'lea':
                    rd.add(('addr', s))
                    continue
                if op.size != 4:
                    rd.add(s)  # partial access: treat as read
                    if n == 0 and ins.mnemonic in WRITE_ONLY:
                        wr.add(('part', s))
                    continue
                if n == 0 and ins.mnemonic in WRITE_ONLY:
                    wr.add(s)
                elif n == 0 and ins.mnemonic not in READ_ONLY:
                    rd.add(s)
                    wr.add(s)
                else:
                    rd.add(s)
        self.idx[a] = len(self.ins)
        self.ins.append(dict(a=a, txt=ins.mnemonic + ' ' + ins.op_str, succ=succ, rd=rd, wr=wr))

    def finish(self):
        for i in self.ins:
            i['s'] = [self.idx[t] for t in i['succ'] if t in self.idx]
        self.pred = collections.defaultdict(list)
        for n, i in enumerate(self.ins):
            for s in i['s']:
                self.pred[s].append(n)

    def livein(self, slot):
        live = set()
        work = [n for n, i in enumerate(self.ins) if slot in i['rd']]
        live.update(work)
        while work:
            n = work.pop()
            for p in self.pred[n]:
                if p in live:
                    continue
                if slot in self.ins[p]['wr'] and slot not in self.ins[p]['rd']:
                    continue
                live.add(p)
                work.append(p)
        return live

    def witness(self, start, slot):
        # BFS forward from start (instruction index, state before executing it)
        prev = {start: None}
        q = collections.deque([start])
        while q:
            n = q.popleft()
            i = self.ins[n]
            if slot in i['rd']:
                path = []
                while n is not None:
                    path.append(n)
                    n = prev[n]
                return path[::-1]
            if slot in i['wr']:
                continue
            for s in i['s']:
                if s not in prev:
                    prev[s] = n
                    q.append(s)
        return None

    def webs(self, slot):
        defs = [n for n, i in enumerate(self.ins) if slot in i['wr']]
        uses = [n for n, i in enumerate(self.ins) if slot in i['rd']]
        parent = {}

        def find(x):
            while parent.setdefault(x, x) != x:
                parent[x] = parent[parent[x]]
                x = parent[x]
            return x
        use_defs = collections.defaultdict(set)
        for d in defs + ['entry']:
            seen = set()
            work = list(self.ins[d]['s']) if d != 'entry' else [0]
            while work:
                n = work.pop()
                if n in seen:
                    continue
                seen.add(n)
                i = self.ins[n]
                if slot in i['rd']:
                    use_defs[n].add(d)
                if slot in i['wr']:
                    continue
                work.extend(i['s'])
        for u, ds in use_defs.items():
            ds = list(ds)
            for d in ds:
                parent[find(('d', d))] = find(('u', u))
        groups = collections.defaultdict(lambda: (set(), set()))
        for d in defs + ['entry']:
            groups[find(('d', d))][0].add(d)
        for u in uses:
            groups[find(('u', u))][1].add(u)
        return [g for g in groups.values() if g[0] != {'entry'} or g[1]]


def _build(data, org, code_end, tables):
    """tables: dict table_addr -> [targets]"""
    f = Func()
    for ins in md.disasm(data[:code_end], org):
        a = ins.address
        nxt = a + ins.size
        m = ins.mnemonic
        succ = []
        if m == 'ret':
            pass
        elif m == 'jmp':
            op = ins.operands[0]
            if op.type == X.X86_OP_IMM:
                succ = [op.imm]
            elif op.type == X.X86_OP_MEM:
                succ = list(tables.get(('jmp', a), []))
        elif m.startswith('j'):
            succ = [nxt, ins.operands[0].imm]
        else:
            succ = [nxt]
        f.add(a, ins, succ)
    f.finish()
    return f


def image(lo, hi):
    data = fe.imgbytes(lo, hi)
    tables = {}
    code_end = len(data)
    tabs = []
    for ins in md.disasm(data, lo):
        if ins.address - lo >= code_end:
            break
        if ins.mnemonic == 'jmp' and ins.operands[0].type == X.X86_OP_MEM:
            t = ins.operands[0].mem.disp & 0xffffffff
            if lo <= t < hi:
                code_end = min(code_end, t - lo)
                tabs.append((ins.address, t))
        # byte tables for two-level switches: mov reg8,[reg+T]
        for op in ins.operands:
            if op.type == X.X86_OP_MEM and lo <= (op.mem.disp & 0xffffffff) < hi and ins.mnemonic != 'jmp':
                code_end = min(code_end, (op.mem.disp & 0xffffffff) - lo)
    for a, t in tabs:
        out = []
        p = t - lo
        while p + 4 <= len(data):
            v = struct.unpack_from('<I', data, p)[0]
            if not (lo <= v < lo + code_end):
                break
            out.append(v)
            p += 4
        tables[('jmp', a)] = out
    return _build(data, lo, code_end, tables)


def ours(unit, src, addr, pfx='2'):
    c, err = fe.compile_unit(unit, os.path.abspath(src), pfx)
    if c is None:
        raise SystemExit('compile fail ' + str(err))
    text = open(src, encoding='latin-1').read()
    sym, lo, hi = fe.ident(text, addr)
    s = c.function(sym)
    sec = c.sections[s.section - 1]
    data = bytes(sec.data)
    rel = {r[0]: r for r in sec.relocations}
    code_end = len(data)
    tabs = []
    for ins in md.disasm(data, 0):
        if ins.address >= code_end:
            break
        for op in ins.operands:
            if op.type == X.X86_OP_MEM:
                # find reloc inside this instruction
                for r in range(ins.address, ins.address + ins.size):
                    if r in rel:
                        _, si, ty = rel[r]
                        sy = c.symbols.get(si)
                        if ty == 6 and sy is not None and sy.section == s.section and sy.value > ins.address:
                            t = sy.value + struct.unpack_from('<I', data, r)[0]
                            code_end = min(code_end, t)
                            if ins.mnemonic == 'jmp':
                                tabs.append((ins.address, t))
    tables = {}
    for a, t in tabs:
        out = []
        p = t
        while p + 4 <= len(data) and p in rel:
            _, si, ty = rel[p]
            sy = c.symbols.get(si)
            v = sy.value + struct.unpack_from('<I', data, p)[0]
            if not (0 <= v < code_end):
                break
            out.append(v)
            p += 4
        tables[('jmp', a)] = out
    return _build(data, 0, code_end, tables)
