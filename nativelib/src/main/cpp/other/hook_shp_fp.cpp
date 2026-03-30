//
// Created by zgy on 2026/1/7.
//
#include "hook_shp_fp.h"
#include "shadowhook.h"
#include "logger.h"

int hook_libshadowhook_0xB72C()
{
    return 0;
}

typedef int (*hook_libshadowhook_0xFD70_t)(char*,size_t);
hook_libshadowhook_0xFD70_t orig_libshadowhook_0xFD70;
int hook_libshadowhook_0xFD70(char* libname,size_t flag)
{
    LOGE("call 0xFD70:%s",libname);
    return orig_libshadowhook_0xFD70(libname,flag);
}

void hook_libshadowhook(size_t base)
{
    shadowhook_hook_func_addr((void*)(base + 0xB72C),
                              (void*)(hook_libshadowhook_0xB72C),
                              nullptr);
    shadowhook_hook_func_addr((void*)(base + 0xFD70),
                              (void*)(hook_libshadowhook_0xFD70),
                              (void**)&orig_libshadowhook_0xFD70);
}

static int idx_data = 0;
static int idx_key = 0;
typedef size_t (*hook_libshpssdk_enc_t)(size_t,size_t,size_t,size_t,size_t);
hook_libshpssdk_enc_t orig_libshpssdk_enc;

size_t hook_libshpssdk_enc(size_t x0,size_t x1,size_t x2,size_t x3,size_t x4)
{
    char path[256] = {};
    sprintf(path,"/storage/emulated/0/Android/data/com.shopee.tw/files/enc_%d",idx_key);
    FILE* f = fopen(path,"wb");
    fwrite((void*)x1,sizeof(char),0x30,f);
    LOGE("save %s done",path);
    fclose(f);
    idx_key++;
    return orig_libshpssdk_enc(x0,x1,x2,x3,x4);
}

typedef size_t (*hook_libshpssdk_data_t)(size_t,size_t,size_t,size_t,size_t,size_t);
hook_libshpssdk_data_t orig_libshpssdk_data;
size_t hook_libshpssdk_data(size_t x0,size_t x1,size_t x2,size_t x3,size_t x4,size_t x5)
{
    char path[256] = {};
    sprintf(path,"/storage/emulated/0/Android/data/com.shopee.tw/files/data_%d",idx_data);
    FILE* f = fopen(path,"wb");
    fwrite((void*)x4,sizeof(char),x5,f);
    LOGE("save %s done",path);
    fclose(f);
    idx_data++;
    return orig_libshpssdk_data(x0,x1,x2,x3,x4,x5);
}

typedef size_t (*hook_libshpssdk_setrisktag_t)(size_t,size_t,size_t);
hook_libshpssdk_setrisktag_t orig_libshpssdk_setrisktag;

size_t hook_libshpssdk_setrisktag(size_t x0,size_t x1,size_t x2)
{
    *(size_t*)x1 = 0;
    return orig_libshpssdk_setrisktag(x0,x1,x2);
}

void removeolddata()
{
    for(int i = 0;i<40;i++)
    {
        char path[256] = {};
        sprintf(path,"/storage/emulated/0/Android/data/com.shopee.tw/files/data_%d",i);
        remove(path);
        sprintf(path,"/storage/emulated/0/Android/data/com.shopee.tw/files/enc_%d",i);
        remove(path);
    }
}

void hook_sp(size_t base)
{
    removeolddata();
    shadowhook_hook_func_addr((void*)(base + 0x64F9F0),
                              (void*)(hook_libshpssdk_enc),
                              (void**)&orig_libshpssdk_enc);

    shadowhook_hook_func_addr((void*)(base + 0x2A68E8),
                              (void*)(hook_libshpssdk_data),
                              (void**)&orig_libshpssdk_data);

    shadowhook_hook_func_addr((void*)(base + 0x27CBC0),
                              (void*)(hook_libshpssdk_setrisktag),
                              (void**)&orig_libshpssdk_setrisktag);
}

typedef size_t (*call_constructors_t)(void*);
call_constructors_t orig_call_constructors;
int hook_call_constructors(void* thiz)
{
    size_t x0 = (size_t)thiz;
    size_t funcbase = *(size_t*)(x0 + 0x98);
    if(funcbase == 0)
    {
        return orig_call_constructors(thiz);
    }
    size_t func1 = *(size_t *)funcbase;
    size_t func2 = *(size_t *)(funcbase + 8);
    if((func1 & 0xfff) == 0xF60 && ((func2 & 0xfff) == 0xFAC))
    {
        LOGE("find shpssdk:%lx",(func1 - 0x175F60));
        hook_sp(func1 - 0x175F60);
    }
    if((func1 & 0xfff) == 0x484 && ((func2 & 0xfff) == 0xfec))
    {
        LOGE("find libshadowhook.so:%lx",(func1 - 0x8484));
        hook_libshadowhook(func1 - 0x8484);
    }
    size_t res = orig_call_constructors(thiz);
    return res;
}

void shp_hook_fp()
{
    shadowhook_hook_sym_name("linker64","__dl__ZN6soinfo17call_constructorsEv",(void*)hook_call_constructors,(void**)&orig_call_constructors);
}