#include <unistd.h>
#include "server/webserver.h"

int main(int argc, const char** argv) {
    
    WebServer server(
        5678, 3, 60000, false,  /* 端口 ET模式 timeoutMs 优雅退出  */
        12, 6); /* 连接池数量 线程池数量*/

    server.Start();
    //return 0;
}