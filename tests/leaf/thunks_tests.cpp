// Recovery leaf tests: thunks.
//
// Carved verbatim out of tests/recovery_leaf_tests.cpp.  Add a case by
// appending a `void test_*()` here plus `LEAF_CASE(LEAF_APPEND, test_*);` at
// the bottom.  This file is the only file that edit touches: the per-family
// case counts are derived from these `LEAF_CASE(` lines at build time, and
// LEAF_APPEND needs no offset because ties there break by name.
#include "leaf_shared.h"

#define LEAF_FAMILY "thunks"

namespace {

// The receiver sits inside a poisoned arena with room ahead of it
// for the largest vtordisp displacement the family uses. Every
// four-byte window in the arena holds a distinct value, so a body
// that reads its vtordisp from the wrong displacement cannot
// happen to read an equal one. The adjusted receiver is only ever
// compared, never dereferenced, so it may land anywhere.
constexpr size_t AdjustorArenaLead = 2048;

constexpr size_t AdjustorArenaTail = 256;

uint8_t g_adjustor_arena[AdjustorArenaLead + AdjustorArenaTail];

uint8_t *adjustor_receiver() {
    for (size_t index = 0; index < sizeof(g_adjustor_arena); ++index) {
        g_adjustor_arena[index] = static_cast<uint8_t>(index * 7u + 1u);
    }
    return g_adjustor_arena + AdjustorArenaLead;
}

void *g_adjustor_seen;

int g_adjustor_calls;

int g_adjustor_args[4];


int __thiscall observe_adjustor_i_i(void *self, int arg0) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(arg0);
    return 0x5A5A;
}

int __thiscall observe_adjustor_i_ii(void *self, int arg0, int arg1) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(arg0);
    g_adjustor_args[1] = static_cast<int>(arg1);
    return 0x5A5A;
}

int __thiscall observe_adjustor_i_p(void *self, void *arg0) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(reinterpret_cast<intptr_t>(arg0));
    return 0x5A5A;
}

int __thiscall observe_adjustor_i_piii(void *self, void *arg0, int arg1,
                                       int arg2, int arg3) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(reinterpret_cast<intptr_t>(arg0));
    g_adjustor_args[1] = static_cast<int>(arg1);
    g_adjustor_args[2] = static_cast<int>(arg2);
    g_adjustor_args[3] = static_cast<int>(arg3);
    return 0x5A5A;
}

void * __thiscall observe_adjustor_p_u(void *self, unsigned int arg0) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(arg0);
    return &g_adjustor_calls;
}

void __thiscall observe_adjustor_v(void *self) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
}

void __thiscall observe_adjustor_v_i(void *self, int arg0) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(arg0);
}

void __thiscall observe_adjustor_v_ii(void *self, int arg0, int arg1) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(arg0);
    g_adjustor_args[1] = static_cast<int>(arg1);
}

void __thiscall observe_adjustor_v_pi(void *self, void *arg0, int arg1) {
    g_adjustor_seen = self;
    ++g_adjustor_calls;
    g_adjustor_args[0] = static_cast<int>(reinterpret_cast<intptr_t>(arg0));
    g_adjustor_args[1] = static_cast<int>(arg1);
}


struct AdjustorCase_i_i {
    int (__fastcall *thunk)(void *, void *, int);
    func_adjustor_i_i **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_i_i g_adjustor_cases_i_i[] = {
    {&adjust_radio_button1_on_key_down, &RadioButtonOnKeyDownTarget, 4, 0},
    {&adjust_dialogs1_on_key_down, &DialogsOnKeyDownTarget, 4, 0},
    {&adjust_check_box1_on_key_down, &CheckBoxOnKeyDownTarget, 4, 0},
    {&adjust_sprite_box1_on_key_down, &SpriteBoxOnKeyDownTarget, 4, 0},
};

struct AdjustorCase_i_ii {
    int (__fastcall *thunk)(void *, void *, int, int);
    func_adjustor_i_ii **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_i_ii g_adjustor_cases_i_ii[] = {
    {&adjust_console1_on_key_click, &ConsoleOnKeyClickTarget, 4, 0},
};

struct AdjustorCase_i_p {
    int (__fastcall *thunk)(void *, void *, void *);
    func_adjustor_i_p **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_i_p g_adjustor_cases_i_p[] = {
    {&adjust_dialogs1_on_scroll_delete, &DialogsOnScrollDeleteTarget, 4, 0},
};

struct AdjustorCase_i_piii {
    int (__fastcall *thunk)(void *, void *, void *, int, int, int);
    func_adjustor_i_piii **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_i_piii g_adjustor_cases_i_piii[] = {
    {&adjust_radio_button1_attach, &RadioButtonAttachTarget, 4, 0},
    {&adjust_dialogs1_attach, &DialogsAttachTarget, 4, 0},
    {&adjust_list_box1_attach, &ListBoxAttachTarget, 4, 0},
    {&adjust_check_box1_attach, &CheckBoxAttachTarget, 4, 0},
    {&adjust_sprite_box1_attach, &SpriteBoxAttachTarget, 4, 0},
    {&adjust_edit_group1_attach, &EditGroupAttachTarget, 4, 0},
};

struct AdjustorCase_p_u {
    void * (__fastcall *thunk)(void *, void *, unsigned int);
    func_adjustor_p_u **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_p_u g_adjustor_cases_p_u[] = {
    {&adjust_dialogs1_scalar_delete, &DialogsScalarDeleteTarget, 4, 0},
    {&adjust_dialogs2_scalar_delete, &DialogsScalarDeleteTarget, 1096, 1092},
    {&adjust_dialogs3_scalar_delete, &DialogsScalarDeleteTarget, 4, 2584},
    {&adjust_map_win1_scalar_delete, &MapWinScalarDeleteTarget, 4, 0},
    {&adjust_map_win2_scalar_delete, &MapWinScalarDeleteTarget, 1096, 1092},
    {&adjust_plan_win2_scalar_delete, &PlanWinScalarDeleteTarget, 1096, 1092},
    {&adjust_console1_scalar_delete, &ConsoleScalarDeleteTarget, 4, 0},
    {&adjust_console2_scalar_delete, &ConsoleScalarDeleteTarget, 1096, 1092},
};

struct AdjustorCase_v {
    void (__fastcall *thunk)(void *, void *);
    func_adjustor_v **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_v g_adjustor_cases_v[] = {
    {&adjust_radio_button1_on_redraw, &RadioButtonOnRedrawTarget, 4, 0},
    {&adjust_radio_button3_on_redraw, &RadioButtonOnRedrawTarget, 4, 2584},
    {&adjust_dialogs1_on_scroll_create, &DialogsOnScrollCreateTarget, 4, 0},
    {&adjust_dialogs1_on_redraw, &DialogsOnRedrawTarget, 4, 0},
    {&adjust_dialogs1_pass_dialog_focus, &DialogsPassDialogFocusTarget, 4, 0},
    {&adjust_dialogs3_on_redraw, &DialogsOnRedrawTarget, 4, 2584},
    {&adjust_map_win1_on_lose_mouse_capture,
     &MapWinOnLoseMouseCaptureTarget, 4, 0},
    {&adjust_map_win1_on_redraw, &MapWinOnRedrawTarget, 4, 0},
    {&adjust_map_win1_on_sys_close, &MapWinOnSysCloseTarget, 4, 0},
    {&adjust_map_win5_on_lose_mouse_capture,
     &MapWinOnLoseMouseCaptureTarget, 4, 1508},
    {&adjust_plan_win1_on_redraw, &PlanWinOnRedrawTarget, 4, 0},
    {&adjust_map_win5_on_sys_close, &MapWinOnSysCloseTarget, 4, 1508},
    {&adjust_map_win4_on_lose_mouse_capture,
     &MapWinOnLoseMouseCaptureTarget, 4, 9000},
    {&adjust_map_win4_on_redraw, &MapWinOnRedrawTarget, 4, 9000},
    {&adjust_console1_on_sys_close, &ConsoleOnSysCloseTarget, 4, 0},
    {&adjust_list_box1_on_redraw, &ListBoxOnRedrawTarget, 4, 0},
    {&adjust_list_box3_on_redraw, &ListBoxOnRedrawTarget, 4, 2584},
    {&adjust_check_box1_on_redraw, &CheckBoxOnRedrawTarget, 4, 0},
    {&adjust_check_box3_on_redraw, &CheckBoxOnRedrawTarget, 4, 2584},
    {&adjust_sprite_box1_on_redraw, &SpriteBoxOnRedrawTarget, 4, 0},
    {&adjust_sprite_box3_on_redraw, &SpriteBoxOnRedrawTarget, 4, 2584},
    {&adjust_edit_group1_on_redraw, &EditGroupOnRedrawTarget, 4, 0},
    {&adjust_edit_group1_pass_dialog_focus,
     &EditGroupPassDialogFocusTarget, 4, 0},
    {&adjust_edit_group3_on_redraw, &EditGroupOnRedrawTarget, 4, 2584},
};

struct AdjustorCase_v_i {
    void (__fastcall *thunk)(void *, void *, int);
    func_adjustor_v_i **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_v_i g_adjustor_cases_v_i[] = {
    {&adjust_radio_button1_dtor, &RadioButtonDtorTarget, 4, 0},
    {&adjust_radio_button1_on_dialog_focus,
     &RadioButtonOnDialogFocusTarget, 4, 0},
    {&adjust_radio_button2_dtor, &RadioButtonDtorTarget, 1096, 1092},
    {&adjust_radio_button3_dtor, &RadioButtonDtorTarget, 4, 2584},
    {&adjust_dialogs1_on_mousewheel, &DialogsOnMousewheelTarget, 4, 0},
    {&adjust_dialogs1_on_dialog_focus, &DialogsOnDialogFocusTarget, 4, 0},
    {&adjust_map_win1_on_button_clicked, &MapWinOnButtonClickedTarget, 4, 0},
    {&adjust_map_win5_on_button_clicked,
     &MapWinOnButtonClickedTarget, 4, 1508},
    {&adjust_map_win4_on_button_clicked,
     &MapWinOnButtonClickedTarget, 4, 9000},
    {&adjust_list_box1_on_mousewheel, &ListBoxOnMousewheelTarget, 4, 0},
    {&adjust_list_box1_on_key_down, &ListBoxOnKeyDownTarget, 4, 0},
    {&adjust_list_box1_on_dialog_focus, &ListBoxOnDialogFocusTarget, 4, 0},
    {&adjust_check_box1_on_dialog_focus, &CheckBoxOnDialogFocusTarget, 4, 0},
    {&adjust_sprite_box1_on_dialog_focus, &SpriteBoxOnDialogFocusTarget, 4, 0},
    {&adjust_edit_group1_on_dialog_focus, &EditGroupOnDialogFocusTarget, 4, 0},
};

struct AdjustorCase_v_ii {
    void (__fastcall *thunk)(void *, void *, int, int);
    func_adjustor_v_ii **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_v_ii g_adjustor_cases_v_ii[] = {
    {&adjust_radio_button1_on_mouse_move, &RadioButtonOnMouseMoveTarget, 4, 0},
    {&adjust_radio_button1_on_mouse_leave,
     &RadioButtonOnMouseLeaveTarget, 4, 0},
    {&adjust_radio_button1_on_left_down, &RadioButtonOnLeftDownTarget, 4, 0},
    {&adjust_radio_button1_on_left_double_click,
     &RadioButtonOnLeftDoubleClickTarget, 4, 0},
    {&adjust_dialogs1_on_mouse_move, &DialogsOnMouseMoveTarget, 4, 0},
    {&adjust_dialogs1_on_mouse_leave, &DialogsOnMouseLeaveTarget, 4, 0},
    {&adjust_dialogs1_on_left_click, &DialogsOnLeftClickTarget, 4, 0},
    {&adjust_dialogs1_on_left_down, &DialogsOnLeftDownTarget, 4, 0},
    {&adjust_dialogs1_on_left_up, &DialogsOnLeftUpTarget, 4, 0},
    {&adjust_dialogs1_on_right_click, &DialogsOnRightClickTarget, 4, 0},
    {&adjust_dialogs1_on_right_down, &DialogsOnRightDownTarget, 4, 0},
    {&adjust_dialogs1_on_right_up, &DialogsOnRightUpTarget, 4, 0},
    {&adjust_dialogs1_on_left_double_click,
     &DialogsOnLeftDoubleClickTarget, 4, 0},
    {&adjust_dialogs1_on_right_double_click,
     &DialogsOnRightDoubleClickTarget, 4, 0},
    {&adjust_dialogs1_on_scrolled, &DialogsOnScrolledTarget, 4, 0},
    {&adjust_dialogs1_on_scrolling, &DialogsOnScrollingTarget, 4, 0},
    {&adjust_map_win1_on_resize, &MapWinOnResizeTarget, 4, 0},
    {&adjust_map_win1_on_mouse_move, &MapWinOnMouseMoveTarget, 4, 0},
    {&adjust_map_win1_on_left_click, &MapWinOnLeftClickTarget, 4, 0},
    {&adjust_map_win1_on_left_down, &MapWinOnLeftDownTarget, 4, 0},
    {&adjust_map_win1_on_left_up, &MapWinOnLeftUpTarget, 4, 0},
    {&adjust_map_win1_on_right_click, &MapWinOnRightClickTarget, 4, 0},
    {&adjust_map_win1_on_right_down, &MapWinOnRightDownTarget, 4, 0},
    {&adjust_map_win1_on_left_double_click,
     &MapWinOnLeftDoubleClickTarget, 4, 0},
    {&adjust_map_win1_on_nc_left_down, &MapWinOnNcLeftDownTarget, 4, 0},
    {&adjust_map_win1_on_nc_hittest, &MapWinOnNcHittestTarget, 4, 0},
    {&adjust_map_win5_on_resize, &MapWinOnResizeTarget, 4, 1508},
    {&adjust_map_win5_on_mouse_move, &MapWinOnMouseMoveTarget, 4, 1508},
    {&adjust_map_win5_on_left_click, &MapWinOnLeftClickTarget, 4, 1508},
    {&adjust_map_win5_on_left_down, &MapWinOnLeftDownTarget, 4, 1508},
    {&adjust_map_win5_on_left_up, &MapWinOnLeftUpTarget, 4, 1508},
    {&adjust_map_win5_on_right_click, &MapWinOnRightClickTarget, 4, 1508},
    {&adjust_map_win5_on_right_down, &MapWinOnRightDownTarget, 4, 1508},
    {&adjust_map_win5_on_left_double_click,
     &MapWinOnLeftDoubleClickTarget, 4, 1508},
    {&adjust_map_win5_on_nc_left_down, &MapWinOnNcLeftDownTarget, 4, 1508},
    {&adjust_map_win5_on_nc_hittest, &MapWinOnNcHittestTarget, 4, 1508},
    {&adjust_map_win4_on_resize, &MapWinOnResizeTarget, 4, 9000},
    {&adjust_map_win4_on_mouse_move, &MapWinOnMouseMoveTarget, 4, 9000},
    {&adjust_map_win4_on_left_click, &MapWinOnLeftClickTarget, 4, 9000},
    {&adjust_map_win4_on_left_down, &MapWinOnLeftDownTarget, 4, 9000},
    {&adjust_map_win4_on_left_up, &MapWinOnLeftUpTarget, 4, 9000},
    {&adjust_map_win4_on_right_click, &MapWinOnRightClickTarget, 4, 9000},
    {&adjust_map_win4_on_right_down, &MapWinOnRightDownTarget, 4, 9000},
    {&adjust_map_win4_on_left_double_click,
     &MapWinOnLeftDoubleClickTarget, 4, 9000},
    {&adjust_console1_on_nc_mouse_move, &ConsoleOnNcMouseMoveTarget, 4, 0},
    {&adjust_console1_on_nc_left_down, &ConsoleOnNcLeftDownTarget, 4, 0},
    {&adjust_console1_on_nc_left_up, &ConsoleOnNcLeftUpTarget, 4, 0},
    {&adjust_console1_on_nc_hittest, &ConsoleOnNcHittestTarget, 4, 0},
    {&adjust_list_box1_on_mouse_move, &ListBoxOnMouseMoveTarget, 4, 0},
    {&adjust_list_box1_on_mouse_leave, &ListBoxOnMouseLeaveTarget, 4, 0},
    {&adjust_list_box1_on_left_down, &ListBoxOnLeftDownTarget, 4, 0},
    {&adjust_list_box1_on_right_down, &ListBoxOnRightDownTarget, 4, 0},
    {&adjust_list_box1_on_left_double_click,
     &ListBoxOnLeftDoubleClickTarget, 4, 0},
    {&adjust_list_box1_on_right_double_click,
     &ListBoxOnRightDoubleClickTarget, 4, 0},
    {&adjust_list_box1_on_scrolled, &ListBoxOnScrolledTarget, 4, 0},
    {&adjust_list_box1_on_scrolling, &ListBoxOnScrollingTarget, 4, 0},
    {&adjust_check_box1_on_mouse_move, &CheckBoxOnMouseMoveTarget, 4, 0},
    {&adjust_check_box1_on_mouse_leave, &CheckBoxOnMouseLeaveTarget, 4, 0},
    {&adjust_check_box1_on_left_down, &CheckBoxOnLeftDownTarget, 4, 0},
    {&adjust_check_box1_on_left_double_click,
     &CheckBoxOnLeftDoubleClickTarget, 4, 0},
    {&adjust_sprite_box1_on_mouse_move, &SpriteBoxOnMouseMoveTarget, 4, 0},
    {&adjust_sprite_box1_on_mouse_leave, &SpriteBoxOnMouseLeaveTarget, 4, 0},
    {&adjust_sprite_box1_on_left_click, &SpriteBoxOnLeftClickTarget, 4, 0},
    {&adjust_sprite_box1_on_left_down, &SpriteBoxOnLeftDownTarget, 4, 0},
    {&adjust_sprite_box1_on_left_up, &SpriteBoxOnLeftUpTarget, 4, 0},
    {&adjust_sprite_box1_on_right_click, &SpriteBoxOnRightClickTarget, 4, 0},
    {&adjust_sprite_box1_on_right_down, &SpriteBoxOnRightDownTarget, 4, 0},
    {&adjust_sprite_box1_on_right_up, &SpriteBoxOnRightUpTarget, 4, 0},
    {&adjust_sprite_box1_on_left_double_click,
     &SpriteBoxOnLeftDoubleClickTarget, 4, 0},
    {&adjust_sprite_box1_on_right_double_click,
     &SpriteBoxOnRightDoubleClickTarget, 4, 0},
};

struct AdjustorCase_v_pi {
    void (__fastcall *thunk)(void *, void *, void *, int);
    func_adjustor_v_pi **slot;
    int displacement;
    int adjust;
};

const AdjustorCase_v_pi g_adjustor_cases_v_pi[] = {
    {&adjust_console1_on_post_redraw_nc_buffer,
     &ConsoleOnPostRedrawNcBufferTarget, 4, 0},
};

// The receiver sits inside a poisoned arena with room ahead of
// it for the largest adjustment the family uses and for the
// vtordisp the two-instruction form reads. Every four-byte
// window holds a distinct value, so a body that reads its
// vtordisp from the wrong place cannot happen to read an equal
// one. Adjusted receivers are only ever compared, never
// dereferenced, so they may land anywhere.
constexpr size_t DeletingArenaLead = 24576;

constexpr size_t DeletingArenaTail = 256;

uint8_t g_deleting_arena[DeletingArenaLead + DeletingArenaTail];

uint8_t *deleting_receiver() {
    for (size_t index = 0; index < sizeof(g_deleting_arena); ++index) {
        g_deleting_arena[index] = static_cast<uint8_t>(index * 7u + 1u);
    }
    return g_deleting_arena + DeletingArenaLead;
}

void *g_deleting_dtor_seen;

int g_deleting_dtor_calls;

void *g_deleting_free_seen;

int g_deleting_free_calls;

void *g_deleting_forward_seen;

unsigned int g_deleting_forward_arg;

int g_deleting_forward_calls;

void __cdecl observe_deleting_free(void *block) {
    g_deleting_free_seen = block;
    ++g_deleting_free_calls;
}


void __thiscall observe_deleting_dtor(void *self) {
    g_deleting_dtor_seen = self;
    ++g_deleting_dtor_calls;
}

void *__thiscall observe_deleting_forward(void *self,
                                          unsigned int arg0) {
    g_deleting_forward_seen = self;
    g_deleting_forward_arg = arg0;
    ++g_deleting_forward_calls;
    return &g_deleting_forward_calls;
}

void *__thiscall observe_deleting_forward_nullary(void *self) {
    g_deleting_forward_seen = self;
    ++g_deleting_forward_calls;
    return &g_deleting_forward_calls;
}


typedef void *(__fastcall *DeletingBody)(void *, void *,
                                         unsigned int);

typedef void *(__fastcall *DeletingBodyNullary)(void *,
                                                void *);

struct DeletingCase {
    DeletingBody body;
    func_deleting_dtor **slot;
};

const DeletingCase g_deleting_cases[] = {
    {&scalar_delete_alpha_movie, &AlphaMovieDtorTarget},
    {&scalar_delete_base_pop, &BasePopDtorTarget},
    {&scalar_delete_popup, &PopupDtorTarget},
    {&scalar_delete_edit_box, &EditBoxDtorTarget},
    {&scalar_delete_alpha_save, &AlphaSaveDtorTarget},
    {&scalar_delete_prod_picker, &ProdPickerDtorTarget},
    {&scalar_delete_base_win, &BaseWinDtorTarget},
    {&scalar_delete_pop_menu, &PopMenuDtorTarget},
    {&scalar_delete_counc_win, &CouncWinDtorTarget},
    {&scalar_delete_credits, &CreditsDtorTarget},
    {&scalar_delete_string_box, &StringBoxDtorTarget},
    {&scalar_delete_datalink, &DatalinkDtorTarget},
    {&scalar_delete_design_win, &DesignWinDtorTarget},
    {&scalar_delete_select_part_win, &SelectPartWinDtorTarget},
    {&scalar_delete_diplo_pop, &DiploPopDtorTarget},
    {&scalar_delete_diplo_win, &DiploWinDtorTarget},
    {&scalar_delete_fame_win, &FameWinDtorTarget},
    {&scalar_delete_gamma, &GammaDtorTarget},
    {&scalar_delete_main_interface, &MainInterfaceDtorTarget},
    {&scalar_delete_interlude, &InterludeDtorTarget},
    {&scalar_delete_monu_win, &MonuWinDtorTarget},
    {&scalar_delete_multi_win, &MultiWinDtorTarget},
    {&scalar_delete_net_msg, &NetMsgDtorTarget},
    {&scalar_delete_net_win, &NetWinDtorTarget},
    {&scalar_delete_new_tech_win, &NewTechWinDtorTarget},
    {&scalar_delete_pick_tech, &PickTechDtorTarget},
    {&scalar_delete_pick_win, &PickWinDtorTarget},
    {&scalar_delete_pref_win, &PrefWinDtorTarget},
    {&scalar_delete_quayle_win, &QuayleWinDtorTarget},
    {&scalar_delete_report_win, &ReportWinDtorTarget},
    {&scalar_delete_setup_win, &SetupWinDtorTarget},
    {&scalar_delete_check_button, &CheckButtonDtorTarget},
    {&scalar_delete_social_win, &SocialWinDtorTarget},
    {&scalar_delete_tut_win, &TutWinDtorTarget},
    {&scalar_delete_world_win, &WorldWinDtorTarget},
    {&scalar_delete_video, &Sub004C86D0Target},
    {&scalar_delete_voice_rx, &VoiceRxDtorTarget},
    {&scalar_delete_voice_tx, &VoiceTxDtorTarget},
    {&scalar_delete_dip_edit, &DipEditDtorTarget},
    {&scalar_delete_alpha_menu, &AlphaMenuDtorTarget},
    {&scalar_delete_replay_win, &ReplayWinDtorTarget},
    {&scalar_delete_multi_debug, &MultiDebugDtorTarget},
    {&scalar_delete_menu, &MenuDtorTarget},
    {&scalar_delete_image_button, &ImageButtonDtorTarget},
    {&scalar_delete_push_button, &PushButtonDtorTarget},
    {&scalar_delete_sub_633160, &Sub00633010Target},
};

// The fixup is observed through the destructor its target
// runs: with bit 0 set, the recorder and the free both see
// the adjusted receiver the fixup computed.
struct DeletingAdjustCase {
    DeletingBody body;
    func_deleting_dtor **slot;
    int adjust;
    int vtordisp;
};

const DeletingAdjustCase g_deleting_adjust_cases[] = {
    {&adjust_this_alpha_movie, &AlphaMovieDtorTarget, 0x444, 0},
    {&adjust_this_base_pop, &BasePopDtorTarget, 0x444, 0},
    {&adjust_this_popup, &PopupDtorTarget, 0x444, 0},
    {&adjust_this_alpha_save, &AlphaSaveDtorTarget, 0x444, 0},
    {&adjust_this_edit_box, &EditBoxDtorTarget, 0x444, 0},
    {&adjust_this_base_win, &BaseWinDtorTarget, 0x444, 0},
    {&adjust_this_prod_picker, &ProdPickerDtorTarget, 0x444, 0},
    {&adjust_this_pop_menu_delete1, &PopMenuDtorTarget, 0x444, 0},
    {&adjust_this_pop_menu_delete2, &PopMenuDtorTarget, 0x537C, 0},
    {&adjust_this_pop_menu_delete3, &PopMenuDtorTarget, 0x57C0, 0},
    {&adjust_this_counc_win, &CouncWinDtorTarget, 0x444, 0},
    {&adjust_this_credits, &CreditsDtorTarget, 0x444, 0},
    {&adjust_this_string_box, &StringBoxDtorTarget, 0x444, 0},
    {&adjust_this_datalink, &DatalinkDtorTarget, 0x444, 0},
    {&adjust_this_design_win, &DesignWinDtorTarget, 0x444, 0},
    {&adjust_this_select_part_win, &SelectPartWinDtorTarget, 0x444, 0},
    {&adjust_this_diplo_pop, &DiploPopDtorTarget, 0x444, 0},
    {&adjust_this_diplo_win, &DiploWinDtorTarget, 0x444, 0},
    {&adjust_this_fame_win, &FameWinDtorTarget, 0x444, 0},
    {&adjust_this_gamma, &GammaDtorTarget, 0x444, 0},
    {&adjust_this_main_interface, &MainInterfaceDtorTarget, 0x444, 0},
    {&adjust_this_interlude, &InterludeDtorTarget, 0x444, 0},
    {&adjust_this_monu_win, &MonuWinDtorTarget, 0x444, 0},
    {&adjust_this_multi_win, &MultiWinDtorTarget, 0x444, 0},
    {&adjust_this_net_msg, &NetMsgDtorTarget, 0x444, 0},
    {&adjust_this_net_win, &NetWinDtorTarget, 0x444, 0},
    {&adjust_this_new_tech_win, &NewTechWinDtorTarget, 0x444, 0},
    {&adjust_this_pick_tech, &PickTechDtorTarget, 0x444, 0},
    {&adjust_this_pick_win, &PickWinDtorTarget, 0x444, 0},
    {&adjust_this_pref_win, &PrefWinDtorTarget, 0x444, 0},
    {&adjust_this_quayle_win, &QuayleWinDtorTarget, 0x444, 0},
    {&adjust_this_report_win, &ReportWinDtorTarget, 0x444, 0},
    {&adjust_this_setup_win, &SetupWinDtorTarget, 0x444, 0},
    {&adjust_this_check_button, &CheckButtonDtorTarget, 0x444, 0},
    {&adjust_this_social_win, &SocialWinDtorTarget, 0x444, 0},
    {&adjust_this_tut_win, &TutWinDtorTarget, 0x444, 0},
    {&adjust_this_world_win, &WorldWinDtorTarget, 0x444, 0},
    {&adjust_this_dip_edit, &DipEditDtorTarget, 0x444, 0},
    {&adjust_this_alpha_menu, &AlphaMenuDtorTarget, 0x444, 0},
    {&adjust_this_replay_win, &ReplayWinDtorTarget, 0x444, 0},
    {&adjust_this_multi_debug, &MultiDebugDtorTarget, 0x444, 0},
    {&adjust_this_menu, &MenuDtorTarget, 0x444, 0},
    {&adjust_this_image_button, &ImageButtonDtorTarget, 0x444, 0},
    {&adjust_this_push_button, &PushButtonDtorTarget, 0x444, 0},
    {&adjust_this_sub_633740, &Sub00633010Target, 0x444, 0},
};

struct DeletingSeamCase_nullary {
    DeletingBodyNullary body;
    func_deleting_forward_nullary **slot;
    int adjust;
    int vtordisp;
};

const DeletingSeamCase_nullary g_deleting_seam_cases_nullary[] = {
    {&adjust_this_sub_404420, &Sub004042B0Target, 0x444, 0},
};

struct DeletingSeamCase {
    DeletingBody body;
    func_deleting_forward **slot;
    int adjust;
    int vtordisp;
};

const DeletingSeamCase g_deleting_seam_cases[] = {
    {&adjust_this_plan_win, &PlanWinScalarDeleteTarget, 0, 1},
    {&adjust_this_sub_612700, &Sub00612710Target, 0, 1},
    {&adjust_this_sub_633730, &Sub006336D0Target, 0x58, 0},
};

// GENERATED by tools/generate_delegation_thunks.py.
struct DelegationCase {
    void *thunk;
    size_t member;
    size_t slot;
    int forwarded;
    int absent;
    bool discards;  // answers `absent` even when it dispatched
};

const DelegationCase g_delegation_cases[] = {
    {reinterpret_cast<void *>(&wave_in_device_set_codec_redirect), 0x14, 0x60, 1, 0x13, false},
    {reinterpret_cast<void *>(&wave_in_device_set_vxw_key_redirect), 0x14, 0x64, 1, 0x13, false},
    {reinterpret_cast<void *>(&midi_play_redirect), 0x3c, 0x1c, 0, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_switch_type_redirect), 0x3c, 0x100, 1, 0x13, false},
    {reinterpret_cast<void *>(&midi_add_switch_range_redirect), 0x3c, 0x108, 2, 0x13, false},
    {reinterpret_cast<void *>(&midi_set_nswitch_threads_redirect), 0x3c, 0x104, 1, 0x13, false},
    {reinterpret_cast<void *>(&midi_map_patch_3_redirect), 0x3c, 0xdc, 3, 0x14, false},
    {reinterpret_cast<void *>(&midi_map_patch_2_redirect), 0x3c, 0xd8, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_clear_patch_redirect), 0x3c, 0xe0, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_reset_redirect), 0x3c, 0xc8, 0, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_track_redirect), 0x3c, 0x84, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_mute_track_redirect), 0x3c, 0x8c, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_unmute_track_redirect), 0x3c, 0xac, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_active_tracks_2_redirect), 0x3c, 0x94, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_active_tracks_1_redirect), 0x3c, 0x90, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_play_trackset_redirect), 0x3c, 0x10c, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_xpose_trackset_redirect), 0x3c, 0x114, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_stop_trackset_redirect), 0x3c, 0x110, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_add_active_trackset_redirect), 0x3c, 0x98, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_active_range_lo_redirect), 0x3c, 0xa0, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_active_range_hi_redirect), 0x3c, 0xa4, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_remove_active_trackset_redirect), 0x3c, 0xa8, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_get_trackset_redirect), 0x3c, 0x9c, 1, 0x0, false},
    {reinterpret_cast<void *>(&midi_get_ntracks_redirect), 0x3c, 0xc4, 0, 0x0, false},
    {reinterpret_cast<void *>(&midi_load_patch_redirect), 0x3c, 0x120, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_unload_patch_redirect), 0x3c, 0x124, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_patch_redirect), 0x3c, 0x128, 2, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_tempo_redirect), 0x3c, 0x88, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_get_time_redirect), 0x3c, 0xd4, 1, 0x0, false},
    {reinterpret_cast<void *>(&midi_get_control_track_redirect), 0x3c, 0xf4, 0, 0x0, false},
    {reinterpret_cast<void *>(&midi_get_total_track_ticks_redirect), 0x3c, 0xf8, 1, 0x0, false},
    {reinterpret_cast<void *>(&midi_set_channel_ctrl_redirect), 0x3c, 0xe4, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_program_ctrl_redirect), 0x3c, 0xe8, 1, 0x14, false},
    {reinterpret_cast<void *>(&midi_set_track_ctrl_redirect), 0x3c, 0xc0, 1, 0x14, false},
    {reinterpret_cast<void *>(&voice_rx_start_redirect), 0x3c, 0x88, 0, 0x0, true},
    {reinterpret_cast<void *>(&voice_rx_stop_redirect), 0x3c, 0x20, 0, 0x0, true},
    {reinterpret_cast<void *>(&voice_rx_unload_redirect), 0x3c, 0x14, 0, 0x13, false},
    {reinterpret_cast<void *>(&voice_rx_add_buffer_redirect), 0x3c, 0x8c, 1, 0x13, false},
    {reinterpret_cast<void *>(&voice_rx_get_buffer_size_redirect), 0x3c, 0x90, 0, 0x0, false},
    {reinterpret_cast<void *>(&voice_tx_release_redirect), 0x3c, 0x38, 0, 0x0, true},
    {reinterpret_cast<void *>(&voice_tx_get_nbuffers_redirect), 0x3c, 0x94, 0, 0x0, false},
    {reinterpret_cast<void *>(&voice_tx_get_next_buffer_redirect), 0x3c, 0x88, 0, 0x0, false},
    {reinterpret_cast<void *>(&voice_tx_return_buffer_redirect), 0x3c, 0x8c, 1, 0x13, false},
};

struct PlainDelegationCase {
    void *thunk;
    size_t slot;
    int forwarded;   // how many reach the delegate
    int declared;    // how many the thunk must POP
    int answer;      // -1 when the thunk returns void
};

const PlainDelegationCase g_plain_delegation_cases[] = {
    {reinterpret_cast<void *>(&alpha_movie_on_key_click_redirect), 0xe8, 0, 2, 1},
    {reinterpret_cast<void *>(&base_win_on_scrolling_redirect), 0xc0, 2, 2, -1},
    {reinterpret_cast<void *>(&base_win_on_iface_scrolling_redirect), 0x4, 2, 2, -1},
    {reinterpret_cast<void *>(&credits_on_key_click_redirect), 0xe8, 0, 2, 1},
    {reinterpret_cast<void *>(&credits_on_left_click_redirect), 0xe8, 0, 2, -1},
    {reinterpret_cast<void *>(&select_part_win_on_scrolling_redirect), 0xc0, 2, 2, -1},
    {reinterpret_cast<void *>(&interlude_on_key_down_redirect), 0xe8, 0, 1, 1},
    {reinterpret_cast<void *>(&interlude_on_left_down_redirect), 0xe8, 0, 2, -1},
    {reinterpret_cast<void *>(&new_tech_win_on_button_clicked_redirect), 0xe8, 0, 1, -1},
    {reinterpret_cast<void *>(&prod_picker_on_scrolling_redirect), 0xc0, 2, 2, -1},
    {reinterpret_cast<void *>(&replay_win_on_left_click_redirect), 0xe8, 0, 2, -1},
    {reinterpret_cast<void *>(&replay_win_on_right_click_redirect), 0xe8, 0, 2, -1},
    {reinterpret_cast<void *>(&replay_win_on_button_clicked_redirect), 0xe8, 0, 1, -1},
    {reinterpret_cast<void *>(&replay_win_on_key_click_redirect), 0xe8, 0, 2, 1},
    {reinterpret_cast<void *>(&gamma_on_scrolling_redirect), 0xc0, 2, 2, -1},
    {reinterpret_cast<void *>(&base_button_on_dialog_focus_redirect), 0xf8, 0, 1, -1},
};

// One recorder per arity, plus a poison for every other slot: the delegate
// must be reached through THE slot the bytes name, not merely through some
// slot of the member's table.
struct DelegationProbe {
    int calls;
    void *self;
    int args[3];
    int poison_calls;
};

DelegationProbe g_delegation_probe;

constexpr int DelegationAnswer = 0x7A7A5A5A;


int __thiscall delegation_observe_0(void *self) {
    ++g_delegation_probe.calls;
    g_delegation_probe.self = self;
    return DelegationAnswer;
}

int __thiscall delegation_observe_1(void *self, int a1) {
    delegation_observe_0(self);
    g_delegation_probe.args[0] = a1;
    return DelegationAnswer;
}

int __thiscall delegation_observe_2(void *self, int a1, int a2) {
    delegation_observe_1(self, a1);
    g_delegation_probe.args[1] = a2;
    return DelegationAnswer;
}

int __thiscall delegation_observe_3(void *self, int a1, int a2, int a3) {
    delegation_observe_2(self, a1, a2);
    g_delegation_probe.args[2] = a3;
    return DelegationAnswer;
}

int __thiscall delegation_poison(void *) {
    ++g_delegation_probe.poison_calls;
    return 0;
}


int call_plain(const PlainDelegationCase &entry, void *self,
               const int (&args)[3]) {
    // The thunk must be entered with every DECLARED argument, not just the
    // forwarded ones: it is installed at the original address and pops what
    // the original's  popped. Calling it with fewer would unbalance
    // the stack whatever the body does.
    switch (entry.declared) {
    case 0:
        return reinterpret_cast<int(__fastcall *)(void *, void *)>(
            entry.thunk)(self, nullptr);
    case 1:
        return reinterpret_cast<int(__fastcall *)(void *, void *, int)>(
            entry.thunk)(self, nullptr, args[0]);
    case 2:
        return reinterpret_cast<int(__fastcall *)(void *, void *, int, int)>(
            entry.thunk)(self, nullptr, args[0], args[1]);
    default:
        return reinterpret_cast<
            int(__fastcall *)(void *, void *, int, int, int)>(
            entry.thunk)(self, nullptr, args[0], args[1], args[2]);
    }
}

int call_delegation(const DelegationCase &entry, void *self,
                    const int (&args)[3]) {
    switch (entry.forwarded) {
    case 0:
        return reinterpret_cast<int(__fastcall *)(void *, void *)>(
            entry.thunk)(self, nullptr);
    case 1:
        return reinterpret_cast<int(__fastcall *)(void *, void *, int)>(
            entry.thunk)(self, nullptr, args[0]);
    case 2:
        return reinterpret_cast<int(__fastcall *)(void *, void *, int, int)>(
            entry.thunk)(self, nullptr, args[0], args[1]);
    default:
        return reinterpret_cast<
            int(__fastcall *)(void *, void *, int, int, int)>(
            entry.thunk)(self, nullptr, args[0], args[1], args[2]);
    }
}

// GENERATED by tools/generate_nullsub_thunks.py.
struct NullsubCase {
    void *thunk;
    int declared;   // stack arguments the thunk must POP
    int answer;     // -1 when it returns void
};

const NullsubCase g_nullsub_cases[] = {
    {reinterpret_cast<void *>(&nullsub_00406a80_redirect), 2, 0},
    {reinterpret_cast<void *>(&nullsub_00406b30_redirect), 0, 0},
    {reinterpret_cast<void *>(&nullsub_0044c820_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c850_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c8a0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c8b0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c8c0_redirect), 0, 0},
    {reinterpret_cast<void *>(&nullsub_0044c8d0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c8e0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c8f0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c900_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c930_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c960_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c970_redirect), 0, 0},
    {reinterpret_cast<void *>(&nullsub_0044c9a0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044c9d0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044ca00_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044ca30_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044ca60_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044cac0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044caf0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044cb00_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044cb10_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0044cb20_redirect), 0, -1},
    {reinterpret_cast<void *>(&mapwin_nullsub1_redirect), 2, -1},
    {reinterpret_cast<void *>(&mapwin_nullsub2_redirect), 2, -1},
    {reinterpret_cast<void *>(&mapwin_nullsub3_redirect), 2, -1},
    {reinterpret_cast<void *>(&mapwin_nullsub4_redirect), 2, -1},
    {reinterpret_cast<void *>(&vehdraw_nullsub1_redirect), 0, -1},
    {reinterpret_cast<void *>(&vehdraw_nullsub2_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_004c8140_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_004c8950_redirect), 0, 0},
    {reinterpret_cast<void *>(&nullsub_004e32c0_redirect), 2, -1},
    {reinterpret_cast<void *>(&nullsub_004e32d0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_004e32e0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_004e32f0_redirect), 2, -1},
    {reinterpret_cast<void *>(&nullsub_004e9b60_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_00515760_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0051d9d0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0051d9e0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_005d5240_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_005fc9e0_redirect), 0, 0},
    {reinterpret_cast<void *>(&nullsub_005fd280_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_005fd290_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_005fd2a0_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_00616b30_redirect), 2, 0},
    {reinterpret_cast<void *>(&nullsub_00616d90_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0062d140_redirect), 0, 0},
    {reinterpret_cast<void *>(&nullsub_0062d150_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0062ee80_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0062ee90_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_00631a10_redirect), 0, -1},
    {reinterpret_cast<void *>(&video_unk7_redirect), 2, 0},
    {reinterpret_cast<void *>(&nullsub_00639380_redirect), 0, -1},
    {reinterpret_cast<void *>(&nullsub_0063c500_redirect), 1, -1},
    {reinterpret_cast<void *>(&nullsub_0063c9c0_redirect), 0, -1},
};

int call_nullsub(const NullsubCase &entry) {
    // Entered with every declared argument, because the pop count is the one
    // thing these bodies actually have to get right.
    switch (entry.declared) {
    case 0:
        return reinterpret_cast<int(__fastcall *)(void *, void *)>(
            entry.thunk)(nullptr, nullptr);
    case 1:
        return reinterpret_cast<int(__fastcall *)(void *, void *, int)>(
            entry.thunk)(nullptr, nullptr, 0x1111);
    default:
        return reinterpret_cast<int(__fastcall *)(void *, void *, int, int)>(
            entry.thunk)(nullptr, nullptr, 0x1111, 0x2222);
    }
}

// GENERATED by tools/generate_global_arith.py.
// `golden` is FNV-1a over the whole slot image after this
// thunk runs from the seed the fixture uses. It is computed
// by an independent simulator in the generator, so the test
// compares against a value rather than against itself.
struct GlobalArithCase { void *thunk; uint32_t golden; };

const GlobalArithCase g_global_arith_cases[] = {
    {reinterpret_cast<void *>(&global_arith_0044c870_redirect), 0x51E56E3DU},
    {reinterpret_cast<void *>(&global_arith_0048d550_redirect), 0xD39D3E09U},
    {reinterpret_cast<void *>(&global_arith_0048d560_redirect), 0x1A17E430U},
    {reinterpret_cast<void *>(&global_arith_0048d570_redirect), 0x6F686B49U},
    {reinterpret_cast<void *>(&global_arith_0048d590_redirect), 0x7DB10D55U},
    {reinterpret_cast<void *>(&global_arith_0048d5b0_redirect), 0x1AF0DB1CU},
    {reinterpret_cast<void *>(&global_arith_0048d5d0_redirect), 0xCF434005U},
    {reinterpret_cast<void *>(&global_arith_0048d5f0_redirect), 0x4A0525DDU},
    {reinterpret_cast<void *>(&global_arith_0048d610_redirect), 0x64ED0577U},
    {reinterpret_cast<void *>(&global_arith_0048d630_redirect), 0x39142E00U},
    {reinterpret_cast<void *>(&global_arith_0048d670_redirect), 0x5D117237U},
    {reinterpret_cast<void *>(&global_arith_0048d690_redirect), 0x6DA30164U},
    {reinterpret_cast<void *>(&global_arith_0048d6b0_redirect), 0xF664387FU},
    {reinterpret_cast<void *>(&global_arith_0048d6d0_redirect), 0x288BC170U},
    {reinterpret_cast<void *>(&global_arith_0048d6f0_redirect), 0xB0871F15U},
    {reinterpret_cast<void *>(&global_arith_0048d710_redirect), 0x65C1BE1CU},
    {reinterpret_cast<void *>(&global_arith_0048d730_redirect), 0x10DF6EFDU},
    {reinterpret_cast<void *>(&global_arith_0048d750_redirect), 0x442A596AU},
    {reinterpret_cast<void *>(&global_arith_0048d760_redirect), 0xAB19DF47U},
    {reinterpret_cast<void *>(&global_arith_0048d770_redirect), 0x2BD7EF9AU},
    {reinterpret_cast<void *>(&global_arith_0048d790_redirect), 0x39AEE4C0U},
    {reinterpret_cast<void *>(&global_arith_0048d7b0_redirect), 0x24D000CCU},
    {reinterpret_cast<void *>(&global_arith_0048d7d0_redirect), 0x0FE23193U},
    {reinterpret_cast<void *>(&global_arith_0048d7f0_redirect), 0x4A025290U},
    {reinterpret_cast<void *>(&global_arith_0048d810_redirect), 0x333CD949U},
    {reinterpret_cast<void *>(&global_arith_005d8360_redirect), 0x90501F32U},
};

int32_t **const g_global_arith_slots[] = {
    &GlobalSlot00686B6C,
    &GlobalSlot00686B70,
    &GlobalSlot00686B74,
    &GlobalSlot00686B78,
    &GlobalSlot00686B7C,
    &GlobalSlot00686B80,
    &GlobalSlot00686B84,
    &GlobalSlot00686B8C,
    &GlobalSlot007591C4,
    &GlobalSlot008577D8,
    &GlobalSlot008577DC,
    &GlobalSlot008577E0,
    &GlobalSlot008577E4,
    &GlobalSlot008577E8,
    &GlobalSlot008577EC,
    &GlobalSlot00857884,
    &GlobalSlot00857888,
    &GlobalSlot0085788C,
    &GlobalSlot00857890,
    &GlobalSlot00857894,
    &GlobalSlot00857898,
    &GlobalSlot0085789C,
    &GlobalSlot008578A0,
    &GlobalSlot008578A4,
    &GlobalSlot008578A8,
    &GlobalSlot008578AC,
    &GlobalSlot008578B0,
    &GlobalSlot008578B4,
    &GlobalSlot008578BC,
    &GlobalSlot008578C0,
    &GlobalSlot008578C4,
    &GlobalSlot008578C8,
    &GlobalSlot008578CC,
    &GlobalSlot008578D0,
    &GlobalSlot009B3A54,
};

// GENERATED by tools/generate_guarded_teardowns.py.
struct GuardedTeardownCase {
    void (__cdecl *thunk)();
    uint8_t **flags;   // null when the body is unguarded
    int mask;
    int steps;
    void **object[2];
    func_thiscall_teardown **target[2];
};

const GuardedTeardownCase g_guarded_teardown_cases[] = {
    {&teardown_00422eb0, nullptr, 0, 1, {reinterpret_cast<void **>(&g_BattleWin), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&SubInterfaceOriginalReleaseIfaceMode), nullptr}},
    {&teardown_00422ec0, nullptr, 0, 1, {reinterpret_cast<void **>(&g_BattleWin), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&SubInterfaceOriginalReleaseIfaceMode), nullptr}},
    {&teardown_004456a0, nullptr, 0, 1, {reinterpret_cast<void **>(&g_PASSOVER_WAVE), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&PopupWavePlay), nullptr}},
    {&teardown_g_mapwin, nullptr, 0, 2, {reinterpret_cast<void **>(&TeardownObject007F5ACC), reinterpret_cast<void **>(&TeardownObject007F5ACC)}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget00420F90), reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005D4DD0)}},
    {&teardown_g_planwin, nullptr, 0, 2, {reinterpret_cast<void **>(&TeardownObject00856DC0), reinterpret_cast<void **>(&TeardownObject00856DC0)}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget0048BD80), reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005D4DD0)}},
    {&teardown_00505d20, nullptr, 0, 1, {reinterpret_cast<void **>(&TeardownObject00915068), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&BufferElementTeardown), nullptr}},
    {&teardown_00505d30, nullptr, 0, 1, {reinterpret_cast<void **>(&TeardownObject0090EA68), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget004327A0), nullptr}},
    {&teardown_g_console, nullptr, 0, 2, {reinterpret_cast<void **>(&TeardownObject00939444), reinterpret_cast<void **>(&TeardownObject00939444)}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget0051D9F0), reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005D4DD0)}},
    {&teardown_00589890, nullptr, 0, 1, {reinterpret_cast<void **>(&TeardownObject009403E0), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&PopupDtorTarget), nullptr}},
    {&teardown_g_buffer_sprite, &TeardownFlags009B37CC, 2, 1, {reinterpret_cast<void **>(&g_BUFFER_SPRITE), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_g_win_buffer, &TeardownFlags009B6E30, 1, 1, {reinterpret_cast<void **>(&g_WIN_BUFFER), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&BufferElementTeardown), nullptr}},
    {&teardown_005eb3b0, &TeardownFlags009B6E30, 2, 1, {reinterpret_cast<void **>(&TeardownObject009B7490), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&BufferElementTeardown), nullptr}},
    {&teardown_g_radiobutton_sprite_1, &TeardownFlags009B8EF0, 1, 1, {reinterpret_cast<void **>(&g_RADIOBUTTON_SPRITE_1), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_g_radiobutton_sprite_2, &TeardownFlags009B8EF0, 2, 1, {reinterpret_cast<void **>(&g_RADIOBUTTON_SPRITE_2), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_0060e5d0, nullptr, 0, 2, {reinterpret_cast<void **>(&g_RADIOBUTTON_SPRITE_2), reinterpret_cast<void **>(&g_RADIOBUTTON_SPRITE_1)}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820)}},
    {&teardown_g_checkbox_sprite_1, &TeardownFlags009B8F58, 1, 1, {reinterpret_cast<void **>(&g_CHECKBOX_SPRITE_1), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_g_checkbox_sprite_2, &TeardownFlags009B8F58, 2, 1, {reinterpret_cast<void **>(&g_CHECKBOX_SPRITE_2), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_0060fd60, nullptr, 0, 2, {reinterpret_cast<void **>(&g_CHECKBOX_SPRITE_1), reinterpret_cast<void **>(&g_CHECKBOX_SPRITE_2)}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820)}},
    {&teardown_g_filewin_sprite_1, &TeardownFlags009B9014, 1, 1, {reinterpret_cast<void **>(&g_FILEWIN_SPRITE_1), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_g_filewin_sprite_2, &TeardownFlags009B9014, 2, 1, {reinterpret_cast<void **>(&g_FILEWIN_SPRITE_2), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_g_filewin_sprite_3, &TeardownFlags009B9014, 4, 1, {reinterpret_cast<void **>(&g_FILEWIN_SPRITE_3), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_g_caviar_buffer_1, &TeardownFlags009B9104, 1, 1, {reinterpret_cast<void **>(&g_CAVIAR_BUFFER_1), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&BufferElementTeardown), nullptr}},
    {&teardown_g_caviar_buffer_2, &TeardownFlags009B9104, 2, 1, {reinterpret_cast<void **>(&g_CAVIAR_BUFFER_2), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&BufferElementTeardown), nullptr}},
    {&teardown_0063bb00, &TeardownFlags009BEAE0, 1, 1, {reinterpret_cast<void **>(&TeardownObject009BEAE8), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
    {&teardown_0063cef0, nullptr, 0, 1, {reinterpret_cast<void **>(&TeardownObject009BEAE8), nullptr}, {reinterpret_cast<func_thiscall_teardown **>(&TeardownTarget005E3820), nullptr}},
};

int g_guarded_calls;

void *g_guarded_object;

// A teardown that calls its own thunk back, once. This is the only thing that
// can see WHEN the flag bit is set: the original stores it at the instruction
// before its tail jump, so a re-entering teardown finds the bit already set
// and does nothing. Setting it afterwards instead would let the re-entry run
// the teardown a second time - which is what the two surviving swap mutants
// did until this existed.
void (__cdecl *g_guarded_reentry)();

int g_guarded_depth;

// Which object each step was handed, in order: the two-step forwarders pass a
// DIFFERENT object per step (RadioButton::close_class closes sprite 2 then
// sprite 1), so a body that reused one object for both would pass a call
// count and fail here.
void *g_guarded_seen[2];

int g_guarded_index;


uint32_t g_guarded_order;

void __thiscall observe_guarded_step_1(void *object);

void __thiscall observe_guarded_teardown(void *object) {
    ++g_guarded_calls;
    g_guarded_object = object;
    g_guarded_order = g_guarded_order * 16 + 1;
    if (g_guarded_index < 2) {
        g_guarded_seen[g_guarded_index++] = object;
    }
    if (g_guarded_reentry != nullptr && g_guarded_depth == 0) {
        ++g_guarded_depth;
        g_guarded_reentry();
        --g_guarded_depth;
    }
}

// A DISTINCT observer for the second step. Both steps of ??__Fg_MAPWIN pass
// the same object to two different destructors - the derived one, then the
// base - so binding both to one observer makes their order invisible, and the
// swap mutant survived until this existed. Destroying a base before its
// derived class is exactly the defect worth catching.
void __thiscall observe_guarded_step_1(void *object) {
    ++g_guarded_calls;
    g_guarded_object = object;
    g_guarded_order = g_guarded_order * 16 + 2;
    if (g_guarded_index < 2) {
        g_guarded_seen[g_guarded_index++] = object;
    }
}


}  // namespace

void test_guarded_teardowns() {
    // Flags, objects and teardown targets all default to addresses in the
    // original image that are unmapped here, so every one is rebound before
    // anything runs.
    for (const GuardedTeardownCase &entry : g_guarded_teardown_cases) {
        uint8_t *const saved_flags = entry.flags ? *entry.flags : nullptr;
        void *saved_object[2] = {};
        func_thiscall_teardown *saved_target[2] = {};
        int marker[2] = {};
        // Two steps can share ONE target binding - RadioButton::close_class
        // closes two sprites through the same Sprite::close seam - and then a
        // per-step observer cannot be installed at all, because the second
        // install overwrites the first. Where they share, the two steps are
        // told apart by their objects instead, which differ in exactly those
        // cases; where the targets differ (??__Fg_MAPWIN destroys the derived
        // class then its base through the SAME object) the observers do it.
        const bool distinct_targets =
            entry.steps < 2 || entry.target[0] != entry.target[1];
        for (int step = 0; step < entry.steps; ++step) {
            saved_object[step] = *entry.object[step];
            saved_target[step] = *entry.target[step];
            *entry.object[step] = &marker[step];
            *entry.target[step] = (step == 0 || !distinct_targets)
                ? &observe_guarded_teardown
                : &observe_guarded_step_1;
        }
        uint8_t flag_byte = 0;
        if (entry.flags) { *entry.flags = &flag_byte; }

        auto reset = [&](uint8_t start) {
            flag_byte = start;
            g_guarded_calls = 0;
            g_guarded_depth = 0;
            g_guarded_reentry = nullptr;
            for (int step = 0; step < entry.steps; ++step) {
                g_guarded_seen[step] = nullptr;
            }
            g_guarded_index = 0;
            g_guarded_order = 0;
        };

        if (entry.flags) {
            // Bit clear: the teardown runs and the bit is set. Every OTHER bit
            // of the shared byte must survive - three of these thunks share
            // one flag byte and differ only in the mask, so a body that stored
            // the mask rather than or-ing it would break its neighbours rather
            // than itself.
            reset(static_cast<uint8_t>(~entry.mask));
            entry.thunk();
            expect(g_guarded_calls == entry.steps);
            expect(flag_byte == 0xFF);

            // Bit already set: nothing runs, nothing changes.
            reset(static_cast<uint8_t>(entry.mask));
            entry.thunk();
            expect(g_guarded_calls == 0);
            expect(flag_byte == static_cast<uint8_t>(entry.mask));

            // At most once, from empty.
            reset(0);
            entry.thunk();
            entry.thunk();
            entry.thunk();
            expect(g_guarded_calls == 1);
            expect(flag_byte == static_cast<uint8_t>(entry.mask));

            // Re-entrant: the bit must already be set by the time the teardown
            // runs, so calling back in finds nothing left to do. Nothing else
            // can observe WHEN the store happens.
            reset(0);
            g_guarded_reentry = entry.thunk;
            entry.thunk();
            g_guarded_reentry = nullptr;
            expect(g_guarded_calls == 1);
            expect(flag_byte == static_cast<uint8_t>(entry.mask));
        } else {
            // Unguarded: every step runs, in order, on its own object. Running
            // twice runs everything twice - there is no once-only guard, and a
            // body that grew one would fail here.
            reset(0);
            entry.thunk();
            expect(g_guarded_calls == entry.steps);
            // Step order, nibble-packed. Two-step bodies destroy the DERIVED
            // class first and the base second; a swap is a use-after-destroy
            // in the original and must not survive here.
            const uint32_t expected_order =
                entry.steps < 2 ? 0x1U : (distinct_targets ? 0x12U : 0x11U);
            expect(g_guarded_order == expected_order);
            for (int step = 0; step < entry.steps; ++step) {
                // Against what the BINDING holds, not against marker[step]:
                // some of these pass the SAME object to both steps
                // (??__Fg_MAPWIN destroys one object through two destructors)
                // while others pass different ones (RadioButton::close_class
                // closes sprite 2 then sprite 1). Comparing to a per-step
                // marker assumes the former never happens, and it does.
                expect(g_guarded_seen[step] == *entry.object[step]);
            }
            entry.thunk();
            expect(g_guarded_calls == entry.steps * 2);
        }

        if (entry.flags) { *entry.flags = saved_flags; }
        for (int step = 0; step < entry.steps; ++step) {
            *entry.object[step] = saved_object[step];
            *entry.target[step] = saved_target[step];
        }
    }
}

void test_global_arith() {
    // Every one of these bodies reads and writes globals that live in the
    // original image and are NOT MAPPED here, so the fixture repoints all of
    // them at local storage before anything is called. Without that this
    // suite page-faults on the first thunk - the same shape as the
    // BufferResetValue520 and BufferPalette faults fixed earlier in this tree.
    constexpr size_t kSlots =
        sizeof(g_global_arith_slots) / sizeof(g_global_arith_slots[0]);
    std::vector<int32_t> storage(kSlots);
    std::vector<int32_t *> saved(kSlots);
    for (size_t index = 0; index < kSlots; ++index) {
        saved[index] = *g_global_arith_slots[index];
        *g_global_arith_slots[index] = &storage[index];
    }

    // Seed every slot distinctly, run every thunk, and require that the whole
    // slot image is a deterministic function of the seed: running the batch
    // twice from the same seed must land on the same bytes, and running it
    // from a different seed must not. That pins that these bodies are pure
    // arithmetic over the slots and nothing else - no hidden state, no
    // dependence on anything the fixture did not set.
    auto seed = [&](int32_t base) {
        for (size_t index = 0; index < kSlots; ++index) {
            storage[index] = base + static_cast<int32_t>(index) * 7;
        }
    };
    auto run_all = [&]() {
        for (const GlobalArithCase &entry : g_global_arith_cases) {
            reinterpret_cast<void(__fastcall *)(void *, void *)>(entry.thunk)(
                nullptr, nullptr);
        }
    };
    seed(0x1000);
    run_all();
    const std::vector<int32_t> first = storage;
    seed(0x1000);
    run_all();
    expect(first == storage);
    seed(0x2000);
    run_all();
    expect(first != storage);

    // Each thunk individually, against a VALUE rather than against itself.
    // The checks above only pin that the batch is a deterministic function of
    // its seed, which every mutant also satisfies - dropping an  still
    // stores something, still deterministically, and those mutants survived
    // until this loop existed. The golden is FNV-1a over the whole slot image
    // after the thunk runs, computed by an independent simulator in the
    // generator; its authority rests on the statement-by-statement
    // verification of the emitted bodies against the disassembly.
    for (const GlobalArithCase &entry : g_global_arith_cases) {
        seed(0x3000);
        const std::vector<int32_t> before = storage;
        reinterpret_cast<void(__fastcall *)(void *, void *)>(entry.thunk)(
            nullptr, nullptr);
        expect(before != storage);
        uint32_t hash = 0x811C9DC5U;
        for (const int32_t slot_value : storage) {
            const uint32_t bits = static_cast<uint32_t>(slot_value);
            for (int shift = 0; shift < 32; shift += 8) {
                hash ^= (bits >> shift) & 0xFFU;
                hash *= 0x01000193U;
            }
        }
        expect(hash == entry.golden);
    }

    for (size_t index = 0; index < kSlots; ++index) {
        *g_global_arith_slots[index] = saved[index];
    }
}

void test_nullsub_thunks() {
    // A do-nothing leaf has exactly two observables: it returns without
    // touching anything, and it pops what it declared. Both are checked here
    // by entering every thunk with a null receiver - which is safe precisely
    // BECAUSE the body reads nothing, and would fault immediately if some
    // body were not the no-op it is catalogued as - and by calling each one
    // repeatedly, since an unbalanced stack shows up as a crash rather than a
    // wrong value.
    for (const NullsubCase &entry : g_nullsub_cases) {
        for (int repeat = 0; repeat < 4; ++repeat) {
            const int result = call_nullsub(entry);
            if (entry.answer >= 0) {
                expect(result == entry.answer);
            }
        }
    }
    expect(sizeof(g_nullsub_cases) / sizeof(g_nullsub_cases[0]) == 56);
}

void test_delegation_thunks() {
    // Every case is the same three questions: does an absent member answer
    // the catalogued constant without dispatching at all, does a present one
    // dispatch through THE named slot with the member as receiver and the
    // arguments in order, and is the delegate's own answer what comes back.
    const int args[3] = {0x11110001, 0x22220002, 0x33330003};
    for (const DelegationCase &entry : g_delegation_cases) {
        std::vector<uint8_t> object(0x400, 0);
        std::vector<uint8_t> member(0x40, 0);
        std::vector<void *> vtable(0x100,
                                   reinterpret_cast<void *>(&delegation_poison));
        vtable[entry.slot / sizeof(void *)] =
            entry.forwarded == 0 ? reinterpret_cast<void *>(&delegation_observe_0)
          : entry.forwarded == 1 ? reinterpret_cast<void *>(&delegation_observe_1)
          : entry.forwarded == 2 ? reinterpret_cast<void *>(&delegation_observe_2)
                                 : reinterpret_cast<void *>(&delegation_observe_3);
        void *const vtable_pointer = vtable.data();
        std::memcpy(member.data(), &vtable_pointer, sizeof(vtable_pointer));

        // Absent member: the constant, and nothing dispatched.
        g_delegation_probe = DelegationProbe{};
        std::vector<uint8_t> expected(object);
        expect(call_delegation(entry, object.data(), args) == entry.absent);
        expect(g_delegation_probe.calls == 0);
        expect(g_delegation_probe.poison_calls == 0);
        // The thunk writes nothing through its own object, ever.
        expect(std::memcmp(object.data(), expected.data(), object.size()) == 0);

        // Present member: dispatched through the named slot, on the member.
        void *const member_pointer = member.data();
        std::memcpy(object.data() + entry.member, &member_pointer,
                    sizeof(member_pointer));
        expected = object;
        g_delegation_probe = DelegationProbe{};
        // Three of these DISCARD the delegate's answer - both their paths
        // land on one  - so the dispatch still has to be
        // observed through the probe rather than through the return value.
        expect(call_delegation(entry, object.data(), args)
               == (entry.discards ? entry.absent : DelegationAnswer));
        expect(g_delegation_probe.calls == 1);
        // The receiver is the MEMBER, not the thunk's own object - the whole
        // point of the shape, and the one thing a plausible-looking body gets
        // wrong.
        expect(g_delegation_probe.self == member.data());
        // Every other slot is poisoned, so reaching the delegate at all proves
        // the slot literal.
        expect(g_delegation_probe.poison_calls == 0);
        for (int index = 0; index < entry.forwarded; ++index) {
            expect(g_delegation_probe.args[index] == args[index]);
        }
        expect(std::memcmp(object.data(), expected.data(), object.size()) == 0);
    }

    // The unguarded family: no member, no absent-member path, and the
    // receiver is the thunk's OWN object rather than something it loaded.
    for (const PlainDelegationCase &entry : g_plain_delegation_cases) {
        std::vector<uint8_t> object(0x400, 0);
        std::vector<void *> vtable(0x100,
                                   reinterpret_cast<void *>(&delegation_poison));
        vtable[entry.slot / sizeof(void *)] =
            entry.forwarded == 0 ? reinterpret_cast<void *>(&delegation_observe_0)
          : entry.forwarded == 1 ? reinterpret_cast<void *>(&delegation_observe_1)
          : entry.forwarded == 2 ? reinterpret_cast<void *>(&delegation_observe_2)
                                 : reinterpret_cast<void *>(&delegation_observe_3);
        void *const vtable_pointer = vtable.data();
        std::memcpy(object.data(), &vtable_pointer, sizeof(vtable_pointer));
        std::vector<uint8_t> expected(object);

        g_delegation_probe = DelegationProbe{};
        const int result = call_plain(entry, object.data(), args);
        expect(g_delegation_probe.calls == 1);
        // The receiver is the object itself here, not a member of it.
        expect(g_delegation_probe.self == object.data());
        expect(g_delegation_probe.poison_calls == 0);
        for (int index = 0; index < entry.forwarded; ++index) {
            expect(g_delegation_probe.args[index] == args[index]);
        }
        if (entry.answer >= 0) {
            // A constant answer, not the delegate's - the delegate returns
            // DelegationAnswer, so a body that passed it through would fail.
            expect(result == entry.answer);
        }
        expect(std::memcmp(object.data(), expected.data(), object.size()) == 0);
    }
}

void test_constant_return_stubs_wave4() {
    // Two more constant returns, checked the way every other one in this file
    // is: a seeded object, the method and its redirect both entered with
    // different arguments, and a byte compare proving neither wrote anything.
    std::vector<uint8_t> popup_storage(sizeof(Popup) + 32);
    std::vector<uint8_t> popup_expected(popup_storage.size());
    auto *popup_stub = reinterpret_cast<Popup *>(popup_storage.data() + 16);
    seed_storage(popup_storage.data(), popup_expected.data(),
                 popup_storage.size());
    std::memcpy(popup_expected.data(), popup_storage.data(),
                popup_storage.size());
    popup_stub->on_redraw_nc(1, 2);
    popup_on_redraw_nc_redirect(popup_stub, nullptr, -1, -2);
    expect_storage_bytes(popup_storage.data(), popup_expected.data(),
                         popup_storage.size());

    std::vector<uint8_t> picker_storage(sizeof(ProdPicker) + 32);
    std::vector<uint8_t> picker_expected(picker_storage.size());
    auto *picker_stub =
        reinterpret_cast<ProdPicker *>(picker_storage.data() + 16);
    seed_storage(picker_storage.data(), picker_expected.data(),
                 picker_storage.size());
    std::memcpy(picker_expected.data(), picker_storage.data(),
                picker_storage.size());
    picker_stub->on_redraw_nc(1, 2);
    prod_picker_on_redraw_nc_redirect(picker_stub, nullptr, -1, -2);
    expect_storage_bytes(picker_storage.data(), picker_expected.data(),
                         picker_storage.size());

    // AlphaMenu and UV2Player carry no fields at all - nothing pins their
    // layout - so the storage here is just an arena to prove the bodies read
    // and write nothing through it.
    std::vector<uint8_t> menu_storage(64);
    std::vector<uint8_t> menu_expected(menu_storage.size());
    auto *menu = reinterpret_cast<AlphaMenu *>(menu_storage.data() + 16);
    seed_storage(menu_storage.data(), menu_expected.data(),
                 menu_storage.size());
    std::memcpy(menu_expected.data(), menu_storage.data(), menu_storage.size());
    expect(menu->requested_height() == 0);
    expect(alpha_menu_requested_height_redirect(menu, nullptr) == 0);
    expect_storage_bytes(menu_storage.data(), menu_expected.data(),
                         menu_storage.size());

    std::vector<uint8_t> player_storage(64);
    std::vector<uint8_t> player_expected(player_storage.size());
    auto *player = reinterpret_cast<UV2Player *>(player_storage.data() + 16);
    seed_storage(player_storage.data(), player_expected.data(),
                 player_storage.size());
    std::memcpy(player_expected.data(), player_storage.data(),
                player_storage.size());
    expect(player->UNK4() == 0);
    expect(uv2_player_unk4_redirect(player, nullptr) == 0);
    expect_storage_bytes(player_storage.data(), player_expected.data(),
                         player_storage.size());

    // energy_limit ignores its argument entirely; every value must answer 10.
    for (const int faction : {0, 1, -1, INT_MAX, INT_MIN}) {
        expect(energy_limit(faction) == 10);
    }
}

void test_deleting_thunks() {
    func_operator_delete *const saved_free = ScrollOperatorDelete;
    ScrollOperatorDelete = &observe_deleting_free;
    for (const DeletingCase &entry : g_deleting_cases) {
        // Every flag value, so a body that freed on any nonzero
        // word rather than on bit 0 is caught by mode 2.
        for (unsigned int mode = 0; mode < 4; ++mode) {
            func_deleting_dtor *const saved = *entry.slot;
            *entry.slot = &observe_deleting_dtor;
            void *const object = deleting_receiver();
            g_deleting_dtor_calls = 0;
            g_deleting_dtor_seen = nullptr;
            g_deleting_free_calls = 0;
            g_deleting_free_seen = nullptr;
            void *const result = entry.body(object, nullptr, mode);
            expect(g_deleting_dtor_calls == 1);
            expect(g_deleting_dtor_seen == object);
            expect(g_deleting_free_calls == static_cast<int>(mode & 1));
            expect(g_deleting_free_seen ==
                   ((mode & 1) ? object : nullptr));
            expect(result == object);
            *entry.slot = saved;
        }
    }
    for (const DeletingAdjustCase &entry : g_deleting_adjust_cases) {
        func_deleting_dtor *const saved = *entry.slot;
        *entry.slot = &observe_deleting_dtor;
        uint8_t *const object = deleting_receiver();
        const int32_t vtordisp = entry.vtordisp
            ? *reinterpret_cast<const int32_t *>(object - 4) : 0;
        void *const expected = object - entry.adjust - vtordisp;
        g_deleting_dtor_calls = 0;
        g_deleting_dtor_seen = nullptr;
        g_deleting_free_calls = 0;
        g_deleting_free_seen = nullptr;
        void *const result = entry.body(object, nullptr, 1u);
        expect(g_deleting_dtor_calls == 1);
        expect(g_deleting_dtor_seen == expected);
        expect(g_deleting_free_calls == 1);
        expect(g_deleting_free_seen == expected);
        expect(result == expected);
        *entry.slot = saved;
    }
    for (const DeletingSeamCase_nullary &entry :
             g_deleting_seam_cases_nullary) {
        func_deleting_forward_nullary *const saved = *entry.slot;
        *entry.slot = &observe_deleting_forward_nullary;
        uint8_t *const object = deleting_receiver();
        const int32_t vtordisp = entry.vtordisp
            ? *reinterpret_cast<const int32_t *>(object - 4)
            : 0;
        void *const expected = object - entry.adjust - vtordisp;
        g_deleting_forward_calls = 0;
        g_deleting_forward_seen = nullptr;
        g_deleting_forward_arg = 0;
        void *const result = entry.body(object, nullptr);
        expect(g_deleting_forward_calls == 1);
        expect(g_deleting_forward_seen == expected);
        expect(result == &g_deleting_forward_calls);
        *entry.slot = saved;
    }
    for (const DeletingSeamCase &entry :
             g_deleting_seam_cases) {
        func_deleting_forward *const saved = *entry.slot;
        *entry.slot = &observe_deleting_forward;
        uint8_t *const object = deleting_receiver();
        const int32_t vtordisp = entry.vtordisp
            ? *reinterpret_cast<const int32_t *>(object - 4)
            : 0;
        void *const expected = object - entry.adjust - vtordisp;
        g_deleting_forward_calls = 0;
        g_deleting_forward_seen = nullptr;
        g_deleting_forward_arg = 0;
        void *const result = entry.body(object, nullptr, 0x2A2Au);
        expect(g_deleting_forward_calls == 1);
        expect(g_deleting_forward_seen == expected);
        expect(result == &g_deleting_forward_calls);
        expect(g_deleting_forward_arg == 0x2A2Au);
        *entry.slot = saved;
    }
    ScrollOperatorDelete = saved_free;
}

void test_adjustor_thunks() {
    for (const AdjustorCase_i_i &entry : g_adjustor_cases_i_i) {
        func_adjustor_i_i *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_i_i;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        int const result = entry.thunk(self, nullptr, 0x1010);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(result == 0x5A5A);
        expect(g_adjustor_args[0] == 0x1010);
        *entry.slot = saved;
    }
    for (const AdjustorCase_i_ii &entry : g_adjustor_cases_i_ii) {
        func_adjustor_i_ii *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_i_ii;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        int const result = entry.thunk(self, nullptr, 0x1010, 0x1111);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(result == 0x5A5A);
        expect(g_adjustor_args[0] == 0x1010);
        expect(g_adjustor_args[1] == 0x1111);
        *entry.slot = saved;
    }
    for (const AdjustorCase_i_p &entry : g_adjustor_cases_i_p) {
        func_adjustor_i_p *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_i_p;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        int const result = entry.thunk(self, nullptr,
                                       reinterpret_cast<void *>(0x1010));
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(result == 0x5A5A);
        expect(g_adjustor_args[0] == 0x1010);
        *entry.slot = saved;
    }
    for (const AdjustorCase_i_piii &entry : g_adjustor_cases_i_piii) {
        func_adjustor_i_piii *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_i_piii;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        int const result = entry.thunk(self, nullptr,
                                       reinterpret_cast<void *>(0x1010),
                                       0x1111, 0x1212, 0x1313);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(result == 0x5A5A);
        expect(g_adjustor_args[0] == 0x1010);
        expect(g_adjustor_args[1] == 0x1111);
        expect(g_adjustor_args[2] == 0x1212);
        expect(g_adjustor_args[3] == 0x1313);
        *entry.slot = saved;
    }
    for (const AdjustorCase_p_u &entry : g_adjustor_cases_p_u) {
        func_adjustor_p_u *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_p_u;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        void * const result = entry.thunk(self, nullptr, 0x1010);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(result == &g_adjustor_calls);
        expect(g_adjustor_args[0] == 0x1010);
        *entry.slot = saved;
    }
    for (const AdjustorCase_v &entry : g_adjustor_cases_v) {
        func_adjustor_v *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_v;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        entry.thunk(self, nullptr);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        *entry.slot = saved;
    }
    for (const AdjustorCase_v_i &entry : g_adjustor_cases_v_i) {
        func_adjustor_v_i *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_v_i;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        entry.thunk(self, nullptr, 0x1010);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(g_adjustor_args[0] == 0x1010);
        *entry.slot = saved;
    }
    for (const AdjustorCase_v_ii &entry : g_adjustor_cases_v_ii) {
        func_adjustor_v_ii *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_v_ii;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        entry.thunk(self, nullptr, 0x1010, 0x1111);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(g_adjustor_args[0] == 0x1010);
        expect(g_adjustor_args[1] == 0x1111);
        *entry.slot = saved;
    }
    for (const AdjustorCase_v_pi &entry : g_adjustor_cases_v_pi) {
        func_adjustor_v_pi *const saved = *entry.slot;
        *entry.slot = &observe_adjustor_v_pi;
        uint8_t *const self = adjustor_receiver();
        const int32_t vtordisp =
            *reinterpret_cast<const int32_t *>(self - entry.displacement);
        g_adjustor_calls = 0;
        g_adjustor_seen = nullptr;
        entry.thunk(self, nullptr, reinterpret_cast<void *>(0x1010), 0x1111);
        expect(g_adjustor_calls == 1);
        expect(g_adjustor_seen == self - vtordisp - entry.adjust);
        expect(g_adjustor_args[0] == 0x1010);
        expect(g_adjustor_args[1] == 0x1111);
        *entry.slot = saved;
    }
}

namespace {

struct InitThunkCase {
    void(__cdecl *thunk)();
    void *slot;                     // the global seam
    func_thiscall_teardown **ctor_slot;
    func_atexit_callback *callback; // the ??__F it registers
};

const InitThunkCase g_init_scalar_cases[] = {
    {&construct_alphamenu_wave, &g_ALPHAMENU_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00402F30U)},
    {&construct_unused_sprite_var02, &g_UNUSED_SPRITE_VAR02, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404460U)},
    {&construct_unused_sprite_var11, &g_UNUSED_SPRITE_VAR11, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404490U)},
    {&construct_unused_sprite_var06, &g_UNUSED_SPRITE_VAR06, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004044C0U)},
    {&construct_unused_sprite_var09, &g_UNUSED_SPRITE_VAR09, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004044F0U)},
    {&construct_unused_sprite_var21, &g_UNUSED_SPRITE_VAR21, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404520U)},
    {&construct_unused_sprite_var05, &g_UNUSED_SPRITE_VAR05, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404550U)},
    {&construct_unused_sprite_var08, &g_UNUSED_SPRITE_VAR08, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404580U)},
    {&construct_unused_sprite_var04, &g_UNUSED_SPRITE_VAR04, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004045B0U)},
    {&construct_unused_sprite_var01, &g_UNUSED_SPRITE_VAR01, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004045E0U)},
    {&construct_unused_sprite_var18, &g_UNUSED_SPRITE_VAR18, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404610U)},
    {&construct_unused_sprite_var03, &g_UNUSED_SPRITE_VAR03, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404640U)},
    {&construct_unused_sprite_var20, &g_UNUSED_SPRITE_VAR20, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404670U)},
    {&construct_unused_sprite_var16, &g_UNUSED_SPRITE_VAR16, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004046A0U)},
    {&construct_unused_sprite_var14, &g_UNUSED_SPRITE_VAR14, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004046D0U)},
    {&construct_unused_sprite_var22, &g_UNUSED_SPRITE_VAR22, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404700U)},
    {&construct_unused_sprite_var10, &g_UNUSED_SPRITE_VAR10, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404730U)},
    {&construct_unused_sprite_var15, &g_UNUSED_SPRITE_VAR15, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404760U)},
    {&construct_unused_sprite_var13, &g_UNUSED_SPRITE_VAR13, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404790U)},
    {&construct_unused_sprite_var17, &g_UNUSED_SPRITE_VAR17, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004047C0U)},
    {&construct_unused_sprite_var19, &g_UNUSED_SPRITE_VAR19, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004047F0U)},
    {&construct_unused_sprite_var12, &g_UNUSED_SPRITE_VAR12, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404820U)},
    {&construct_unused_sprite_var07, &g_UNUSED_SPRITE_VAR07, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00404850U)},
    {&construct_basewin_wave, &g_BASEWIN_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00408400U)},
    {&construct_credits_wave, &g_CREDITS_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00428770U)},
    {&construct_designwin_wave, &g_DESIGNWIN_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x004328A0U)},
    {&construct_menu_up_wave, &g_MENU_UP_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x004454C0U)},
    {&construct_menu_down_wave, &g_MENU_DOWN_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x004454F0U)},
    {&construct_scoot_wave, &g_SCOOT_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00445520U)},
    {&construct_ok_wave, &g_OK_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00445550U)},
    {&construct_passover_wave, &g_PASSOVER_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00445580U)},
    {&construct_fx, &g_FX, &FXInitCtor, reinterpret_cast<func_atexit_callback *>(0x004455B0U)},
    {&construct_ambience, &g_AMBIENCE, &AmbienceInitCtor, reinterpret_cast<func_atexit_callback *>(0x004455E0U)},
    {&construct_pcx_parse_temp_buffer1, &g_PCX_PARSE_TEMP_BUFFER1, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044CC40U)},
    {&construct_iface_std_popups_top_left_sprite, &g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044CFE0U)},
    {&construct_iface_std_popups_top_right_sprite, &g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D010U)},
    {&construct_iface_std_popups_bot_left_sprite, &g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D040U)},
    {&construct_iface_std_popups_bot_right_sprite, &g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D070U)},
    {&construct_iface_std_popups_top_mid_sprite, &g_IFACE_STD_POPUPS_TOP_MID_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D0A0U)},
    {&construct_iface_std_popups_bot_mid_sprite, &g_IFACE_STD_POPUPS_BOT_MID_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D0D0U)},
    {&construct_iface_std_popups_mid_left_sprite, &g_IFACE_STD_POPUPS_MID_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D100U)},
    {&construct_iface_std_popups_mid_right_sprite, &g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D130U)},
    {&construct_iface_std_popups_middle_buffer, &g_IFACE_STD_POPUPS_MIDDLE_BUFFER, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D160U)},
    {&construct_unused_caviardata_var1, &g_UNUSED_CAVIARDATA_VAR1, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D190U)},
    {&construct_ssf_caviardata, &g_SSF_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D1C0U)},
    {&construct_sdp_caviardata, &g_SDP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D1F0U)},
    {&construct_sas_caviardata, &g_SAS_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D220U)},
    {&construct_scd_caviardata, &g_SCD_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D250U)},
    {&construct_scj_caviardata, &g_SCJ_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D280U)},
    {&construct_sags_caviardata, &g_SAGS_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D2B0U)},
    {&construct_sft_caviardata, &g_SFT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D2E0U)},
    {&construct_vhr_caviardata1, &g_VHR_CAVIARDATA1, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D310U)},
    {&construct_sht_caviardata, &g_SHT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D340U)},
    {&construct_srb_caviardata, &g_SRB_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D370U)},
    {&construct_asas_caviardata, &g_ASAS_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D3A0U)},
    {&construct_reslaser_caviardata, &g_RESLASER_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D3D0U)},
    {&construct_resbolt_caviardata, &g_RESBOLT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D400U)},
    {&construct_funload_caviardata, &g_FUNLOAD_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D430U)},
    {&construct_tecload_caviardata, &g_TECLOAD_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D460U)},
    {&construct_sp_disswave_caviardata, &g_SP_DISSWAVE_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D490U)},
    {&construct_sp_marined_caviardata, &g_SP_MARINED_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D4C0U)},
    {&construct_sp_nanoo_caviardata, &g_SP_NANOO_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D4F0U)},
    {&construct_sp_soporific_caviardata, &g_SP_SOPORIFIC_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D520U)},
    {&construct_aa01_caviardata, &g_AA01_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D550U)},
    {&construct_aa_rover_caviardata, &g_AA_ROVER_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D580U)},
    {&construct_ax_caviardata, &g_AX_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D650U)},
    {&construct_aa_caviardata, &g_AA_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D680U)},
    {&construct_acolpod_caviardata, &g_ACOLPOD_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D6B0U)},
    {&construct_at_caviardata, &g_AT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D6E0U)},
    {&construct_vta_caviardata, &g_VTA_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D710U)},
    {&construct_atp_caviardata, &g_ATP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D740U)},
    {&construct_ssfa_caviardata, &g_SSFA_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D770U)},
    {&construct_sfta_caviardata, &g_SFTA_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D7A0U)},
    {&construct_vw00_caviardata, &g_VW00_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D7D0U)},
    {&construct_viptawl_caviardata, &g_VIPTAWL_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D800U)},
    {&construct_viptasgn_caviardata, &g_VIPTASGN_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D830U)},
    {&construct_viptapsi_caviardata, &g_VIPTAPSI_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D860U)},
    {&construct_ptmod_caviardata, &g_PTMOD_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D930U)},
    {&construct_vb_caviardata, &g_VB_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D960U)},
    {&construct_vbp_caviardata, &g_VBP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D990U)},
    {&construct_vgmc_caviardata, &g_VGMC_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D9C0U)},
    {&construct_vgmcp_caviardata, &g_VGMCP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044D9F0U)},
    {&construct_vlights_caviardata, &g_VLIGHTS_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DA20U)},
    {&construct_vpt_caviardata, &g_VPT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DA50U)},
    {&construct_a_caviardata, &g_A_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DA80U)},
    {&construct_apwall_caviardata, &g_APWALL_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DAB0U)},
    {&construct_asgen_caviardata, &g_ASGEN_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DAE0U)},
    {&construct_apsid_caviardata, &g_APSID_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DB10U)},
    {&construct_va01_caviardata, &g_VA01_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DB40U)},
    {&construct_vhr_caviardata2, &g_VHR_CAVIARDATA2, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DBC0U)},
    {&construct_vi_caviardata, &g_VI_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DC40U)},
    {&construct_vgmt_caviardata, &g_VGMT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DC70U)},
    {&construct_vgmtp_caviardata, &g_VGMTP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DCA0U)},
    {&construct_unused_caviardata_var2, &g_UNUSED_CAVIARDATA_VAR2, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DCD0U)},
    {&construct_vwntu_caviardata, &g_VWNTU_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DD00U)},
    {&construct_vt_caviardata, &g_VT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DD30U)},
    {&construct_drop_caviardata, &g_DROP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DD60U)},
    {&construct_droplet_caviardata, &g_DROPLET_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DD90U)},
    {&construct_vcl_caviardata, &g_VCL_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DDC0U)},
    {&construct_vclt00_caviardata, &g_VCLT00_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DE40U)},
    {&construct_vht_vbp_caviardata, &g_VHT_VBP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DE70U)},
    {&construct_vhtp_caviardata, &g_VHTP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DEA0U)},
    {&construct_vhttp_caviardata, &g_VHTTP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DF20U)},
    {&construct_vsp_caviardata, &g_VSP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044DFA0U)},
    {&construct_vsptf_caviardata, &g_VSPTF_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E020U)},
    {&construct_vsptb_caviardata, &g_VSPTB_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E050U)},
    {&construct_vfl_caviardata, &g_VFL_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E080U)},
    {&construct_vgs_caviardata, &g_VGS_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E0B0U)},
    {&construct_vgsp_caviardata, &g_VGSP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E0E0U)},
    {&construct_vjtp_caviardata, &g_VJTP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E110U)},
    {&construct_vcu_caviardata, &g_VCU_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E190U)},
    {&construct_vcup_caviardata, &g_VCUP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E1C0U)},
    {&construct_vcuw_caviardata, &g_VCUW_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E240U)},
    {&construct_vct_caviardata, &g_VCT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E270U)},
    {&construct_vctp_caviardata, &g_VCTP_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E2A0U)},
    {&construct_vctb_caviardata, &g_VCTB_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E2D0U)},
    {&construct_vwntt_caviardata, &g_VWNTT_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E350U)},
    {&construct_vwnst_caviardata, &g_VWNST_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E380U)},
    {&construct_vwnaa_caviardata, &g_VWNAA_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E3B0U)},
    {&construct_vm_caviardata, &g_VM_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E430U)},
    {&construct_vm13_caviardata, &g_VM13_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E460U)},
    {&construct_nw_caviardata, &g_NW_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E530U)},
    {&construct_ni_caviardata, &g_NI_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E560U)},
    {&construct_nlc_caviardata, &g_NLC_CAVIARDATA, &CaviarDataInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E590U)},
    {&construct_radius1_texture, &g_RADIUS1_TEXTURE, &TextureInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E610U)},
    {&construct_radius2_texture, &g_RADIUS2_TEXTURE, &TextureInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E640U)},
    {&construct_flat_arid_land_texture, &g_FLAT_ARID_LAND_TEXTURE, &TextureInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E710U)},
    {&construct_dune_land_texture, &g_DUNE_LAND_TEXTURE, &TextureInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044E830U)},
    {&construct_rainfall_single_tile_texture, &g_RAINFALL_SINGLE_TILE_TEXTURE, &TextureInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044EB80U)},
    {&construct_ter1_mine_sprite, &g_TER1_MINE_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044ECA0U)},
    {&construct_ter1_solar_collector_sprite, &g_TER1_SOLAR_COLLECTOR_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044ECD0U)},
    {&construct_ter1_tidal_harness_sprite, &g_TER1_TIDAL_HARNESS_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044ED00U)},
    {&construct_ter1_mining_platform_sprite, &g_TER1_MINING_PLATFORM_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044ED30U)},
    {&construct_ter1_tut_blank_sprite, &g_TER1_TUT_BLANK_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044ED60U)},
    {&construct_ter1_kelp_farm_sprite, &g_TER1_KELP_FARM_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044ED90U)},
    {&construct_ter1_condenser_sprite, &g_TER1_CONDENSER_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044EDC0U)},
    {&construct_ter1_echelon_mirror_sprite, &g_TER1_ECHELON_MIRROR_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044EDF0U)},
    {&construct_ter1_borehole_sprite, &g_TER1_BOREHOLE_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044EE20U)},
    {&construct_ter1_borehole_cluster_sprite, &g_TER1_BOREHOLE_CLUSTER_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044EE50U)},
    {&construct_ter1_monolith_sprite, &g_TER1_MONOLITH_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F1F0U)},
    {&construct_ter1_bunker_sprite, &g_TER1_BUNKER_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F220U)},
    {&construct_ter1_airbase_sprite, &g_TER1_AIRBASE_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F250U)},
    {&construct_ter1_sensor_array_sprite, &g_TER1_SENSOR_ARRAY_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F280U)},
    {&construct_red_alien_head_icon_sprite, &g_RED_ALIEN_HEAD_ICON_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F6C0U)},
    {&construct_red_male_head_icon_sprite, &g_RED_MALE_HEAD_ICON_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F790U)},
    {&construct_null_resource_icon_sprite, &g_NULL_RESOURCE_ICON_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F7C0U)},
    {&construct_icon_tile_square_sprite, &g_ICON_TILE_SQUARE_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F840U)},
    {&construct_battle_mind_worm_sprite, &g_BATTLE_MIND_WORM_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F910U)},
    {&construct_battle_isle_deep_sprite, &g_BATTLE_ISLE_DEEP_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F940U)},
    {&construct_battle_locusts_chiron_sprite, &g_BATTLE_LOCUSTS_CHIRON_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F970U)},
    {&construct_battle_fungal_tower_sprite, &g_BATTLE_FUNGAL_TOWER_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F9A0U)},
    {&construct_battle_spore_launcher_sprite, &g_BATTLE_SPORE_LAUNCHER_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044F9D0U)},
    {&construct_battle_sealurk_sprite, &g_BATTLE_SEALURK_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FA00U)},
    {&construct_scroll_bar_filler_icon_sprites, &g_SCROLL_BAR_FILLER_ICON_SPRITES, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FC10U)},
    {&construct_scroll_bar_small_filler_icon_sprite, &g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FC40U)},
    {&construct_iface_general_windows_top_left_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FD10U)},
    {&construct_iface_general_windows_top_right_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FD40U)},
    {&construct_iface_general_windows_bot_left_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FD70U)},
    {&construct_iface_general_windows_bot_right_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FDA0U)},
    {&construct_iface_general_windows_mid_left_sprite, &g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FDD0U)},
    {&construct_iface_general_windows_mid_right_sprite, &g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FE00U)},
    {&construct_iface_general_windows_top_mid_sprite, &g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FE30U)},
    {&construct_iface_general_windows_bot_mid_sprite, &g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FE60U)},
    {&construct_iface_general_windows_noncap_mid_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FE90U)},
    {&construct_iface_general_windows_noncap_left_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FEC0U)},
    {&construct_iface_general_windows_noncap_right_sprite, &g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FEF0U)},
    {&construct_unused_sprite_var23, &g_UNUSED_SPRITE_VAR23, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FF20U)},
    {&construct_unused_sprite_var24, &g_UNUSED_SPRITE_VAR24, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FF50U)},
    {&construct_unused_sprite_var25, &g_UNUSED_SPRITE_VAR25, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FF80U)},
    {&construct_unused_sprite_var26, &g_UNUSED_SPRITE_VAR26, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FFB0U)},
    {&construct_unused_sprite_var27, &g_UNUSED_SPRITE_VAR27, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0044FFE0U)},
    {&construct_unused_sprite_var28, &g_UNUSED_SPRITE_VAR28, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450010U)},
    {&construct_unused_sprite_var29, &g_UNUSED_SPRITE_VAR29, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450040U)},
    {&construct_unused_sprite_var30, &g_UNUSED_SPRITE_VAR30, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450070U)},
    {&construct_unused_sprite_var31, &g_UNUSED_SPRITE_VAR31, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004500A0U)},
    {&construct_unused_sprite_var32, &g_UNUSED_SPRITE_VAR32, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004500D0U)},
    {&construct_unused_sprite_var33, &g_UNUSED_SPRITE_VAR33, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450100U)},
    {&construct_unused_sprite_var34, &g_UNUSED_SPRITE_VAR34, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450130U)},
    {&construct_unused_sprite_var35, &g_UNUSED_SPRITE_VAR35, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450160U)},
    {&construct_unused_sprite_var36, &g_UNUSED_SPRITE_VAR36, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450190U)},
    {&construct_unused_sprite_var37, &g_UNUSED_SPRITE_VAR37, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004501C0U)},
    {&construct_unused_sprite_var38, &g_UNUSED_SPRITE_VAR38, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004501F0U)},
    {&construct_unused_sprite_var39, &g_UNUSED_SPRITE_VAR39, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450220U)},
    {&construct_unused_sprite_var40, &g_UNUSED_SPRITE_VAR40, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450250U)},
    {&construct_unused_sprite_var41, &g_UNUSED_SPRITE_VAR41, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450280U)},
    {&construct_unused_sprite_var42, &g_UNUSED_SPRITE_VAR42, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004502B0U)},
    {&construct_unused_sprite_var43, &g_UNUSED_SPRITE_VAR43, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004502E0U)},
    {&construct_unused_sprite_var44, &g_UNUSED_SPRITE_VAR44, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450310U)},
    {&construct_unused_sprite_var45, &g_UNUSED_SPRITE_VAR45, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450340U)},
    {&construct_unused_sprite_var46, &g_UNUSED_SPRITE_VAR46, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450370U)},
    {&construct_unused_sprite_var47, &g_UNUSED_SPRITE_VAR47, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004503F0U)},
    {&construct_unused_sprite_var48, &g_UNUSED_SPRITE_VAR48, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450420U)},
    {&construct_unused_sprite_var49, &g_UNUSED_SPRITE_VAR49, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450450U)},
    {&construct_unused_sprite_var50, &g_UNUSED_SPRITE_VAR50, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450480U)},
    {&construct_unused_sprite_var51, &g_UNUSED_SPRITE_VAR51, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004504B0U)},
    {&construct_unused_sprite_var52, &g_UNUSED_SPRITE_VAR52, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004504E0U)},
    {&construct_unused_sprite_var53, &g_UNUSED_SPRITE_VAR53, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450510U)},
    {&construct_unused_sprite_var54, &g_UNUSED_SPRITE_VAR54, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450540U)},
    {&construct_unused_sprite_var55, &g_UNUSED_SPRITE_VAR55, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450570U)},
    {&construct_unused_sprite_var56, &g_UNUSED_SPRITE_VAR56, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004505A0U)},
    {&construct_unused_sprite_var57, &g_UNUSED_SPRITE_VAR57, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004505D0U)},
    {&construct_unused_sprite_var58, &g_UNUSED_SPRITE_VAR58, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450600U)},
    {&construct_unused_sprite_var59, &g_UNUSED_SPRITE_VAR59, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450630U)},
    {&construct_unused_sprite_var60, &g_UNUSED_SPRITE_VAR60, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450660U)},
    {&construct_unused_sprite_var61, &g_UNUSED_SPRITE_VAR61, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450690U)},
    {&construct_unused_sprite_var62, &g_UNUSED_SPRITE_VAR62, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004506C0U)},
    {&construct_unused_sprite_var63, &g_UNUSED_SPRITE_VAR63, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004506F0U)},
    {&construct_unused_sprite_var64, &g_UNUSED_SPRITE_VAR64, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450720U)},
    {&construct_unused_sprite_var65, &g_UNUSED_SPRITE_VAR65, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450750U)},
    {&construct_unused_sprite_var66, &g_UNUSED_SPRITE_VAR66, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450780U)},
    {&construct_unused_sprite_var67, &g_UNUSED_SPRITE_VAR67, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004507B0U)},
    {&construct_unused_sprite_var68, &g_UNUSED_SPRITE_VAR68, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004507E0U)},
    {&construct_unused_sprite_var69, &g_UNUSED_SPRITE_VAR69, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450810U)},
    {&construct_unused_sprite_var70, &g_UNUSED_SPRITE_VAR70, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450840U)},
    {&construct_unused_sprite_var71, &g_UNUSED_SPRITE_VAR71, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450870U)},
    {&construct_unused_sprite_var72, &g_UNUSED_SPRITE_VAR72, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004508A0U)},
    {&construct_unused_sprite_var73, &g_UNUSED_SPRITE_VAR73, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x004508D0U)},
    {&construct_unused_sprite_var74, &g_UNUSED_SPRITE_VAR74, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450900U)},
    {&construct_unused_sprite_var75, &g_UNUSED_SPRITE_VAR75, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450C00U)},
    {&construct_unused_sprite_var76, &g_UNUSED_SPRITE_VAR76, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450C30U)},
    {&construct_unused_sprite_var77, &g_UNUSED_SPRITE_VAR77, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450C60U)},
    {&construct_unused_sprite_var78, &g_UNUSED_SPRITE_VAR78, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450C90U)},
    {&construct_unused_sprite_var79, &g_UNUSED_SPRITE_VAR79, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450CC0U)},
    {&construct_unused_sprite_var80, &g_UNUSED_SPRITE_VAR80, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450CF0U)},
    {&construct_unused_sprite_var81, &g_UNUSED_SPRITE_VAR81, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450D20U)},
    {&construct_unused_sprite_var82, &g_UNUSED_SPRITE_VAR82, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450D50U)},
    {&construct_unused_sprite_var83, &g_UNUSED_SPRITE_VAR83, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00450D80U)},
    {&construct_maininterface_wave, &g_MAININTERFACE_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x004595B0U)},
    {&construct_jackal_font, &g_JACKAL_FONT, &FontInitCtor, reinterpret_cast<func_atexit_callback *>(0x0045F940U)},
    {&construct_multiwin_wave, &g_MULTIWIN_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00477E60U)},
    {&construct_prefwin_buttongroup, &g_PREFWIN_BUTTONGROUP, &ButtonGroupInitCtor, reinterpret_cast<func_atexit_callback *>(0x0048D540U)},
    {&construct_vehdraw_caviar, &g_VEHDRAW_CAVIAR, &CaviarInitCtor, reinterpret_cast<func_atexit_callback *>(0x004BF700U)},
    {&construct_vehdraw_buffer, &g_VEHDRAW_BUFFER, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x004BF730U)},
    {&construct_wave_device, &g_WAVE_DEVICE, &WaveDeviceInitCtor, reinterpret_cast<func_atexit_callback *>(0x004C5C70U)},
    {&construct_console_timer, &g_CONSOLE_TIMER, &TimeInitCtor, reinterpret_cast<func_atexit_callback *>(0x0050E9A0U)},
    {&construct_top_menu_wave, &g_TOP_MENU_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x00584D20U)},
    {&construct_crash_landing_wave, &g_CRASH_LANDING_WAVE, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x005AE110U)},
    {&construct_wave_general, &g_WAVE_GENERAL, &WaveInitCtor, reinterpret_cast<func_atexit_callback *>(0x005B9C30U)},
    {&construct_buffer_sprite, &g_BUFFER_SPRITE, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x005D71F0U)},
    {&construct_buffer, &g_BUFFER, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x005E37D0U)},
    {&construct_win_buffer, &g_WIN_BUFFER, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x005EB370U)},
    {&construct_radiobutton_sprite_1, &g_RADIOBUTTON_SPRITE_1, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0060D080U)},
    {&construct_radiobutton_sprite_2, &g_RADIOBUTTON_SPRITE_2, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0060D0C0U)},
    {&construct_checkbox_sprite_1, &g_CHECKBOX_SPRITE_1, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0060E610U)},
    {&construct_checkbox_sprite_2, &g_CHECKBOX_SPRITE_2, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x0060E650U)},
    {&construct_filewin_sprite_1, &g_FILEWIN_SPRITE_1, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x006137B0U)},
    {&construct_filewin_sprite_2, &g_FILEWIN_SPRITE_2, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x006137F0U)},
    {&construct_filewin_sprite_3, &g_FILEWIN_SPRITE_3, &SpriteInitCtor, reinterpret_cast<func_atexit_callback *>(0x00613830U)},
    {&construct_stringtable, &StringTable, &StringsInitCtor, reinterpret_cast<func_atexit_callback *>(0x006168C0U)},
    {&construct_caviar_buffer_1, &g_CAVIAR_BUFFER_1, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x00616AC0U)},
    {&construct_caviar_buffer_2, &g_CAVIAR_BUFFER_2, &BufferInitCtor, reinterpret_cast<func_atexit_callback *>(0x00616B00U)},
};

struct InitArrayCase {
    void(__cdecl *thunk)();
    void *slot;
    uint32_t element_size;
    int count;
    func_thiscall_teardown **ctor_slot;
    func_thiscall_teardown **dtor_slot;
    func_atexit_callback *callback;
};

const InitArrayCase g_init_array_cases[] = {
    {&construct_cpu_waves, &g_CPU_WAVES, 0x6C, 45, &WaveElementCtor, &WaveElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00445480U)},
    {&construct_iface_close_x_sprites, &g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CCD0U)},
    {&construct_iface_box_sprites1, &g_IFACE_BOX_SPRITES1, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CD20U)},
    {&construct_iface_box_sprites2, &g_IFACE_BOX_SPRITES2, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CD70U)},
    {&construct_iface_box_sprites3, &g_IFACE_BOX_SPRITES3, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CDC0U)},
    {&construct_iface_box_sprites4, &g_IFACE_BOX_SPRITES4, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CE10U)},
    {&construct_iface_box_sprites5, &g_IFACE_BOX_SPRITES5, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CE60U)},
    {&construct_iface_box_sprites6, &g_IFACE_BOX_SPRITES6, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CEB0U)},
    {&construct_iface_box_sprites7, &g_IFACE_BOX_SPRITES7, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CF00U)},
    {&construct_iface_box_sprites8, &g_IFACE_BOX_SPRITES8, 0x2C, 51, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CF50U)},
    {&construct_iface_box_sprite_buffers, &g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, &BufferElementCtor, &BufferElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044CFA0U)},
    {&construct_aa_wing_caviardata, &g_AA_WING_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044D5C0U)},
    {&construct_acp_caviardata, &g_ACP_CAVIARDATA, 0xC, 4, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044D610U)},
    {&construct_viptr_caviardata, &g_VIPTR_CAVIARDATA, 0xC, 4, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044D8A0U)},
    {&construct_vipta_caviardata, &g_VIPTA_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044D8F0U)},
    {&construct_vr_caviardata, &g_VR_CAVIARDATA, 0xC, 4, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044DB80U)},
    {&construct_vrc_caviardata, &g_VRC_CAVIARDATA, 0xC, 4, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044DC00U)},
    {&construct_unused_caviardata_var3, &g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044DE00U)},
    {&construct_vhta0_caviardata, &g_VHTA0_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044DEE0U)},
    {&construct_vhttpa0_caviardata, &g_VHTTPA0_CAVIARDATA, 0xC, 3, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044DF60U)},
    {&construct_vspa0_caviardata, &g_VSPA0_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044DFE0U)},
    {&construct_vjt0_caviardata, &g_VJT0_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E150U)},
    {&construct_vcua0_caviardata, &g_VCUA0_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E200U)},
    {&construct_vct0_caviardata, &g_VCT0_CAVIARDATA, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E310U)},
    {&construct_vw_caviardata, &g_VW_CAVIARDATA, 0xC, 16, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E3F0U)},
    {&construct_vpbr0_caviardata, &g_VPBR0_CAVIARDATA, 0xC, 4, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E4A0U)},
    {&construct_unused_caviardata_var4, &g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E4F0U)},
    {&construct_unused_caviardata_var5, &g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, &CaviarDataElementCtor, &CaviarDataElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E5D0U)},
    {&construct_rocky_textures, &g_ROCKY_TEXTURES, 0x70, 4, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E680U)},
    {&construct_ocean_textures, &g_OCEAN_TEXTURES, 0x70, 2, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E6D0U)},
    {&construct_moist_land_textures, &g_MOIST_LAND_TEXTURES, 0x70, 16, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E750U)},
    {&construct_rainy_land_textures, &g_RAINY_LAND_TEXTURES, 0x70, 16, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E7A0U)},
    {&construct_jungle_land_textures, &g_JUNGLE_LAND_TEXTURES, 0x70, 15, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E7F0U)},
    {&construct_sunny_mesa_textures, &g_SUNNY_MESA_TEXTURES, 0x70, 8, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E870U)},
    {&construct_rainfall_single_tile_textures, &g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E8C0U)},
    {&construct_road_textures, &g_ROAD_TEXTURES, 0x70, 9, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E910U)},
    {&construct_magtube_textures, &g_MAGTUBE_TEXTURES, 0x70, 9, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E960U)},
    {&construct_river_textures, &g_RIVER_TEXTURES, 0x70, 16, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044E9B0U)},
    {&construct_mount_planet_textures, &g_MOUNT_PLANET_TEXTURES, 0x70, 3, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EA00U)},
    {&construct_garland_crater_textures, &g_GARLAND_CRATER_TEXTURES, 0x70, 3, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EA50U)},
    {&construct_fungus_textures, &g_FUNGUS_TEXTURES, 0x70, 30, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EAA0U)},
    {&construct_farm_textures, &g_FARM_TEXTURES, 0x70, 9, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EAF0U)},
    {&construct_forest_textures, &g_FOREST_TEXTURES, 0x70, 16, &TextureElementCtor, &TextureElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EB40U)},
    {&construct_ter1_white_org_yel_tile_sprites, &g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EBC0U)},
    {&construct_ter1_bottom_left_tile_sprites, &g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EC10U)},
    {&construct_ter1_unused_sprites2, &g_TER1_UNUSED_SPRITES2, 0x2C, 2, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EC60U)},
    {&construct_ter1_manifold_nexus_sprites, &g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EE90U)},
    {&construct_ter1wreck_unity_wreckage_sprites, &g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EEE0U)},
    {&construct_ter1wreck_unity_wreckage_alt_sprites, &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EF30U)},
    {&construct_fossil_field_ridge_sprites, &g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EF80U)},
    {&construct_ter1_unused_sprites1, &g_TER1_UNUSED_SPRITES1, 0x2C, 5, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044EFD0U)},
    {&construct_ter1_farm_sprites, &g_TER1_FARM_SPRITES, 0x2C, 5, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F020U)},
    {&construct_ter1_soil_enricher_sprites, &g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F070U)},
    {&construct_ter1_sea_land_resource_sprites, &g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F0C0U)},
    {&construct_ter1_landmark_resource_sprites, &g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F110U)},
    {&construct_glow_sprites, &g_GLOW_SPRITES, 0x2C, 2, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F160U)},
    {&construct_ter1_unity_pod_sprites, &g_TER1_UNITY_POD_SPRITES, 0x2C, 6, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F1B0U)},
    {&construct_rainfall_double_tile_sprites, &g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F2C0U)},
    {&construct_veh_sprites, &g_VEH_SPRITES, 0x2C, 152, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F310U)},
    {&construct_flags_veh_sprites, &g_FLAGS_VEH_SPRITES, 0x2C, 112, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F360U)},
    {&construct_icons_general_sprites, &g_ICONS_GENERAL_SPRITES, 0x2C, 16, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F3B0U)},
    {&construct_resource_icon_sprites, &g_RESOURCE_ICON_SPRITES, 0x2C, 32, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F400U)},
    {&construct_citizen_lg_cursor_sprites, &g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F450U)},
    {&construct_specialist_lg_cursor_sprites, &g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F4A0U)},
    {&construct_citizen_sm_cursor_sprites, &g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F4F0U)},
    {&construct_specialist_sm_cursor_sprites, &g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F540U)},
    {&construct_al_citizen_lg_cursor_sprites, &g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F590U)},
    {&construct_al_specialist_lg_cursor_sprites, &g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F5E0U)},
    {&construct_al_citizen_sm_cursor_sprites, &g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F630U)},
    {&construct_al_specialist_sm_cursor_sprites, &g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F680U)},
    {&construct_silver_menu_icon_sprites, &g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F700U)},
    {&construct_silver_checkbox_icon_sprites, &g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F750U)},
    {&construct_peace_sign_sprites, &g_PEACE_SIGN_SPRITES, 0x2C, 2, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F800U)},
    {&construct_xi_boom_veh_sprites, &g_XI_BOOM_VEH_SPRITES, 0x2C, 144, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F880U)},
    {&construct_xf_boom_veh_sprites, &g_XF_BOOM_VEH_SPRITES, 0x2C, 64, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044F8D0U)},
    {&construct_tech_icon_sprites, &g_TECH_ICON_SPRITES, 0x2C, 89, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FA40U)},
    {&construct_facility_icon_sprites, &g_FACILITY_ICON_SPRITES, 0x2C, 70, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FA90U)},
    {&construct_secret_project_icon_sprites, &g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FAE0U)},
    {&construct_iface_mp_combo_arrow_sprites, &g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FB30U)},
    {&construct_scroll_bar_arrow_icon_sprites, &g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FB80U)},
    {&construct_scroll_bar_small_arrow_icon_sprites, &g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FBD0U)},
    {&construct_iface_lock_sprites, &g_IFACE_LOCK_SPRITES, 0x2C, 2, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FC80U)},
    {&construct_unused_sprites_var01, &g_UNUSED_SPRITES_VAR01, 0x2C, 8, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0044FCD0U)},
    {&construct_iface_tech_tree_arrow_sprites, &g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x004503B0U)},
    {&construct_unused_sprites_var02, &g_UNUSED_SPRITES_VAR02, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450940U)},
    {&construct_unused_sprites_var03, &g_UNUSED_SPRITES_VAR03, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450990U)},
    {&construct_unused_sprites_var04, &g_UNUSED_SPRITES_VAR04, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x004509E0U)},
    {&construct_unused_sprites_var05, &g_UNUSED_SPRITES_VAR05, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450A30U)},
    {&construct_unused_sprites_var06, &g_UNUSED_SPRITES_VAR06, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450A80U)},
    {&construct_unused_sprites_var07, &g_UNUSED_SPRITES_VAR07, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450AD0U)},
    {&construct_unused_sprites_var08, &g_UNUSED_SPRITES_VAR08, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450B20U)},
    {&construct_unused_sprites_var09, &g_UNUSED_SPRITES_VAR09, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450B70U)},
    {&construct_unused_sprites_var10, &g_UNUSED_SPRITES_VAR10, 0x2C, 3, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450BC0U)},
    {&construct_basewin_sprites, &g_BASEWIN_SPRITES, 0x2C, 27, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00450DC0U)},
    {&construct_iface_green_right_arrow_sprite, &g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x00471380U)},
    {&construct_newtechwin_sprites, &g_NEWTECHWIN_SPRITES, 0x2C, 6, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x004838B0U)},
    {&construct_cursor_sprites, &g_CURSOR_SPRITES, 0x2C, 12, &SpriteElementCtor, &SpriteElementTeardown, reinterpret_cast<func_atexit_callback *>(0x0052DAF0U)},
    {&construct_fonts, &g_FONTS, 0x28, 48, &FontElementCtor, &FontElementTeardown, reinterpret_cast<func_atexit_callback *>(0x005882D0U)},
    {&construct_txtindex, &TxtIndexGlobal, 0x118, 4, &TextIndexElementCtor, &TextIndexElementTeardown, reinterpret_cast<func_atexit_callback *>(0x005FD510U)},
};

struct InitOpaqueCase {
    void(__cdecl *thunk)();
    func_thiscall_teardown **ctor_slot;
    void *storage;
    func_atexit_callback *callback;
};

const InitOpaqueCase g_init_opaque_cases[] = {
    {&construct_basewin, &BaseWinCtorTarget,
     reinterpret_cast<void *>(0x006A7628),
     reinterpret_cast<func_atexit_callback *>(0x004083D0U)},
    {&construct_battlewin, &BattleWinCtorTarget,
     reinterpret_cast<void *>(0x006EEED8),
     reinterpret_cast<func_atexit_callback *>(0x004219D0U)},
    {&construct_councwin, &CouncWinCtorTarget,
     reinterpret_cast<void *>(0x006FEC80),
     reinterpret_cast<func_atexit_callback *>(0x004243C0U)},
    {&construct_datalink, &DatalinkCtorTarget,
     reinterpret_cast<void *>(0x00703EA0),
     reinterpret_cast<func_atexit_callback *>(0x00428FB0U)},
    {&construct_designwin, &DesignWinCtorTarget,
     reinterpret_cast<void *>(0x0071F2B0),
     reinterpret_cast<func_atexit_callback *>(0x00432870U)},
    {&construct_diplopop, &DiploPopCtorTarget,
     reinterpret_cast<void *>(0x00733990),
     reinterpret_cast<func_atexit_callback *>(0x0043EFE0U)},
    {&construct_diplowin, &DiploWinCtorTarget,
     reinterpret_cast<void *>(0x0073ACD8),
     reinterpret_cast<func_atexit_callback *>(0x00440F20U)},
    {&construct_famewin, &FameWinCtorTarget,
     reinterpret_cast<void *>(0x0074DAF8),
     reinterpret_cast<func_atexit_callback *>(0x004483F0U)},
    {&construct_infowin, &InfoWinCtorTarget,
     reinterpret_cast<void *>(0x007AD2A0),
     reinterpret_cast<func_atexit_callback *>(0x004562E0U)},
    {&construct_maininterface, &MainInterfaceCtorTarget,
     reinterpret_cast<void *>(0x007AE820),
     reinterpret_cast<func_atexit_callback *>(0x00459580U)},
    {&construct_messagewin, &MessageWinCtorTarget,
     reinterpret_cast<void *>(0x007F67F8),
     reinterpret_cast<func_atexit_callback *>(0x00471340U)},
    {&construct_monuwin, &MonuWinCtorTarget,
     reinterpret_cast<void *>(0x007F9F58),
     reinterpret_cast<func_atexit_callback *>(0x00472220U)},
    {&construct_multiwin, &MultiWinCtorTarget,
     reinterpret_cast<void *>(0x007FD648),
     reinterpret_cast<func_atexit_callback *>(0x00477E30U)},
    {&construct_netwin, &NetWinCtorTarget,
     reinterpret_cast<void *>(0x0080A6F8),
     reinterpret_cast<func_atexit_callback *>(0x0047B010U)},
    {&construct_newtechwin, &NewTechWinCtorTarget,
     reinterpret_cast<void *>(0x00811E40),
     reinterpret_cast<func_atexit_callback *>(0x00483870U)},
    {&construct_pickwin, &PickWinCtorTarget,
     reinterpret_cast<void *>(0x00822718),
     reinterpret_cast<func_atexit_callback *>(0x00488770U)},
    {&construct_prefwin, &PrefWinCtorTarget,
     reinterpret_cast<void *>(0x008578D8),
     reinterpret_cast<func_atexit_callback *>(0x0048D510U)},
    {&construct_quaylewin, &QuayleWinCtorTarget,
     reinterpret_cast<void *>(0x00872CB0),
     reinterpret_cast<func_atexit_callback *>(0x00495190U)},
    {&construct_reportif, &ReportIfCtorTarget,
     reinterpret_cast<void *>(0x00885F38),
     reinterpret_cast<func_atexit_callback *>(0x00496920U)},
    {&construct_reportwin, &ReportWinCtorTarget,
     reinterpret_cast<void *>(0x00876478),
     reinterpret_cast<func_atexit_callback *>(0x00496950U)},
    {&construct_socialwinparent, &SocialWinCtorTarget,
     reinterpret_cast<void *>(0x008A6270),
     reinterpret_cast<func_atexit_callback *>(0x004AE9D0U)},
    {&construct_statuswin, &StatusWinCtorTarget,
     reinterpret_cast<void *>(0x008C5568),
     reinterpret_cast<func_atexit_callback *>(0x004B3FC0U)},
    {&construct_tutwin, &TutWinCtorTarget,
     reinterpret_cast<void *>(0x008C6E68),
     reinterpret_cast<func_atexit_callback *>(0x004BA5E0U)},
    {&construct_worldwin, &WorldWinCtorTarget,
     reinterpret_cast<void *>(0x008E9F60),
     reinterpret_cast<func_atexit_callback *>(0x004C38D0U)},
    {&construct_midi_device, &Midi_DeviceCtorTarget,
     reinterpret_cast<void *>(0x0090D950),
     reinterpret_cast<func_atexit_callback *>(0x004C5CA0U)},
    {&construct_wave_in_device, &Wave_In_DeviceCtorTarget,
     reinterpret_cast<void *>(0x0090DB50),
     reinterpret_cast<func_atexit_callback *>(0x004C5CD0U)},
    {&construct_netdaemon, &NetDaemonCtorTarget,
     reinterpret_cast<void *>(0x0093CD90),
     reinterpret_cast<func_atexit_callback *>(0x0052DB30U)},
    {&construct_fontqueue_val2, &FontQueueCtorTarget,
     reinterpret_cast<void *>(0x0093FB88),
     reinterpret_cast<func_atexit_callback *>(0x00559250U)},
    {&construct_fontqueue_val1, &FontQueueCtorTarget,
     reinterpret_cast<void *>(0x0093FAE8),
     reinterpret_cast<func_atexit_callback *>(0x00559280U)},
    {&construct_palette1, &PaletteCtorTarget,
     reinterpret_cast<void *>(0x0094C590),
     reinterpret_cast<func_atexit_callback *>(0x005BEC60U)},
    {&construct_palette2, &PaletteCtorTarget,
     reinterpret_cast<void *>(0x009523A0),
     reinterpret_cast<func_atexit_callback *>(0x005BEC90U)},
    {&construct_multidebug, &MultiDebugCtorTarget,
     reinterpret_cast<void *>(0x009B22F0),
     reinterpret_cast<func_atexit_callback *>(0x005C97E0U)},
};

struct InitOpaqueCase_i {
    void(__cdecl *thunk)();
    func_opaque_ctor_i **ctor_slot;
    void *storage;
    int a0;
    func_atexit_callback *callback;
};

const InitOpaqueCase_i g_init_opaque_cases_i[] = {
    {&construct_mapwin, &MapWinCtorTarget,
     reinterpret_cast<void *>(0x007D4060), 1,
     reinterpret_cast<func_atexit_callback *>(0x004620A0U)},
    {&construct_planwin, &PlanWinCtorTarget,
     reinterpret_cast<void *>(0x00834D70), 1,
     reinterpret_cast<func_atexit_callback *>(0x0048AE20U)},
    {&construct_console, &ConsoleCtorTarget,
     reinterpret_cast<void *>(0x009156B0), 1,
     reinterpret_cast<func_atexit_callback *>(0x0050E870U)},
};

struct InitOpaqueCase_iii {
    void(__cdecl *thunk)();
    func_opaque_ctor_iii **ctor_slot;
    void *storage;
    int a0;
    int a1;
    int a2;
    func_atexit_callback *callback;
};

const InitOpaqueCase_iii g_init_opaque_cases_iii[] = {
    {&construct_netmsg1, &NetMsgCtorTarget,
     reinterpret_cast<void *>(0x00805338), 4096, 8, 0,
     reinterpret_cast<func_atexit_callback *>(0x0047A790U)},
    {&construct_netmsg2, &NetMsgCtorTarget,
     reinterpret_cast<void *>(0x007FFF80), -5, 40, 1,
     reinterpret_cast<func_atexit_callback *>(0x0047A7C0U)},
};

struct InitOpaqueArrayCase {
    void(__cdecl *thunk)();
    void *storage;
    uint32_t element_size;
    int count;
    func_thiscall_teardown **ctor_slot;
    func_thiscall_teardown **dtor_slot;
    func_atexit_callback *callback;
};

const InitOpaqueArrayCase g_init_opaque_array_cases[] = {
    {&construct_factionart,
     reinterpret_cast<void *>(0x0078E978), 0x65C, 8,
     &FactionArtCtorTarget, &FactionArtElementTeardown,
     reinterpret_cast<func_atexit_callback *>(0x0044CC80U)},
};

func_atexit_callback *g_init_registered;

int g_init_atexit_calls;

int g_init_atexit_calls_at_ctor = -1;

int __cdecl observe_game_atexit(func_atexit_callback *callback) {
    g_init_registered = callback;
    ++g_init_atexit_calls;
    return 0;
}

void *g_init_constructed;

int g_init_ctor_calls;


void __thiscall observe_init_ctor(void *object) {
    g_init_constructed = object;
    ++g_init_ctor_calls;
    // Sampled so the order assertion below proves construction precedes
    // registration, the original's push-after-call order.
    g_init_atexit_calls_at_ctor = g_init_atexit_calls;
}


void *g_init_vector_array;

uint32_t g_init_vector_size;

int g_init_vector_count;

func_thiscall_teardown *g_init_vector_ctor;

func_thiscall_teardown *g_init_vector_dtor;

int g_init_vector_calls;

void __stdcall observe_vector_ctor(void *array, unsigned int element_size,
                                   int count, func_thiscall_teardown *ctor,
                                   func_thiscall_teardown *dtor) {
    g_init_vector_array = array;
    g_init_vector_size = element_size;
    g_init_vector_count = count;
    g_init_vector_ctor = ctor;
    g_init_vector_dtor = dtor;
    ++g_init_vector_calls;
    g_init_atexit_calls_at_ctor = g_init_atexit_calls;
}

int g_init_dtor_sentinel;

int g_init_opaque_args[4];


void __thiscall observe_opaque_ctor_i(void *object, int a0) {
    g_init_constructed = object;
    ++g_init_ctor_calls;
    g_init_opaque_args[0] = a0;
    g_init_atexit_calls_at_ctor = g_init_atexit_calls;
}

void __thiscall observe_opaque_ctor_iii(void *object, int a0, int a1, int a2) {
    g_init_constructed = object;
    ++g_init_ctor_calls;
    g_init_opaque_args[0] = a0;
    g_init_opaque_args[1] = a1;
    g_init_opaque_args[2] = a2;
    g_init_atexit_calls_at_ctor = g_init_atexit_calls;
}


}  // namespace

void test_init_thunks() {
    func_game_atexit *const saved_atexit = GameAtexit;
    GameAtexit = &observe_game_atexit;

    // Scalar thunks: the construction seam must be handed exactly the
    // rebound global, once, before exactly one registration of exactly the
    // catalogued ??__F address.
    for (const InitThunkCase &entry : g_init_scalar_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_ctor = *entry.ctor_slot;
        *slot = fake;
        *entry.ctor_slot = &observe_init_ctor;
        g_init_ctor_calls = 0;
        g_init_constructed = nullptr;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_ctor_calls == 1);
        expect(g_init_constructed == fake);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved_ctor;
        *slot = saved_slot;
    }

    // Array thunks hand the walk to the iterator seam: the observed call
    // must carry the rebound array, the exact size and count, and both
    // rebound per-element seams - proving the body reads every seam rather
    // than any baked literal - before the one registration.
    auto *const saved_iterator = VectorCtorIterator;
    VectorCtorIterator = &observe_vector_ctor;
    for (const InitArrayCase &entry : g_init_array_cases) {
        alignas(4) uint8_t fake[4] = {};
        auto **slot = static_cast<void **>(entry.slot);
        void *const saved_slot = *slot;
        func_thiscall_teardown *const saved_ctor = *entry.ctor_slot;
        func_thiscall_teardown *const saved_dtor = *entry.dtor_slot;
        *slot = fake;
        *entry.ctor_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_init_dtor_sentinel);
        *entry.dtor_slot =
            reinterpret_cast<func_thiscall_teardown *>(&g_init_dtor_sentinel) + 1;
        g_init_vector_calls = 0;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_vector_calls == 1);
        expect(g_init_vector_array == fake);
        expect(g_init_vector_size == entry.element_size);
        expect(g_init_vector_count == entry.count);
        expect(g_init_vector_ctor ==
               reinterpret_cast<func_thiscall_teardown *>(
                   &g_init_dtor_sentinel));
        expect(g_init_vector_dtor ==
               reinterpret_cast<func_thiscall_teardown *>(
                   &g_init_dtor_sentinel) + 1);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved_ctor;
        *entry.dtor_slot = saved_dtor;
        *slot = saved_slot;
    }
    for (const InitOpaqueCase &entry :
             g_init_opaque_cases) {
        func_thiscall_teardown *const saved =
            *entry.ctor_slot;
        *entry.ctor_slot = &observe_init_ctor;
        g_init_ctor_calls = 0;
        g_init_constructed = nullptr;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_ctor_calls == 1);
        expect(g_init_constructed == entry.storage);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved;
    }
    for (const InitOpaqueCase_i &entry :
             g_init_opaque_cases_i) {
        func_opaque_ctor_i *const saved =
            *entry.ctor_slot;
        *entry.ctor_slot = &observe_opaque_ctor_i;
        g_init_ctor_calls = 0;
        g_init_constructed = nullptr;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_ctor_calls == 1);
        expect(g_init_constructed == entry.storage);
        expect(g_init_opaque_args[0] == entry.a0);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved;
    }
    for (const InitOpaqueCase_iii &entry :
             g_init_opaque_cases_iii) {
        func_opaque_ctor_iii *const saved =
            *entry.ctor_slot;
        *entry.ctor_slot = &observe_opaque_ctor_iii;
        g_init_ctor_calls = 0;
        g_init_constructed = nullptr;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_ctor_calls == 1);
        expect(g_init_constructed == entry.storage);
        expect(g_init_opaque_args[0] == entry.a0);
        expect(g_init_opaque_args[1] == entry.a1);
        expect(g_init_opaque_args[2] == entry.a2);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved;
    }
    for (const InitOpaqueArrayCase &entry :
             g_init_opaque_array_cases) {
        func_thiscall_teardown *const saved_ctor =
            *entry.ctor_slot;
        func_thiscall_teardown *const saved_dtor =
            *entry.dtor_slot;
        *entry.ctor_slot =
            reinterpret_cast<func_thiscall_teardown *>(
                &g_init_dtor_sentinel);
        *entry.dtor_slot =
            reinterpret_cast<func_thiscall_teardown *>(
                &g_init_dtor_sentinel) + 1;
        g_init_vector_calls = 0;
        g_init_atexit_calls = 0;
        g_init_registered = nullptr;
        g_init_atexit_calls_at_ctor = -1;
        entry.thunk();
        expect(g_init_vector_calls == 1);
        expect(g_init_vector_array == entry.storage);
        expect(g_init_vector_size == entry.element_size);
        expect(g_init_vector_count == entry.count);
        expect(g_init_vector_ctor ==
               reinterpret_cast<func_thiscall_teardown *>(
                   &g_init_dtor_sentinel));
        expect(g_init_vector_dtor ==
               reinterpret_cast<func_thiscall_teardown *>(
                   &g_init_dtor_sentinel) + 1);
        expect(g_init_atexit_calls == 1);
        expect(g_init_registered == entry.callback);
        expect(g_init_atexit_calls_at_ctor == 0);
        *entry.ctor_slot = saved_ctor;
        *entry.dtor_slot = saved_dtor;
    }
    VectorCtorIterator = saved_iterator;
    GameAtexit = saved_atexit;
}

// Self-registration.  Each case carries its baseline position in the original
// main() call list, so the run order does not depend on link or static-init
// order.
namespace {
LEAF_CASE(225, test_guarded_teardowns);
LEAF_CASE(226, test_global_arith);
LEAF_CASE(227, test_nullsub_thunks);
LEAF_CASE(228, test_delegation_thunks);
LEAF_CASE(223, test_constant_return_stubs_wave4);
LEAF_CASE(224, test_deleting_thunks);
LEAF_CASE(222, test_adjustor_thunks);
LEAF_CASE(221, test_init_thunks);
}  // namespace
