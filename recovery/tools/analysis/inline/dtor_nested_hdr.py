HDR='''
extern void dm2(); extern void dm3();
struct GW { virtual ~GW(); int x[4]; };
struct FB : GW { FB(); void close(); virtual ~FB() { close(); } };
struct S : GW { FB f1; FB f2; S(); void close(); virtual ~S() { dm2(); close(); } };
struct P : GW { S s; P(); void close(); virtual ~P() { dm(); close(); } };
'''
