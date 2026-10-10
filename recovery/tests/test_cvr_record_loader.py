"""Actual CVR loader/reader/release bodies over an explicit in-memory IO model.

This is not a game/filesystem integration proof or a malformed-file safety
claim. In particular, the original requires count/index chunks before use,
trusts several payload sizes, and leaks partially built records on failure.
The host's pointer-sized typed structures are not a Win32 layout witness.
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_caviar_matrix_operations import braced


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
FUNCTIONS = ("vox_read_exact", "vox_alloc_block", "vox_expand_palette", "vox_read_length_field",
             "vox_read_chunks", "vox_read_colour_chunks", "vox_read_geometry_chunks",
             "vox_read_dword", "vox_chunk_payload", "vox_load_record", "vox_read_parts",
             "vox_read_part", "vox_read_shadow", "vox_read_shape_patches", "caviar_free_record")


def assert_native_layouts(obj):
    """Used by the coordinated VC6 epoch; host fixtures cannot prove Win32 layout."""
    from match_recovery.codeview import class_layout
    record_fields=("flags_","state_","version_","description_","palette_name_","palette_",
                   "field_18_","parts_name_","step_count_","part_count_","slot_table_",
                   "steps_","parts_","shadows_","field_38_","field_3C_","user_length_")
    expected={
        "CaviarData":(12,{"field_0_":0,"fileDescriptor_":4,"record_":8}),
        "VoxInstance":(12,{"instance_colour_":0,"instance_vector_":4,"instance_matrix_":8}),
        "CvrRecord":(0x44,{name:index*4 for index,name in enumerate(record_fields)}),
        "CvrPart":(12,{"colours_":0,"vectors_":4,"matrices_":8}),
        "CvrStep":(8,{"lead_":0,"shape_":4}),
        "CvrShadow":(12,{"field_0_":0,"field_4_":4,"field_8_":8}),
        "CvrShape":(43,{"field_0_":0,"byte_size_":4,"scale_":8,
                       "next_shape_offset_":12,"field_10_":16,
                       "axis_x_":20,"axis_y_":22,"axis_z_":24,"offset_x_":26,"offset_y_":28,"offset_z_":30,
                       "raster_origin_":32,"item_count_":38,"item_stream_":42}),
        "Caviar":(0x13D0,{"scene_scale_":0,"field_4_":4,"field_8_":8,"buffer_":20,"scene_state_":24,
                          "light_vector_":0x60,"shadow_vector_":0x6C,
                          **{f"field_{offset:X}_":offset for offset in range(0x78,0xA5,4)},
                          "camera_view_":0xA5,"field_108_":0x108,"instance_steps_":0x10C,
                          "slots_":0x74C,"instance_palettes_":0x10AC,"field_13CC_":0x13CC}),
    }
    for name,(size,members) in expected.items():
        layout=class_layout(obj,name)
        assert layout["size"]==size,(name,layout)
        assert layout["members"]==members,(name,layout["members"],members)
    palette=class_layout(obj,"Caviar")["member_types"]["instance_palettes_"]
    assert palette["kind"]=="array" and palette["count"]==200 and palette["size"]==800
    # VC6 encodes pointers to primitive types as simple CodeView types; the
    # native reader currently calls those non_class_value. This observation
    # proves array extent/stride, not pointer identity. The actual declaration
    # and pointer-valued consumers are separately compiled in the host test.


def actual_source():
    source = (ROOT / "src/caviar.cpp").read_text()
    header = (ROOT / "src/caviar.h").read_text()
    parts = {}
    for name in FUNCTIONS:
        match, = re.finditer(r"^(?:extern \"C\" )?(?:static )?[\w *]+(?:__cdecl )?" + name + r"\([^;{]*\)\s*\{", source, re.M)
        parts[name] = braced(source, match.start())
    begin = source.index("template<class T>\ninline int vox_alloc_typed")
    parts["template"] = braced(source, begin)
    parts["callback"] = re.search(r"typedef char \(__cdecl \*vox_user_chunk_fn\)[^;]+;", source).group()
    types = ("VOX_Vect", "VOX_Matrix", "CvrPart", "CvrShape", "CvrStep", "CvrShadow", "CvrRecord", "CvrChunkHeader")
    parts["types"] = "\n".join(braced(header, header.index("struct " + name + " {")) + ";" for name in types)
    parts["palette_member"] = re.search(r"uint8_t \*instance_palettes_\[200\];",header).group()
    writers = re.findall(r"instance_palettes_\[field_13CC_\] = [^;]+;",source)
    assert len(writers)==8
    expected=("palette+0x302","palette+0x302","object","object","palette+0x302","object","palette+0x302","object")
    parts["palette_writers"]="\n".join(f"field_13CC_={n}; {body} require(instance_palettes_[{n}]=={value});"
                                         for n,(body,value) in enumerate(zip(writers,expected)))
    parts["palette_copy"]=re.search(r"memcpy\(handle->colour_table_, self.instance_palettes_\[instance\], 0x1800\);",source).group()
    return parts


def run_host(tmp_path, parts):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    declarations = "\n".join(parts[name].split("{",1)[0]+";" for name in FUNCTIONS)
    code = r'''
        #include <cstdint>
        #include <cstdlib>
        #include <cstring>
        #include <vector>
        #include <string>
        #include <algorithm>
        #define __cdecl
        TYPES
        DECLARATIONS
        using Bytes=std::vector<unsigned char>;
        CALLBACK_TYPE
        vox_user_chunk_fn vox_user_chunk;
        // Only the message window is modeled here, not the native callback/
        // projection overlap or the complete shared object's layout.
        struct { char message_[256]; } CaviarOutputMessageWindow;
        char (&vox_message_buffer)[256]=CaviarOutputMessageWindow.message_;
        const char *vox_msg_bad_file_format="Bad file format";
        const char *vox_msg_reading_file="Reading file";
        const char *vox_msg_no_room="No room";
        const char *vox_msg_user_chunk="Error reading user data chunk";
        const float vox_threshold_seven=7.0f;
        Bytes input;size_t position;
        std::vector<void*> allocations,freed;
        std::vector<unsigned long> allocation_sizes;
        int reads,allocs,seeks,fail_read,fail_alloc,fail_seek,user_mode,user_calls;
        CvrRecord *loaded,*free_record;
        CvrPart alternate_part;unsigned char late_pointer[1];
        bool mutate_free,shorten_free;void **watched_failure;
        bool capture_count,mutate_count;int *captured_count;int mutate_at,mutate_to;
        void require(bool ok) { if(!ok) std::exit(42); }
        void vox_stage_error(const char *text) {
            if(watched_failure) require(*watched_failure==nullptr);
            std::strcpy(vox_message_buffer,text);
        }
        void *allocate(unsigned long size) {
            ++allocs;if(allocs==fail_alloc)return 0;
            require(size<1000000);
            void *result=std::malloc(size+32);require(result!=nullptr);
            std::memset(result,0xCD,size+32);allocations.push_back(result);allocation_sizes.push_back(size);
            if(mutate_count && captured_count && allocs==mutate_at){*captured_count=mutate_to;mutate_count=false;}
            return result;
        }
        void release(void *pointer) {
            freed.push_back(pointer);
            if(mutate_free && free_record) {
                if(pointer==free_record->description_) free_record->palette_name_=reinterpret_cast<char*>(late_pointer);
                if(pointer==free_record->parts_[0].colours_) free_record->parts_=&alternate_part;
            }
            if(shorten_free && free_record && pointer==free_record->parts_[0].colours_) free_record->part_count_=1;
        }
        int read_bytes(int fd,char *out,long size) {
            require(fd==17);++reads;if(reads==fail_read)return -1;
            if(size>input.size()-position)return -1;
            if(capture_count && size==4)captured_count=reinterpret_cast<int*>(out);
            std::memcpy(out,input.data()+position,size);position+=size;return int(size);
        }
        long seek_bytes(int fd,long count,int origin) {
            require(fd==17 && origin==1);++seeks;if(seeks==fail_seek)return -1;
            require(count>=0 && size_t(count)<=input.size()-position);
            position+=size_t(count);return long(position);
        }
        auto vox_alloc_slot=&allocate;auto vox_free_slot=&release;
        auto vox_read_slot=&read_bytes;auto vox_seek_slot=&seek_bytes;
        TEMPLATE
        BODIES
        void word(Bytes &bytes,uint32_t value) {
            for(int n=0;n<4;++n)bytes.push_back(static_cast<unsigned char>(value>>(n*8)));
        }
        Bytes number(uint32_t value) {Bytes result;word(result,value);return result;}
        void join(Bytes &to,const Bytes &from){to.insert(to.end(),from.begin(),from.end());}
        Bytes chunk(uint32_t tag,const Bytes &payload) {
            Bytes result;word(result,tag);word(result,uint32_t(payload.size()+8));join(result,payload);return result;
        }
        Bytes fixture(bool patch=true,bool user=true,int count=1,uint32_t shape_bytes=32) {
            Bytes file;
            join(file,chunk(0x01000000,number(0x40E00000))); // version 7
            join(file,chunk(0x02000000,Bytes{'d','e','s','c'}));
            Bytes palette;
            join(palette,chunk(0x01010000,Bytes{'p','a','l'}));
            join(palette,chunk(0x01020000,Bytes(0x310,0x71)));
            join(palette,chunk(0x01030000,Bytes{0x91,0x92}));
            join(file,chunk(0x03000000,palette));
            Bytes parts;
            join(parts,chunk(0x04010000,Bytes{'p','a','r','t','s'}));
            join(parts,chunk(0x04020000,number(count)));
            join(parts,chunk(0x04030000,number(1)));
            Bytes part;join(part,chunk(0x04040100,Bytes{'o','n','e'}));
            Bytes shape(shape_bytes,0x51);std::memcpy(shape.data()+4,number(shape_bytes).data(),4);
            Bytes colour;join(colour,chunk(0x04040202,number(256)));
            join(colour,chunk(0x04040201,shape));join(part,chunk(0x04040200,colour));
            Bytes geometry;join(geometry,chunk(0x04040301,Bytes{0x35}));
            join(geometry,chunk(0x04040302,Bytes(12,0x12)));
            join(geometry,chunk(0x04040303,Bytes(36,0x24)));join(part,chunk(0x04040300,geometry));
            Bytes shadow;join(shadow,chunk(0x04040401,number(73)));
            join(shadow,chunk(0x04040402,Bytes{0x44,0x45}));join(shadow,chunk(0x04040403,Bytes{0x88}));
            join(part,chunk(0x04040400,shadow));join(parts,chunk(0x04040000,part));
            if(patch) {
                Bytes patches;join(patches,chunk(0x04050001,number(0)));
                Bytes prefix(42,0xA3);std::memcpy(prefix.data(),number(0x12345678).data(),4);
                std::memcpy(prefix.data()+4,number(64).data(),4);
                prefix.push_back(0xE1);prefix.push_back(0xE2);prefix.push_back(0xE3);
                join(patches,chunk(0x04050002,prefix));join(parts,chunk(0x04050000,patches));
            }
            join(file,chunk(0x04000000,parts));
            if(user) {join(file,chunk(0x0F000000,Bytes{1,2,3}));join(file,chunk(0x0F000000,Bytes{4,5,6}));}
            join(file,chunk(0x7F000000,Bytes{7,8}));
            return chunk(0x00525643,file);
        }
        char second_user(int fd,CvrRecord *record,CvrChunkHeader *header) {
            require(record==loaded && header->tag==0x0F000000 && vox_message_buffer[0]==0);
            ++user_calls;unsigned char bytes[3];require(vox_read_exact(fd,bytes,3)==0);
            require(bytes[0]==4);header->length=72;return 0;
        }
        char user(int fd,CvrRecord *record,CvrChunkHeader *header) {
            require(header->length==11 && vox_message_buffer[0]==0 && record->state_==0);
            loaded=record;++user_calls;
            if(user_mode==1)return 1;
            if(user_mode==2){std::strcpy(vox_message_buffer,"callback detail");return 1;}
            unsigned char bytes[3];if(vox_read_exact(fd,bytes,3)!=0)return 1;
            require(bytes[0]==1);header->length=36;vox_user_chunk=&second_user;return 0;
        }
        void reset(const Bytes &bytes) {
            for(void *p:allocations)std::free(p);
            allocations.clear();allocation_sizes.clear();freed.clear();input=bytes;position=0;reads=allocs=seeks=0;
            fail_read=fail_alloc=fail_seek=-1;user_mode=user_calls=0;
            vox_user_chunk=nullptr;std::strcpy(vox_message_buffer,"old error");
            loaded=free_record=nullptr;mutate_free=shorten_free=false;watched_failure=nullptr;
            capture_count=mutate_count=false;captured_count=nullptr;mutate_at=mutate_to=1;
        }
        void check_loaded(CvrRecord *r,bool patch) {
            require(r && r->state_==3 && r->version_==7.0f && r->part_count_==1 && r->step_count_==1);
            require(r->field_38_==0 && r->field_3C_==0 && r->flags_==1);
            require(std::strcmp(r->description_,"desc")==0 && std::strcmp(r->palette_name_,"pal")==0);
            require(std::strcmp(r->parts_name_,"parts")==0 && std::strcmp(r->slot_table_[0],"one")==0);
            require(r->palette_[0x302]==0x71 && r->field_18_[1]==0x92);
            require(r->parts_[0].colours_[0]==0x35 && r->steps_[0].lead_==256);
            require(r->shadows_[0].field_0_==73 && r->shadows_[0].field_4_[1]==0x45 && r->shadows_[0].field_8_[0]==0x88);
            if(patch){
                auto bytes=reinterpret_cast<unsigned char*>(r->steps_[0].shape_);
                require(r->steps_[0].shape_->field_0_==0x12345678 && r->steps_[0].shape_->byte_size_==96);
                require(bytes[42]==0xE1 && bytes[44]==0xE3 && bytes[64]==0x51 && bytes[95]==0x51);
                require(bytes[45]==0xCD && bytes[96]==0xCD);
            }
            require(position==input.size());
        }
        int main() {
            reset(fixture());vox_user_chunk=&user;auto r=vox_load_record(17);check_loaded(r,true);
            require(user_calls==2 && r->user_length_==64 && seeks==1);
            auto desc=r->description_;auto names=r->slot_table_;auto part=r->parts_;auto steps=r->steps_;auto shadows=r->shadows_;
            std::vector<void*> expected={desc,r->palette_name_,r->palette_,r->field_18_,r->parts_name_,names[0],names,
                part[0].colours_,part[0].vectors_,part[0].matrices_,part,steps[0].shape_,steps,
                shadows[0].field_4_,shadows[0].field_8_,shadows,r};
            caviar_free_record(r);require(freed==expected);
            reset(fixture(true,false,1,128));r=vox_load_record(17);
            require(r && r->steps_[0].shape_->byte_size_==192 && position==input.size());
            Bytes old_shape(128,0x51);std::memcpy(old_shape.data()+4,number(128).data(),4);
            require(std::memcmp(reinterpret_cast<unsigned char*>(r->steps_[0].shape_)+64,old_shape.data(),128)==0);
            reset(fixture());r=vox_load_record(17);check_loaded(r,true);require(r->user_length_==0 && seeks==3 && vox_message_buffer[0]==0);
            int total_reads=reads,total_allocs=allocs;
            for(int n=1;n<=total_reads;++n){reset(fixture());fail_read=n;require(vox_load_record(17)==nullptr);require(freed.empty());}
            for(int n=1;n<=total_allocs;++n){reset(fixture());fail_alloc=n;require(vox_load_record(17)==nullptr);require(freed.empty());require(std::strcmp(vox_message_buffer,"No room")==0);}
            reset(fixture());fail_seek=1;require(vox_load_record(17)==nullptr && std::strcmp(vox_message_buffer,"Bad file format")==0);
            for(int mode=1;mode<=2;++mode){reset(fixture());user_mode=mode;vox_user_chunk=&user;require(vox_load_record(17)==nullptr);
                require(std::strcmp(vox_message_buffer,mode==1?"Error reading user data chunk":"callback detail")==0);require(freed.empty());}
            reset(fixture(false,false,2));require(vox_load_record(17)==nullptr && std::strcmp(vox_message_buffer,"Number of objects mismatch")==0);
            reset(chunk(0,{}));require(vox_load_record(17)==nullptr && allocs==0);
            Bytes over;word(over,0x01000000);word(over,100);reset(chunk(0x00525643,over));
            require(vox_load_record(17)==nullptr && std::strcmp(vox_message_buffer,"Bad file format")==0);
            reset(chunk(0x00525643,{}));r=vox_load_record(17);require(r && r->state_==3 && r->part_count_==0 && r->user_length_==0);
            reset({});void *slot=reinterpret_cast<void*>(1);watched_failure=&slot;fail_alloc=1;
            require(vox_alloc_block(12,&slot)==-1 && slot==nullptr);
            reset(fixture(false,false));r=vox_load_record(17);check_loaded(r,false);
            free_record=r;mutate_free=true;alternate_part=r->parts_[0];
            alternate_part.vectors_=reinterpret_cast<VOX_Vect*>(late_pointer);r->slot_table_[0]=nullptr;
            caviar_free_record(r);
            require(freed[1]==late_pointer && freed[5]==nullptr && freed[8]==late_pointer && freed[10]==&alternate_part);
            reset({});CvrRecord short_record={};CvrPart two_parts[2]={};
            two_parts[0].colours_=late_pointer;two_parts[1].colours_=late_pointer+1;
            short_record.part_count_=2;short_record.parts_=two_parts;free_record=&short_record;shorten_free=true;
            caviar_free_record(&short_record);require(freed.size()==3 && freed[0]==late_pointer && freed[1]==two_parts);
            Bytes old_payload(0x1302);
            for(size_t n=0;n<old_payload.size();++n)old_payload[n]=static_cast<unsigned char>((n*7+n/256*19)%251);
            Bytes old_palette;join(old_palette,chunk(0x01020000,old_payload));
            Bytes old_file;join(old_file,chunk(0x01000000,number(0x40C00000)));join(old_file,chunk(0x03000000,old_palette));
            reset(chunk(0x00525643,old_file));r=vox_load_record(17);
            require(r && r->palette_[2]==old_payload[2] && position==input.size());
            for(int row=0;row<24;++row)for(int col=0;col<256;++col)
                require(r->palette_[0x302+row*256+col]==old_payload[0x302+(row*2/3)*256+col]);
            require(r->palette_[0x1B02]==0xCD);
            // The actual old-format reader makes four ordered reads. A failed
            // read cannot publish any expanded rows, but keeps prior prefix bytes.
            for(int failure=0;failure<=4;++failure) {
                reset(old_payload);fail_read=failure;
                unsigned char guarded[0x1B04];std::memset(guarded,0xCD,sizeof(guarded));
                int result=vox_expand_palette(17,guarded+1);
                require(result==(failure ? -1:0) && reads==(failure ? failure:4));
                size_t copied=failure==1?0:failure==2?1:failure==3?2:0x302;
                require(std::memcmp(guarded+1,old_payload.data(),copied)==0);
                require(guarded[0]==0xCD && guarded[0x1B03]==0xCD);
                if(failure)for(size_t n=copied;n<0x1B02;++n)require(guarded[1+n]==0xCD);
                else for(int row=0;row<24;++row)for(int col=0;col<256;++col)
                    require(guarded[1+0x302+row*256+col]==old_payload[0x302+(row*2/3)*256+col]);
            }
            Bytes inferred_shape(32,0);std::memcpy(inferred_shape.data()+4,number(32).data(),4);
            Bytes inferred_colour=chunk(0x04040201,inferred_shape);
            Bytes inferred_part=chunk(0x04040200,inferred_colour);
            reset(inferred_part);CvrRecord inferred={};inferred.flags_=0x81;inferred.part_count_=1;
            require(vox_read_part(17,&inferred,0,int(input.size()+8))==0);
            require(inferred.flags_==0x80 && inferred.steps_[0].lead_==32 && position==input.size());
            for(int trial=0;trial<2;++trial) {
                reset(chunk(0x04020000,number(trial)));capture_count=mutate_count=true;CvrRecord count_record={};
                mutate_at=mutate_to=trial+1;
                require(vox_read_parts(17,&count_record,int(input.size()+8))==-1 && count_record.part_count_==trial+1);
                require(allocation_sizes[0]==trial*sizeof(char*) && allocation_sizes[1]==sizeof(CvrPart));
                for(size_t n=0;n<allocations.size();++n) for(unsigned long byte=0;byte<32;++byte)
                    require(static_cast<unsigned char*>(allocations[n])[allocation_sizes[n]+byte]==0xCD);
            }
            {
                uint8_t palette[0x1B02],object_colour[0x1800],rendered[0x1800];
                std::memset(palette,0x31,sizeof(palette));std::memset(object_colour,0x57,sizeof(object_colour));
                CvrRecord palette_record={};palette_record.palette_=palette;CvrRecord *record=&palette_record;
                struct Definition {CvrRecord *record_;} definition_value={record};Definition *definition=&definition_value;
                uint8_t *object=object_colour;
                struct PaletteTable { PALETTE_MEMBER } self={};
                uint8_t *(&instance_palettes_)[200]=self.instance_palettes_;int field_13CC_;
                PALETTE_WRITERS
                struct Handle {uint8_t *colour_table_;} handle_value={rendered};Handle *handle=&handle_value;
                for(int instance=0;instance<8;++instance) {
                    PALETTE_COPY
                    require(std::memcmp(rendered,instance_palettes_[instance],sizeof(rendered))==0);
                }
                require(instance_palettes_[8]==nullptr && instance_palettes_[199]==nullptr);
            }
            reset({});
        }
    '''.replace("TYPES",parts["types"]).replace("CALLBACK_TYPE",parts["callback"]).replace("DECLARATIONS",declarations).replace("TEMPLATE",parts["template"]).replace("BODIES","\n".join(parts[n] for n in FUNCTIONS))
    code=code.replace("PALETTE_MEMBER",parts["palette_member"]).replace("PALETTE_WRITERS",parts["palette_writers"]).replace("PALETTE_COPY",parts["palette_copy"])
    source,executable=tmp_path/"loader.cpp",tmp_path/"loader"
    source.write_text(code)
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=undefined,bounds",
        "-fno-sanitize-recover=all",str(source),"-o",str(executable)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(executable)],capture_output=True,text=True,timeout=15)


def test_actual_loader_closure_and_callback_failure_order(tmp_path):
    result=run_host(tmp_path,actual_source())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("damage",["magic","user_length","user_error","patch_offset","patch_length",
                                   "count_check","release_cache","release_count","allocator_publication","names_count_reload","parts_count_reload","forward_overlap_copy",
                                   "palette_row","palette_read_failure","palette_extent"])
def test_behavior_damage_controls(tmp_path,damage):
    parts=actual_source()
    if damage=="magic":parts["vox_load_record"]=parts["vox_load_record"].replace("0x00525643","0x00525644")
    elif damage=="user_length":parts["vox_load_record"]=parts["vox_load_record"].replace("record->user_length_ = vox_chunk_payload(header.length)","record->user_length_ = 3")
    elif damage=="user_error":parts["vox_load_record"]=parts["vox_load_record"].replace("if (strlen(CaviarOutputMessageWindow.message_) == 0)","if (true)")
    elif damage=="patch_offset":parts["vox_read_shape_patches"]=parts["vox_read_shape_patches"].replace("prefix + sizeof(uint32_t)","prefix")
    elif damage=="patch_length":parts["vox_read_shape_patches"]=parts["vox_read_shape_patches"].replace("(uint32_t)payload - sizeof(prefix)","(uint32_t)payload - sizeof(prefix) - 8u")
    elif damage=="count_check":parts["vox_read_parts"]=parts["vox_read_parts"].replace("if (expected_count != index)","if (false)")
    elif damage=="release_cache":
        parts["caviar_free_record"]=parts["caviar_free_record"].replace("    if (record->parts_ != 0)","    CvrPart *cached = record->parts_;\n    if (record->parts_ != 0)")
        parts["caviar_free_record"]=parts["caviar_free_record"].replace("record->parts_[index]","cached[index]")
    elif damage=="release_count":
        parts["caviar_free_record"]=parts["caviar_free_record"].replace("    char *first =", "    int cached_count = record->part_count_;\n    char *first =")
        parts["caviar_free_record"]=parts["caviar_free_record"].replace("index < record->part_count_","index < cached_count")
    elif damage=="allocator_publication":
        parts["template"]=parts["template"].replace("    *out = block;","").replace("vox_stage_error(vox_msg_no_room);","vox_stage_error(vox_msg_no_room);\n        *out = block;").replace("    return 0;","    *out = block;\n    return 0;")
    elif damage=="names_count_reload":
        parts["vox_read_parts"]=parts["vox_read_parts"].replace("memset(record->slot_table_, 0, names_bytes)","memset(record->slot_table_, 0, (uint32_t)expected_count * sizeof(*record->slot_table_))")
    elif damage=="parts_count_reload":
        parts["vox_read_parts"]=parts["vox_read_parts"].replace("memset(record->parts_, 0, parts_bytes)","memset(record->parts_, 0, (uint32_t)expected_count * sizeof(*record->parts_))")
    elif damage=="palette_row":
        parts["vox_expand_palette"]=parts["vox_expand_palette"].replace("(shade / 3)","(shade / 4)")
    elif damage=="palette_read_failure":
        parts["vox_expand_palette"]=parts["vox_expand_palette"].replace("if (vox_read_exact(fd, shades, sizeof(shades)) != 0) return -1;","if (vox_read_exact(fd, shades, sizeof(shades)) != 0) return 0;")
    elif damage=="palette_extent":
        parts["vox_expand_palette"]=parts["vox_expand_palette"].replace("shade < 0x30","shade < 0x2E")
    else:
        parts["vox_read_shape_patches"]=parts["vox_read_shape_patches"].replace("memmove(bytes + prefix_size, bytes, old_size);","for (uint32_t n=0;n<old_size;++n) bytes[prefix_size+n]=bytes[n];")
    result=run_host(tmp_path,parts)
    assert result.returncode != 0


def test_pinned_original_dispatch_and_patch_boundaries():
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image=PeImage(path.read_bytes(),PIN);decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    extents=((0x6381F0,0x638425),(0x6388C0,0x638AC2),(0x638AD0,0x638CB9),(0x638F30,0x639086),(0x639090,0x6391E6))
    expected=({0x638650,0x638620,0x6386B0,0x6386D0,0x638720,0x6388C0,0x639390},
              {0x638650,0x638620,0x639090,0x638AD0,0x639390},
              {0x638650,0x638620,0x638CC0,0x638DE0,0x638F30,0x639390},
              {0x638650,0x638620,0x639390},{0x638650,0x6474E0,0x639390})
    instructions={}
    for (start,end),targets in zip(extents,expected):
        body=list(decoder.disasm(image.read(Span(start,end)),start))
        assert body[-1].address+body[-1].size==end and body[-1].mnemonic=="ret"
        assert {int(i.op_str,16) for i in body if i.mnemonic=="call" and i.op_str.startswith("0x")}==targets
        instructions.update((i.address,(i.mnemonic,i.op_str)) for i in body)
    anchors={0x638219:("cmp","dword ptr [esp + 0x10], 0x525643"),0x638230:("push","0x44"),
        0x638251:("mov","ecx, 0x11"),0x638307:("mov","eax, dword ptr [0x9c0834]"),
        0x63833B:("call","eax"),0x638340:("test","al, al"),0x638353:("mov","dword ptr [ecx + 0x40], eax"),
        0x638394:("mov","dword ptr [edx + 4], 3"),0x639109:("push","0x2a"),
        0x63912B:("mov","ecx, dword ptr [esp + 0x24]"),0x639154:("add","ebp, -0x2a"),
        0x639173:("mov","dword ptr [ebx + 4], edx")}
    for address,value in anchors.items():assert instructions[address]==value
    # This recognized CRT dependency has a memmove contract even though the
    # old seed called it memcpy_0. Inspect entry/overlap branch, not jump tables.
    runtime={i.address:(i.mnemonic,i.op_str) for start,end in ((0x6474E0,0x647500),(0x647678,0x647697))
             for i in decoder.disasm(image.read(Span(start,end)),start)}
    assert runtime[0x6474F4]==("cmp","edi, esi")
    assert runtime[0x6474F6]==("jbe","0x647500")
    assert runtime[0x6474F8]==("cmp","edi, eax")
    assert runtime[0x6474FA]==("jb","0x647678")
    assert runtime[0x647693]==("std","")
    assert runtime[0x647694]==("rep movsd","dword ptr es:[edi], dword ptr [esi]")
    assert runtime[0x647696]==("cld","")


def test_pinned_old_palette_reads_and_indexing():
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image=PeImage(path.read_bytes(),PIN)
    body=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x6391F0,0x6392DC)),0x6391F0))
    assert body[-1].address+body[-1].size==0x6392DC and body[-1].mnemonic=="ret"
    assert [(i.address,i.op_str) for i in body if i.mnemonic=="call"]==[
        (0x6391F5,"0x645550"),(0x63920F,"0x638650"),(0x63922F,"0x638650"),
        (0x639252,"0x638650"),(0x639276,"0x638650")]
    ins={i.address:(i.mnemonic,i.op_str) for i in body}
    anchors={0x63920B:("push","1"),0x63922B:("push","1"),
        0x63924B:("push","0x300"),0x63926F:("push","0x1000"),
        0x639291:("lea","ebx, [esi + 0x302]"),0x63929B:("mov","eax, 0x55555556"),
        0x6392A6:("imul","ecx"),0x6392AA:("add","ecx, 2"),
        0x6392AD:("shr","eax, 0x1f"),0x6392B0:("add","edx, eax"),
        0x6392B2:("shl","edx, 8"),0x6392B5:("add","edx, edi"),
        0x6392B7:("cmp","ecx, 0x30"),0x6392BA:("mov","dl, byte ptr [esp + edx + 0xc]"),
        0x6392BE:("mov","byte ptr [esi - 0x100], dl"),0x6392C8:("cmp","edi, 0x100")}
    for address,value in anchors.items():assert ins[address]==value
    for test,branch,error in ((0x639217,0x639219,0x63921B),(0x639237,0x639239,0x63923B),
                              (0x63925A,0x63925C,0x63925E),(0x63927E,0x639280,0x639282)):
        assert ins[test]==("test","eax, eax") and ins[branch][0]=="je"
        assert ins[error]==("or","eax, 0xffffffff")
