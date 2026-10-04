// Host behavior control for actual source, not a game/runtime acceptance proof.
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <new>
static void *allocations[128];static bool live[128];
static int allocation_count,deletion_count;
void *operator new(std::size_t size) throw(std::bad_alloc) {
    assert(allocation_count<128);void *memory=std::malloc(size);
    if(!memory)throw std::bad_alloc();
    allocations[allocation_count]=memory;live[allocation_count++]=true;return memory;
}
void operator delete(void *memory) throw() {
    if(!memory)return;
    for(int i=allocation_count-1;i>=0;--i)if(allocations[i]==memory&&live[i]) {
        live[i]=false;++deletion_count;std::free(memory);return;
    }
    assert(!"unowned or double deletion");
}
static bool is_live(void *memory) {
    for(int i=allocation_count-1;i>=0;--i)if(allocations[i]==memory&&live[i])return true;
    return false;
}
// @@DECLARATIONS@@
// @@BODIES@@
static void text(const char *actual,const char *expected) {
    assert(actual!=0);assert(std::strcmp(actual,expected)==0);
}
static void initialization() {
    PathRecord owner;std::memset(&owner,0xA5,sizeof(owner));
    assert(owner.initialize()==&owner);
    assert(owner.unknown_00_==0&&owner.unknown_04_==0&&owner.unknown_08_==0&&owner.unknown_0C_==0);
    assert(owner.unknown_10_==0&&owner.unknown_14_==0&&owner.unknown_18_==250);
    assert(owner.path_==0&&owner.directory_==0&&owner.name_==0);
    int before=deletion_count;owner.close();assert(deletion_count==before);
}
static void path_and_name_replacement() {
    PathRecord owner;owner.initialize();
    char first[]="C:\\original\\alpha",plain[]="plain",forward[]="folder/file";
    owner.set_path(first);text(owner.path_,first);assert(owner.path_!=first);
    int before=deletion_count;owner.set_path(plain);text(owner.path_,plain);assert(deletion_count==before+1);
    owner.set_path(forward);text(owner.path_,forward);
    char name1[]="metadata",name2[]="second";
    owner.set_name(name1);text(owner.name_,name1);assert(owner.name_!=name1);
    before=deletion_count;owner.set_name(name2);text(owner.name_,name2);assert(deletion_count==before+1);
    before=deletion_count;owner.set_name(0);assert(owner.name_==0&&deletion_count==before+1);
    owner.set_path(0);assert(owner.path_==0&&owner.directory_==0);
    owner.close();
}
static void directories_and_release_omission() {
    PathRecord owner;owner.initialize();
    char directory[]="D:\\fresh",oldpath[]="C:\\old\\alpha",newdir[]="E:\\next\\";
    owner.set_directory(directory);text(owner.directory_,"D:\\fresh\\");assert(owner.path_==0);
    owner.set_path(oldpath);text(owner.path_,"D:\\fresh\\alpha");
    int before=deletion_count;owner.set_directory(newdir);
    assert(deletion_count==before+2);text(owner.directory_,newdir);text(owner.path_,"E:\\next\\alpha");
    char forward[]="folder/file";owner.set_path(forward);text(owner.path_,"E:\\next\\folder/file");
    char trailing[]="X:\\trailing\\";owner.set_path(trailing);text(owner.path_,"E:\\next\\");
    owner.set_path(0);assert(owner.path_==0);text(owner.directory_,newdir);
    char renamed[]="name";owner.set_name(renamed);owner.set_path(oldpath);
    char *path=owner.path_,*name=owner.name_,*dir=owner.directory_;
    before=deletion_count;owner.close();assert(deletion_count==before+2);
    assert(owner.path_==path&&owner.name_==name&&owner.directory_==dir);
    assert(!is_live(path)&&!is_live(name)&&is_live(dir));
    ::operator delete(dir); // Fixture cleanup of the original omitted free.
}
static void directory_rebases_without_backslashes() {
    PathRecord owner;owner.initialize();char bare[]="bare",dir[]="prefix",root[]="\\root";
    owner.set_path(bare);owner.set_directory(dir);text(owner.path_,"prefix\\bare");
    text(owner.directory_,"prefix\\");
    owner.set_path(root);text(owner.path_,"prefix\\root");
    owner.set_path(0);owner.set_directory(dir);assert(owner.path_==0);
    owner.close();::operator delete(owner.directory_);
}
int main() {
    initialization();path_and_name_replacement();directories_and_release_omission();
    directory_rebases_without_backslashes();
    for(int i=0;i<allocation_count;++i)assert(!live[i]);
}
