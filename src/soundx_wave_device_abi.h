#pragma once

// Borrowed call view for the provider returned by soundx.dll ordinal 5.
// This is not the game's Wave_Device/SoundDeviceBase hierarchy and does not
// describe the provider's complete SDK class. Only consumed operations and
// physical slots needed to reach them are declared here. The provider is
// created and destroyed by the DLL exports; this view is never instantiated.
//
// The pinned provider table has 42 entries (0x00 through 0xA4). Reserved
// entries retain only observed call-frame arity and effects; they carry no
// recovered semantic names and product code must not call them.
class WaveDeviceProviderCallView {
public:
    virtual int reserved_00(uint32_t) = 0;
    virtual int reserved_04() = 0;
    virtual void reserved_08() = 0;
    virtual int init(void *, unsigned long) = 0;
    virtual void release() = 0;
    virtual int get_ndevices() = 0;
    virtual int select(unsigned long) = 0;
    virtual void get_description(unsigned long, char *, unsigned long) = 0;
    virtual void set_volume(uint32_t) = 0;
    virtual int reserved_24() = 0;
    // The observed implementation has `ret 4`; its return channel is unknown.
    virtual void reserved_28(uint32_t) = 0;
    virtual int reserved_2c(uint32_t) = 0;
    virtual int reserved_30() = 0;
    virtual void update_sound() = 0;
    virtual void set_rate(uint32_t) = 0;
    virtual int get_rate() = 0;
    virtual void reserved_40(uint32_t) = 0;
    virtual uint32_t reserved_44() = 0;
    virtual void suspend() = 0;
    virtual void restart() = 0;
    virtual int start_raw_dump(char *) = 0;
    virtual int stop_raw_dump() = 0;
    virtual int get_hw_mem_size() = 0;
    virtual void reserved_5c() = 0;
    virtual void enable() = 0;
    virtual void disable() = 0;
    virtual int is_disabled() = 0;
    virtual int set_hwnd(HWND) = 0;
    virtual int get_ds() = 0;
    virtual bool is_eax() = 0;
    // Keep the observed physical overload order explicit: pointer at 0x7C,
    // then the one-word value at 0x78 in the vtable.
    virtual int set_eax(EAX_REVERB_PROPERTIES *) = 0;  // physical slot 0x7C
    virtual int set_eax(uint32_t) = 0;                 // physical slot 0x78
    virtual int set_eax_mix(float) = 0;
    virtual bool is_3d() = 0;
    virtual int set_listener_position(float, float, float) = 0;
    virtual int get_listener_position(float *, float *, float *) = 0;
    virtual int set_listener_xpos(float) = 0;
    virtual int get_listener_xpos(float *) = 0;
    virtual int set_listener_ypos(float) = 0;
    virtual int get_listener_ypos(float *) = 0;
    virtual int set_listener_zpos(float) = 0;
    virtual int get_listener_zpos(float *) = 0;
};
