#include <cassert>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <vector>

#define __cdecl
#define __stdcall
#define MaxPlayerNum 8
#define ERROR_SUCCESS 0L
#define REG_BINARY 3
typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef long LONG;
typedef void *HKEY;
typedef const char *LPCSTR;
#define HKEY_LOCAL_MACHINE ((HKEY)0x80000002u)
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

std::string Events;
void event(const char *s) { Events += s; Events += '|'; }

char StringTemp[64];
int IsMultiplayerNet;
int ConsoleControlTurnActive;
int UnkGlobal0093A934;
uint32_t LockEnableMask;
int FactionAfterFirstLog;
int HumanMaskAfterFirstLog;
int FactionAfterFirstOpen;
int *FactionOwner;
std::vector<int> LoggedFactions;
std::vector<int> LoggedOthers;
std::vector<int> OpenFactions;
std::vector<int> OpenOthers;
std::vector<int> CommunicatedFactions;
std::vector<int> CommunicatedOthers;
uint32_t FactionDiplomacyFlags[MaxPlayerNum];
uint8_t factions_status_byte(int byte) {
    return reinterpret_cast<uint8_t *>(&LockEnableMask)[byte];
}

struct NetDaemon {
    int OpenResult;
    uint32_t lock_state_[1][10];
    uint32_t &factions_waiting_mask() { return lock_state_[0][7]; }
    void net_tasks() { event("net_tasks"); }
    int open_channel(int a, int b) {
        OpenFactions.push_back(a); OpenOthers.push_back(b);
        event((a == 2 && b == 3) ? "open23" : "open-other");
        if (FactionAfterFirstOpen != 0) {
            *FactionOwner = FactionAfterFirstOpen;
            FactionAfterFirstOpen = 0;
        }
        return OpenResult;
    }
} NetDaemonNet;
uint32_t &ConsoleFactionsWaitingMask = NetDaemonNet.factions_waiting_mask();
void wait_task() { event("wait_task"); }
void communicate(int a, int b, int mode) {
    CommunicatedFactions.push_back(a); CommunicatedOthers.push_back(b);
    assert(mode == 0);
    event("communicate");
}
void log_say(const char *message, int a, int b, int c) {
    assert(std::strcmp(message, "Initiating diplomatic channel") == 0);
    assert(c == 0);
    LoggedFactions.push_back(a); LoggedOthers.push_back(b);
    event("log");
    if (FactionAfterFirstLog != 0) {
        *FactionOwner = FactionAfterFirstLog;
        FactionAfterFirstLog = 0;
    }
    if (HumanMaskAfterFirstLog >= 0) {
        reinterpret_cast<uint8_t *>(&LockEnableMask)[0] =
            static_cast<uint8_t>(HumanMaskAfterFirstLog);
        HumanMaskAfterFirstLog = -1;
    }
}

char TamperWriteApplicationKey[] = "application";
char TamperWriteClass[1];
char TamperWriteValueMVM[] = "MVM";
char TamperWriteValueMVB[] = "MVB";
struct TamperRecord { int words[6]; };
int TamperRegistryCursor;
TamperRecord TamperRegistryRecords[50];
int RegistryResult;
std::vector<std::string> RegistryCalls;
const BYTE *RegistryMvmData;
const BYTE *RegistryMvbData;
LONG RegCreateKeyExA(HKEY root, LPCSTR key, DWORD, char *, DWORD, DWORD,
                     void *, HKEY *out, DWORD *) {
    assert(root == HKEY_LOCAL_MACHINE);
    assert(std::strcmp(key, "application") == 0);
    RegistryCalls.push_back("create"); *out = reinterpret_cast<HKEY>(1);
    return RegistryResult;
}
LONG RegSetValueExA(HKEY, LPCSTR name, DWORD, DWORD type,
                    const BYTE *data, DWORD size) {
    assert(type == REG_BINARY);
    if (std::strcmp(name, "MVM") == 0) {
        assert(size == 4); RegistryMvmData = data;
        RegistryCalls.push_back("MVM:4");
    } else {
        assert(std::strcmp(name, "MVB") == 0 && size == 1200);
        RegistryMvbData = data; RegistryCalls.push_back("MVB:1200");
    }
    return 0;
}
LONG RegCloseKey(HKEY) { RegistryCalls.push_back("close"); return 0; }

typedef int (__cdecl *func_sound_dll_release_result)(void *);
void *SoundDllProcs[11];
int WaveDeviceReleaseGuard;
int ShutdownClearsGuard;
typedef void *HMODULE;
HMODULE SoundDllModule;
std::vector<std::string> SoundEvents;
int release_callback(void *p) {
    SoundEvents.push_back("release-device"); assert(p != 0); return 9;
}
void shutdown_callback() {
    SoundEvents.push_back("sound-dll-shutdown");
    if (ShutdownClearsGuard) WaveDeviceReleaseGuard = 0;
}
void free_library(HMODULE) { SoundEvents.push_back("free-library"); }

void *TrackedNames[2];
bool TrackedNameFreed[2];
void *operator new(size_t size) throw(std::bad_alloc) {
    void *p = std::malloc(size);
    if (p == 0) throw std::bad_alloc();
    return p;
}
void operator delete(void *p) throw() {
    if (p == TrackedNames[0] && !TrackedNameFreed[0]) {
        SoundEvents.push_back("free-name");
        TrackedNameFreed[0] = true;
    } else if (p == TrackedNames[1] && !TrackedNameFreed[1]) {
        SoundEvents.push_back("free-name");
        TrackedNameFreed[1] = true;
    }
    std::free(p);
}
struct Sound;
Sound *SoundChainHead;
struct Sound {
    void *device_;
    void *fname_;
    Sound *next_;
    int id_;
    int detach() {
        SoundEvents.push_back("detach");
        assert(SoundChainHead == this);
        SoundChainHead = next_;
        return 0;
    }
};
struct Midi_Device { int release() { SoundEvents.push_back("midi-release"); return 0; } } g_MIDI_DEVICE;
struct WaveDevice { void release() { SoundEvents.push_back("wave-release"); } } WaveDeviceGlobal;
struct Wave_In_Device { int release() { SoundEvents.push_back("wavein-release"); return 0; } } g_WAVE_IN_DEVICE;
void unload_sound_dll() {
    WaveDeviceReleaseGuard = 0;
    if (SoundDllModule) { free_library(SoundDllModule); SoundDllModule = 0; }
    std::memset(SoundDllProcs, 0, sizeof(SoundDllProcs));
    SoundEvents.push_back("unload");
}

struct Win { virtual ~Win() {} virtual void show(int visible) { assert(visible == 1); } };
struct GraphicWin : Win { void show(int value) { assert(value == 1); event("opening-show"); } };
struct Console : GraphicWin {
    int faction_id_;
    void close() { event("console"); }
} ConsoleGlobal;
Win *BasePopDefaultField322C;
GraphicWin *OpeningWindow;
GraphicWin *&opening_window() { return OpeningWindow; }
struct DesignWin { void close() { event("design"); } } TutWinDesWindow;
struct MultiDebug { void close() { event("debug"); } } WinBubbleWindow;
struct Path { void shutdown() { event("path"); } } Paths;
void map_shutdown() { event("map"); }
void close_opening() { event("wrong-close-opening"); }
struct MapWin : GraphicWin {
    ~MapWin() { event("mapwin-delete"); }
    void show(int value) { assert(value == 1); event("mapwin-show"); }
};
MapWin *MapWinTable[8];
const size_t MapWinTableSlots = 8;

/* CANDIDATE_BODIES */

void reset() {
    Events.clear(); RegistryCalls.clear(); SoundEvents.clear();
    FactionOwner = &ConsoleGlobal.faction_id_;
    LoggedFactions.clear(); LoggedOthers.clear();
    OpenFactions.clear(); OpenOthers.clear();
    CommunicatedFactions.clear(); CommunicatedOthers.clear();
    std::memset(StringTemp, 0, sizeof StringTemp);
    IsMultiplayerNet = 0; ConsoleGlobal.faction_id_ = 2;
    ConsoleControlTurnActive = 0; ConsoleFactionsWaitingMask = 0;
    UnkGlobal0093A934 = 0; LockEnableMask = 0;
    FactionAfterFirstLog = 0; HumanMaskAfterFirstLog = -1;
    FactionAfterFirstOpen = 0;
    std::memset(FactionDiplomacyFlags, 0, sizeof FactionDiplomacyFlags);
    NetDaemonNet.OpenResult = 0; RegistryResult = ERROR_SUCCESS;
    RegistryMvmData = RegistryMvbData = 0;
    TamperRegistryCursor = 11;
    std::memset(TamperRegistryRecords, 0, sizeof TamperRegistryRecords);
    std::memset(SoundDllProcs, 0, sizeof SoundDllProcs);
    SoundChainHead = 0; WaveDeviceReleaseGuard = 0; SoundDllModule = 0;
    ShutdownClearsGuard = 0;
    TrackedNames[0] = TrackedNames[1] = 0;
    TrackedNameFreed[0] = TrackedNameFreed[1] = false;
    OpeningWindow = 0; BasePopDefaultField322C = 0;
    for (int i = 0; i < 8; ++i) MapWinTable[i] = 0;
}

int main(int argc, char **argv) {
    assert(argc == 2);
    reset();
    const std::string which(argv[1]);
    if (which == "space") {
        std::strcpy(StringTemp, "star"); space();
        assert(std::strcmp(StringTemp, "star ") == 0);
    } else if (which == "wait2") {
        IsMultiplayerNet = 1; wait_2();
        assert(Events == "net_tasks|");
        Events.clear(); IsMultiplayerNet = 0; wait_2();
        assert(Events == "wait_task|");
    } else if (which == "waitloop") {
        IsMultiplayerNet = 1; FactionDiplomacyFlags[3] = 1;
        NetDaemonNet.OpenResult = 0; wait_loop();
        assert(Events == "net_tasks|log|open23|communicate|");
        Events.clear(); LockEnableMask = 1u << 3; wait_loop();
        assert(Events == "net_tasks|");
        Events.clear(); LockEnableMask = 0; ConsoleControlTurnActive = 1;
        wait_loop(); assert(Events == "net_tasks|");
        Events.clear(); ConsoleControlTurnActive = 0; NetDaemonNet.OpenResult = 1;
        wait_loop(); assert(Events == "net_tasks|log|open23|");
        Events.clear(); NetDaemonNet.OpenResult = 0; UnkGlobal0093A934 = 1;
        wait_loop(); assert(Events == "net_tasks|");
        UnkGlobal0093A934 = 0; ConsoleFactionsWaitingMask = 1 << 2;
        Events.clear(); wait_loop(); assert(Events == "net_tasks|");
        Events.clear(); IsMultiplayerNet = 0; wait_loop();
        assert(Events == "wait_task|");
    } else if (which == "waitloop-callback-reloads") {
        IsMultiplayerNet = 1;
        NetDaemonNet.OpenResult = 0;
        FactionDiplomacyFlags[3] = 1;
        FactionDiplomacyFlags[4] = 1;
        FactionDiplomacyFlags[6] = 1;
        FactionAfterFirstLog = 1;
        HumanMaskAfterFirstLog = 1u << 4;
        FactionAfterFirstOpen = 5;
        wait_loop();
        assert(Events ==
               "net_tasks|log|open-other|communicate|log|open-other|communicate|");
        assert(LoggedOthers.size() == 2 && LoggedOthers[0] == 3 &&
               LoggedOthers[1] == 6);
        assert(LoggedFactions[0] == 2 && LoggedFactions[1] == 5);
        assert(OpenFactions.size() == 2 && OpenFactions[0] == 1 &&
               OpenFactions[1] == 5);
        assert(OpenOthers[0] == 3 && OpenOthers[1] == 6);
        assert(CommunicatedFactions.size() == 2 &&
               CommunicatedFactions[0] == 5 && CommunicatedFactions[1] == 5);
        assert(CommunicatedOthers[0] == 3 && CommunicatedOthers[1] == 6);
    } else if (which == "tamper-ok") {
        set_tamper();
        assert(RegistryCalls.size() == 4);
        assert(RegistryCalls[0] == "create" && RegistryCalls[1] == "MVM:4");
        assert(RegistryCalls[2] == "MVB:1200" && RegistryCalls[3] == "close");
        assert(RegistryMvmData == reinterpret_cast<const BYTE *>(&TamperRegistryCursor));
        assert(RegistryMvbData == reinterpret_cast<const BYTE *>(TamperRegistryRecords));
    } else if (which == "tamper-fail") {
        RegistryResult = 5; set_tamper();
        assert(RegistryCalls.size() == 1 && RegistryCalls[0] == "create");
    } else if (which == "release-guard") {
        assert(release_sound() == 1 && SoundEvents.empty());
    } else if (which == "release-full") {
        int a = 4, b = 5;
        char *name_a = static_cast<char *>(::operator new(8));
        char *name_b = static_cast<char *>(::operator new(8));
        assert(name_a != 0 && name_b != 0);
        TrackedNames[0] = name_a; TrackedNames[1] = name_b;
        TrackedNameFreed[0] = TrackedNameFreed[1] = false;
        Sound one = {&a, name_a, 0, 1}; Sound two = {&b, name_b, 0, 2};
        one.next_ = &two; SoundChainHead = &one; WaveDeviceReleaseGuard = 1;
        SoundDllModule = reinterpret_cast<HMODULE>(7);
        SoundDllProcs[1] = reinterpret_cast<void *>(&release_callback);
        SoundDllProcs[3] = reinterpret_cast<void *>(&shutdown_callback);
        assert(release_sound() == 0);
        assert(SoundChainHead == 0 && one.device_ == 0 && two.device_ == 0);
        assert(one.fname_ == 0 && two.fname_ == 0 && WaveDeviceReleaseGuard == 0);
        assert(SoundEvents[0] == "sound-dll-shutdown");
        assert(SoundEvents[1] == "release-device" && SoundEvents[2] == "free-name");
        assert(SoundEvents[3] == "detach" && SoundEvents[4] == "release-device");
        assert(SoundEvents[5] == "free-name" && SoundEvents[6] == "detach");
        assert(SoundEvents[7] == "midi-release" && SoundEvents[8] == "wave-release");
        assert(SoundEvents[9] == "wavein-release" && SoundEvents[10] == "free-library");
        assert(SoundEvents[11] == "unload");
        for (unsigned i = 0; i < 11; ++i) assert(SoundDllProcs[i] == 0);
    } else if (which == "release-callback-clears-guard") {
        int handle = 4;
        char *name = static_cast<char *>(::operator new(8));
        TrackedNames[0] = name;
        Sound one = {&handle, name, 0, 1};
        SoundChainHead = &one; WaveDeviceReleaseGuard = 1;
        ShutdownClearsGuard = 1;
        SoundDllProcs[1] = reinterpret_cast<void *>(&release_callback);
        SoundDllProcs[3] = reinterpret_cast<void *>(&shutdown_callback);
        assert(release_sound() == 0);
        assert(one.device_ == 0 && one.fname_ == 0 && SoundChainHead == 0);
        for (unsigned i = 0; i < SoundEvents.size(); ++i)
            assert(SoundEvents[i] != "release-device");
    } else if (which == "system-close") {
        GraphicWin opening; MapWin primary; MapWin first; MapWin third;
        OpeningWindow = &opening;
        MapWinTable[0] = &primary;
        MapWinTable[1] = &first; MapWinTable[3] = &third;
        system_close();
        assert(MapWinTable[0] == &primary);
        assert(MapWinTable[1] == 0 && MapWinTable[3] == 0);
        assert(OpeningWindow == 0 && BasePopDefaultField322C == &ConsoleGlobal);
        assert(Events == "opening-show|design|debug|console|path|map|mapwin-show|mapwin-show|");
    } else {
        return 2;
    }
    return 0;
}
