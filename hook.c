#define _GNU_SOURCE

#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define RESET "\x1b[0m"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <malloc.h>
#include <inttypes.h>

// gcc -o hook.so -fPIC -shared hook.c -ldl
// LD_PRELOAD=./hook.so ./myexecutable

// in GDB: set exec-wrapper env 'LD_PRELOAD=./hook.so'


#define BEGIN_HOOK \
        reentrancy_guard++;

#define HOOK \
    if (reentrancy_guard == 1)

#define END_HOOK \
        reentrancy_guard--;


int reentrancy_guard;



void* (*real_malloc)(size_t size); 
void* malloc(size_t size) {
    
    BEGIN_HOOK

    real_malloc = dlsym(RTLD_NEXT, "malloc");
    void* chunk = real_malloc(size);

    HOOK {
        size_t s = malloc_usable_size(chunk);
        fprintf(stderr, BLUE "MALLOC(%4ld):  %p-%p (%4ld Bytes)\n" RESET, size, chunk, chunk+s, s);
    }

    END_HOOK

    return chunk;
}

void (*real_free)(void* ptr);
void free(void* ptr) {

    BEGIN_HOOK

    real_free = dlsym(RTLD_NEXT, "free");

    HOOK {
        size_t s = malloc_usable_size(ptr);
        fprintf(stderr, GREEN "FREE:          %p-%p (%4ld Bytes)\n" RESET, ptr, ptr+s, s);
    }

    END_HOOK

    return real_free(ptr);
}

void* (*real_realloc)(void* ptr, size_t size);
void* realloc(void* ptr, size_t size) {

    BEGIN_HOOK

    real_realloc = dlsym(RTLD_NEXT, "realloc");
    void* old_chunk = ptr;
    void* chunk = real_realloc(ptr, size);

    HOOK {
        size_t old_s = malloc_usable_size(old_chunk);
        fprintf(stderr, YELLOW "REALLOC(%4ld): " GREEN "%p-%p (%4ld Bytes)\n" RESET, size,  old_chunk, old_chunk+old_s, old_s);
        size_t s = malloc_usable_size(chunk);
        fprintf(stderr, YELLOW "'------------> " BLUE "%p-%p (%4ld Bytes)\n" RESET, chunk, chunk+s, s);
    }

    END_HOOK

    return chunk;
}
