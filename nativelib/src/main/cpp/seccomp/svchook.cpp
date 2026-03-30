//
// Created by zgy on 2026/1/3.
//

#include "sys/syscall.h"
#include "svchook.h"
uint32_t my_syscall_tag = 0x11ffaae3;

size_t raw_syscall(size_t _number, size_t x0, size_t x1, size_t x2, size_t x3, size_t x4, size_t x5){
    size_t result;
    __asm__ __volatile__ (  // __volatile__ 防止编译器优化掉系统调用
            "MOV X8, %1\n\t"
            "MOV X0, %2\n\t"
            "MOV X1, %3\n\t"
            "MOV X2, %4\n\t"
            "MOV X3, %5\n\t"
            "MOV X4, %6\n\t"
            "MOV X5, %7\n\t"
            "SVC #0\n\t"
            "MOV %0, X0"
            : "=r" (result)
            : "r" (_number), "r" (x0), "r" (x1), "r" (x2), "r" (x3), "r" (x4), "r" (x5)
            : "x0", "x1", "x2", "x3", "x4", "x5", "x8"
            );
    return result;
}

void sigsys_handler(int signo, siginfo_t *info, void *data) {
    int my_signo = info->si_signo;
    LOGD("sig_handler2, signo: %x\n", my_signo);
    auto* uc = (ucontext_t*)data;
    mcontext_t* mc = &uc->uc_mcontext;
    unsigned long sysno = mc->regs[8];
    size_t ret;
    switch (sysno) {
        case __NR_exit:
            LOGD("[svc exit]: %llu,%llx,%llx",mc->regs[0],mc->pc,mc->regs[30]);
            ret = raw_syscall(sysno,mc->regs[0],mc->regs[1],mc->regs[2],mc->regs[3],mc->regs[4],my_syscall_tag);
            ((ucontext_t *) data)->uc_mcontext.regs[0] = ret;
            break;
        case __NR_exit_group:
            LOGD("[svc exit_group]: %llu,%llx,%llx",mc->regs[0],mc->pc,mc->regs[30]);
            ret = raw_syscall(sysno,mc->regs[0],mc->regs[1],mc->regs[2],mc->regs[3],mc->regs[4],my_syscall_tag);
            ((ucontext_t *) data)->uc_mcontext.regs[0] = ret;
            break;
        default:
            break;
    }
}

void hook_svc()
{
    LOGE("try hook svc by seccomp");
    /*hook svc exit,exit_group*/
    struct sock_filter filter[] = {
            BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
            BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit, 0, 2),
            BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, args[5])),
            BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, my_syscall_tag, 4, 5),
            BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),
            BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_exit_group, 0, 2),
            BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, args[5])),
            BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, my_syscall_tag, 0, 1),
            BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW),
            BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_TRAP)
    };

    sock_fprog prog{};
    prog.filter = filter;
    prog.len = (unsigned short) (sizeof(filter) / sizeof(filter[0]));
    if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) == -1) {
        LOGE("prctl PR_SET_NO_NEW_PRIVS fail");
    }
    if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog) == -1) {
        LOGE("prctl PR_SET_SECCOMP fail");
        return ;
    }
    struct sigaction sa{};
    sigset_t sigset;
    sigfillset(&sigset);
    sa.sa_sigaction = sigsys_handler;
    sa.sa_mask = sigset;
    sa.sa_flags = SA_SIGINFO;
    if (sigaction(SIGSYS, &sa, nullptr) == -1) {
        LOGE("sigsys_handler init fail.");
        return;
    }
    LOGE("seccomp install!");
}