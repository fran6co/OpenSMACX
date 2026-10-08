// C++98 host controls for member.inc. This models only the current typed
// interfaces used by the method; it does not copy game bytes or replace game
// owners. Compile from this directory with a C++98 host compiler.
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

typedef unsigned int uint32_t;
const int MaxPlayerNum = 8;

struct Player {
  int is_leader_female;
  char title_leader[32];
  char name_leader[32];
  int noun_gender;
  int is_noun_plural;
  char noun_faction[32];
};

struct PlayerData {
  unsigned int diplo_treaties[8];
  int energy_reserves;
};

Player Players[MaxPlayerNum];
PlayerData PlayersData[MaxPlayerNum];
struct GameDataStateProjection {
  struct Fields { struct LockMasks { unsigned int enable_mask_; } lock_masks_; } fields_;
} GameDataStateStorage;
int IsMultiplayerNet;
int IsMultiplayerPBEM;
int Language;
int GenderDefault;
int PluralityDefault;
int DiploOffersDirty;
int ParseNumTable[10];
char StringTemp[1032];
char PopupStartCaption[256];
char *ScriptPopupFile;
int energy_prompt_result;
int energy_prompt_value;
int x_pop_result;
int popup_exec_result;
int diplo_lock_result;
int last_message_opcode;
int message_count;
int last_x_pop_return;
int last_net_pact_my;
int last_net_pact_other;
int net_pact_count;
int unlock_count;
int last_parse_file_was_script;
int last_parse_section_was_attack;
int last_parse_gender;
int last_parse_plurality;
int last_chat_id;
int chat_count;
int popup_construct_count;
int popup_destruct_count;
int popup_close_count;
int popup_start_count;
int popup_exec_count;
char last_popup_label[64];
char last_x_pop_label[64];
int last_x_pop_used_wait;
int last_offer_side[32];
int last_offer_type[32];
int last_offer_value[32];
int add_offer_count;
int do_it_count;
int release_modal_count;
int redraw_count;
int random_values[32];
int random_value_count;
int random_value_index;
int selected_tech[MaxPlayerNum][0x59];
std::vector<std::string> dialog_rows;

int test_rand(void) {
  if (random_value_index >= random_value_count) return 0;
  return random_values[random_value_index++];
}

struct Console {
  void chat(int faction_id) { ++chat_count; last_chat_id = faction_id; }
} ConsoleGlobal;

struct Dialogs {
  int item(char *text, int id) {
    char row[1200];
    std::sprintf(row, "%d:%s", id, text);
    dialog_rows.push_back(row);
    return 0;
  }
};

struct Popup {
  Dialogs dialogs_;
  Popup() { ++popup_construct_count; }
  ~Popup() { close(); ++popup_destruct_count; }
  void close() { ++popup_close_count; }
  void start(char *, const char *label, int, char *, int) {
    ++popup_start_count;
    std::strncpy(last_popup_label, label, sizeof(last_popup_label) - 1);
    last_popup_label[sizeof(last_popup_label) - 1] = 0;
  }
  Dialogs &dialogs() { return dialogs_; }
  int exec(int, int (*)()) { ++popup_exec_count; return popup_exec_result; }
};

struct NetMessage {
  void pop(const char *, int, int, const char *) { ++message_count; }
} NetMsgPrimary;

struct DiploWin {
  int offer_count_[2];
  int offer_type_[2][8];
  int offer_value_[2][8];
  int faction_id_;
  int faction_id_with_;
  int field_A24_;
  int field_A28_;

  DiploWin() : faction_id_(0), faction_id_with_(1), field_A24_(0), field_A28_(0) {
    clear();
  }
  void clear() {
    offer_count_[0] = offer_count_[1] = 0;
    std::memset(offer_type_, 0, sizeof(offer_type_));
    std::memset(offer_value_, 0, sizeof(offer_value_));
  }
  void add_offer(int side, int type, int value) {
    assert(side >= 0 && side < 2);
    const int count = offer_count_[side];
    assert(count < 8);
    last_offer_side[add_offer_count] = side;
    last_offer_type[add_offer_count] = type;
    last_offer_value[add_offer_count] = value;
    ++add_offer_count;
    offer_type_[side][count] = type;
    offer_value_[side][count] = value;
    ++offer_count_[side];
    DiploOffersDirty = 1;
  }
  void do_it() { ++do_it_count; }
  void release_modal() { ++release_modal_count; }
  void redraw() { ++redraw_count; }
  void on_button_clicked(int button_id);
};

int message_data(int opcode, int, int, int, int, int) {
  last_message_opcode = opcode;
  ++message_count;
  return 0;
}

int has_tech(int tech_id, int faction_id) {
  return selected_tech[faction_id][tech_id];
}

int is_alive(uint32_t faction_id) {
  return (GameDataStateStorage.fields_.lock_masks_.enable_mask_ >> 8) & (1u << faction_id);
}

void say_tech(char *output, int tech_id, int) {
  char text[32];
  std::sprintf(text, "TECH%d", tech_id);
  std::strcat(output, text);
}

char *label_get(int label_id) {
  assert(label_id == 0x132);
  return const_cast<char *>("END TREATIES");
}

int pop_wait() { return 0; }

int X_pop_ask_number(const char *label, int, int (*)()) {
  assert(std::strcmp(label, "DIPLOENERGY") == 0);
  ParseNumTable[0] = energy_prompt_value;
  return energy_prompt_result;
}

int X_pop(const char *label, int (*callback)()) {
  std::strncpy(last_x_pop_label, label, sizeof(last_x_pop_label) - 1);
  last_x_pop_label[sizeof(last_x_pop_label) - 1] = 0;
  last_x_pop_return = x_pop_result;
  last_x_pop_used_wait = callback == pop_wait;
  return x_pop_result;
}

int parse_says(int id, const char *, int, int) {
  assert(id >= 0 && id <= 2);
  last_parse_gender = GenderDefault;
  last_parse_plurality = PluralityDefault;
  return 0;
}

void parse_it(char *file, const char *section) {
  last_parse_file_was_script = file == ScriptPopupFile;
  last_parse_section_was_attack = std::strcmp(section, "ATTACKSTRING") == 0;
  std::strcat(StringTemp, " ATTACK");
}

int diplo_lock(int) { return diplo_lock_result; }
void diplo_unlock() { ++unlock_count; }
void net_pact_ends(int my, int other, int) {
  ++net_pact_count;
  last_net_pact_my = my;
  last_net_pact_other = other;
}

#define rand test_rand
// @@DIPLO_BUTTON_METHOD@@
#undef rand

static void reset_controls(DiploWin &window) {
  window.clear();
  window.faction_id_ = 0;
  window.faction_id_with_ = 1;
  window.field_A24_ = 0;
  window.field_A28_ = 0;
  std::memset(Players, 0, sizeof(Players));
  std::memset(PlayersData, 0, sizeof(PlayersData));
  std::memset(selected_tech, 0, sizeof(selected_tech));
  std::memset(last_offer_side, 0, sizeof(last_offer_side));
  std::memset(last_offer_type, 0, sizeof(last_offer_type));
  std::memset(last_offer_value, 0, sizeof(last_offer_value));
  std::memset(StringTemp, 0, sizeof(StringTemp));
  std::memset(last_popup_label, 0, sizeof(last_popup_label));
  std::memset(last_x_pop_label, 0, sizeof(last_x_pop_label));
  dialog_rows.clear();
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0;
  IsMultiplayerNet = IsMultiplayerPBEM = Language = 0;
  GenderDefault = PluralityDefault = 0;
  DiploOffersDirty = 0;
  ParseNumTable[0] = 0;
  energy_prompt_result = 1;
  energy_prompt_value = 0;
  x_pop_result = 1;
  popup_exec_result = -1;
  diplo_lock_result = 0;
  last_message_opcode = message_count = 0;
  last_x_pop_return = last_x_pop_used_wait = 0;
  last_net_pact_my = last_net_pact_other = net_pact_count = 0;
  unlock_count = last_parse_file_was_script = last_parse_section_was_attack = 0;
  last_parse_gender = last_parse_plurality = 0;
  last_chat_id = chat_count = 0;
  popup_construct_count = popup_destruct_count = popup_close_count = 0;
  popup_start_count = popup_exec_count = 0;
  add_offer_count = do_it_count = release_modal_count = redraw_count = 0;
  random_value_count = random_value_index = 0;
  std::memset(random_values, 0, sizeof(random_values));
  std::strcpy(PopupStartCaption, "SCRIPT");
  ScriptPopupFile = PopupStartCaption;
}

static void controls_dispatch_and_simple_offers() {
  DiploWin window;
  reset_controls(window);
  window.on_button_clicked(-2);
  window.on_button_clicked(4);
  assert(release_modal_count == 2);
  assert(popup_construct_count == 2 && popup_destruct_count == 2 &&
         popup_close_count == 2);

  reset_controls(window);
  window.on_button_clicked(-1);
  window.on_button_clicked(13);  // beyond the native table's upper guard
  assert(release_modal_count == 0 && message_count == 0 && add_offer_count == 0);
  assert(popup_construct_count == 2 && popup_destruct_count == 2 &&
         popup_close_count == 2);

  reset_controls(window);
  window.offer_count_[0] = 3;
  window.on_button_clicked(0);
  assert(window.offer_count_[0] == 0);
  assert(last_message_opcode == 0x1508 && release_modal_count == 1);
  assert(popup_construct_count == 1 && popup_destruct_count == 1 &&
         popup_close_count == 1);

  reset_controls(window);
  window.on_button_clicked(9);
  window.on_button_clicked(10);
  window.on_button_clicked(11);
  assert(add_offer_count == 3);
  assert(last_offer_side[0] == 0 && last_offer_type[0] == 4);
  assert(last_offer_side[1] == 0 && last_offer_type[1] == 3);
  assert(last_offer_side[2] == 0 && last_offer_type[2] == 2);

  reset_controls(window);
  window.field_A28_ = 1;
  IsMultiplayerPBEM = 1;
  window.on_button_clicked(2);
  assert(DiploOffersDirty == 1 && window.field_A24_ == 1);
  assert(redraw_count == 1 && do_it_count == 1);
  assert(last_message_opcode == 0x150A);

  reset_controls(window);
  IsMultiplayerNet = 1;
  window.on_button_clicked(3);
  assert(DiploOffersDirty == 1 && window.field_A24_ == 2);
  assert(window.offer_count_[1] == 0 && add_offer_count == 0);
  assert(popup_destruct_count == 1);

  reset_controls(window);
  random_values[0] = 1;  // energy offer branch
  random_values[1] = 10; // energy amount: (10*5+5)*5 = 275
  random_values[2] = 3;  // no optional treaty offer
  random_values[3] = 0;  // native writes field_A28_ = 1
  random_value_count = 4;
  window.on_button_clicked(3);
  assert(window.offer_count_[1] == 1 && window.offer_type_[1][0] == 1 &&
         window.offer_value_[1][0] == 275);
  assert(window.field_A28_ == 1 && redraw_count == 2);

  reset_controls(window);
  random_values[0] = 0;  // technology offer branch
  random_values[1] = 5;  // first technology
  random_values[2] = 0;  // no second technology
  random_values[3] = 3;  // no optional treaty offer
  random_values[4] = 1;  // native raw write at this+0xA2C
  random_value_count = 5;
  window.on_button_clicked(3);
  assert(window.offer_count_[1] == 1 && window.offer_type_[1][0] == 0 &&
         window.offer_value_[1][0] == 5);
  assert(window.offer_count_[0] == 0 && window.offer_type_[0][0] == 2 &&
         redraw_count == 2);

  reset_controls(window);
  random_values[0] = 1;
  random_values[1] = 0;
  random_values[2] = 3;
  random_values[3] = 2;  // native third arm leaves both fields unchanged
  random_value_count = 4;
  window.on_button_clicked(3);
  assert(window.field_A28_ == 0 && window.offer_type_[0][0] == 0 &&
         redraw_count == 2);
}

static void controls_header_literals_and_popup_return() {
  DiploWin window;
  reset_controls(window);
  selected_tech[0][7] = 1;
  Language = 0;
  popup_exec_result = -1;
  window.on_button_clicked(5);
  assert(std::strcmp(last_popup_label, "DIPLOTECH") == 0);
  assert(!dialog_rows.empty());
  assert(dialog_rows[0].find("7:*TECH7 (already has)") == 0);
  assert(window.offer_count_[0] == 0);  // canceled popup must not add an offer
  assert(popup_exec_count == 1 && popup_destruct_count == 1);

  reset_controls(window);
  selected_tech[0][7] = 1;
  popup_exec_result = 2;
  window.on_button_clicked(5);
  assert(std::strcmp(last_popup_label, "DIPLOTECH") == 0);
  assert(window.offer_count_[0] == 1);
  assert(window.offer_type_[0][0] == 5 && window.offer_value_[0][0] == 2);

  reset_controls(window);
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << 2;
  Players[2].title_leader[0] = 'T';
  Players[2].name_leader[0] = 'N';
  Players[2].noun_faction[0] = 'F';
  PlayersData[0].diplo_treaties[2] = 0;
  PlayersData[1].diplo_treaties[2] = 0;
  PlayersData[2].diplo_treaties[0] = 8;
  PlayersData[2].diplo_treaties[1] = 8;
  popup_exec_result = 2;
  window.on_button_clicked(8);
  assert(std::strcmp(last_popup_label, "DIPLOVENDETTA") == 0);
  assert(last_parse_file_was_script && last_parse_section_was_attack);
  // The original tests my->faction and other->faction. Opposite directed
  // entries are set, so a symmetric-receiver mutation would drop this row.
  assert(window.offer_count_[0] == 1 && window.offer_type_[0][0] == 5 &&
         window.offer_value_[0][0] == 2);

  reset_controls(window);
  // The chooser with no eligible rows returns before exec, but its automatic
  // Popup still closes and destructs on that early path.
  window.offer_count_[0] = 1;
  window.offer_type_[0][0] = 0;
  window.offer_value_[0][0] = 0x270F;
  window.on_button_clicked(5);
  assert(popup_start_count == 1 && popup_exec_count == 0);
  assert(popup_construct_count == 1 && popup_destruct_count == 1 &&
         popup_close_count == 1);
}

static void controls_automatic_offer_receiver_and_energy_return() {
  DiploWin window;
  reset_controls(window);
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 1u << (8 + 2);
  selected_tech[0][3] = 1;
  PlayersData[0].diplo_treaties[2] = 8;
  PlayersData[2].diplo_treaties[0] = 0;
  PlayersData[1].diplo_treaties[2] = 0;
  PlayersData[2].diplo_treaties[1] = 8; // opposite directed cell catches inversion
  window.on_button_clicked(7);
  // Native +0x441e10 reads REL(my, faction) and REL(other, faction), scans
  // existing entries against this tech id, then pushes `(tech, 0, 0)` in
  // reverse stack order; the C++ call is add_offer(0, 0, tech).
  bool saw_scanned_tech = false;
  bool saw_faction_tech = false;
  bool saw_ending = false;
  for (int i = 0; i < add_offer_count; ++i) {
    if (last_offer_side[i] == 0 && last_offer_type[i] == 0 &&
        last_offer_value[i] == 3) saw_scanned_tech = true;
    if (last_offer_side[i] == 0 && last_offer_type[i] == 0 &&
        last_offer_value[i] == 0x59 + 2) saw_faction_tech = true;
    if (last_offer_side[i] == 0 && last_offer_type[i] == 0 &&
        last_offer_value[i] == 0x270F) saw_ending = true;
  }
  assert(saw_scanned_tech && saw_faction_tech && saw_ending);

  reset_controls(window);
  window.offer_count_[0] = 1;
  window.offer_type_[0][0] = 1;
  window.offer_value_[0][0] = 12;
  PlayersData[0].energy_reserves = 40;
  energy_prompt_result = 1;
  energy_prompt_value = 90;
  window.on_button_clicked(6);
  assert(add_offer_count == 0 && popup_destruct_count == 1);

  reset_controls(window);
  window.offer_count_[0] = 1;
  window.offer_type_[0][0] = 1;
  window.offer_value_[0][0] = 12;
  PlayersData[0].energy_reserves = 40;
  energy_prompt_result = 0;
  energy_prompt_value = 90;
  window.on_button_clicked(6);
  assert(ParseNumTable[0] == 40);
  assert(add_offer_count == 1 && last_offer_side[0] == 0 &&
         last_offer_type[0] == 1 && last_offer_value[0] == 40);
}

static void controls_end_treaty_confirmation_return_state() {
  DiploWin window;
  reset_controls(window);
  PlayersData[0].diplo_treaties[1] = 0;
  window.on_button_clicked(12);
  assert(std::strcmp(last_x_pop_label, "DONTHAVEPACT") == 0);
  assert(net_pact_count == 0 && unlock_count == 0);

  reset_controls(window);
  PlayersData[0].diplo_treaties[1] = 1;
  x_pop_result = 0;  // native zero result cancels this confirmation
  window.on_button_clicked(12);
  assert(std::strcmp(last_x_pop_label, "REALLYENDPACT") == 0);
  assert(last_x_pop_used_wait && net_pact_count == 0);

  reset_controls(window);
  PlayersData[0].diplo_treaties[1] = 1;
  x_pop_result = 1;
  window.on_button_clicked(12);
  assert(net_pact_count == 1 && unlock_count == 1);
  assert(last_net_pact_my == 0 && last_net_pact_other == 1);
  assert(DiploOffersDirty == 1);
}

int main() {
  controls_dispatch_and_simple_offers();
  controls_header_literals_and_popup_return();
  controls_automatic_offer_receiver_and_energy_return();
  controls_end_treaty_confirmation_return_state();
  return 0;
}
