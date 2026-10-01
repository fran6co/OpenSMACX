#include <assert.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
#define WINAPI
typedef char *LPSTR;
typedef uint32_t DWORD;
enum { WM_KEYDOWN=0x100, WM_LBUTTONDOWN=0x201, WM_RBUTTONDOWN=0x204,
       VK_RETURN=13, VK_ESCAPE=27, VK_SPACE=32 };
struct MSG { int message, wParam; };
typedef int (*BufferWriteRawCallback)(LPSTR,int,int,int);
struct Buffer {
    BufferWriteRawCallback field_8_;
    int count;
    char text[32];
    Buffer() : field_8_(0), count(0) { memset(text,0,sizeof(text)); }
    /* SETTER */
    int write_raw_l(LPSTR,int,int,int);
};
struct UV2Player {
    uint32_t field_10C_;
    Buffer buffer_;
    uint32_t write_delay_;
    UV2Player();
    static int __cdecl on_key();
    int write_timed(LPSTR,int,int,int);
};
UV2Player *UV2PlayerActive;
int UV2PlayerQuitRequested;
extern "C" int uv2_player_writer_redirect(LPSTR,int,int,int);
int peek_result, get_result;
MSG queued;
int PeekMessageA(MSG *msg,int window,int low,int high,int flags) {
    assert(window==0&&low==0&&high==0&&flags==0);
    *msg=queued;return peek_result;
}
int GetMessageA(MSG *,int window,int low,int high) {
    assert(window==0&&low==0&&high==0);return get_result;
}
UV2Player *clock_owner;
int clock_calls, change_delay;
DWORD tick;
DWORD timeGetTime() {
    assert(++clock_calls<40);
    if(change_delay&&clock_calls==2)clock_owner->write_delay_=0;
    return tick++;
}
int Buffer::write_raw_l(LPSTR str,int x,int y,int len) {
    assert(field_8_==0&&y==17&&len==1);
    text[count++]=*str;
    return x+2;
}
/* PRODUCT */
int main(int argc,char **argv) {
    assert(argc==2);
    UV2Player player;
    player.write_delay_=0;
    UV2PlayerActive=&player;clock_owner=&player;
    char text[]="abc";
    if(strcmp(argv[1],"constructor")==0) {
        UV2Player initialized;
        assert(initialized.field_10C_==0&&initialized.buffer_.count==0&&
               initialized.buffer_.field_8_==0);
    } else if(strcmp(argv[1],"keys")==0) {
        peek_result=1;get_result=1;
        const int keys[]={VK_RETURN,VK_ESCAPE,VK_SPACE};
        for(int i=0;i<3;++i){queued.message=WM_KEYDOWN;queued.wParam=keys[i];
            UV2PlayerQuitRequested=0;assert(player.on_key()==1&&UV2PlayerQuitRequested==1);}
        queued.message=WM_LBUTTONDOWN;assert(player.on_key()==1);
        queued.message=WM_RBUTTONDOWN;assert(player.on_key()==1);
        queued.message=WM_KEYDOWN;queued.wParam=99;UV2PlayerQuitRequested=7;
        assert(player.on_key()==0&&UV2PlayerQuitRequested==7);
        queued.wParam=VK_RETURN;get_result=-1;assert(player.on_key()==1);
        get_result=0;assert(player.on_key()==0);peek_result=0;assert(player.on_key()==0);
    } else if(strcmp(argv[1],"member")==0) {
        player.buffer_.field_8_=&uv2_player_writer_redirect;
        assert(player.write_timed(text,3,17,3)==9);
        assert(player.buffer_.count==3&&memcmp(player.buffer_.text,text,3)==0);
        assert(player.buffer_.field_8_==&uv2_player_writer_redirect);
        assert(player.write_timed(text,8,17,-1)==8&&player.buffer_.count==3);
    } else if(strcmp(argv[1],"callback")==0) {
        player.buffer_.field_8_=&uv2_player_writer_redirect;
        assert(uv2_player_writer_redirect(text,1,17,2)==5&&player.buffer_.count==2);
        assert(player.buffer_.field_8_==&uv2_player_writer_redirect);
        assert(uv2_player_writer_redirect(text,4,17,0)==4);
        UV2PlayerActive=0;assert(uv2_player_writer_redirect(text,4,17,2)==0);
    } else if(strcmp(argv[1],"clock")==0) {
        player.write_delay_=100;change_delay=1;
        assert(uv2_player_writer_redirect(text,1,17,1)==3);
        assert(clock_calls==2&&player.write_delay_==0);
        tick=UINT32_MAX;clock_calls=0;change_delay=0;player.write_delay_=2;
        assert(player.write_timed(text,0,17,1)==2&&clock_calls==3);
    } else assert(false);
    return 0;
}
