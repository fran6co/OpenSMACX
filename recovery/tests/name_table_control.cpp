// Host semantic control: actual declarations/bodies, not historical ABI proof.
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <new>
static void *allocations[256];
static unsigned sizes[256];
static bool live[256];
static int allocation_count, deletion_count;
void *operator new(std::size_t size) throw(std::bad_alloc) {
    assert(allocation_count<256);
    void *memory=std::malloc(size);
    if(!memory)throw std::bad_alloc();
    allocations[allocation_count]=memory;sizes[allocation_count]=unsigned(size);
    live[allocation_count++]=true;return memory;
}
void operator delete(void *memory) throw() {
    if(!memory)return;
    for(int i=allocation_count-1;i>=0;--i)if(allocations[i]==memory&&live[i]) {
        live[i]=false;++deletion_count;std::free(memory);return;
    }
    assert(!"unowned or double deletion");
}
// @@DECLARATIONS@@
// @@BODIES@@
static NameTableEntry entry(unsigned low,unsigned high,const char *name,unsigned status=1) {
    NameTableEntry result;assert(result.initialize()==&result);
    result.low_index_=low;result.high_index_=high;result.status_=status;
    std::strcpy(result.name_,name);return result;
}
static void empty(const NameTable &owner,unsigned low,unsigned end) {
    for(unsigned i=low;i<end;++i)assert(owner.slots_[i]==0);
}
static void initialization() {
    NameTableEntry e;std::memset(&e,0xA5,sizeof(e));
    assert(e.initialize()==&e);
    assert(e.high_index_==0&&e.low_index_==0&&e.status_==1);
    for(unsigned i=0;i<sizeof(e.name_);++i)assert(e.name_[i]==0);
    NameTable table;std::memset(&table,0xA5,sizeof(table));
    assert(table.initialize()==&table);empty(table,0,128);
    assert(table.name_==0&&table.flags_==2&&table.unknown_208_==0&&table.unknown_20c_==0);
    assert(table.unknown_210_==0&&table.unknown_214_==250);
    assert(table.get_name()==0);
}
static void ordinary() {
    NameTable table;table.initialize();
    NameTableEntry original=entry(2,4,"C:\\old\\alpha",7);
    table.install(&original);
    NameTableEntry *clone=table.slots_[2];assert(clone!=&original);
    assert(clone==table.slots_[3]&&clone==table.slots_[4]);
    assert(clone->high_index_==4&&clone->low_index_==2&&clone->status_==7);
    assert(std::strcmp(clone->name_,original.name_)==0);empty(table,0,2);empty(table,5,128);
    char directory[]="D:\\fresh";table.rewrite_directory(directory);
    assert(std::strcmp(clone->name_,"D:\\fresh\\alpha")==0);
    char second[]="E:\\next\\";table.rewrite_directory(second);
    assert(std::strcmp(clone->name_,"E:\\next\\alpha")==0);
    int before=deletion_count;assert(table.remove_range(&original)==5);
    assert(deletion_count==before+1);empty(table,0,128);
    char first_name[]="first",second_name[]="second-longer";
    table.set_name(first_name);assert(std::strcmp(table.name_,first_name)==0&&table.name_!=first_name);
    assert(table.get_name()==table.name_);
    before=deletion_count;table.set_name(second_name);assert(deletion_count==before+1);
    assert(std::strcmp(table.name_,second_name)==0);
    assert(table.clear()==0);assert(table.name_==0);empty(table,0,128);
}
static void clear_and_endpoints() {
    NameTable table;table.initialize();
    NameTableEntry first=entry(0,1,"first"),last=entry(120,127,"last");
    table.install(&first);table.install(&last);int before=deletion_count;
    assert(table.clear()==0);assert(deletion_count==before+2);empty(table,0,128);
    NameTableEntry overflow=entry(0,~0u,"wrap");
    assert(table.remove_range(&overflow)==0);empty(table,0,128);
    NameTableEntry singleton=entry(127,127,"one");table.install(&singleton);
    assert(table.remove_range(&singleton)==128);empty(table,0,128);
    table.install(&singleton);before=deletion_count;
    assert(table.clear()==0);assert(deletion_count==before+1);empty(table,0,128);
}
static void displaced_range_and_original_skip() {
    NameTable table;table.initialize();
    NameTableEntry old=entry(2,4,"old"),peer=entry(5,5,"peer"),replacement=entry(3,6,"new",3);
    table.install(&old);table.install(&peer);
    NameTableEntry *skipped=table.slots_[5];int before=deletion_count;
    table.install(&replacement);
    // Original advances high+1 twice after removing a populated range. The
    // immediate next slot is skipped; do not silently fix the leaked peer.
    assert(deletion_count==before+1);assert(table.slots_[2]==0);
    NameTableEntry *installed=table.slots_[3];
    for(unsigned i=3;i<=6;++i)assert(table.slots_[i]==installed);
    assert(installed->low_index_==3&&installed->high_index_==6&&installed->status_==3);
    assert(std::strcmp(installed->name_,"new")==0);
    assert(table.clear()==0);empty(table,0,128);
    ::operator delete(skipped); // Fixture cleanup of the original's omitted free.
    NameTableEntry reversed=entry(9,7,"reversed");before=allocation_count;
    table.install(&reversed);empty(table,0,128);
    assert(allocation_count==before+1);
    NameTableEntry *uninstalled=static_cast<NameTableEntry *>(allocations[before]);
    assert(uninstalled->low_index_==7&&uninstalled->high_index_==7);
    ::operator delete(uninstalled); // Original allocates this uninstalled clone.
}
int main() {
    initialization();ordinary();clear_and_endpoints();displaced_range_and_original_skip();
    for(int i=0;i<allocation_count;++i)assert(!live[i]);
}
