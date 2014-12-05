#include "gvki/GlobalLogFile.h"
#include <cstdlib>
#include <iostream>

using namespace std;

namespace gvki
{
    GlobalLogFile& GlobalLogFile::singleton()
    {
        static GlobalLogFile g;
        return g;
    }

    GlobalLogFile::GlobalLogFile()
    {
        char* logFile = getenv("GVKI_LOG_FILE");

        if (!logFile)
            return;

        output.open(logFile, ofstream::app);

        if (!output.good())
            cerr << "Failed to open log file \"" << logFile << "\"." << endl;
        else
            output << "*** START" << endl << endl;
    }

    GlobalLogFile::~GlobalLogFile()
    {
        if (output.good())
        {
            output << "*** END" << endl << endl;
            output.close();
        }
    }
}
