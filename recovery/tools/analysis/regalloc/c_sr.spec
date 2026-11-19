// Strength reduction of a large-stride table index (MATCHING_GUIDE 2.7, setup_game 0x005B3920).
// python3 cand.py c_sr.spec --grep 'add\s+e.., \d{3,}|P@@'
== base
struct PL { int pad[100]; int cat[2]; int model[2]; int eff[2]; int rest[250]; };
struct SC { struct { int v[11]; } me[4]; };
extern PL P[8]; extern SC SCs[3]; extern int rand();
void probe(int g) {
    int rejected;
    int seen[12];
    int clear;
    for (clear = 0; clear < 12; clear++) seen[clear] = 0;
    for (int sf = 1; sf < 8; sf++) {
        rejected = 0;
        int category; int model;
        P[sf].cat[0] = -1;
        P[sf].model[0] = 0;
        P[sf].eff[0] = -1;
        do {
            category = rand() % 3;
            model = rand() % 3 + 1;
        } while (seen[category * 3 + model] && ++rejected < sf + 1);
        if (rejected <= sf + 1) {
            P[sf].cat[0] = category;
            P[sf].model[0] = model;
            seen[category * 3 + model] = 1;
            for (int effect = 0; effect < 11; effect++) {
                if (SCs[category].me[model].v[effect] > 0) {
                    P[sf].eff[0] = effect;
                    break;
                }
            }
        }
    }
}

== ptrinc
struct PL { int pad[100]; int cat[2]; int model[2]; int eff[2]; int rest[250]; };
struct SC { struct { int v[11]; } me[4]; };
extern PL P[8]; extern SC SCs[3]; extern int rand();
void probe(int g) {
    int rejected;
    int seen[12];
    int clear;
    for (clear = 0; clear < 12; clear++) seen[clear] = 0;
    PL *p = &P[1];
    for (int sf = 1; sf < 8; sf++, p++) {
        rejected = 0;
        int category; int model;
        p->cat[0] = -1;
        p->model[0] = 0;
        p->eff[0] = -1;
        do {
            category = rand() % 3;
            model = rand() % 3 + 1;
        } while (seen[category * 3 + model] && ++rejected < sf + 1);
        if (rejected <= sf + 1) {
            p->cat[0] = category;
            p->model[0] = model;
            seen[category * 3 + model] = 1;
            for (int effect = 0; effect < 11; effect++) {
                if (SCs[category].me[model].v[effect] > 0) {
                    p->eff[0] = effect;
                    break;
                }
            }
        }
    }
}

== nobreak
struct PL { int pad[100]; int cat[2]; int model[2]; int eff[2]; int rest[250]; };
struct SC { struct { int v[11]; } me[4]; };
extern PL P[8]; extern SC SCs[3]; extern int rand();
void probe(int g) {
    int rejected;
    int seen[12];
    int clear;
    for (clear = 0; clear < 12; clear++) seen[clear] = 0;
    for (int sf = 1; sf < 8; sf++) {
        rejected = 0;
        int category; int model;
        P[sf].cat[0] = -1;
        P[sf].model[0] = 0;
        P[sf].eff[0] = -1;
        do {
            category = rand() % 3;
            model = rand() % 3 + 1;
        } while (seen[category * 3 + model] && ++rejected < sf + 1);
        if (rejected <= sf + 1) {
            P[sf].cat[0] = category;
            P[sf].model[0] = model;
            seen[category * 3 + model] = 1;
            for (int effect = 0; effect < 11; effect++) {
                if (SCs[category].me[model].v[effect] > 0) {
                    P[sf].eff[0] = effect;
                    effect = 99;
                }
            }
        }
    }
}

== noeffloop
struct PL { int pad[100]; int cat[2]; int model[2]; int eff[2]; int rest[250]; };
struct SC { struct { int v[11]; } me[4]; };
extern PL P[8]; extern SC SCs[3]; extern int rand();
void probe(int g) {
    int rejected;
    int seen[12];
    int clear;
    for (clear = 0; clear < 12; clear++) seen[clear] = 0;
    for (int sf = 1; sf < 8; sf++) {
        rejected = 0;
        int category; int model;
        P[sf].cat[0] = -1;
        P[sf].model[0] = 0;
        P[sf].eff[0] = -1;
        do {
            category = rand() % 3;
            model = rand() % 3 + 1;
        } while (seen[category * 3 + model] && ++rejected < sf + 1);
        if (rejected <= sf + 1) {
            P[sf].cat[0] = category;
            P[sf].model[0] = model;
            seen[category * 3 + model] = 1;
            P[sf].eff[0] = g;
        }
    }
}

== nodo
struct PL { int pad[100]; int cat[2]; int model[2]; int eff[2]; int rest[250]; };
struct SC { struct { int v[11]; } me[4]; };
extern PL P[8]; extern SC SCs[3]; extern int rand();
void probe(int g) {
    int rejected;
    int seen[12];
    int clear;
    for (clear = 0; clear < 12; clear++) seen[clear] = 0;
    for (int sf = 1; sf < 8; sf++) {
        rejected = 0;
        int category; int model;
        P[sf].cat[0] = -1;
        P[sf].model[0] = 0;
        P[sf].eff[0] = -1;
        category = rand() % 3; model = rand() % 3 + 1;
        if (rejected <= sf + 1) {
            P[sf].cat[0] = category;
            P[sf].model[0] = model;
            seen[category * 3 + model] = 1;
            for (int effect = 0; effect < 11; effect++) {
                if (SCs[category].me[model].v[effect] > 0) {
                    P[sf].eff[0] = effect;
                    break;
                }
            }
        }
    }
}

== nouse_sf
struct PL { int pad[100]; int cat[2]; int model[2]; int eff[2]; int rest[250]; };
struct SC { struct { int v[11]; } me[4]; };
extern PL P[8]; extern SC SCs[3]; extern int rand();
void probe(int g) {
    int rejected;
    int seen[12];
    int clear;
    for (clear = 0; clear < 12; clear++) seen[clear] = 0;
    for (int sf = 1; sf < 8; sf++) {
        rejected = 0;
        int category; int model;
        P[sf].cat[0] = -1;
        P[sf].model[0] = 0;
        P[sf].eff[0] = -1;
        do {
            category = rand() % 3;
            model = rand() % 3 + 1;
        } while (seen[category * 3 + model] && ++rejected < 9);
        if (rejected <= 9) {
            P[sf].cat[0] = category;
            P[sf].model[0] = model;
            seen[category * 3 + model] = 1;
            for (int effect = 0; effect < 11; effect++) {
                if (SCs[category].me[model].v[effect] > 0) {
                    P[sf].eff[0] = effect;
                    break;
                }
            }
        }
    }
}

