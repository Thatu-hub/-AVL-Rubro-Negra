// plataforma.h - ajustes pra rodar igual no Windows e no Linux
// prepara_console: no Windows faz os acentos aparecerem certo
// segundos_agora: relogio pra medir o tempo
#ifndef PLATAFORMA_H
#define PLATAFORMA_H

#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 199309L   // precisa disso pro clock_gettime com -std=c99
#endif

#ifdef _WIN32
// funcoes do Windows (kernel32)
__declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned int cp);
__declspec(dllimport) int __stdcall QueryPerformanceCounter(long long *t);
__declspec(dllimport) int __stdcall QueryPerformanceFrequency(long long *f);
#else
#include <time.h>
#endif

static inline void prepara_console(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
}

static inline double segundos_agora(void) {
#ifdef _WIN32
    static long long freq = 0;
    long long t;
    if (freq == 0) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t);
    return (double) t / (double) freq;
#else
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
#endif
}

#endif
