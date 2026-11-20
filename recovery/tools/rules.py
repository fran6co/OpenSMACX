#!/usr/bin/env python3
"""rules.py <rule> <unit> <pfx> [--write] : greedy single-site rewrites inside non-exact functions.
A site is kept when no fe-exact function of the unit is lost and the unit's total diff drops.
Rules: see RULES (ifswap, elifswap, ternary, cmpswap, earlyret, declswap, stmtswap, declsplit, unname, labelget,
       labelraw, expand, contract, ternret, iftern, armdup, freshctr). analysis/fnsweep.py runs them on one function
       ranked by the sdeck key.  Run under `uv run python`."""
import sys,re,os,json
S=os.path.dirname(os.path.abspath(__file__)); sys.path.insert(0,S); import fe
R=fe.R
MARK=re.compile(r'// ORIGINAL: (0x[0-9A-F]{8}) \S+ 0x[0-9A-Fa-f]+-0x[0-9A-Fa-f]+([^\n]*)')
def spans_nonexact(t):
    ms=[(m.start(),'BYTE_EXACT' in m.group(2)) for m in MARK.finditer(t)]
    out=[]
    for i,(p,ex) in enumerate(ms):
        if not ex: out.append((p, ms[i+1][0] if i+1<len(ms) else len(t)))
    return out
def mask(t):
    """same-length copy with comments/strings/chars blanked (keeps newlines)"""
    out=list(t); i=0; n=len(t)
    while i<n:
        c=t[i]
        if c=='/' and i+1<n and t[i+1]=='/':
            j=t.find('\n',i); j=n if j<0 else j
            for k in range(i,j): out[k]=' '
            i=j
        elif c=='/' and i+1<n and t[i+1]=='*':
            j=t.find('*/',i+2); j=n if j<0 else j+2
            for k in range(i,j):
                if out[k]!='\n': out[k]=' '
            i=j
        elif c in '"\'':
            j=i+1
            while j<n and t[j]!=c:
                if t[j]=='\\': j+=1
                j+=1
            for k in range(i+1,min(j,n)): out[k]=' ' if out[k]!='\n' else '\n'
            i=j+1
        else: i+=1
    return ''.join(out)
def match(mt,i,op,cl):
    d=0; n=len(mt)
    while i<n:
        if mt[i]==op: d+=1
        elif mt[i]==cl:
            d-=1
            if d==0: return i
        i+=1
    return -1
def skipws(mt,i):
    while i<len(mt) and mt[i] in ' \t\r\n': i+=1
    return i
# ---- rules: each yields (start,end,replacement,label) candidates at or after pos in masked text
def rule_ifswap(t,mt,pos):
    for m in re.finditer(r'\bif\s*\(',mt[pos:]):
        s=pos+m.start(); p0=pos+m.end()-1
        p1=match(mt,p0,'(',')')
        if p1<0: continue
        b0=skipws(mt,p1+1)
        if b0>=len(mt) or mt[b0]!='{': continue
        b1=match(mt,b0,'{','}')
        if b1<0: continue
        e=skipws(mt,b1+1)
        if mt[e:e+4]!='else' or (mt[e+4:e+5].isalnum() or mt[e+4:e+5]=='_'): continue
        c0=skipws(mt,e+4)
        if c0>=len(mt) or mt[c0]!='{': continue
        c1=match(mt,c0,'{','}')
        if c1<0: continue
        cond=t[p0+1:p1]
        rep='if (!('+cond+')) '+t[c0:c1+1]+' else '+t[b0:b1+1]
        yield (s,c1+1,rep,'ifswap')
def rule_ternary(t,mt,pos):
    # T v = C ? A : B;   ->  T v; if (C) v = A; else v = B;
    for m in re.finditer(r'\n([ \t]+)((?:const )?[A-Za-z_][\w:\* ]*?[ \*&])(\w+) = ([^;?\n]+) \? ([^;:?\n]+) : ([^;?\n]+);',mt[pos:]):
        s=pos+m.start()+1; e=pos+m.end()
        ind=m.group(1); ty=m.group(2).replace('const ','').strip(); v=m.group(3)
        g=[t[pos+m.start(k):pos+m.end(k)] for k in (4,5,6)]
        if '&' in ty: continue
        rep=ind+ty+' '+v+';\n'+ind+'if ('+g[0]+') '+v+' = '+g[1]+';\n'+ind+'else '+v+' = '+g[2]+';'
        yield (s,e,rep,'ternary->if')
    # v = C ? A : B;  (assignment, not declaration)
    for m in re.finditer(r'\n([ \t]+)([\w\.\->\[\]]+) = ([^;?\n]+) \? ([^;:?\n]+) : ([^;?\n]+);',mt[pos:]):
        s=pos+m.start()+1; e=pos+m.end()
        ind=m.group(1); v=t[pos+m.start(2):pos+m.end(2)]
        g=[t[pos+m.start(k):pos+m.end(k)] for k in (3,4,5)]
        rep=ind+'if ('+g[0]+') '+v+' = '+g[1]+';\n'+ind+'else '+v+' = '+g[2]+';'
        yield (s,e,rep,'ternary-assign->if')
RULES={'ifswap':rule_ifswap,'ternary':rule_ternary}
OPND=r'(?:[A-Za-z_][\w\.\[\]]*(?:->[\w\.\[\]]+)*|-?\d[\w]*)'
MIRROR={'<':'>','>':'<','<=':'>=','>=':'<=','==':'==','!=':'!='}
def rule_cmpswap(t,mt,pos):
    rx=re.compile(r'(?:(?<=\()|(?<=&& )|(?<=\|\| ))('+OPND+r') (<=|>=|==|!=|<|>) ('+OPND+r')(?=\)| &&| \|\||;| \?)')
    for m in rx.finditer(mt,pos):
        a,op,b=m.group(1),m.group(2),m.group(3)
        if a==b: continue
        yield (m.start(),m.end(),b+' '+MIRROR[op]+' '+a,'cmpswap')
def rule_earlyret(t,mt,pos):
    # if (C) { A; return X; }  REST(ending in return)  }   ->   if (!(C)) { REST }  A; return X;  }
    for m in re.finditer(r'\bif\s*\(',mt[pos:]):
        s=pos+m.start(); p0=pos+m.end()-1
        p1=match(mt,p0,'(',')')
        if p1<0: continue
        b0=skipws(mt,p1+1)
        if b0>=len(mt) or mt[b0]!='{': continue
        b1=match(mt,b0,'{','}')
        if b1<0: continue
        inner=mt[b0+1:b1].rstrip()
        if not re.search(r'\breturn\b[^;{}]*;$',inner): continue
        e=skipws(mt,b1+1)
        if mt[e:e+4]=='else': continue
        # the if must be a direct statement of an enclosing block: find the enclosing '}' 
        d=0; k=b1+1; n=len(mt); end=-1
        while k<n:
            if mt[k]=='{': d+=1
            elif mt[k]=='}':
                if d==0: end=k; break
                d-=1
            k+=1
        if end<0: continue
        rest=mt[b1+1:end].rstrip()
        if not rest.strip() or not re.search(r'\breturn\b[^;{}]*;$',rest): continue
        if len(t[b1+1:end])>6000: continue
        # 'case'/'default' labels or other labels in REST make the move unsafe
        if re.search(r'\b(case|default)\b',rest) or re.search(r'\n\s*\w+:\s*\n',rest): continue
        cond=t[p0+1:p1]
        rep='if (!('+cond+')) {'+t[b1+1:end].rstrip()+'\n    }'+t[b0+1:b1].rstrip()+'\n'
        yield (s,end,rep,'earlyret')
RULES['cmpswap']=rule_cmpswap; RULES['earlyret']=rule_earlyret
SIMPLE=r'[\w\.\[\] \+\-\*&|<>]*'
def rule_declswap(t,mt,pos):
    rx=re.compile(r'\n([ \t]+(?:const )?[A-Za-z_][\w:]*[ \*]+\w+(?: = '+SIMPLE+r')?;)\n([ \t]+(?:const )?[A-Za-z_][\w:]*[ \*]+\w+(?: = '+SIMPLE+r')?;)(?=\n)')
    p=pos
    while True:
        m=rx.search(mt,p)
        if not m: return
        l1=t[m.start(1):m.end(1)]; l2=t[m.start(2):m.end(2)]
        if not re.match(r'\s*(return|goto|delete|else|case)\b',l1) and not re.match(r'\s*(return|goto|delete|else|case)\b',l2):
            yield (m.start(1),m.end(2),l2+'\n'+l1,'declswap')
        p=m.start(2)-1
def rule_stmtswap(t,mt,pos):
    LHS=r'[A-Za-z_][\w\.\[\]]*(?:->[\w\.\[\]]+)*'
    rx=re.compile(r'\n([ \t]+)('+LHS+r') = ('+SIMPLE+r');\n\1('+LHS+r') = ('+SIMPLE+r');(?=\n)')
    p=pos
    while True:
        m=rx.search(mt,p)
        if not m: return
        a,ra,b,rb=m.group(2),m.group(3),m.group(4),m.group(5)
        fa=re.split(r'\.|->',a)[-1]; fb=re.split(r'\.|->',b)[-1]
        fa0=re.sub(r'\[.*','',fa); fb0=re.sub(r'\[.*','',fb)
        ok = fa0!=fb0 and not re.search(r'\b'+re.escape(fa0)+r'\b',rb+' '+b) and not re.search(r'\b'+re.escape(fb0)+r'\b',ra+' '+a)
        if ok:
            ind=m.group(1)
            l1=t[m.start(1):m.start(4)-1].split('\n')[0]; l2=ind+t[m.start(4):m.end(5)]+';'
            yield (m.start(1),m.end(5)+1,l2+'\n'+l1,'stmtswap')
        p=m.start(4)-len(m.group(1))-1
RULES['declswap']=rule_declswap; RULES['stmtswap']=rule_stmtswap
def rule_declsplit(t,mt,pos):
    # `T v = e;` inside a function  ->  `T v;` at the top of the function and `v = e;` in place
    rx=re.compile(r'\n([ \t]+)((?:unsigned |signed )?[A-Za-z_][\w:]*(?: \*+| ))(\w+) = ([^;\n]+);(?=\n)')
    p=pos
    while True:
        m=rx.search(mt,p)
        if not m: return
        p=m.end()
        ty=m.group(2).strip(); name=m.group(3)
        if ty in ('return','const','static','else','case','goto','delete','typedef') or ty.startswith('const') or len(m.group(1))<8: continue
        # top of the enclosing function: last line before m that starts in column 0 and ends with '{'
        tops=[x for x in re.finditer(r'\n[^\s/#][^\n]*\{[ \t]*(?=\n)',mt[:m.start()])]
        if not tops: continue
        top=tops[-1].end()
        if '}' in [l[:1] for l in mt[top:m.start()].split('\n')]: continue   # a column-0 '}' in between: not inside that function
        ind=m.group(1)
        decl='\n    '+t[m.start(2):m.end(2)].rstrip()+(' ' if not t[m.start(2):m.end(2)].rstrip().endswith('*') else '')+name+';'
        rep=t[top:m.start()]+'\n'+ind+name+' = '+t[m.start(4):m.end(4)]+';'
        yield (top,m.end(),decl+rep,'declsplit',m.end())
RULES['declsplit']=rule_declsplit
def rule_unname(t,mt,pos):
    # `T v = SIMPLE;` -> drop the local and read SIMPLE at every use in the rest of its block.
    # Not semantics-preserving in general (SIMPLE may change before a use): run with RULE_EXACT_ONLY=1,
    # where only a byte-exact result is accepted.
    rx=re.compile(r'\n([ \t]+)(?:const )?(?:unsigned |signed )?[A-Za-z_][\w:]*(?: \*+| )(\w+) = ([A-Za-z_][\w\.\[\]]*(?:->[\w\.\[\]]+)*);(?=\n)')
    p=pos
    while True:
        m=rx.search(mt,p)
        if not m: return
        p=m.end()
        if len(m.group(1))<4: continue
        v=m.group(2); expr=t[m.start(3):m.end(3)]
        d=0; k=m.end(); n=len(mt); end=-1
        while k<n:
            if mt[k]=='{': d+=1
            elif mt[k]=='}':
                if d==0: end=k; break
                d-=1
            k+=1
        if end<0: continue
        rest=t[m.end():end]; mrest=mt[m.end():end]
        if re.search(r'\b'+v+r'\b\s*(=[^=]|\+=|-=|\*=|/=|\|=|&=|\+\+|--|<<=|>>=)',mrest) or re.search(r'(\+\+|--|&)\s*'+v+r'\b',mrest): continue
        if not re.search(r'\b'+v+r'\b',mrest): continue
        out=[]; last=0
        for u in re.finditer(r'(?<![\w\.>])'+v+r'\b',mrest):
            out.append(rest[last:u.start()]); out.append(expr); last=u.end()
        out.append(rest[last:])
        yield (m.start()+1,end,''.join(out).lstrip('\n'),'unname',m.end())
RULES['unname']=rule_unname
def rule_labelget(t,mt,pos):
    for m in re.finditer(r'StringTable\.get\(Labels\.strings_ptr\[([^\]]+)\]\)',mt[pos:]):
        yield (pos+m.start(),pos+m.end(),'label_get('+t[pos+m.start(1):pos+m.end(1)]+')','labelget')
def rule_labelraw(t,mt,pos):
    for m in re.finditer(r'\blabel_get\(([^()]+)\)',mt[pos:]):
        yield (pos+m.start(),pos+m.end(),'StringTable.get(Labels.strings_ptr['+t[pos+m.start(1):pos+m.end(1)]+'])','labelraw')
RULES['labelget']=rule_labelget; RULES['labelraw']=rule_labelraw
_INL=None
def _inlines():
    global _INL
    if _INL is None:
        import glob
        _INL={}
        for hp in glob.glob(R+'/src/**/*.h',recursive=True):
            ht=open(hp,encoding='latin-1').read()
            for m in re.finditer(r'(?:^|\n)(?:MEASURED )?(?:static )?inline [\w \*&:<>]+?\b(?:__cdecl )?(\w+)\(([^()]*)\) \{\s*return ([^;{}]+);\s*\}',ht):
                if '::' in m.group(0).split('(')[0]: continue
                ps=[x.strip().split(' ')[-1].lstrip('*&') for x in m.group(2).split(',') if x.strip() and x.strip()!='void']
                _INL[m.group(1)]=(ps,' '.join(m.group(3).split()))
        for k in ('label_get','map_tiles','console_map_win','opening_window','MapAbstract'): _INL.pop(k,None)
    return _INL
def rule_expand(t,mt,pos):
    # call of a one-return free inline -> its expression with the arguments substituted (in parentheses)
    inl=_inlines()
    rx=re.compile(r'(?<![\w\.>:])('+'|'.join(map(re.escape,inl))+r')\(')
    for m in rx.finditer(mt,pos):
        ps,expr=inl[m.group(1)]
        e=match(mt,m.end()-1,'(',')')
        if e<0: continue
        # split arguments at top-level commas
        args=[]; d=0; last=m.end()
        for k in range(m.end(),e):
            c=mt[k]
            if c in '([{': d+=1
            elif c in ')]}': d-=1
            elif c==',' and d==0: args.append(t[last:k].strip()); last=k+1
        tail=t[last:e].strip()
        if tail or ps: args.append(tail)
        if len(args)!=len(ps): continue
        ok=True; out=expr
        for a,pn in zip(args,ps):
            uses=len(re.findall(r'\b'+re.escape(pn)+r'\b',expr))
            simple=re.fullmatch(r'[\w\.\[\]\->: ]+',a) is not None
            if uses!=1 and not simple: ok=False; break
        if not ok: continue
        # simultaneous substitution
        def sub(mm):
            i=ps.index(mm.group(0)); a=args[i]
            return a if re.fullmatch(r'[\w\.\[\]]+',a) else '('+a+')'
        if ps: out=re.sub(r'\b('+'|'.join(map(re.escape,ps))+r')\b',sub,expr)
        yield (m.start(),e+1,'('+out+')','expand:'+m.group(1))
RULES['expand']=rule_expand
A_=r'([\w\.\[\]]+(?:->[\w\.\[\]]+)*|\([^()]*\))'
CONTRACT=[
 (r'map_loc\('+A_+r', '+A_+r'\)->bit\b(?!2)', r'bit_at(\1, \2)'),
 (r'map_loc\('+A_+r', '+A_+r'\)->bit2\b', r'bit2_at(\1, \2)'),
 (r'map_loc\('+A_+r', '+A_+r'\)->region\b', r'region_at(\1, \2)'),
 (r'\(map_loc\('+A_+r', '+A_+r'\)->climate >> 5\)', r'alt_at(\1, \2)'),
 (r'map_loc\('+A_+r', '+A_+r'\)->contour\b', r'alt_detail_at(\1, \2)'),
 (r'\(PlayersData\['+A_+r'\]\.diplo_treaties\['+A_+r'\] & '+A_+r'\)', r'has_treaty(\1, \2, \3)'),
 (r'PlayersData\['+A_+r'\]\.diplo_treaties\['+A_+r'\] & (DTREATY_\w+)', r'has_treaty(\1, \2, \3)'),
 (r'get_proto_triad\(Vehs\['+A_+r'\]\.proto_id\)', r'get_triad(\1)'),
 (r'Chassis\[VehPrototypes\['+A_+r'\]\.chassis_id\]\.triad', r'get_proto_triad(\1)'),
 (r'Weapon\[VehPrototypes\['+A_+r'\]\.weapon_id\]\.offense_rating', r'get_proto_offense_rating(\1)'),
 (r'Armor\[VehPrototypes\['+A_+r'\]\.armor_id\]\.defense_rating', r'get_proto_defense_rating(\1)'),
 (r'GameDataStateStorage\.fields_\.secret_project_\.project_base\['+A_+r'\]', r'base_project(\1)'),
]
def rule_contract(t,mt,pos):
    best=None
    for rx,rep in CONTRACT:
        m=re.compile(rx).search(mt,pos)
        if m and (best is None or m.start()<best[0].start()): best=(m,rx,rep)
    while best:
        m,rx,rep=best
        mm=re.compile(rx).match(t,m.start())
        if mm: yield (m.start(),m.end(),mm.expand(rep),'contract:'+rep.split('(')[0])
        nxt=None
        for rx2,rep2 in CONTRACT:
            m2=re.compile(rx2).search(mt,m.start()+1)
            if m2 and (nxt is None or m2.start()<nxt[0].start()): nxt=(m2,rx2,rep2)
        best=nxt
RULES['contract']=rule_contract
def rule_ternret(t,mt,pos):
    # inline helper body `return C ? A : B;` -> `if (C) { return A; } return B;`
    for m in re.finditer(r'(inline [^\n{;]*\{\s*)return ([^;?{}]+?) \? ([^;:{}]+?) : ([^;{}]+?);(\s*\})',mt[pos:]):
        s0=pos+m.start(2)-len('return '); e0=pos+m.end(4)+1
        g=[t[pos+m.start(k):pos+m.end(k)] for k in (2,3,4)]
        yield (s0,e0,'if ('+g[0].strip()+') { return '+g[1].strip()+'; } return '+g[2].strip()+';','ternret')
RULES['ternret']=rule_ternret
LV_=r'[A-Za-z_][\w\.\[\]]*(?:->[\w\.\[\]]+)*'
def _stmt_end(mt,i):
    """i at the first char of a statement body (after optional '{'): return end of a single `x;` statement or -1"""
    j=mt.find(';',i)
    return j
def rule_iftern(t,mt,pos):
    # MATCHING_GUIDE 3.7 (round 40): one store of a value chosen by a condition is a ternary.
    # if (C) V = A; else V = B;   (each arm optionally braced)   ->   V = C ? A : B;
    # V = B; if (C) V = A;                                          ->   V = C ? A : B;
    out=[]
    arm=r'(?:\{\s*('+LV_+r') = ([^;{}]+);\s*\}|('+LV_+r') = ([^;{}]+);)'
    rx=re.compile(r'\bif\s*\(')
    for m in rx.finditer(mt,pos):
        s=m.start(); p0=m.end()-1; p1=match(mt,p0,'(',')')
        if p1<0: continue
        cond=t[p0+1:p1]
        a=re.compile(r'\s*'+arm).match(mt,p1+1)
        if not a: continue
        va=(a.group(1) or a.group(3)); ea=(a.group(2) if a.group(1) else a.group(4))
        va_t=t[a.start(1 if a.group(1) else 3):a.end(1 if a.group(1) else 3)]
        ea_t=t[a.start(2 if a.group(1) else 4):a.end(2 if a.group(1) else 4)]
        el=re.compile(r'\s*else\s*'+arm).match(mt,a.end())
        base=re.split(r'[\.\[]|->',va)[0]
        if el:
            vb=(el.group(1) or el.group(3))
            if vb.replace(' ','')!=va.replace(' ',''): continue
            eb_t=t[el.start(2 if el.group(1) else 4):el.end(2 if el.group(1) else 4)]
            nxt=skipws(mt,el.end())
            if mt[nxt:nxt+4]=='else': continue
            out.append((s,el.end(),va_t+' = ('+cond.strip()+') ? '+ea_t.strip()+' : '+eb_t.strip()+';','iftern'))
        else:
            # previous statement `V = B;` on the line before
            k=t.rfind('\n',0,s); k2=t.rfind('\n',0,k)
            if k<0 or k2<0: continue
            prev=mt[k2+1:k]
            pm=re.match(r'([ \t]*)('+re.escape(va)+r') = ([^;{}]+);\s*$',prev)
            if not pm: continue
            nxt=skipws(mt,a.end())
            if mt[nxt:nxt+4]=='else': continue
            if re.search(r'\b'+re.escape(base)+r'\b',mt[p0:p1]): continue
            eb_t=t[k2+1+pm.start(3):k2+1+pm.end(3)]
            out.append((k2+1+pm.start(2),a.end(),va_t+' = ('+cond.strip()+') ? '+ea_t.strip()+' : '+eb_t.strip()+';','prevstore-tern'))
    out.sort(key=lambda c:c[0])
    for c in out: yield c
RULES['iftern']=rule_iftern
_CTRL=re.compile(r'\s*(return|break|continue|goto|case|default|if|else|for|while|do|switch|delete)\b')
def rule_armdup(t,mt,pos):
    # MATCHING_GUIDE 3.7 (round 40): a test far from its branch means both arms repeat a common prefix.
    # S1; ..; Sk; if (C) {A} else {B}   ->   if (C) {S1..Sk A} else {S1..Sk B}   (k = 1..4; S simple statements
    # that do not write anything C reads)
    out=[]
    for m in re.finditer(r'\bif\s*\(',mt):
        s=m.start(); p0=m.end()-1; p1=match(mt,p0,'(',')')
        if p1<0: continue
        b0=skipws(mt,p1+1)
        if b0>=len(mt) or mt[b0]!='{': continue
        b1=match(mt,b0,'{','}')
        if b1<0: continue
        e=skipws(mt,b1+1)
        if mt[e:e+4]!='else' or (mt[e+4:e+5].isalnum() or mt[e+4:e+5]=='_'): continue
        c0=skipws(mt,e+4)
        if c0>=len(mt) or mt[c0]!='{': continue
        c1=match(mt,c0,'{','}')
        if c1<0: continue
        # the if must start its line, and not be an `else if`
        ls=mt.rfind('\n',0,s)
        if mt[ls+1:s].strip(): continue
        condids=set(re.findall(r'[A-Za-z_]\w*',mt[p0:p1]))
        stmts=[]; k=ls
        while len(stmts)<4:
            k2=mt.rfind('\n',0,k)
            if k2<0: break
            line=mt[k2+1:k]
            if not line.strip(): k=k2; continue
            if not re.match(r'[ \t]+[^;{}]*;\s*$',line) or _CTRL.match(line): break
            if re.match(r'[ \t]*(?:const |unsigned |signed |static )*[A-Za-z_][\w:]*(?:[ \*&]+)\w+( = [^;]*)?;\s*$',line) and not re.match(r'\s*\w+ = ',line) and not re.match(r'\s*[\w\.\[\]\->]+\(',line): break  # declaration
            lhs=re.match(r'\s*([A-Za-z_][\w\.\[\]]*(?:->[\w\.\[\]]+)*)\s*(?:[\+\-\*/\|&\^]|<<|>>)?=[^=]',line)
            ids=set(re.findall(r'[A-Za-z_]\w*',lhs.group(1))) if lhs else set()
            if re.search(r'\+\+|--',line): ids|=set(re.findall(r'[A-Za-z_]\w*',line))
            if ids & condids: break
            if set(re.findall(r'[A-Za-z_]\w*',line)) & condids: break
            k3=mt.rfind('\n',0,k2)
            pl=mt[k3+1:k2].rstrip()
            if pl and pl[-1] not in ';{}': break
            # a call may change anything C reads; allow only when C reads no member/global state through a call
            stmts.insert(0,(k2+1,k))
            k=k2
            ss=stmts[0][0]
            moved=''.join(t[a:b+1] for a,b in stmts)
            ind=re.match(r'[ \t]*',t[ss:]).group(0)
            arm_ind=re.match(r'\n([ \t]*)',t[b0+1:]) 
            ai=arm_ind.group(1) if arm_ind else ind+'    '
            mv=''.join('\n'+ai+t[a:b].strip() for a,b in stmts)
            rep=t[s:b0+1]+mv+t[b0+1:c0+1]+mv+t[c0+1:c1+1]
            out.append((ss,c1+1,rep,'armdup%d'%len(stmts)))
    out=[c for c in out if c[0]>=pos]
    out.sort(key=lambda c:c[0])
    for c in out: yield c
RULES['armdup']=rule_armdup
def rule_freshctr(t,mt,pos):
    # MATCHING_GUIDE 3.7 (round 40): a loop counter in its own frame slot is a new variable.
    # for (X = I; ...X...) {..X..}  with X a function-scope int whose value is not read after the loop
    #   ->  for (int X_fN = I; ...) {..X_fN..}
    n=0
    for m in re.finditer(r'\bfor\s*\(\s*([A-Za-z_]\w*) = ',mt):
        if m.start()<pos: continue
        x=m.group(1)
        p0=mt.find('(',m.start()); p1=match(mt,p0,'(',')')
        if p1<0: continue
        b0=skipws(mt,p1+1)
        if mt[b0]!='{': continue
        b1=match(mt,b0,'{','}')
        if b1<0: continue
        # enclosing function: last column-0 line ending with '{' before
        tops=[q for q in re.finditer(r'\n[^\s/#][^\n]*\{[ \t]*(?=\n)',mt[:m.start()])]
        if not tops: continue
        top=tops[-1].end()
        fe_=mt.find('\n}',b1)
        if fe_<0: continue
        if not re.search(r'\b(?:int|unsigned|short|char|long)\b[^;\n(]*\b'+x+r'\b',mt[top:m.start()]): continue
        after=mt[b1+1:fe_]
        u=re.search(r'\b'+x+r'\b',after)
        if u:
            nx=after[u.end():u.end()+4]
            if not re.match(r'\s*=[^=]',nx): continue
        n+=1
        nn=x+'_f%d'%n
        hdr=re.sub(r'\b'+x+r'\b',nn,t[p0:p1+1],count=0)
        hdr='('+'int '+hdr[1:].lstrip()
        body=re.sub(r'(?<![\w\.>])'+x+r'\b',nn,t[b0:b1+1])
        yield (m.start(),b1+1,'for '+hdr+' '+body,'freshctr')
RULES['freshctr']=rule_freshctr
def _stmt_full_end(mt,i):
    """end (exclusive) of the statement at i: a block, an if/else chain, or a simple `x;`"""
    i=skipws(mt,i)
    if mt[i]=='{':
        e=match(mt,i,'{','}'); return e+1 if e>=0 else -1
    m=re.match(r'if\s*\(',mt[i:])
    if m:
        p0=i+m.end()-1; p1=match(mt,p0,'(',')')
        if p1<0: return -1
        e=_stmt_full_end(mt,p1+1)
        if e<0: return -1
        k=skipws(mt,e)
        if mt[k:k+4]=='else' and not (mt[k+4].isalnum() or mt[k+4]=='_'):
            return _stmt_full_end(mt,k+4)
        return e
    j=mt.find(';',i); return j+1 if j>=0 else -1
def rule_elifswap(t,mt,pos):
    # if (C) {A} else if (D) ...   ->   if (!(C)) { if (D) ... } else {A}   (ifswap for an else-if chain head)
    for m in re.finditer(r'\bif\s*\(',mt[pos:]):
        s=pos+m.start(); p0=pos+m.end()-1
        if re.search(r'else\s*$',mt[max(0,s-12):s]): continue
        p1=match(mt,p0,'(',')')
        if p1<0: continue
        b0=skipws(mt,p1+1)
        if mt[b0]!='{': continue
        b1=match(mt,b0,'{','}')
        if b1<0: continue
        e=skipws(mt,b1+1)
        if mt[e:e+4]!='else' or (mt[e+4].isalnum() or mt[e+4]=='_'): continue
        c0=skipws(mt,e+4)
        if not re.match(r'if\s*\(',mt[c0:]): continue
        c1=_stmt_full_end(mt,c0)
        if c1<0: continue
        cond=t[p0+1:p1]
        yield (s,c1,'if (!('+cond+')) {\n'+t[c0:c1]+'\n} else '+t[b0:b1+1],'elifswap')
RULES['elifswap']=rule_elifswap
def main():
    rule=RULES[sys.argv[1]]; unit=sys.argv[2]; unit=unit if unit.startswith('src/') else 'src/'+unit; pfx=sys.argv[3]
    path=R+'/'+unit; text=open(path,encoding='latin-1').read()
    addrs=[int(m.group(1),16) for m in MARK.finditer(text)]
    tmp=fe.SCR+'/rule_%s_%s.cpp'%(os.path.basename(unit).replace('.','_'),pfx)
    def score(t):
        open(tmp,'w',encoding='latin-1').write(t)
        c,err=fe.compile_unit(unit,tmp,pfx)
        if c is None: return None
        ex=set(); tot=0; per={}; st=set()
        for a in addrs:
            r=fe.evaluate(c,t,a)
            if 'error' in r: continue
            per[a]=r['diff']; tot+=r['diff']
            if r['exact']: ex.add(a)
            oi=[o[1] for k,o,c_ in r['rows'] if o]; ci=[c_[1] for k,o,c_ in r['rows'] if c_]
            fo=next((x for x in oi[:12] if x.startswith('sub esp')),None); fc=next((x for x in ci[:12] if x.startswith('sub esp')),None)
            if fo==fc: st.add((a,'frame'))
            if sum(x.startswith('call') for x in oi)==sum(x.startswith('call') for x in ci): st.add((a,'calls'))
        return ex,tot,per,st
    base=score(text)
    if base is None: print(json.dumps(dict(unit=unit,tried=0,accepted=[],total=-1))); return
    cur=text; pos=0; acc=[]; tried=0; base0_exact=set(base[0])
    while True:
        mt=mask(cur); sp=spans_nonexact(cur)
        cand=None
        for c in rule(cur,mt,pos):
            if os.environ.get('RULE_ALL_SPANS')=='1' or any(a<=c[0]<b for a,b in sp): cand=c; break
        if cand is None: break
        s,e,rep,label=cand[:4]; resume=cand[4] if len(cand)>4 else None
        new=cur[:s]+rep+cur[e:]
        tried+=1
        sc=score(new)
        # No ratchet on exact functions (AGENTS.md, 'Structure first'): a site is kept when it
        # breaks no structural match (frame, call count) and the unit's difference drops.
        # structure first: a site that gains a frame or call-count match is kept whatever the row count does
        exact_only=os.environ.get('RULE_EXACT_ONLY')=='1'
        if sc and ((sc[0]-base[0]) if exact_only else (not (base[3]-sc[3]) and ((sc[3]-base[3]) or sc[1]<=base[1]-int(os.environ.get('RULE_MIN','3'))))):
            line=cur.count('\n',0,s)+1
            ch=[('%08X'%a,base[2][a],sc[2][a]) for a in sc[2] if sc[2][a]!=base[2].get(a)]
            acc.append((line,label,ch,['%08X'%a for a in sc[0]-base[0]]))
            cur=new; base=sc; pos=s+len(rep) if resume is None else resume+len(rep)-(e-s)
        else: pos=s+1 if resume is None else resume
        if tried>int(os.environ.get('RULE_MAX','400')): break
    print(json.dumps(dict(unit=unit,tried=tried,accepted=acc,total=base[1],exact_lost=sorted('%08X'%a for a in base0_exact-base[0]))))
    if '--write' in sys.argv and acc: open(path,'w',encoding='latin-1').write(cur)
if __name__=='__main__': main()
