# joint-solve deck: body spellings per header inline, tagged by cost-ladder rung
# r1 = one return expression; r2 = one condition with embedded assignment; r3 = guard + final ternary;
# r4 = two guards / nested ifs with named local; r5+ = further named locals; p = parameter re-read
# (2026-11-19 par-4, round 7).  FEAS below is the `jointsolve.py copies` result: spellings whose copy stays exact.
INLINES = {}

def add(name, file, head, rungs, **kw):
    INLINES[name] = dict(file=file, head=head, rungs=rungs, **kw)

add('range', 'src/general.h', r'^MEASURED inline int __cdecl range\(int input, int min, int max\) \{', [
    ('r4_two_guards', '''
    if (input < min || max < min) {
        return min;
    }
    if (input > max) {
        return max;
    }
    return input;'''),
    ('r1_ternary', '''
    return (input < min || max < min) ? min : input > max ? max : input;'''),
    ('r3_guard_ternary', '''
    if (input < min || max < min) {
        return min;
    }
    return input > max ? max : input;'''),
    ('r4_nested', '''
    if (input >= min && max >= min) {
        if (input > max) {
            return max;
        }
        return input;
    }
    return min;'''),
    ('r4_else_chain', '''
    if (input < min || max < min) {
        return min;
    } else if (input > max) {
        return max;
    }
    return input;'''),
])

add('on_map', 'src/map.h', r'^MEASURED inline BOOL __cdecl on_map\(int x, int y\) \{', [
    ('r1_expr', '''
    return y >= 0 && y < (int)MapLatitudeBounds && x >= 0 && x < (int)MapLongitudeBounds;'''),
    ('r3_if_true', '''
    if (y >= 0 && y < (int)MapLatitudeBounds && x >= 0 && x < (int)MapLongitudeBounds) {
        return true;
    }
    return false;'''),
    ('r4_two_guards', '''
    if (y < 0 || y >= (int)MapLatitudeBounds) {
        return false;
    }
    if (x < 0 || x >= (int)MapLongitudeBounds) {
        return false;
    }
    return true;'''),
    ('r3_guard_expr', '''
    if (y < 0 || y >= (int)MapLatitudeBounds) {
        return false;
    }
    return x >= 0 && x < (int)MapLongitudeBounds;'''),
    ('r5_named', '''
    BOOL inside = y >= 0 && y < (int)MapLatitudeBounds && x >= 0 && x < (int)MapLongitudeBounds;
    return inside;'''),
    ('r1_ternary', '''
    return (y >= 0 && y < (int)MapLatitudeBounds && x >= 0 && x < (int)MapLongitudeBounds) ? true : false;'''),
])

def who(bits, limit):
    return [
    ('r3_guard_ternary', '''
    Map *tile = map_loc(x, y);
    if (!(tile->bit & %s)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    return owner >= %s ? -1 : owner;''' % (bits, limit)),
    ('r4_two_guards', '''
    Map *tile = map_loc(x, y);
    if (!(tile->bit & %s)) {
        return -1;
    }
    int owner = tile->val2 & 0xF;
    if (owner >= %s) {
        return -1;
    }
    return owner;''' % (bits, limit)),
    ('r2_embedded', '''
    Map *tile = map_loc(x, y);
    int owner;
    if ((tile->bit & %s) && (owner = tile->val2 & 0xF) < %s) {
        return owner;
    }
    return -1;''' % (bits, limit)),
    ('r4_nested', '''
    Map *tile = map_loc(x, y);
    if (tile->bit & %s) {
        int owner = tile->val2 & 0xF;
        if (owner < %s) {
            return owner;
        }
    }
    return -1;''' % (bits, limit)),
    ('r3_notile', '''
    if (!(map_loc(x, y)->bit & %s)) {
        return -1;
    }
    int owner = map_loc(x, y)->val2 & 0xF;
    return owner >= %s ? -1 : owner;''' % (bits, limit)),
    ('r4_notile', '''
    if (!(map_loc(x, y)->bit & %s)) {
        return -1;
    }
    int owner = map_loc(x, y)->val2 & 0xF;
    if (owner >= %s) {
        return -1;
    }
    return owner;''' % (bits, limit)),
    ('r3_owner_at', '''
    if (!(bit_at(x, y) & %s)) {
        return -1;
    }
    int owner = owner_at(x, y);
    return owner >= %s ? -1 : owner;''' % (bits, limit)),
    ('r4_owner_at', '''
    if (!(bit_at(x, y) & %s)) {
        return -1;
    }
    int owner = owner_at(x, y);
    if (owner >= %s) {
        return -1;
    }
    return owner;''' % (bits, limit)),
    ]

add('veh_who', 'src/map.h', r'^MEASURED inline int __cdecl veh_who\(int x, int y\) \{', who('BIT_VEH_IN_TILE', '8'))
add('base_who', 'src/map.h', r'^MEASURED inline int __cdecl base_who\(int x, int y\) \{', who('BIT_BASE_IN_TILE', 'MaxPlayerNum'))
add('anything_at', 'src/map.h', r'^MEASURED inline int __cdecl anything_at\(int x, int y\) \{', who('(BIT_VEH_IN_TILE | BIT_BASE_IN_TILE)', 'MaxPlayerNum'))

add('proto_power', 'src/veh.h', r'^MEASURED inline int __cdecl proto_power\(int veh_id\) \{', [
    ('r1_ternary', '''
    return VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT ? 1
        : range(VehPrototypes[Vehs[veh_id].proto_id].reactor_id, 1, 100) * 10;'''),
    ('r4_guard', '''
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    return range(VehPrototypes[Vehs[veh_id].proto_id].reactor_id, 1, 100) * 10;'''),
    ('r5_proto_local', '''
    int proto_id = Vehs[veh_id].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    return range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;'''),
    ('r5_proto_ternary', '''
    int proto_id = Vehs[veh_id].proto_id;
    return VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT ? 1
        : range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;'''),
])

add('veh_top', 'src/veh.h', r'^MEASURED inline int __cdecl veh_top\(int veh_id\) \{', [
    ('r5_named_top', '''
    int top_veh_id = veh_id;
    if (veh_id < 0) {
        return -1;
    }
    while (Vehs[top_veh_id].prev_veh_id_stack >= 0) {
        top_veh_id = Vehs[top_veh_id].prev_veh_id_stack;
    }
    return top_veh_id;'''),
    ('p_param', '''
    if (veh_id < 0) {
        return -1;
    }
    while (Vehs[veh_id].prev_veh_id_stack >= 0) {
        veh_id = Vehs[veh_id].prev_veh_id_stack;
    }
    return veh_id;'''),
    ('r5_named_after_guard', '''
    if (veh_id < 0) {
        return -1;
    }
    int top_veh_id = veh_id;
    while (Vehs[top_veh_id].prev_veh_id_stack >= 0) {
        top_veh_id = Vehs[top_veh_id].prev_veh_id_stack;
    }
    return top_veh_id;'''),
    ('p_for', '''
    if (veh_id < 0) {
        return -1;
    }
    for (; Vehs[veh_id].prev_veh_id_stack >= 0; veh_id = Vehs[veh_id].prev_veh_id_stack) {
    }
    return veh_id;'''),
])

add('veh_moves', 'src/veh.h', r'^MEASURED inline int __cdecl veh_moves\(int veh_id\) \{', [
    ('r1_expr', '''
    return range(speed(veh_id, false) - Vehs[veh_id].moves_expended, 0, 999);'''),
    ('r5_named', '''
    int moves = speed(veh_id, false) - Vehs[veh_id].moves_expended;
    return range(moves, 0, 999);'''),
])

add('x_dist', 'src/map.h', r'^MEASURED inline int __cdecl x_dist\(int x_point_a, int x_point_b\) \{', [
    ('r3_named_guard_ternary', '''
    int dist = abs(x_point_a - x_point_b);
    if (!(MapIsFlat & 1)) {
        return dist > (int)MapLongitude ? MapLongitudeBounds - dist : dist;
    }
    return dist;'''),
    ('r4_assign', '''
    int dist = abs(x_point_a - x_point_b);
    if (!(MapIsFlat & 1) && dist > (int)MapLongitude) {
        dist = MapLongitudeBounds - dist;
    }
    return dist;'''),
    ('r4_two_returns', '''
    int dist = abs(x_point_a - x_point_b);
    if (!(MapIsFlat & 1) && dist > (int)MapLongitude) {
        return MapLongitudeBounds - dist;
    }
    return dist;'''),
    ('r1_named_ternary', '''
    int dist = abs(x_point_a - x_point_b);
    return (!(MapIsFlat & 1) && dist > (int)MapLongitude) ? MapLongitudeBounds - dist : dist;'''),
    ('r4_flat_guard', '''
    int dist = abs(x_point_a - x_point_b);
    if (MapIsFlat & 1) {
        return dist;
    }
    return dist > (int)MapLongitude ? MapLongitudeBounds - dist : dist;'''),
])

add('vector_dist2', 'src/map.h', r'^MEASURED inline int __cdecl vector_dist\(int x_distance, int y_distance\) \{', [
    ('r5_named', '''
    x_distance = abs(x_distance);
    y_distance = abs(y_distance);
    int largest = (x_distance > y_distance) ? x_distance : y_distance;
    int smallest = (x_distance < y_distance) ? x_distance : y_distance;
    // Preserve `>> 1`, not `/ 2` (arithmetic shift semantics).
    return largest - ((((y_distance + x_distance) >> 1) - smallest + 1) >> 1);'''),
    ('r5_largest_only', '''
    x_distance = abs(x_distance);
    y_distance = abs(y_distance);
    int largest = (x_distance > y_distance) ? x_distance : y_distance;
    return largest - ((((y_distance + x_distance) >> 1) - ((x_distance < y_distance) ? x_distance : y_distance) + 1) >> 1);'''),
    ('r6_abs_locals', '''
    int dx = abs(x_distance);
    int dy = abs(y_distance);
    int largest = (dx > dy) ? dx : dy;
    int smallest = (dx < dy) ? dx : dy;
    return largest - ((((dy + dx) >> 1) - smallest + 1) >> 1);'''),
], sym=r'^\?vector_dist@@YAHHH@Z', ident='vector_dist')

add('vector_dist4', 'src/map.h', r'^MEASURED inline int __cdecl vector_dist\(int x_point_a, int y_point_a, int x_point_b, int y_point_b\) \{', [
    ('r5_named', '''
    int x_distance = x_dist(x_point_a, x_point_b);
    int y_distance = abs(y_point_a - y_point_b);
    return vector_dist(x_distance, y_distance);'''),
    ('r1_expr', '''
    return vector_dist(x_dist(x_point_a, x_point_b), abs(y_point_a - y_point_b));'''),
    ('r5_one_named', '''
    int x_distance = x_dist(x_point_a, x_point_b);
    return vector_dist(x_distance, abs(y_point_a - y_point_b));'''),
], sym=r'^\?vector_dist@@YAHHHHH@Z', ident='vector_dist')

add('has_project', 'src/base.h', r'^MEASURED inline BOOL __cdecl has_project\(int project_id, int faction_id\) \{', [
    ('r4_named_guard', '''
    // WORKAROUND(inline): calling base_project() here makes base_minerals
    // (0x004E9CB0) exact but flips allocate_energy's (0x005267B0) PlayersData
    // load-order ties; the operator[] read keeps both TUs' claims.
    int base_id = GameDataStateStorage.fields_.secret_project_[project_id];
    if (base_id < 0) {
        return false;
    }
    return Bases[base_id].faction_id_current == faction_id;'''),
    ('r4_base_project', '''
    int base_id = base_project(project_id);
    if (base_id < 0) {
        return false;
    }
    return Bases[base_id].faction_id_current == faction_id;'''),
    ('r1_named_and', '''
    int base_id = GameDataStateStorage.fields_.secret_project_[project_id];
    return base_id >= 0 && Bases[base_id].faction_id_current == faction_id;'''),
    ('r3_named_ternary', '''
    int base_id = GameDataStateStorage.fields_.secret_project_[project_id];
    return base_id < 0 ? false : Bases[base_id].faction_id_current == faction_id;'''),
    ('r4_nested', '''
    int base_id = GameDataStateStorage.fields_.secret_project_[project_id];
    if (base_id >= 0) {
        return Bases[base_id].faction_id_current == faction_id;
    }
    return false;'''),
])

add('is_known', 'src/map.h', r'^MEASURED inline BOOL __cdecl is_known\(int x, int y, int faction_id\) \{', [
    ('r1_expr', '''
    return (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED
        || map_loc(x, y)->visibility & (1 << faction_id));'''),
    ('r4_guard', '''
    if (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED) {
        return true;
    }
    return (map_loc(x, y)->visibility & (1 << faction_id)) != 0;'''),
    ('r3_if_true', '''
    if (PlayersData[faction_id].flags & PFLAG_MAP_REVEALED
        || map_loc(x, y)->visibility & (1 << faction_id)) {
        return true;
    }
    return false;'''),
])

add('has_treaty', 'src/faction_inline.h', r'^MEASURED inline int __cdecl has_treaty\(int faction_id, int faction_id_with, int treaty\) \{', [
    ('r1_expr', '''
    return PlayersData[faction_id].diplo_treaties[faction_id_with] & treaty;'''),
    ('r5_named', '''
    int treaties = PlayersData[faction_id].diplo_treaties[faction_id_with];
    return treaties & treaty;'''),
])

add('has_agenda', 'src/faction_inline.h', r'^MEASURED inline int __cdecl has_agenda\(int faction_id, int faction_id_with, int agenda\) \{', [
    ('r1_expr', '''
    return PlayersData[faction_id].diplo_agenda[faction_id_with] & agenda;'''),
    ('r5_named', '''
    int agendas = PlayersData[faction_id].diplo_agenda[faction_id_with];
    return agendas & agenda;'''),
])

add('is_ocean', 'src/map.h', r'^MEASURED inline BOOL __cdecl is_ocean\(int x, int y\) \{', [
    ('r1_expr', '''
    return (map_loc(x, y)->climate & ~0x1F) < ALT_BIT_SHORE_LINE;'''),
    ('r5_named', '''
    Map *tile = map_loc(x, y);
    return (tile->climate & ~0x1F) < ALT_BIT_SHORE_LINE;'''),
    ('r5_alt', '''
    int altitude = map_loc(x, y)->climate & ~0x1F;
    return altitude < ALT_BIT_SHORE_LINE;'''),
])

add('abstract_at', 'src/map.h', r'^MEASURED inline uint8_t __cdecl abstract_at\(int x, int y\) \{', [
    ('r1_expr', '''
    return (MapAbstract())[(x >> 1) + y * (MapAbstractLongBounds >> 1)];'''),
    ('r5_named', '''
    uint8_t region = (MapAbstract())[(x >> 1) + y * (MapAbstractLongBounds >> 1)];
    return region;'''),
    ('r5_index', '''
    int index = (x >> 1) + y * (MapAbstractLongBounds >> 1);
    return (MapAbstract())[index];'''),
])

add('veh_next_stack_id', 'src/veh.h', r'^MEASURED inline int __cdecl veh_next_stack_id\(int veh_id\) \{', [
    ('r1_ternary', '''
  return veh_id >= 0 ? Vehs[veh_id].next_veh_id_stack : veh_id;'''),
    ('r4_guard', '''
  if (veh_id < 0) {
    return veh_id;
  }
  return Vehs[veh_id].next_veh_id_stack;'''),
])

add('is_port', 'src/map.h', r'^MEASURED inline BOOL __cdecl is_port\(int base_id, BOOL is_base_radius\) \{', [
    ('r1_expr', '''
    return is_coast(Bases[base_id].x, Bases[base_id].y, is_base_radius);'''),
    ('r5_named', '''
    Base *base = &Bases[base_id];
    return is_coast(base->x, base->y, is_base_radius);'''),
])

add('rnd', 'src/crt_random.h', r'^MEASURED inline unsigned int __cdecl rnd\(int bounds, char \*input\) \{', [
    ('r5_named_value', '''
    if (bounds - 1 <= 0) {
        return 0;
    }
    int value = rand() % bounds;
    return value;'''),
    ('r4_guard', '''
    if (bounds - 1 <= 0) {
        return 0;
    }
    return rand() % bounds;'''),
    ('r1_ternary', '''
    return bounds - 1 <= 0 ? 0 : rand() % bounds;'''),
])

add('veh_health', 'src/veh.h', r'^MEASURED inline int __cdecl veh_health\(int veh_id\) \{', [
    ('r6_named_both', '''
    int proto_id = Vehs[veh_id].proto_id;
    int health;
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        health = 1;
    } else {
        health = range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10;
    }
    return range(health - Vehs[veh_id].dmg_incurred, 0, 9999);'''),
    ('r5_proto_ternary', '''
    int proto_id = Vehs[veh_id].proto_id;
    return range((VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT ? 1
        : range(VehPrototypes[proto_id].reactor_id, 1, 100) * 10) - Vehs[veh_id].dmg_incurred, 0, 9999);'''),
    ('r1_proto_power', '''
    return range(proto_power(veh_id) - Vehs[veh_id].dmg_incurred, 0, 9999);'''),
])

add('has_fac', 'src/base.h', r'^MEASURED inline BOOL __cdecl has_fac\(int facility_id, int base_id, int queue_count\) \{', [
    ('r6_queued', '''
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    BOOL is_built = (Bases[base_id].facilities_built[offset] & mask) != 0;
    if (is_built || !queue_count) {
        return is_built;
    }
    // BUG IN THE ORIGINAL: no upper bound on queue_count (queue_production_id
    // holds 10 entries); reproduced deliberately.
    for (int i = 0; i < queue_count; i++) {
        int queued = Bases[base_id].queue_production_id[i];
        if (queued == -facility_id) {
            return true;
        }
    }
    return false;'''),
    ('r5_flat', '''
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    BOOL is_built = (Bases[base_id].facilities_built[offset] & mask) != 0;
    if (is_built || !queue_count) {
        return is_built;
    }
    // BUG IN THE ORIGINAL: no upper bound on queue_count (queue_production_id
    // holds 10 entries); reproduced deliberately.
    for (int i = 0; i < queue_count; i++) {
        if (Bases[base_id].queue_production_id[i] == -facility_id) {
            return true;
        }
    }
    return false;'''),
    ('r7_queued_base', '''
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    Base *base = &Bases[base_id];
    BOOL is_built = (base->facilities_built[offset] & mask) != 0;
    if (is_built || !queue_count) {
        return is_built;
    }
    // BUG IN THE ORIGINAL: no upper bound on queue_count (queue_production_id
    // holds 10 entries); reproduced deliberately.
    for (int i = 0; i < queue_count; i++) {
        int queued = base->queue_production_id[i];
        if (queued == -facility_id) {
            return true;
        }
    }
    return false;'''),
    ('r7_two_guards', '''
    if (facility_id >= FacilityRepStart) {
        return false;
    }
    int offset;
    int mask;
    bitmask(facility_id, &offset, &mask);
    BOOL is_built = (Bases[base_id].facilities_built[offset] & mask) != 0;
    if (is_built) {
        return is_built;
    }
    if (!queue_count) {
        return is_built;
    }
    // BUG IN THE ORIGINAL: no upper bound on queue_count (queue_production_id
    // holds 10 entries); reproduced deliberately.
    for (int i = 0; i < queue_count; i++) {
        int queued = Bases[base_id].queue_production_id[i];
        if (queued == -facility_id) {
            return true;
        }
    }
    return false;'''),
])

FEAS = {'range': ['r4_two_guards', 'r1_ternary', 'r4_nested', 'r4_else_chain'], 'on_map': ['r1_expr', 'r3_if_true', 'r4_two_guards', 'r3_guard_expr', 'r5_named'], 'veh_who': ['r3_guard_ternary', 'r4_two_guards', 'r2_embedded', 'r4_nested', 'r3_notile', 'r4_notile'], 'base_who': ['r3_guard_ternary', 'r4_two_guards', 'r2_embedded', 'r4_nested', 'r3_notile', 'r4_notile', 'r3_owner_at', 'r4_owner_at'], 'anything_at': ['r3_guard_ternary', 'r4_two_guards', 'r2_embedded', 'r4_nested', 'r3_notile', 'r4_notile', 'r3_owner_at', 'r4_owner_at'], 'proto_power': ['r1_ternary', 'r4_guard', 'r5_proto_local', 'r5_proto_ternary'], 'veh_top': ['r5_named_top', 'p_param', 'r5_named_after_guard', 'p_for'], 'veh_moves': ['r1_expr', 'r5_named'], 'x_dist': ['r3_named_guard_ternary', 'r4_flat_guard'], 'vector_dist2': ['r5_named', 'r5_largest_only', 'r6_abs_locals'], 'vector_dist4': ['r5_named', 'r1_expr', 'r5_one_named'], 'has_project': ['r4_named_guard', 'r4_base_project', 'r3_named_ternary'], 'is_known': ['r1_expr', 'r3_if_true'], 'has_treaty': ['r1_expr', 'r5_named'], 'has_agenda': ['r1_expr', 'r5_named'], 'is_ocean': ['r1_expr', 'r5_named', 'r5_alt'], 'abstract_at': ['r1_expr', 'r5_named', 'r5_index'], 'veh_next_stack_id': ['r1_ternary', 'r4_guard'], 'is_port': ['r1_expr'], 'rnd': ['r5_named_value', 'r4_guard', 'r1_ternary'], 'veh_health': ['r6_named_both', 'r5_proto_ternary', 'r1_proto_power'], 'has_fac': ['r6_queued', 'r5_flat']}
for _n, _s in INLINES.items():
    _s["feasible"] = FEAS.get(_n, [])

INLINES['proto_power']['rungs'] += [
    ('r5_reactor_local', '''
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    int reactor = VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
    return range(reactor, 1, 100) * 10;'''),
    ('r5_reactor_ternary', '''
    int reactor = VehPrototypes[Vehs[veh_id].proto_id].reactor_id;
    return VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT ? 1 : range(reactor, 1, 100) * 10;'''),
    ('r1_cast', '''
    return VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT ? 1
        : range((int)(unsigned int)VehPrototypes[Vehs[veh_id].proto_id].reactor_id, 1, 100) * 10;'''),
    ('r6_proto_reactor', '''
    int proto_id = Vehs[veh_id].proto_id;
    if (VehPrototypes[proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    int reactor = VehPrototypes[proto_id].reactor_id;
    return range(reactor, 1, 100) * 10;'''),
    ('r5_power_local', '''
    int power;
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        power = 1;
    } else {
        power = range(VehPrototypes[Vehs[veh_id].proto_id].reactor_id, 1, 100) * 10;
    }
    return power;'''),
]
INLINES['proto_power']['feasible'] += ['r5_reactor_local','r5_reactor_ternary','r1_cast','r6_proto_reactor','r5_power_local']

INLINES['proto_power']['rungs'] += [
    ('r1_reactor_power', '''
    return VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT ? 1
        : reactor_power(VehPrototypes[Vehs[veh_id].proto_id].reactor_id);'''),
    ('r4_guard_reactor_power', '''
    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_ALIEN_ARTIFACT) {
        return 1;
    }
    return reactor_power(VehPrototypes[Vehs[veh_id].proto_id].reactor_id);'''),
]
# these two need `inline int reactor_power(int reactor) { return range(reactor, 1, 100) * 10; }` above proto_power in veh.h
INLINES['proto_power']['feasible'] += ['r1_reactor_power','r4_guard_reactor_power']

# round 2 of the descent: spellings already rejected by exact callers (shape) or measured are left out
_keep = {
    'range': ['r4_two_guards'],
    'on_map': ['r1_expr', 'r3_if_true'],
    'veh_who': ['r3_guard_ternary', 'r4_two_guards', 'r2_embedded', 'r4_nested'],
    'base_who': ['r4_two_guards', 'r3_guard_ternary', 'r2_embedded', 'r4_nested'],
    'anything_at': ['r4_two_guards', 'r3_guard_ternary', 'r2_embedded', 'r4_nested'],
    'proto_power': ['r1_ternary', 'r4_guard', 'r5_proto_local', 'r5_proto_ternary', 'r5_reactor_local', 'r6_proto_reactor'],
}
for _n, _l in _keep.items():
    INLINES[_n]['feasible'] = _l
_order = ['veh_top', 'veh_moves', 'x_dist', 'vector_dist2', 'vector_dist4', 'has_project', 'is_known', 'has_treaty',
          'has_agenda', 'is_ocean', 'abstract_at', 'veh_next_stack_id', 'rnd', 'veh_health', 'has_fac', 'proto_power',
          'anything_at', 'base_who', 'veh_who', 'on_map', 'is_port', 'range']
INLINES = dict((k, INLINES[k]) for k in _order)
