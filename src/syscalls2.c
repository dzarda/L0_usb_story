#include "ch.h"

void __NO_RETURN _exit(int __status) {
    chSysHalt("_exit");
    while (true) {
    }
}

void __NO_RETURN _kill(int __status) {
    chSysHalt("_kill");
    while (true) {
    }
}

int _getpid(int __status) {
    return 0;
}
