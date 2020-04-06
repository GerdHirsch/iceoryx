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
	static constexpr std::memory_order relaxed = std::memory_order_relaxed;
	/** all doors are open*/
	Gate() : gate(0){}
	/** opens all doors */
	~Gate(){ gate.store(0, relaxed); }

	/** doors are 1..n, 0 := all doors are open */
	void lock(std::size_t door=1ul){
		this->gate.store(door, relaxed);
	}
	/** doors are 1..n, 0 := all doors are open */
	void unlock(){
		this->gate.store(0ul, relaxed);
	}
	void passThrough(std::size_t door=1ul) const{
		while(this->gate.load(relaxed) == door){
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
	~ObservableGate(){ this->arrived.store(0, relaxed);}

	void passThrough(std::size_t door=1ul) {
		bool arrivedSet(false);
		while(this->gate.load(relaxed) == door){
			this->arrived.store(door, relaxed);
			if(!arrivedSet){ // atomic operation only the first time
				this->arrived.store(door, relaxed);
				arrivedSet = true;
			}
			std::this_thread::yield();
		}
		this->arrived.store(0, relaxed); // passThrough the gate
	}
	void waitForArrival(std::size_t door){
		while(!isArrived(door)){
			std::this_thread::yield();
		}
	}
	std::size_t arrivedDoor(){ return arrived.load(relaxed);}
	bool isArrived(std::size_t door){ return arrived.load(relaxed) == door;}
private:
	std::atomic<std::size_t> arrived;
};

}//namespace

#endif /* SRC_GATE_HPP_ */
