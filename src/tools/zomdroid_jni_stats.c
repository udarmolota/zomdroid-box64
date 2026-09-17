#include <stdio.h>
#include <string.h>
#include "zomdroid_jni_stats.h"

int zomdroid_jni_stats_on = 0;

#define ZJS_SLOTS 1024   /* power of two; a session binds a few hundred JNI symbols */

typedef struct {
    uintptr_t fnc;            /* 0 = free slot; claimed with a CAS, never released */
    const char* lib;
    const char* sym;
    uint64_t calls, cycles;   /* since the last dump */
    uint64_t calls_total, cycles_total;
} zjs_entry_t;

static zjs_entry_t zjs_table[ZJS_SLOTS];

static zjs_entry_t* zjs_slot(uintptr_t fnc) {
    if (fnc == 0) return 0;
    uint64_t h = ((uint64_t)fnc >> 2) * 0x9E3779B97F4A7C15ull;
    for (int probe = 0; probe < ZJS_SLOTS; probe++) {
        zjs_entry_t* e = &zjs_table[(h + probe) & (ZJS_SLOTS - 1)];
        uintptr_t cur = __atomic_load_n(&e->fnc, __ATOMIC_ACQUIRE);
        if (cur == fnc) return e;
        if (cur == 0) {
            uintptr_t expected = 0;
            if (__atomic_compare_exchange_n(&e->fnc, &expected, fnc, 0,
                                            __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
                return e;
            if (expected == fnc) return e;
        }
    }
    return 0; /* table full: the call is simply not counted */
}

void zomdroid_jni_stats_enable(void) {
    __atomic_store_n(&zomdroid_jni_stats_on, 1, __ATOMIC_RELEASE);
}

void zomdroid_jni_stats_register(uintptr_t fnc, const char* lib, const char* sym) {
    zjs_entry_t* e = zjs_slot(fnc);
    if (!e) return;
    /* The names are string literals / static table entries in the linker: they outlive us. */
    e->lib = lib;
    e->sym = sym;
}

void zomdroid_jni_stats_record(uintptr_t fnc, uint64_t cycles) {
    zjs_entry_t* e = zjs_slot(fnc);
    if (!e) return;
    __atomic_fetch_add(&e->calls, 1, __ATOMIC_RELAXED);
    __atomic_fetch_add(&e->cycles, cycles, __ATOMIC_RELAXED);
}

static uint64_t zjs_freq(void) {
#if defined(__aarch64__)
    uint64_t f;
    __asm__ volatile("mrs %0, cntfrq_el0" : "=r"(f));
    return f ? f : 1;
#else
    return 1;
#endif
}

/* The short JNI name: "Java_zombie_iso_LightingJNI_getDarkMulti" -> "getDarkMulti". */
static const char* zjs_short(const char* sym) {
    if (!sym) return "?";
    const char* p = strrchr(sym, '_');
    return (p && p[1]) ? p + 1 : sym;
}

void zomdroid_jni_stats_dump(void) {
    if (!zomdroid_jni_stats_on) return;
    const double us_per_cycle = 1e6 / (double)zjs_freq();

    /* Snapshot and reset the interval counters, so one dump = one interval. */
    static zjs_entry_t snap[ZJS_SLOTS];
    int n = 0;
    for (int i = 0; i < ZJS_SLOTS; i++) {
        zjs_entry_t* e = &zjs_table[i];
        if (!__atomic_load_n(&e->fnc, __ATOMIC_ACQUIRE)) continue;
        uint64_t c = __atomic_exchange_n(&e->calls, 0, __ATOMIC_RELAXED);
        uint64_t t = __atomic_exchange_n(&e->cycles, 0, __ATOMIC_RELAXED);
        if (!c) continue;
        e->calls_total += c;
        e->cycles_total += t;
        snap[n] = *e; snap[n].calls = c; snap[n].cycles = t; n++;
    }
    if (!n) { printf("[JNISTAT] no emulated JNI calls in this interval\n"); fflush(stdout); return; }

    /* Per-library totals first - that is the headline. */
    const char* libs[16]; uint64_t lcalls[16], lcycles[16]; int nl = 0;
    for (int i = 0; i < n; i++) {
        const char* l = snap[i].lib ? snap[i].lib : "?";
        int j = 0;
        for (; j < nl; j++) if (strcmp(libs[j], l) == 0) break;
        if (j == nl) { if (nl == 16) continue; libs[nl] = l; lcalls[nl] = 0; lcycles[nl] = 0; nl++; }
        lcalls[j] += snap[i].calls; lcycles[j] += snap[i].cycles;
    }
    printf("[JNISTAT] interval totals:");
    for (int j = 0; j < nl; j++)
        printf(" %s calls=%llu time=%.1fms", libs[j], (unsigned long long)lcalls[j],
               lcycles[j] * us_per_cycle / 1000.0);
    printf("\n");

    /* Then the top symbols by time - selection sort, n is small. */
    for (int k = 0; k < 12 && k < n; k++) {
        int best = k;
        for (int i = k + 1; i < n; i++) if (snap[i].cycles > snap[best].cycles) best = i;
        zjs_entry_t tmp = snap[k]; snap[k] = snap[best]; snap[best] = tmp;
        const zjs_entry_t* e = &snap[k];
        printf("[JNISTAT]   %-16s %-28s calls=%-8llu total=%8.2fms avg=%7.0fns\n",
               e->lib ? e->lib : "?", zjs_short(e->sym), (unsigned long long)e->calls,
               e->cycles * us_per_cycle / 1000.0,
               e->calls ? (e->cycles * us_per_cycle * 1000.0) / (double)e->calls : 0.0);
    }
    fflush(stdout);
}
