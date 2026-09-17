#ifndef ZOMDROID_JNI_STATS_H
#define ZOMDROID_JNI_STATS_H
/*
 * Zomdroid diagnostic: how many times, and for how long, each emulated JNI function runs.
 *
 * Every JNI call the game makes into an x86_64 library goes through RunFunctionFmt -> DynaCall.
 * Counting there, keyed by the emulated function address, answers the question "which library and
 * which functions are eating the frame" without touching the generated JNI bridges - whose
 * register plumbing (float returns in particular) is better left alone. Off unless
 * zomdroid_jni_stats_enable() was called; then the cost per call is two counter reads and one
 * atomic add. Names arrive separately from the linker, which is the only place that knows them.
 */
#include <stdint.h>

extern int zomdroid_jni_stats_on;

void zomdroid_jni_stats_enable(void);
void zomdroid_jni_stats_register(uintptr_t fnc, const char* lib, const char* sym);
void zomdroid_jni_stats_record(uintptr_t fnc, uint64_t cycles);
void zomdroid_jni_stats_dump(void);

/* The generic timer counter: a register read, no call, no floating point. */
static inline uint64_t zomdroid_jni_stats_now(void) {
#if defined(__aarch64__)
    uint64_t v;
    __asm__ volatile("mrs %0, cntvct_el0" : "=r"(v));
    return v;
#else
    return 0;
#endif
}

#endif
