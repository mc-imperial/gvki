#ifndef GVKI_GLOBAL_LOG_FILE_H
#define GVKI_GLOBAL_LOG_FILE_H

#include <fstream>
namespace gvki
{

class GlobalLogFile
{
    private:
        std::ofstream output;
    public:
        GlobalLogFile();
        GlobalLogFile(const GlobalLogFile&); /* = delete; */
        ~GlobalLogFile();
        static GlobalLogFile& singleton();

        // Provide an "ostream like" operator<<()
        // that writes to the underlying file if
        // logging to a file is enabled, otherwise
        // is does nothing.
        template <class T>
        GlobalLogFile& operator <<(const T& rhs)
        {
            if (output.is_open())
            {
                output << rhs;
                output.flush();
            }

            return *this;
        }
};

}

#endif
