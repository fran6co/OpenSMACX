// ORIGINAL: 0x005F05D0 FILE
// name      ?set_border_thickness@Win@@QAEXH@Z
// size      61 bytes
// spans     0x005F05D0-0x005F060D
// prototype void (__thiscall ?set_border_thickness@Win@@QAEXH@Z)(Win* this, int thickness)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005F0606
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F05D0
// measured tier  NO_COMPILE
// refusal        u005f05d0.cpp(8) : error C2653: 'Win' : is not a class or namespace name u005f05d0.cpp(9) : error C2673: 'set_border_thickness' : global functions do not have '
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f05d0/unit.cpp
// and score it with tools/agent_brief.py.
class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003(int, int, int);  // <-- used
};

void Win::set_border_thickness(int a1) {
    if (*reinterpret_cast<unsigned char *>(reinterpret_cast<char *>(this) + 0x9C) & 2) {
        border_thickness_ = a1;
        reinterpret_cast<VCall2 *>(this)->slot003(
            client_rect_.right - client_rect_.left,
            client_rect_.bottom - client_rect_.top,
            0);
    }
}
