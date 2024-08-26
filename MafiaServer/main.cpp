#include "Include.h"
#include "Server.h"
#include "ChatFilter.h"

ChatFilter filter;

int main()
{
    cout << SERVER_NAME " " MAFIA_PROTOCOL_VERSION << endl;
    cout << BUILD " Build, Compiled with GCC " GCC_VERSION << endl << endl;

/*#if defined PORT_6002
    Port listen_port = 6002;
#else
    Port listen_port = 5301;
#endif*/
    Port listen_port = 6002;

    cout << "Listening port : " << listen_port << endl;

    filter.read("dirty_words.txt");

    cout << "Server Started." << endl;

    Server server;
    server.run(listen_port);

    return 0;
}