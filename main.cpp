#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "server/webserver.h"

int main()
{
    WebServer webServer(9306, 4);
    webServer.Run();

    return 0;
}
