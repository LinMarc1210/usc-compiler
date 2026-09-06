//
//  ProfileReader.h
//  uscc
//
//  Shared parser for edge-profile text files produced by PA2's
//  -EN / -EO instrumentation (run through lli, post-processed by
//  gen_profiles.sh). Used by PA5 SpecLICM and PA6 profile-guided
//  register allocation.
//
//---------------------------------------------------------
//  Distributed under the BSD license.
//---------------------------------------------------------
#pragma once

#include <map>
#include <string>
#include <tuple>
#include <cstdint>

namespace uscc
{
namespace opt
{

struct EdgeKey
{
	std::string fn, src, dst;
	bool operator<(const EdgeKey &o) const
	{
		return std::tie(fn, src, dst) < std::tie(o.fn, o.src, o.dst);
	}
};

typedef std::map<EdgeKey, uint64_t> ProfileMap;

// Parse an edge-profile text file and return the populated
// ProfileMap. If the file cannot be opened, returns an empty map
// (callers treat empty-map as "no profile loaded, fall back to
// pre-profile behavior").
ProfileMap parseProfileFile(const std::string &path);

} // opt
} // uscc
