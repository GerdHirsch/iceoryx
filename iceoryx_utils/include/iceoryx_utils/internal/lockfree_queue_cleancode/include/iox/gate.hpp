/*
 * Gate.hpp
 *
 *  Created on: Aug 29, 2019
 *      Author: user
 */

#ifndef SRC_GATE_HPP_
#define SRC_GATE_HPP_

#include <atomic>
#include <thread>
#include <iostream>

namespace iox{

class Gate{
public:
	static constexpr std::memory_order storeOrder = std::memory_order_relaxed;
	static constexpr std::memory_order loadOrder = std::memory_order_relaxed;
//	static constexpr std::memory_order storeOrder = std::memory_order_release;
//	static constexpr std::memory_order loadOrder = std::memory_order_acquire;
	/** all doors are open*/
	Gate() : gate(0){}
	/** opens all doors */
	~Gate(){ gate.store(0, storeOrder); }

	/** doors are 1..n, 0 := all doors are open */
	void lock(std::size_t door=1ul){
		this->gate.store(door, storeOrder);
	}
	/** doors are 1..n, 0 := all doors are open */
	void unlock(){
		this->gate.store(0ul, storeOrder);
	}
	void passThrough(std::size_t door=1ul) const{
		while(this->gate.load(loadOrder) == door){
			std::this_thread::yield();
		}
	}
protected:
	std::atomic<std::size_t> gate;
};
class ObservableGate : public Gate{
public:
	using base_type = Gate;

	ObservableGate() : Gate(), arrived(0){}
	~ObservableGate(){ this->arrived.store(0, storeOrder);}

	void passThrough(std::size_t door=1ul) {
		bool arrivedSet(false);
		while(this->gate.load(loadOrder) == door){
			if(!arrivedSet){ // atomic operation only the first time
				this->arrived.store(door, storeOrder);
				arrivedSet = true;
			}
			std::this_thread::yield();
		}
		this->arrived.store(0, storeOrder); // passThrough the gate
	}
	void waitForArrival(std::size_t door){
		while(!isArrived(door)){
			std::this_thread::yield();
		}
	}
	std::size_t arrivedDoor(){ return arrived.load(storeOrder);}
	bool isArrived(std::size_t door){ return arrived.load(storeOrder) == door;}
private:
	std::atomic<std::size_t> arrived;
};

}//namespace

#endif /* SRC_GATE_HPP_ */
