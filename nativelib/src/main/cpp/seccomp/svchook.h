//
// Created by zgy on 2026/1/3.
//

#ifndef HAPPYTRACE_SVCHOOK_H
#define HAPPYTRACE_SVCHOOK_H
#include "linux/seccomp.h"
#include <linux/filter.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <asm-generic/unistd.h>
#include "logger.h"

void hook_svc();
size_t raw_syscall(size_t __number, size_t x0, size_t x1, size_t x2, size_t x3, size_t x4, size_t x5);
#endif //HAPPYTRACE_SVCHOOK_H
