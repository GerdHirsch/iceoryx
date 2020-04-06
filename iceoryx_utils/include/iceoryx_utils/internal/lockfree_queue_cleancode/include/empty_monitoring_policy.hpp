#pragma once

#include <cstddef>

namespace iox{

class EmptyMonitoringPolicy{
public:
	constexpr EmptyMonitoringPolicy(){}

	static void checkPoint(std::size_t door=1ul){}
	static void passThrough(std::size_t door=1ul){}
	static void skipped(){}
	static void push(){}
	static void pop(){}
	static void emptyPop(){}
	static void popContention(){}
	static void pushContention(){}
};
template<class BasePolicy = EmptyMonitoringPolicy, std::size_t Owner=0>
class ContentionCountingPoliy : public BasePolicy
{
public:
	static void popContention(){ ++numPopContention();}
	static void pushContention(){ ++numPushContention();}
	static size_t& numPopContention(){
		static size_t contention{};
		return contention;
	}
	static size_t& numPushContention(){
		static size_t contention{};
		return contention;
	}
};
}//end namespace
