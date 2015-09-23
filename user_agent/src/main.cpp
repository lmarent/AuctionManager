/* ------------------------- main() ------------------------- */

#include "ParserFcts.h"
#include "stdincpp.h"
#include "Agent.h"
#include "ConstantsAgent.h"
#include "Constants.h"

using namespace auction;

const char *NETAGENT_VERSION = "NET_AGENT version " VERSION ", (c) 2014-2015 Universidad de los Andes, Colombia";


const char *NETAGENT_OPTIONS = "compile options: "
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
        // start up the agent (this blocks until Ctrl-C !)
        cout << NETAGENT_VERSION << endl;
#ifdef DEBUG
        cout << NETAGENT_VERSION << endl;
#endif
        auto_ptr<Agent> agent(new Agent(argc, argv));

        // going into main loop
        agent->run();

    } catch (Error &e) {
        cerr << "Terminating Agent on error: " << e.getError() << endl;
        exit(1);
    }
}
