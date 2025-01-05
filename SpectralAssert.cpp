#include "SpectralAssert.h"
#include "iostream"
#include "Editor.h"

#include <sstream>


void SpectralAssert::CallAssert(const char* expr_str, bool expr, const char* file, int line, const std::string& msg)
{
	if (expr == false)
	{
        std::ostringstream oss;
        oss << "-----------------------------------------------------------------------\n"
            << "Assert failed:\t" << msg << "\n"
            << "Expression:\t" << expr_str << "\n"
            << "Source:\t\t" << file << ", line " << line << "\n"
            << "-----------------------------------------------------------------------\n";

        Logger::Error(oss.str());
		abort();
	}
}
