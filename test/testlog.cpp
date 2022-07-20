/*
 * @Author       : mark
 * @Date         : 2020-06-20
 * @copyleft Apache 2.0
 */ 
#include "../log/log.h"
#include "../pool/threadpool.h"
#include <features.h>

#if __GLIBC__ == 2 && __GLIBC_MINOR__ < 30
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#endif

void TestLog() {
    int cnt = 0, level = 0;
    Log::Instance()->init(level, "./testlog1", ".log", 0);
    // for(level = 3; level >= 0; level--) {
        level = 2;
        Log::Instance()->SetLevel(level);
        for(int j = 0; j < 10; j++ ){
            // for(int i = 0; i < 4; i++) {
                LOG_BASE(level,"%s 111111111 %d ============= ", "Test", cnt++);
            // }
        }
    // }
    // cnt = 0;
    // Log::Instance()->init(level, "./testlog2", ".log", 5000);
    // for(level = 0; level < 4; level++) {
    //     Log::Instance()->SetLevel(level);
    //     for(int j = 0; j < 10000; j++ ){
    //         for(int i = 0; i < 4; i++) {
    //             LOG_BASE(i,"%s 222222222 %d ============= ", "Test", cnt++);
    //         }
    //     }
    // }
}

int main() {
    TestLog();
}