/*
 * GatedMonitoringPolicy.h
 *
 *  Created on: 08.02.2020
 *      Author: Gerd
 */

#ifndef INCLUDE_GATE_GATEDMONITORINGPOLICY_H_
#define INCLUDE_GATE_GATEDMONITORINGPOLICY_H_

#include "empty_monitoring_policy.hpp"
#include "gate.hpp"


namespace iox{

template<class BasePolicy = iox::EmptyMonitoringPolicy>
class GatedMonitoringPolicy : public BasePolicy{
public:
	mutable ObservableGate gate;

	void lock(std::size_t door=1ul) {
		gate.lock(door);
	}
	bool isArrived(std::size_t door){
		return gate.isArrived(door);
	}
	void waitForArrival(std::size_t door){
		gate.waitForArrival(door);
	}
	void unlock(){
		gate.unlock();
	}
	void checkPoint(std::size_t door=1ul) const{
		gate.passThrough(door);
	}
	void passThrough(std::size_t door=1ul) const{
		gate.passThrough(door);
	}
};

} //namespace


#endif /* INCLUDE_GATE_GATEDMONITORINGPOLICY_H_ */
