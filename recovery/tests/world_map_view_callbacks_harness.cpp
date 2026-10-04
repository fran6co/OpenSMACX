// Host projection for the actual global map-view callbacks, not game proof.
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#define __cdecl

std::vector<std::string> calls;
struct Win {
    int identity;
    Win(int value) : identity(value) {}
    virtual ~Win() {}
    virtual void show(int mode) {
        assert(mode == 0);
        calls.push_back(identity == 1 ? "detail.show" : "world.show");
    }
    virtual void hide() {
        calls.push_back(identity == 1 ? "detail.hide" : "world.hide");
    }
};
struct MapStorage { int state[7]; };
struct MapWin : MapStorage, virtual Win {
    MapWin(int value) : Win(value) {}
};
struct WorldWin : Win {
    unsigned int detail_map_selected_;
    WorldWin() : Win(2), detail_map_selected_(7) {}
    // Global callbacks must qualify Win, bypassing these derived overrides.
    void show(int) { calls.push_back("wrong.world.override.show"); }
    void hide() { calls.push_back("wrong.world.override.hide"); }
};
struct ButtonGroup {
    int identity;
    int set(int id, int value) {
        assert(identity == 1 && value == 0);
        assert(id == 1006 || id == 1007);
        calls.push_back(id == 1007 ? "group.detail" : "group.world");
        return 29;
    }
};
struct MainInterface {
    ButtonGroup groups[4];
    ButtonGroup &button_group(int index) {
        assert(index == 1);
        return groups[index];
    }
};
MainInterface MainInterfaceGlobal;
MapWin *MapWinTable[2];
WorldWin WorldClimateWorldWin;

// ACTUAL_DETAIL_CALLBACK
// ACTUAL_WORLD_CALLBACK

int main() {
    MainInterfaceGlobal.groups[1].identity = 1;
    MapWin primary(0), detail(1);
    MapWinTable[0] = &primary;
    MapWinTable[1] = &detail;
    show_detail_map_view();
    assert(calls.size() == 3);
    assert(calls[0] == "group.detail" && calls[1] == "detail.show" && calls[2] == "world.hide");
    assert(WorldClimateWorldWin.detail_map_selected_ == 7);
    calls.clear();
    show_world_map_view();
    assert(calls.size() == 3);
    assert(calls[0] == "group.world" && calls[1] == "detail.hide" && calls[2] == "world.show");
    assert(WorldClimateWorldWin.detail_map_selected_ == 7);
    std::puts("global view callbacks preserve presentation order and selection state");
}
