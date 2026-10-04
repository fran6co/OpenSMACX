// Actual-source interaction control; not native ABI or game acceptance.
#include <cassert>
#include <vector>
#define __cdecl

static void (*callback)(char *);
static int observed_technology, observed_faction, observed_link, help_calls;
void help_tech_info(void (*emit)(char *), int technology, int faction, int link) {
    callback = emit;
    observed_technology = technology;
    observed_faction = faction;
    observed_link = link;
    ++help_calls;
    char text[] = "ignored technology output";
    emit(text);
    assert(text[0] == 'i');
}
class Sound {
public:
    Sound(int id, Sound *next) : id_(id), next_(next) {}
    virtual int unload();
    int id_;
    Sound *next_;
};
static Sound *SoundChainHead;
static Sound *insertion;
static std::vector<int> unloaded;
int Sound::unload() {
    assert(SoundChainHead == this);
    unloaded.push_back(id_);
    SoundChainHead = next_;
    if (insertion) {
        insertion->next_ = SoundChainHead;
        SoundChainHead = insertion;
        insertion = 0;
    }
    return -1;
}

// @@CALLBACK@@
// @@HELP_WRAPPER@@
// @@SOUND_CHAIN@@

int main() {
    technology_help_without_output(91, 37, 5);
    assert(callback == discard_technology_help_text);
    assert(help_calls == 1 && observed_technology == 37);
    assert(observed_faction == 5 && observed_link == -1);
    technology_help_without_output(-99, -3, 0);
    assert(help_calls == 2 && observed_technology == -3);
    assert(observed_faction == 0 && observed_link == -1);
    assert(unload_active_sound_chain() == 11);
    Sound second(2, 0), first(1, &second), injected(3, 0);
    SoundChainHead = &first;
    insertion = &injected;
    assert(unload_active_sound_chain() == 11);
    assert(SoundChainHead == 0 && unloaded.size() == 3);
    assert(unloaded[0] == 1 && unloaded[1] == 3 && unloaded[2] == 2);
    assert(unload_active_sound_chain() == 11);
    assert(unloaded.size() == 3);
}
