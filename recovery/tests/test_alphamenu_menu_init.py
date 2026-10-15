"""Actual-source menu population controls; service doubles are not game runtime proof."""

def test_alphamenu_menu_init_population():
    from pathlib import Path
    import subprocess
    import tempfile
    
    
    candidate = (Path(__file__).resolve().parents[2] / "src/alphamenu.cpp").read_text()
    signature = "int AlphaMenu::menu_init()"
    start = candidate.index(signature)
    opening = candidate.index("{", start)
    depth = 0
    for position in range(opening, len(candidate)):
        if candidate[position] == "{":
            depth += 1
        elif candidate[position] == "}":
            depth -= 1
            if depth == 0:
                body = candidate[start : position + 1]
                break
    else:
        raise AssertionError("unclosed AlphaMenu::menu_init")
    
    prefix = r"""
    #include <cassert>
    #include <cstdio>
    #include <cstring>
    #include <string>
    #include <vector>
    using BOOL = int;
    static const unsigned int RULES_IRONMAN = 0x400;
    struct GameDataStateFields { unsigned int game_rules_; };
    struct GameDataStateWindow { GameDataStateFields fields_; };
    GameDataStateWindow GameDataStateStorage = {};
    struct RulesTerraforming { char *name; char *name_sea; int req; int req_sea;
      unsigned int bit; unsigned int incompatible; int rate; char *shortcuts; };
    RulesTerraforming Terraforming[20]{};
    struct RulesOrder { char *order; char *order_sea; char *letter; };
    RulesOrder Order[30]{};
    char StringTemp[1032]{};
    struct Strings {
      char *get(int id) {
        static char text[32];
        std::snprintf(text, sizeof(text), "K%d", id);
        return text;
      }
      char *get(const char *key) { return const_cast<char *>(key); }
    } StringTable;
    int read_rules_calls = 0;
    int read_rules(BOOL) { ++read_rules_calls; return 1; }
    std::vector<std::string> opened_sections;
    int fail_section_index = -1;
    int text_open(const char *file, const char *section) {
      assert(std::strcmp(file, "MENU") == 0);
      opened_sections.push_back(section);
      return static_cast<int>(opened_sections.size()) - 1 == fail_section_index;
    }
    int text_index = 0;
    char *text_get() {
      static char value[32];
      std::snprintf(value, sizeof(value), "T%d", text_index++);
      return value;
    }
    struct Menu {
      std::vector<std::string> menus, items, separators, hidden, disabled;
      int add_menu(int id, char *text, int justify) {
        menus.push_back(std::to_string(id) + ":" + text + ":" + std::to_string(justify)); return 0;
      }
      int add_menu_item(int menu, int id, char *text) {
        items.push_back(std::to_string(menu) + ":" + std::to_string(id) + ":" + text); return 0;
      }
      int add_separator(int menu, int id) {
        separators.push_back(std::to_string(menu) + ":" + std::to_string(id)); return 0;
      }
      int hide_menu_item(int menu, int id) {
        hidden.push_back(std::to_string(menu) + ":" + std::to_string(id)); return 0;
      }
      int disable_menu_item(int menu, int id) {
        disabled.push_back(std::to_string(menu) + ":" + std::to_string(id)); return 0;
      }
    };
    struct AlphaMenu : Menu {
      int checks = 0, adjusted = 0, checked_id = 0;
      int menu_init();
      void check(int id) { ++checks; checked_id = id; }
      virtual void adjust() { ++adjusted; }
    };
    """
    
    tests = r"""
    static const char *sections[] = {
      "GAME", "HQ", "NETWORK", "MAP", "ACTION",
      "TERRAFORM", "SCENARIO", "EDITOR", "HELP"
    };
    void success_control(bool ironman) {
      fail_section_index = -1; opened_sections.clear(); text_index = 0;
      read_rules_calls = 0; GameDataStateStorage.fields_.game_rules_ = ironman ? RULES_IRONMAN : 0;
      AlphaMenu menu;
      for (int i = 0; i != 20; ++i) {
        Terraforming[i].shortcuts = const_cast<char *>("shortcut");
        Order[i + 4].order = const_cast<char *>("order");
      }
      assert(menu.menu_init() == 0);
      assert(read_rules_calls == 1 && opened_sections.size() == 9);
      for (int i = 0; i != 9; ++i) assert(opened_sections[i] == sections[i]);
      if (menu.items.size() != 181 || menu.separators.size() != 41) {
        std::printf("counts: menus=%lu items=%lu separators=%lu\\n",
                    static_cast<unsigned long>(menu.menus.size()),
                    static_cast<unsigned long>(menu.items.size()),
                    static_cast<unsigned long>(menu.separators.size()));
      }
      assert(menu.menus.size() == 9 && menu.items.size() == 199);
      assert(menu.separators.size() == 43 && menu.disabled.size() == 1);
      assert(menu.checks == 1 && menu.checked_id == -1 && menu.adjusted == 1);
      assert(menu.hidden.size() == (ironman ? 9u : 8u));
      if (ironman) assert(menu.hidden.back() == "0:90");
      assert(menu.items[91] == "9:2304:order|shortcut");
      assert(menu.items[109] == "9:2322:order|shortcut");
      assert(menu.items.back() == "7:1873:T188");
    }
    void open_failure_controls() {
      for (int failed = 0; failed != 9; ++failed) {
        fail_section_index = failed; opened_sections.clear(); text_index = 0;
        AlphaMenu menu;
        assert(menu.menu_init() == 1);
        assert(opened_sections.size() == static_cast<std::size_t>(failed + 1));
        assert(menu.checks == 0 && menu.adjusted == 0);
      }
    }
    int main() {
      success_control(false);
      success_control(true);
      open_failure_controls();
    }
    """
    
    program = prefix + "\n" + body + "\n" + tests
    with tempfile.TemporaryDirectory(prefix="alphamenu-init-host-") as temp:
        binary = str(Path(temp) / "control")
        built = subprocess.run(
            ["g++", "-std=c++11", "-Wall", "-Wextra", "-Werror", "-x", "c++", "-", "-o", binary],
            input=program,
            text=True,
            capture_output=True,
            timeout=30,
        )
        if built.returncode:
            raise SystemExit(built.stderr)
        result = subprocess.run([binary], text=True, capture_output=True, timeout=30)
        if result.returncode:
            raise SystemExit(result.stdout + result.stderr or f"host control exited {result.returncode}")
        print("PASS: extracted AlphaMenu::menu_init ordered success/early-failure controls")
    
