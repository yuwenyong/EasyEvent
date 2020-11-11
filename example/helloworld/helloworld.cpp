//
// Created by yuwenyong on 2020/10/27.
//


#include "EasyEvent/EasyEvent.h"

using namespace EasyEvent;


int main (int argc, char **argv) {
    EasyEvent::TaskPool taskPool;
    taskPool.start(1);
    auto v1 = taskPool.submit([]() {
       printf("Task 1\n");
       return 5;
    });
    auto v2 = taskPool.submit([]() {
        printf("Task 2\n");
        return 6;
    });
    auto v3 = taskPool.submit([]() {
       printf("Task 3\n");
    });

    taskPool.submit([]() {
        printf("Task 4\n");
        return 8;
    });
    taskPool.stop();
//    taskPool.wait();
    printf("Task 1: %d\n", v1.get());
    printf("Task 2: %d\n", v2.get());
    printf("Task 3: %d\n", isReady(v3) ? 1 : 0);
    return 0;
}
