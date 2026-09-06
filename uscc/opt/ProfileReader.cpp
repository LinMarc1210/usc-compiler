//
//  ProfileReader.cpp
//
//  Parser implementation for the EDGE_PROFILE / EDGES / END_PROFILE
//  block format produced by gen_profiles.sh.
//
//---------------------------------------------------------
//  Distributed under the BSD license.
//---------------------------------------------------------

#include "ProfileReader.h"
#include <llvm/Support/raw_ostream.h>

#include <fstream>
#include <cctype>
#include <cstring>

namespace uscc
{
namespace opt
{

namespace {

std::string trim(std::string s)
{
	while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front())))
		s.erase(s.begin());
	while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back())))
		s.pop_back();
	return s;
}

} // anonymous

ProfileMap parseProfileFile(const std::string &path)
{
	ProfileMap m;
	std::ifstream f(path.c_str());
	if (!f)
	{
		llvm::errs() << "ProfileReader: cannot open profile '" << path << "'\n";
		return m;
	}
	std::string line, curFn;
	bool inEdges = false;
	while (std::getline(f, line))
	{
		std::string::size_type p;
		if ((p = line.find("EDGE_PROFILE:")) != std::string::npos)
		{
			curFn = trim(line.substr(p + std::strlen("EDGE_PROFILE:")));
			inEdges = false;
		}
		else if (trim(line) == "EDGES:")
		{
			inEdges = true;
		}
		else if (line.find("END_PROFILE") != std::string::npos)
		{
			inEdges = false;
			curFn.clear();
		}
		else if (inEdges)
		{
			std::string::size_type arrow = line.find("->");
			std::string::size_type colon = line.rfind(':');
			if (arrow == std::string::npos || colon == std::string::npos || colon < arrow)
				continue;
			std::string src = trim(line.substr(0, arrow));
			std::string dst = trim(line.substr(arrow + 2, colon - (arrow + 2)));
			uint64_t cnt = 0;
			try { cnt = std::stoull(trim(line.substr(colon + 1))); }
			catch (...) { continue; }
			EdgeKey k;
			k.fn = curFn;
			k.src = src;
			k.dst = dst;
			m[k] = cnt;
		}
	}
	return m;
}

} // opt
} // uscc
