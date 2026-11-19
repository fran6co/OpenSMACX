"""nat.py SRC OUT func[,func]  : respell hand-expanded get_noun/get_title/get_name in the named functions"""
import re,sys
def nat(g):
    P=r'(\w+(?:\[[^\]]+\])?)'
    g=re.sub(r'GenderDefault = Players\[(\w+)\]\.noun_gender;\n\s*PluralityDefault = Players\[\1\]\.is_noun_plural;\n(\s*)parse_says\((\d+), Players\[\1\]\.noun_faction, -1, -1\);',r'parse_says(\3, get_noun(\1), -1, -1);',g)
    g=re.sub(r'parse_set\(Players\[(\w+)\]\.noun_gender, Players\[\1\]\.is_noun_plural\);\n(\s*)parse_says\((\d+), Players\[\1\]\.noun_faction, -1, -1\);',r'parse_says(\3, get_noun(\1), -1, -1);',g)
    for f,n in (('title_leader','get_title'),('name_leader','get_name')):
        g=re.sub(r'(?:PluralityDefault = (?:FALSE|0);\n\s*GenderDefault = Players\[(\w+)\]\.is_leader_female;|GenderDefault = Players\[(\w+)\]\.is_leader_female;\n\s*PluralityDefault = (?:FALSE|0);)\n(\s*)parse_says\((\d+), Players\[\w+\]\.%s, -1, -1\);'%f,lambda m:'parse_says(%s, %s(%s), -1, -1);'%(m.group(4),n,m.group(1) or m.group(2)),g)
        g=re.sub(r'parse_set\(Players\[(\w+)\]\.is_leader_female, (?:0|FALSE)\);\n(\s*)parse_says\((\d+), Players\[\1\]\.%s, -1, -1\);'%f,r'parse_says(\3, %s(\1), -1, -1);'%n,g)
    return g
def fn_span(t,name):
    m=re.search(r'\n[\w][^\n;]*\b%s\([^;{]*\)\s*\{\n'%re.escape(name),t)
    a=m.start(); b=t.index('\n}\n',a); return a,b
if __name__=='__main__':
    src,out,names=sys.argv[1:4]
    t=open(src,encoding='latin-1').read(); n0=t
    for name in names.split(','):
        a,b=fn_span(t,name); t=t[:a]+nat(t[a:b])+t[b:]
    open(out,'w',encoding='latin-1').write(t); print('changed' if t!=n0 else 'same')
