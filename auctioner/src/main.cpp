/* ------------------------- main() ------------------------- */

#include "ParserFcts.h"
#include "stdincpp.h"
#include "Auctioner.h"
#include "ConstantsAum.h"
#include "Constants.h"

using namespace auction;

const char *NETAUM_VERSION = "NETAuM version " VERSION ", (c) 2014-2015 Universidad de los Andes, Colombia";


const char *NETAUM_OPTIONS = "compile options: "
"multi-threading support = "
#ifdef ENABLE_THREADS
"[YES]"
#else
"[NO]"
#endif
", secure sockets (SSL) support = "
#ifdef USE_SSL
"[YES]"
#else
"[NO]"
#endif
" ";

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
