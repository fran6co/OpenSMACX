"""Actual-source controls for technology acquisition phases and lifetimes."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_body(signature="void __cdecl tech_achieved("):
    source = (ROOT / "src/technology.cpp").read_text()
    start = source.index(signature)
    open_brace = source.index("{", start)
    depth = 0
    for end in range(open_brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start : end + 1]
    raise AssertionError("unterminated tech_achieved body")


def run_host(tmp_path, body):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host g++ required")
    source = r'''
        #include <cstdint>
        #include <cstring>
        #include <cstdio>
        #define __cdecl
        constexpr int MaxReactorNum = 4;
        constexpr int MaxVehProtoFactionNum = 64;
        constexpr int MaxPlayerNum = 8;
        constexpr int MaxSocialCatNum = 4;
        constexpr uint16_t PROTO_ACTIVE = 1;
        constexpr uint32_t TFLAG_SECRETS = 1;
        constexpr uint32_t TFLAG_REVEALS_MAP = 8;
        constexpr int RFLAG_TECHSHARE = 0x20, RFLAG_ALIEN = 0x80;
        constexpr int DTREATY_HAVE_INFILTRATOR = 0x1000, DTREATY_PACT = 1;
        constexpr int FacilitySPStart = 70;
        constexpr int FAC_PLANETARY_DATALINKS = 11;
        constexpr int FALSE = 0;
        int popup_destructions = 0, popup_inits = 0, popup_starts = 0,
            popup_strings = 0, popup_width = 0, popup_flags = 0;
        char popup_file[32] = {}, popup_caption[128] = {};
        char popup_string_args[3][16] = {};
        char PopupStartCaption[256] = "popup-caption";
        char ScriptPopupFileText[16] = "SCRIPT";
        char *ScriptPopupFile = ScriptPopupFileText;
        char popup_last_text_file[256] = {}, popup_last_text[256] = {};
        int popup_execs = 0;
        int xpop_result = 0;
        int spritebox_constructs = 0, spritebox_destructions = 0;
        int spritebox_calls = 0, spritebox_arg0_nonnull = 0;
        char *spritebox_sprite_arg = nullptr;
        int event_order = 0, spritebox_ctor_order = 0, spritebox_call_order = 0;
        int spritebox_dtor_order = 0, popup_blurb_order = 0;
        int popup_caption_order = 0;
        int secret_popup_order = 0, secret_log_order = 0,
            desktop_update_order = 0;
        int newtech_calls = 0, newtech_tech = -1, newtech_other = -1;
        int fx_calls = 0, fx_last_id = -1, fx_order = 0;
        int help_info_calls = 0, help_info_tech = -1, help_info_faction = -1,
            help_info_order = 0;
        int netmsg_calls = 0, netmsg_duration = -1, netmsg_channel = -1;
        const char *netmsg_label = nullptr, *netmsg_target = nullptr;
        int popp_calls = 0, popp_flags = -1;
        const char *popp_caption = nullptr, *popp_label = nullptr,
                   *popp_picture = nullptr;
        const char *xpop_label = nullptr;
        int parse_say_ids[8] = {}, parse_say_inputs[8] = {}, parse_say_calls = 0;
        struct GraphicWin;
        struct Sprite { int identity = 0; };
        Sprite g_TECH_ICON_SPRITES[89];
        struct SpriteBox {
          SpriteBox() {
            ++spritebox_constructs; spritebox_ctor_order = ++event_order;
          }
          ~SpriteBox() {
            ++spritebox_destructions; spritebox_dtor_order = ++event_order;
          }
          int sprite(Sprite *first, char *second, int) {
            ++spritebox_calls;
            spritebox_call_order = ++event_order;
            spritebox_arg0_nonnull = first != nullptr;
            spritebox_sprite_arg = second;
            return 0;
          }
        };
        struct Popup {
          ~Popup() { ++popup_destructions; }
          int init(int, long) { ++popup_inits; return 0; }
          void set_width(int width) { popup_width = width; }
          int start(char *file, const char *caption, int, char *, int flags,
                    GraphicWin *) {
            ++popup_starts;
            popup_blurb_order = ++event_order;
            std::strncpy(popup_file, file, sizeof(popup_file) - 1);
            std::strncpy(popup_caption, caption, sizeof(popup_caption) - 1);
            popup_flags = flags;
            return 0;
          }
          int start(char *file, const char *caption, int, char *, int) {
            ++popup_starts;
            popup_caption_order = ++event_order;
            std::strncpy(popup_last_text_file, file,
                         sizeof(popup_last_text_file) - 1);
            std::strncpy(popup_last_text, caption,
                         sizeof(popup_last_text) - 1);
            return 0;
          }
          int string(char *text) {
            if (popup_strings < 3)
              std::strncpy(popup_string_args[popup_strings], text,
                           sizeof(popup_string_args[popup_strings]) - 1);
            ++popup_strings;
            return 0;
          }
          int exec(int, int (*)()) { ++popup_execs; return 0; }
        };
        struct NewTechWin { void exec(int tech, int other) {
          ++newtech_calls; newtech_tech = tech; newtech_other = other;
        } };
        NewTechWin NewTechWindow;
        void help_tech_info(Popup *, int tech, int faction) {
          ++help_info_calls; help_info_tech = tech;
          help_info_faction = faction; help_info_order = ++event_order;
        }
        struct FX { void play(int id) {
          ++fx_calls; fx_last_id = id; fx_order = ++event_order;
        } };
        FX g_FX;
        struct NetMsg {
          void pop(const char *label, int duration, int channel,
                   const char *target) {
            ++netmsg_calls;
            netmsg_label = label;
            netmsg_duration = duration;
            netmsg_channel = channel;
            netmsg_target = target;
          }
        } netmsg;
        NetMsg &NetMsgPrimary = netmsg;
        int synch_calls = 0, synch_opcode = -1, synch_d = -1, synch_flags = -1;
        struct NetDaemon {
          void synch(int16_t opcode, int, int, int, char *, int d,
                     int16_t flags) {
            ++synch_calls; synch_opcode = opcode; synch_d = d;
            synch_flags = flags;
          }
        } NetDaemonNet;
        struct PlayerData {
          uint32_t flags = 0;
          uint32_t traded_maps = 0;
          uint32_t diplo_treaties[8] = {};
          int diff_level = 0;
          int tech_id_researching = -1;
          int tech_achieved = 0;
          int tech_ranking = 0;
          int unk_26 = 0;
          int theory_of_everything = 0;
        };
        struct Player {
          char name_leader[24] = "leader";
          char noun_faction[24] = "xxxxFaction";
          char adj_name_faction[128] = "xxxxFaction Adjective";
          int noun_gender = 0;
          int is_noun_plural = 0;
          int rule_flags = 0;
          int rule_sharetech = 0;
          int faction_bonus_count = 0;
          int faction_bonus_id[8] = {};
          int faction_bonus_val1[8] = {};
          int faction_bonus_val2[8] = {};
        };
        struct RulesFacility { const char *name = nullptr; int preq_tech = -1; };
        struct RulesTechnology {
          uint32_t flags = 0;
          const char *name = nullptr;
          int preq_tech_1 = -1, preq_tech_2 = -1;
        };
        struct RulesWeapon { int8_t offense_rating = -1; };
        RulesWeapon Weapon[26];
        struct Map { uint8_t visibility = 0; uint32_t bit = 0; uint32_t bit2 = 0; uint32_t bit_visible[7] = {}; };
        struct Base {
          uint8_t visibility = 0;
          uint8_t faction_id_current = 0;
          int8_t population_size = 0;
          uint8_t faction_pop_size_intel[8] = {};
          int facilities[100] = {};
        };
        struct VehPrototype { char veh_name[32] = {}; uint32_t ability_flags = 0; uint8_t chassis_id = 0, weapon_id = 0, armor_id = 0, reactor_id = 0, carry_capacity = 0, cost = 0, plan = 0; int8_t unk_1 = 0; uint8_t obsolete_factions = 0; int8_t combat_factions = 0, icon_offset = 0, padding = 0; uint16_t flags = 0; int16_t preq_tech = -1; };
        struct RulesReactor { const char *name = nullptr; const char *name_short = nullptr; int16_t preq_tech = -1; uint16_t power = 0; };
        struct GraphicWin { int redraws = 0; void redraw() { ++redraws; } };
        int design_window_shutdowns = 0, prototype_upgrades = 0;
        struct DesignWin {
          void shut_that_badboy_down() { ++design_window_shutdowns; }
        };
        struct WorldWin : GraphicWin {};
        struct { int faction_id_ = 0; } ConsoleGlobal;
        PlayerData PlayersData[8];
        Player Players[8];
        RulesFacility Facility[100];
        struct TestSecretProject {
          int voice_of_planet = -1;
          int planetary_datalinks = -1;
          int empath_guild = -1;
        } SecretProject;
        RulesTechnology Technology[89];
        struct RulesAbility { const char *name = nullptr; int16_t preq_tech = -1; };
        RulesAbility Ability[29];
        struct RulesSocialCategory {
          int preq_tech[4] = {-1, -1, -1, -1};
          const char *name[4] = {};
        };
        RulesSocialCategory SocialCategories[4];
        Map Maps[8];
        Map *map_tiles() { return Maps; }
        uint32_t MapArea = 0;
        Base Bases[8];
        int BaseCurrentCount = 0;
        VehPrototype VehPrototypes[512];
        RulesReactor Reactor[4];
        uint32_t LockEnableMask = 0;
        int TurnCurrentNum = 0, TechAchievedGate = 0;
        int GamePreferences = 0, FirstProposedPrototype = -1;
        constexpr int PREF_BSC_AUTO_DESIGN_VEH = 0x4000;
        uint8_t GameTechAchieved[89] = {};
        int IsMultiplayerNet = 0;
        int ExpansionEnabled = 1, GovernorFaction = -1;
        int DiplomacyActive = 0, CommlinkPending = 0, TurnRedrawPending = 0;
        int UnkBitfield1 = 0;
        WorldWin WorldClimateWorldWin;
        int treaty_faction = -1, treaty_id = -1, treaty_kind = -1;
        int ambience_calls = 0, drawn_maps = 0, dirty_calls = 0,
            spotted[8] = {};
        int populate_pending_designs = 0;
        int tech_effects_calls = 0, interlude_calls = 0;
        int interlude_ids[8] = {}, interlude_modes[8] = {};
        int mutate_local_after_spot = 0, mutate_local_after_design = 0;
        int GenderDefault = 0, PluralityDefault = 0;
        char TechName[80] = {};
        char StringTemp[1032] = {};
        int pruned = -1, upgraded = -1;
        int label_offset_seen = -1, label_lookup_count = 0;
        const char *parsed_noun = nullptr;
        int own_gender_seen = -1, own_plural_seen = -1;
        int parsed_say_id = -1, parsed_say_input = -1, tech_popup_calls = 0;
        int map_notice_tech = -1;
        int social_avail_calls = 0, social_avail_category = -1,
            social_avail_model = -1, social_avail_faction = -1,
            social_avail_result = 1, social_popup_calls = 0,
            social_popup_result = 0, social_select_calls = 0;
        char *social_popup_caption = nullptr;
        const char *social_popup_label = nullptr;
        int log_calls = 0, logged_tech = -1, logged_faction = -1,
            logged_other_faction = -1;
        int secrets_calls = 0, secrets_faction = -1;
        int secret_log_calls = 0, secret_log_faction = -1,
            secret_log_tech = -1, secret_log_mask = -1;
        const char *secret_log_message = nullptr,
                   *secret_log_string_arg = nullptr;
        int desktop_updates = 0;
        const char *last_parse_says_one = nullptr;
        int proposed_faction = -1, proposed_chassis = -1, proposed_weapon = -1;
        int proposed_armor = -1, proposed_abilities = -1, proposed_reactor = -1;
        int proposed_category = -1;
        const char *proposed_name = nullptr;
        int design_new_calls = 0, design_new_factions[8] = {},
            design_new_protos[8] = {};
        int proposal_result = -1;
        constexpr int RULE_FREEFAC = 12;
        constexpr int RULE_FREEABIL = 15;
        constexpr int TRUE = 1;
        int ambience(int) { ++ambience_calls; return 0; }
        char *_itoa(int value, char *out, int radix) {
          (void)radix;
          std::sprintf(out, "%d", value);
          return out;
        }
        int considered_design_faction = -1;
        void consider_designs(uint32_t faction) {
          considered_design_faction = static_cast<int>(faction);
          if (populate_pending_designs) {
            VehPrototypes[69].flags = 0x81;
            VehPrototypes[70].flags = 0x101;
          }
          if (mutate_local_after_design) ConsoleGlobal.faction_id_ = 0;
        }
        int parse_say(int id, int input, int, int) {
          parsed_say_id = id;
          parsed_say_input = input;
          if (parse_say_calls < 8) {
            parse_say_ids[parse_say_calls] = id;
            parse_say_inputs[parse_say_calls] = input;
          }
          ++parse_say_calls;
          return 0;
        }
        int parse_says(int id, const char *text, int, int) {
          if (id == 1) last_parse_says_one = text;
          if (id == 2) parsed_noun = text;
          if (id == 1) {
            own_gender_seen = GenderDefault;
            own_plural_seen = PluralityDefault;
          }
          return 0;
        }
        const char *label_get(int offset) {
          ++label_lookup_count;
          label_offset_seen = offset;
          return "alien-name";
        }
        void say_tech(char *out, int id, int) { (void)id; out[0] = 'T'; out[1] = 0; }
        void log_say(const char *message, const char *leader, int tech,
                     int faction, int other_faction) {
          if (std::strcmp(message, "Learning a SECRET") == 0) {
            ++secret_log_calls; secret_log_message = message;
            secret_log_string_arg = leader; secret_log_faction = tech;
            secret_log_tech = faction; secret_log_mask = other_faction;
            secret_log_order = ++event_order;
            return;
          }
          if (std::strcmp(message, "Gaining a TECH") != 0 ||
              std::strcmp(leader, "leader") != 0) return;
          ++log_calls;
          logged_tech = tech;
          logged_faction = faction;
          logged_other_faction = other_faction;
        }
        int X_pop(const char *label, int (__cdecl *)()) {
          ++tech_popup_calls; xpop_label = label;
          if (std::strncmp(label, "THESECRET", 9) == 0)
            secret_popup_order = ++event_order;
          return xpop_result;
        }
        int x_pop_caption_label(char *caption, const char *label,
                                int (__cdecl *)()) {
          ++social_popup_calls; social_popup_caption = caption;
          social_popup_label = label;
          const int result = social_popup_result;
          if (result == 2) social_popup_result = 1;
          return result;
        }
        int society_avail(int category, int model, int faction) {
          ++social_avail_calls; social_avail_category = category;
          social_avail_model = model; social_avail_faction = faction;
          return social_avail_result;
        }
        void social_select(int faction) {
          (void)faction; ++social_select_calls;
        }
        char TutorialPopupCaptionText[] = "TUTOR";
        char *TutorialPopupCaption = TutorialPopupCaptionText;
        int popp(const char *caption, const char *label, int flags,
                 const char *picture, int (*)()) {
          ++popp_calls; popp_caption = caption; popp_label = label;
          popp_flags = flags; popp_picture = picture;
          map_notice_tech = parsed_say_input;
          return 1;
        }
        void set_dirty() { ++dirty_calls; }
        void draw_map(int) { ++drawn_maps; }
        void spot_base(int id, int) {
          ++spotted[id];
          if (mutate_local_after_spot) ConsoleGlobal.faction_id_ = 0;
        }
        void treaty_on(int faction, int id, int treaty) { treaty_faction = faction; treaty_id = id; treaty_kind = treaty; }
        int propose_proto(int faction, int chassis, int weapon, int armor, int abilities, int reactor, int category, char *name) {
          proposed_faction=faction; proposed_chassis=chassis; proposed_weapon=weapon; proposed_armor=armor;
          proposed_abilities=abilities; proposed_reactor=reactor; proposed_category=category; proposed_name=name;
          return proposal_result;
        }
        void prune_protos(int faction, int proto, int) { pruned = faction * 1000 + proto; }
        void upgrade_any_prototypes(int faction) { upgraded = faction; }
        void design_new_veh(int faction, int proto) {
          if (design_new_calls < 8) {
            design_new_factions[design_new_calls] = faction;
            design_new_protos[design_new_calls] = proto;
          }
          ++design_new_calls;
        }
        void upgrade_prototypes(int, int) { ++prototype_upgrades; }
        void tech_effects(int) { ++tech_effects_calls; }
        void mon_secrets_of_tech(int faction) {
          ++secrets_calls; secrets_faction = faction;
        }
        void interlude(int id, char *, int mode, int) {
          if (interlude_calls < 8) {
            interlude_ids[interlude_calls] = id;
            interlude_modes[interlude_calls] = mode;
          }
          ++interlude_calls;
        }
        void desktop_update() {
          ++desktop_updates; desktop_update_order = ++event_order;
        }
        void set_fac(int facility, int base, int set) {
          Bases[base].facilities[facility] = set;
        }
        BODY
        int main() {
          tech_achieved(2, -1, 3, 0);
          if (popup_destructions != 1 || ambience_calls || drawn_maps ||
              treaty_id != -1 || log_calls) return 1;

          MapArea = 3;
          Maps[0].visibility = 1u << 2;
          Maps[0].bit = 0x0f;
          Maps[0].bit_visible[1] = 0x0e;
          Maps[1].visibility = 1u << 2;
          Maps[1].bit = 0x03;
          Maps[1].bit_visible[1] = 0x05;
          Bases[0].faction_id_current = 2;
          Bases[1].faction_id_current = 2;
          Bases[1].visibility = 1u << 2;
          BaseCurrentCount = 2;
          ConsoleGlobal.faction_id_ = 1;
          mutate_local_after_spot = 1;
          tech_achieved(1, 0x270f, 2, 0);
          if (popup_destructions != 2 || log_calls != 1 || logged_tech != 0x270f ||
              logged_faction != 1 || logged_other_faction != 2) return 19;
          mutate_local_after_spot = 0;
          if (PlayersData[1].traded_maps != (1u << 2) ||
              PlayersData[2].traded_maps != (1u << 2)) return 2;
          if (!(Maps[0].visibility & (1u << 1)) ||
              Maps[0].bit_visible[0] != 0x0e ||
              Maps[1].bit_visible[0] != 0x01) return 3;
          if (spotted[0] != 1 || spotted[1] != 1) return 4;
          if (drawn_maps != 0 || WorldClimateWorldWin.redraws != 0 ||
              (UnkBitfield1 & 4) != 0) return 5;
          ConsoleGlobal.faction_id_ = 1;
          BaseCurrentCount = 0;
          tech_achieved(1, 0x270f, 2, 0);
          if (drawn_maps != 1 || WorldClimateWorldWin.redraws != 1 ||
              (UnkBitfield1 & 4) == 0) return 15;

          treaty_faction = treaty_id = treaty_kind = -1;
          tech_achieved(4, 0x5b, 6, 0);
          if (treaty_faction != 4 || treaty_id != 2 || treaty_kind != 8 ||
              popup_destructions != 4 || log_calls != 3 ||
              logged_tech != 0x5b) return 6;

          VehPrototypes[0].chassis_id = 2;
          VehPrototypes[0].weapon_id = 3;
          VehPrototypes[0].armor_id = 4;
          VehPrototypes[0].reactor_id = 1;
          VehPrototypes[0].ability_flags = 0x104;
          proposal_result = 7;
          LockEnableMask = 1u << 5;
          IsMultiplayerNet = 1;
          tech_achieved(5, 0x61, 0, 0);
          if (popup_destructions != 5 || proposed_faction != 5 ||
              proposed_chassis != 2 || proposed_weapon != 3 ||
              proposed_armor != 4 || proposed_abilities != 0x104 || proposed_reactor != 1 ||
              proposed_category != -2 || proposed_name != VehPrototypes[0].veh_name) return 7;
          if (!(VehPrototypes[7].obsolete_factions & 1) || pruned != -1 || upgraded != -1) return 8;

          LockEnableMask = 0;
          IsMultiplayerNet = 0;
          proposal_result = 9;
          tech_achieved(3, 0x62, 0, 0);
          if (popup_destructions != 6 || pruned != 3009 || upgraded != 3) return 9;

          const char reactor_tier_three[] = "tier-three-reactor";
          Reactor[2].name = reactor_tier_three;
          Reactor[2].preq_tech = 4;
          VehPrototypes[130].flags = PROTO_ACTIVE;
          VehPrototypes[130].reactor_id = 2;
          proposal_result = 150;
          ConsoleGlobal.faction_id_ = 2;
          Players[2].noun_gender = 1;
          Players[2].is_noun_plural = 1;
          Players[2].faction_bonus_count = 2;
          Players[2].faction_bonus_id[0] = RULE_FREEFAC;
          Players[2].faction_bonus_val1[0] = 3;
          Players[2].faction_bonus_id[1] = RULE_FREEFAC;
          Players[2].faction_bonus_val1[1] = 4;
          Players[2].faction_bonus_count = 3;
          Players[2].faction_bonus_id[2] = RULE_FREEABIL;
          Players[2].faction_bonus_val1[2] = 1;
          Facility[3].preq_tech = 4;
          Facility[4].preq_tech = 5;
          BaseCurrentCount = 2;
          Bases[0].faction_id_current = 2;
          Bases[1].faction_id_current = 1;
          const int parse_before_reactor = parse_say_calls;
          const int spritebox_before_reactor = spritebox_calls;
          const int popup_starts_before_reactor = popup_starts;
          tech_achieved(2, 4, 0, 0);
          if (tech_popup_calls != 1 ||
              parse_say_calls != parse_before_reactor + 3 ||
              parse_say_ids[parse_before_reactor] != 0 ||
              parse_say_inputs[parse_before_reactor] != static_cast<int>(
                  reinterpret_cast<intptr_t>(reactor_tier_three))) return 17;
          if (popup_inits != 1 || popup_starts != 2 || popup_width != 0x1f4 ||
              popup_flags != 0x80 || popup_strings != 3 ||
              std::strcmp(popup_file, "BLURBSX") != 0 ||
              std::strcmp(popup_caption, "TECH4") != 0 ||
              std::strcmp(popup_string_args[0], "^") != 0 ||
              std::strcmp(popup_string_args[1], "^------") != 0 ||
              std::strcmp(popup_string_args[2], "^") != 0 ||
              own_gender_seen != 1 || own_plural_seen != 1) return 20;
          if (Bases[0].facilities[3] != 1 ||
              Bases[1].facilities[3] != 0 ||
              Bases[0].facilities[4] != 0) return 23;
          if (netmsg_calls != 1 ||
              std::strcmp(netmsg_label, "FREEFACTECH") != 0 ||
              netmsg_duration != -5000 || netmsg_channel != 0 ||
              synch_calls != 1 || synch_opcode != 0x14 || synch_d != 1 ||
              synch_flags != 0x2101) return 29;
          if (popup_starts != 2 ||
              std::strcmp(popup_last_text_file, "popup-caption") != 0 ||
              std::strcmp(popup_last_text, "WEDEVELOP") != 0 ||
              newtech_calls != 1 || newtech_tech != 4 || newtech_other != 0 ||
              fx_calls != 1 || fx_last_id != 0x33 ||
              help_info_calls != 1 || help_info_tech != 4 ||
              help_info_faction != 2 ||
              !(popup_caption_order < help_info_order &&
                help_info_order < fx_order)) return 21;
          if (spritebox_constructs != spritebox_before_reactor + 1 ||
              spritebox_destructions != spritebox_before_reactor + 1 ||
              spritebox_calls != spritebox_before_reactor + 1 ||
              spritebox_arg0_nonnull != 0 ||
              spritebox_sprite_arg != reinterpret_cast<char *>(
                  &g_TECH_ICON_SPRITES[4]) ||
              popup_starts != popup_starts_before_reactor + 2 ||
              !(popup_blurb_order < spritebox_ctor_order &&
                spritebox_ctor_order < spritebox_call_order &&
                spritebox_call_order < spritebox_dtor_order &&
                spritebox_dtor_order < popup_caption_order)) return 32;

          Technology[10].name = "TECHNAME";
          IsMultiplayerNet = 1;
          DiplomacyActive = 1;
          const int newtech_calls_before_modal = newtech_calls;
          tech_achieved(2, 10, 0, 0);
          if (netmsg_calls != 2 || netmsg_duration != 0x1388 ||
              netmsg_channel != 0 || netmsg_target != nullptr ||
              std::strcmp(netmsg_label, "TECHOBTAINED") != 0 ||
              parsed_say_id != 0 || parsed_say_input != static_cast<int>(
                  reinterpret_cast<intptr_t>(Technology[10].name)) ||
              newtech_calls != newtech_calls_before_modal) return 24;
          DiplomacyActive = 0;
          CommlinkPending = 1;
          tech_achieved(2, 13, 0, 0);
          if (netmsg_calls != 3 ||
              newtech_calls != newtech_calls_before_modal) return 27;
          CommlinkPending = 0;
          TurnRedrawPending = 1;
          tech_achieved(2, 14, 0, 0);
          if (netmsg_calls != 4 ||
              newtech_calls != newtech_calls_before_modal) return 28;
          TurnRedrawPending = 0;
          tech_achieved(2, 11, 0, 0);
          if (newtech_calls != newtech_calls_before_modal + 1 ||
              newtech_tech != 11) return 25;
          IsMultiplayerNet = 0;
          ConsoleGlobal.faction_id_ = 1;
          const int help_before_remote_tech = help_info_calls;
          const int exec_before_remote_tech = popup_execs;
          tech_achieved(2, 15, 0, 0);
          if (help_info_calls != help_before_remote_tech ||
              popup_execs != exec_before_remote_tech + 1) return 36;
          ConsoleGlobal.faction_id_ = 2;
          IsMultiplayerNet = 0;
          const char tech_twelve[] = "TECH12";
          const char ability_name[] = "FREE ABILITY";
          Technology[12].name = tech_twelve;
          Technology[12].flags = TFLAG_SECRETS;
          Ability[1].name = ability_name;
          Ability[1].preq_tech = 12;
          const int netmsg_before_ability = netmsg_calls;
          const int parse_before_ability = parse_say_calls;
          const int secrets_before_ability = secrets_calls;
          tech_achieved(2, 12, 0, 0);
          if (netmsg_calls != netmsg_before_ability + 1 ||
              secrets_calls != secrets_before_ability + 1 ||
              secrets_faction != 2 ||
              std::strcmp(netmsg_label, "FREEABILTECH") != 0 ||
              netmsg_duration != -5000 || netmsg_channel != 0 ||
              parse_say_calls != parse_before_ability + 2 ||
              parse_say_ids[parse_before_ability] != 0 ||
              parse_say_inputs[parse_before_ability] != static_cast<int>(
                  reinterpret_cast<intptr_t>(tech_twelve)) ||
              parse_say_ids[parse_before_ability + 1] != 1 ||
              parse_say_inputs[parse_before_ability + 1] != static_cast<int>(
                  reinterpret_cast<intptr_t>(ability_name))) return 26;
          Players[2].faction_bonus_count = 4;
          Players[2].faction_bonus_id[3] = RULE_FREEFAC;
          Players[2].faction_bonus_val1[3] = 5;
          Facility[5].preq_tech = 16;
          BaseCurrentCount = 0;
          PlayersData[2].diff_level = 2;
          SocialCategories[0].preq_tech[0] = 16;
          SocialCategories[0].preq_tech[1] = 16;
          SocialCategories[0].name[1] = "social-model";
          social_popup_result = 1;
          const int sync_before_empty_free_fac = synch_calls;
          tech_achieved(2, 16, 0, 0);
          if (synch_calls != sync_before_empty_free_fac + 1 ||
              social_avail_calls != 2 || social_avail_category != 0 ||
              social_avail_model != 1 || social_avail_faction != 2 ||
              social_popup_calls != 2 ||
              social_popup_caption != TutorialPopupCaption ||
              std::strcmp(social_popup_label, "SOCIETY") != 0 ||
              social_select_calls != 2) return 37;
          SocialCategories[0].preq_tech[0] = 17;
          SocialCategories[1].preq_tech[0] = 17;
          social_popup_result = 2;
          const int social_avail_before_stop = social_avail_calls;
          const int social_popups_before_stop = social_popup_calls;
          const int social_select_before_stop = social_select_calls;
          tech_achieved(2, 17, 0, 0);
          if (social_avail_calls != social_avail_before_stop + 2 ||
              social_popup_calls != social_popups_before_stop + 3 ||
              social_select_calls != social_select_before_stop + 3) return 38;
          PlayersData[2].diff_level = 1;
          ConsoleGlobal.faction_id_ = 7;
          Facility[9].preq_tech = 0x2F;
          SecretProject.voice_of_planet = 1;
          GameTechAchieved[34] = 1u << 7;
          GameTechAchieved[57] = 1u << 7;
          const int interludes_before_tech47 = interlude_calls;
          const int effects_before_tech47 = tech_effects_calls;
          const int desktop_before_tech47 = desktop_updates;
          tech_achieved(7, 0x2F, 0, 0);
          if (tech_effects_calls != effects_before_tech47 + 1 ||
              desktop_updates != desktop_before_tech47 + 1 ||
              interlude_calls != interludes_before_tech47 + 4 ||
              interlude_ids[interludes_before_tech47] != 1 ||
              interlude_ids[interludes_before_tech47 + 1] != 2 ||
              interlude_ids[interludes_before_tech47 + 2] != 0xF ||
              interlude_ids[interludes_before_tech47 + 3] != 0x10 ||
              interlude_modes[interludes_before_tech47 + 3] != 0) return 30;
          ConsoleGlobal.faction_id_ = 2;
          ConsoleGlobal.faction_id_ = 1;
          Players[1].rule_flags = 0x80;
          Players[2].rule_flags = 0;
          PlayersData[2].diplo_treaties[1] = 0;
          PlayersData[1].diplo_treaties[2] = 8;
          const int interludes_before_tech4 = interlude_calls;
          tech_achieved(1, 4, 0, 0);
          if (interlude_calls != interludes_before_tech4 + 1 ||
              interlude_ids[interludes_before_tech4] != 0x24) return 31;
          ConsoleGlobal.faction_id_ = 2;
          tech_popup_calls = 0;
          VehPrototypes[130].flags = 0;
          tech_achieved(2, 4, 0, 0);
          if (tech_popup_calls != 0) return 18;

          PlayersData[6].tech_id_researching = 0;
          PlayersData[6].unk_26 = 1;
          TurnCurrentNum = 1;
          tech_achieved(6, 0, 0, 0);
          if (PlayersData[6].tech_id_researching != -1 ||
              PlayersData[6].tech_achieved != 1 ||
              PlayersData[6].tech_ranking != 2 ||
              PlayersData[6].unk_26 != 0 ||
              GameTechAchieved[0] != (1u << 6)) return 10;

          TechAchievedGate = 1;
          tech_achieved(7, 1, 0, 0);
          if (PlayersData[7].tech_ranking != 0 ||
              PlayersData[7].unk_26 != 0) return 11;

          TechAchievedGate = 0;
          tech_achieved(7, 0x58, 0, 0);
          if (PlayersData[7].tech_ranking != 2 ||
              PlayersData[7].theory_of_everything != 1) return 12;

          tech_achieved(7, 3, -2, 0);
          if (label_lookup_count != 1 || label_offset_seen != 0x35c / 4 ||
              std::strcmp(parsed_noun, "alien-name") != 0) return 14;
          if (std::strcmp(popup_last_text, "THEYACQUIRE") != 0 ||
              popup_execs != 5) return 22;

          GamePreferences = PREF_BSC_AUTO_DESIGN_VEH;
          LockEnableMask = 1u << 1;
          ConsoleGlobal.faction_id_ = 1;
          mutate_local_after_design = 1;
          VehPrototypes[68].flags = 0xC3;
          VehPrototypes[4].flags = PROTO_ACTIVE;
          VehPrototypes[4].preq_tech = 2;
          tech_achieved(1, 2, 0, 0);
          if (VehPrototypes[68].flags != 3 ||
              considered_design_faction != 1 ||
              FirstProposedPrototype != -1) return 13;
          mutate_local_after_design = 0;
          ConsoleGlobal.faction_id_ = 1;
          tech_achieved(1, 2, 0, 0);
          if (FirstProposedPrototype != 4) return 16;

          ConsoleGlobal.faction_id_ = 1;
          LockEnableMask = 1u << 1;
          GamePreferences = PREF_BSC_AUTO_DESIGN_VEH;
          VehPrototypes[5].flags = PROTO_ACTIVE;
          VehPrototypes[5].preq_tech = 20;
          VehPrototypes[5].armor_id = 2;
          VehPrototypes[6].flags = PROTO_ACTIVE;
          VehPrototypes[6].preq_tech = 20;
          VehPrototypes[6].armor_id = 2;
          VehPrototypes[15].flags = PROTO_ACTIVE;
          VehPrototypes[15].preq_tech = 20;
          VehPrototypes[15].armor_id = 2;
          Weapon[2].offense_rating = 0;
          populate_pending_designs = 1;
          xpop_result = 1;
          const int design_prompts_before = tech_popup_calls;
          const int design_windows_before = design_window_shutdowns;
          tech_achieved(1, 20, 0, 0);
          if (tech_popup_calls != design_prompts_before + 1 ||
              std::strcmp(xpop_label, "ASKSEEDESIGN") != 0 ||
              design_new_calls != 2 || design_new_factions[0] != -1 ||
              design_new_protos[0] != 6 || design_new_factions[1] != -1 ||
              design_new_protos[1] != 69 ||
              VehPrototypes[69].flags != PROTO_ACTIVE ||
              design_window_shutdowns != design_windows_before + 1 ||
              prototype_upgrades != 1) {
            return 33;
          }
          VehPrototypes[5].preq_tech = 21;
          VehPrototypes[6].preq_tech = 21;
          xpop_result = 0;
          const int declined_prompts_before = tech_popup_calls;
          tech_achieved(1, 21, 0, 0);
          if (tech_popup_calls != declined_prompts_before + 1 ||
              design_new_calls != 2 ||
              VehPrototypes[69].flags != PROTO_ACTIVE ||
              prototype_upgrades != 2) return 34;

          Technology[22].flags = TFLAG_REVEALS_MAP;
          Technology[22].name = "REVEAL22";
          PlayersData[1].flags = 0;
          ConsoleGlobal.faction_id_ = 1;
          TurnCurrentNum = 1;
          BaseCurrentCount = 2;
          MapArea = 2;
          Maps[0].visibility = 0;
          Maps[0].bit = 0x1234;
          Maps[0].bit_visible[0] = 0;
          Maps[1].visibility = 0;
          Maps[1].bit = 0x5678;
          Maps[1].bit_visible[0] = 0;
          Bases[0].visibility = 0;
          Bases[0].population_size = 7;
          Bases[0].faction_pop_size_intel[0] = 0xA5;
          Bases[0].faction_pop_size_intel[1] = 0;
          const int popup_map_before = popp_calls;
          const int draw_map_before = drawn_maps;
          const int dirty_before = dirty_calls;
          const int world_redraw_before = WorldClimateWorldWin.redraws;
          tech_achieved(1, 22, 0, 0);
          if ((PlayersData[1].flags & 0x200) == 0 ||
              Maps[0].visibility != 2 || Maps[1].visibility != 2 ||
              Maps[0].bit_visible[0] != 0x1234 ||
              Maps[1].bit_visible[0] != 0x5678 ||
              Bases[0].visibility != 2 ||
              Bases[0].faction_pop_size_intel[0] != 0xA5 ||
              Bases[0].faction_pop_size_intel[1] != 7 ||
              map_notice_tech != static_cast<int>(reinterpret_cast<intptr_t>(
                  Technology[22].name)) ||
              popp_calls != popup_map_before + 1 ||
              std::strcmp(popp_caption, "SCRIPT") != 0 ||
              std::strcmp(popp_label, "SEEMAP2") != 0 || popp_flags != 0 ||
              std::strcmp(popp_picture, "native_sm.pcx") != 0 ||
              drawn_maps != draw_map_before + 1 ||
              dirty_calls != dirty_before + 1 ||
              WorldClimateWorldWin.redraws != world_redraw_before + 1)
            return 35;

          Technology[18].name = "TECH18";
          Technology[18].flags = TFLAG_SECRETS;
          GameTechAchieved[18] = 0;
          ConsoleGlobal.faction_id_ = 2;
          Players[2].adj_name_faction[4] = 'A';
          const int secret_popups_before = tech_popup_calls;
          const int secret_logs_before = secret_log_calls;
          const int desktop_before_secret = desktop_updates;
          tech_achieved(2, 18, 0, 0);
          if ((PlayersData[2].flags & 0x40) == 0 ||
              tech_popup_calls != secret_popups_before + 1 ||
              std::strcmp(xpop_label, "THESECRET0") != 0 ||
              last_parse_says_one != Players[2].adj_name_faction + 4 ||
              secret_log_calls != secret_logs_before + 1 ||
              std::strcmp(secret_log_message, "Learning a SECRET") != 0 ||
              secret_log_string_arg != reinterpret_cast<const char *>(
                  &PlayersData[0].traded_maps) ||
              secret_log_faction != 2 || secret_log_tech != 18 ||
              secret_log_mask != (1u << 2) ||
              !(secret_popup_order < secret_log_order &&
                secret_log_order < desktop_update_order) ||
              desktop_updates != desktop_before_secret + 1) return 39;
          const int secret_popups_after_first = tech_popup_calls;
          const int secret_logs_after_first = secret_log_calls;
          tech_achieved(2, 18, 0, 0);
          if (tech_popup_calls != secret_popups_after_first ||
              secret_log_calls != secret_logs_after_first) return 40;

          SecretProject.planetary_datalinks = 0;
          Bases[0].faction_id_current = 1;
          TurnCurrentNum = 0;
          const int popup_destructions_before_chain = popup_destructions;
          tech_achieved(0, 19, 2, 0);
          if (popup_destructions != popup_destructions_before_chain + 1)
            return 41;

          // Native Datalinks waits for three positive factions and skips
          // disabled rules, existing owners, and faction-zero acquisitions.
          GamePreferences = 0; LockEnableMask = 0;
          ConsoleGlobal.faction_id_ = 7;
          Bases[0].faction_id_current = 2;
          GameTechAchieved[23] = (1u << 1) | (1u << 4);
          int owner_techs = PlayersData[2].tech_achieved;
          tech_achieved(3, 23, 0, 0);
          if ((GameTechAchieved[23] & (1u << 2)) == 0 ||
              PlayersData[2].tech_achieved != owner_techs + 1) return 42;
          GameTechAchieved[24] = 1u << 1;
          tech_achieved(3, 24, 0, 0);
          if ((GameTechAchieved[24] & (1u << 2)) != 0) return 43;
          Technology[25].preq_tech_1 = -2;
          GameTechAchieved[25] = (1u << 1) | (1u << 4);
          tech_achieved(3, 25, 0, 0);
          if ((GameTechAchieved[25] & (1u << 2)) != 0) return 44;

          // Generic rule_sharetech is separate from Datalinks. Recursion
          // carries the technology as arg2 and the freebie marker as arg4.
          SecretProject.planetary_datalinks = -1;
          ExpansionEnabled = 0;
          Players[2].rule_sharetech = 3;
          GameTechAchieved[26] = (1u << 1) | (1u << 4);
          owner_techs = PlayersData[2].tech_achieved;
          tech_achieved(3, 26, 0, 0);
          if ((GameTechAchieved[26] & (1u << 2)) == 0 ||
              PlayersData[2].tech_achieved != owner_techs + 1 ||
              std::strcmp(popup_last_text, "WEGETSHARETECH") != 0) return 45;
          ExpansionEnabled = 1;
          Players[2].rule_flags = RFLAG_TECHSHARE;
          Players[2].rule_sharetech = 1;
          GameTechAchieved[27] = (1u << 1) | (1u << 4);
          tech_achieved(3, 27, 0, 0);
          if ((GameTechAchieved[27] & (1u << 2)) != 0) return 46;
          PlayersData[1].diplo_treaties[2] = 0x1000;
          PlayersData[2].diplo_treaties[1] = 0;
          GameTechAchieved[28] = (1u << 1) | (1u << 4);
          tech_achieved(3, 28, 0, 0);
          if ((GameTechAchieved[28] & (1u << 2)) != 0) return 47;
          PlayersData[2].diplo_treaties[1] = 0x1000;
          GameTechAchieved[29] = (1u << 1) | (1u << 4);
          tech_achieved(3, 29, 0, 0);
          if ((GameTechAchieved[29] & (1u << 2)) == 0) return 48;
          PlayersData[2].diplo_treaties[1] = 0;
          SecretProject.empath_guild = 0;
          GameTechAchieved[30] = (1u << 1) | (1u << 4);
          tech_achieved(3, 30, 0, 0);
          if ((GameTechAchieved[30] & (1u << 2)) == 0) return 49;
          Bases[0].faction_id_current = 3;
          GovernorFaction = 2;
          GameTechAchieved[31] = (1u << 1) | (1u << 4);
          tech_achieved(3, 31, 0, 0);
          if ((GameTechAchieved[31] & (1u << 2)) == 0) return 50;
          Players[1].rule_flags = Players[3].rule_flags =
              Players[4].rule_flags = RFLAG_ALIEN;
          GameTechAchieved[32] = (1u << 1) | (1u << 4);
          tech_achieved(3, 32, 0, 0);
          if ((GameTechAchieved[32] & (1u << 2)) != 0) return 51;
          Players[2].rule_flags = 0;
          GameTechAchieved[33] = (1u << 1) | (1u << 4);
          tech_achieved(3, 33, 0, 0);
          if ((GameTechAchieved[33] & (1u << 2)) == 0) return 52;
          GameTechAchieved[88] = (1u << 1) | (1u << 4);
          tech_achieved(3, 88, 0, 0);
          if ((GameTechAchieved[88] & (1u << 2)) != 0) return 53;

          // Equal/higher reactors must not generate upgrade proposals.
          Players[2].rule_sharetech = 0;
          ConsoleGlobal.faction_id_ = 2;
          VehPrototypes[130].flags = PROTO_ACTIVE;
          VehPrototypes[130].reactor_id = 3;
          proposed_faction = -1;
          tech_achieved(2, 4, 0, 0);
          if (proposed_faction != -1) return 54;
          VehPrototypes[130].reactor_id = 4;
          tech_achieved(2, 4, 0, 0);
          if (proposed_faction != -1) return 55;
        }
    '''.replace("BODY", body)
    cpp = tmp_path / "tech_achieved.cpp"
    exe = tmp_path / "tech_achieved"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_actual_body_early_return_transfer_treaty_prototype_and_discovery_paths(tmp_path):
    body = actual_body("static bool technology_shareable(") + "\n" + actual_body()
    assert "// BODY GOES HERE." not in body
    result = run_host(tmp_path, body)
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("before,after", [
    ("prototype.reactor_id >= reactor_level", "prototype.reactor_id < reactor_level"),
    ("(base.visibility & (1u << faction_id_with)) != 0 ||",
     "(base.visibility & (1u << faction_id_with)) == 0 &&"),
    ("PlayersData[faction_id].diplo_treaties[other] & 8",
     "PlayersData[other].diplo_treaties[faction_id] & 8"),
    ("tech_achieved(chained_faction, tech_id, -1, 0)",
     "tech_achieved(chained_faction, -1, tech_id, 0)"),
    ("tech_achieved(receiver, tech_id, -2, 1)",
     "tech_achieved(receiver, -1, tech_id, 1)"),
    ("social_model < 4", "social_model < 1"),
    ("while (selection == 2)", "while (false)"),
    ("faction_pop_size_intel[faction_id]", "faction_pop_size_intel[0]"),
])
def test_native_branch_controls_reject_reconstruction_regressions(tmp_path, before, after):
    body = actual_body("static bool technology_shareable(") + "\n" + actual_body()
    assert before in body
    result = run_host(tmp_path, body.replace(before, after))
    assert result.returncode != 0, "damaged native branch unexpectedly passed"
