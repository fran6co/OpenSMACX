%% DU=U(d); U(d);,U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d);
== iv_vs_d
void probe() {
    D(d); U(d);
    for (int i = 0; i < 10; i++) { use_a(garr[i]); }
    {DU}
}
%% DL=U(d);,U(d); U(d);,U(d); U(d); U(d);,U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d);,U(d); U(d); U(d); U(d); U(d); U(d); U(d); U(d);
== iv_inloop_d
void probe() {
    D(d);
    for (int i = 0; i < 10; i++) { use_a(garr[i]); {DL} }
    U(d);
}
%% IU=use_b(i);,use_b(i); use_b(i);,use_b(i); use_b(i); use_b(i);,use_b(i); use_b(i); use_b(i); use_b(i);
== iv_i_used
void probe() {
    for (int i = 0; i < 10; i++) { use_a(garr[i]); {IU} }
}
