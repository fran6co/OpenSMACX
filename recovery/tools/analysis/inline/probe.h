// Shared declarations for the /Ob2 inline-budget probes (ib.py).
extern void mk(int);   // marker inside an inline body: push K; call mk = site K expanded
extern void mk2(int);
extern void at(int);   // return-site marker for teardown probes
extern void dm();      // marker inside an inline destructor
extern int cond();
extern void call();
extern int g, g2, g3, g4, g5, g6, g7, g8, g9; extern int ga[];
extern void use(int);
