// ORIGINAL: 0x00494230 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00494230
// name           ?on_left_double_click@ProdPicker@@QAEXHH@Z
// size           65 bytes
// measured tier  NO_COMPILE
// refusal        u00494230.cpp(49) : error C2653: 'ProdPicker' : is not a class or namespace name u00494230.cpp(50) : error C2673: 'on_left_double_click' : global functions do n
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00494230/unit.cpp
// and score it with tools/agent_brief.py.
class VCall2 { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019(int, int);  // <-- used
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044(int);  // <-- used
};

void ProdPicker::on_left_double_click(int a1, int a2) {
    reinterpret_cast<VCall2 *>(this)->slot019(a1, a2);
    if (field_B68_ == 0) {
        reinterpret_cast<VCall2 *>(this)->slot044(-1);
    } else {
        reinterpret_cast<VCall2 *>(this)->slot044(5);
    }
}
