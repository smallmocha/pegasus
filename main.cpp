#include "server/webserver.h"

int main()
{
    WebServer webServer(9306, 4);
    webServer.Run();

    return 0;
}
