#include "client.h"

int main()
{
    NetWatchdogClient client("localhost");
    client.Run();

    return 0;
}