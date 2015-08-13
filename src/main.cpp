/* ------------------------- main() ------------------------- */

#include "stdincpp.h"
#include "Auctioner.h"


// Log functions are not used before the logger is initialized

int main(int argc, char *argv[])
{

    try {
        // start up the netmate (this blocks until Ctrl-C !)
        cout << NETAUM_VERSION << endl;
#ifdef DEBUG
        cout << NETAUM_OPTIONS << endl;
#endif
        auto_ptr<Auctioner> auction(new Auctioner(argc, argv));

        // going into main loop
        auction->run();

    } catch (Error &e) {
        cerr << "Terminating Auctioner on error: " << e.getError() << endl;
        exit(1);
    }
}
