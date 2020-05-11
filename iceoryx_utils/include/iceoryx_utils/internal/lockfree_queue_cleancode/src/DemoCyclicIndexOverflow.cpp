/*
 * DemoCyclicIndex.cpp
 *
 *  Created on: Mar 23, 2020
 *      Author: user
 */
#include "../include/iox/cyclic_index.hpp"

#include <iostream>
#include <limits>
using namespace std;

void demoCyclicIndexOperatorPlus(){
	cout << "demoCyclicIndexOperatorPlus" << endl;

	constexpr auto CycleLength = 10u;
	using Index = iox::CyclicIndex<CycleLength>; // two indices [0][1] two values to push(..)

	Index readPosition;
	Index writePosition(0, 1);

		cout
			<< "readPosition.getIndex()	:   " << readPosition.getIndex() << endl
			<< "readPosition.getCycle()	:   " << readPosition.getCycle() << endl
			;
		cout
			<< "writePosition.getIndex()	:   " << writePosition.getIndex() << endl
			<< "writePosition.getCycle()	:   " << writePosition.getCycle() << endl
			;
	if(readPosition + CycleLength == writePosition){
		cout << "operator+ succeed" << endl;
	}else{
		cout << "operator+ failed" << endl;
	}



}

void demoCyclicIndexOverflow(){
	cout << "demoCyclicIndexOverflow" << endl;

	constexpr auto CycleLength = 2u;
	using Index = iox::CyclicIndex<CycleLength>; // two indices [0][1] two values to push(..)
	using indexvalue_t = Index::NativeType;
	constexpr auto MAXCycle = 	std::numeric_limits<Index::NativeType>::max();

	//=================================
	// 1
	//=================================
	Index tail(1, MAXCycle-1); // 1 Bit für den index, der rest für den aba counter
	cout
	<< "tail.getIndex()	:   " << tail.getIndex() << endl
	<< "tail.getCycle()	:   " << tail.getCycle() << endl
	;
	tail = tail + 1;
	cout
	<< "tail.getIndex()	:   " << tail.getIndex() << endl
	<< "tail.getCycle()	:   " << tail.getCycle() << endl
	;
	tail = tail + 1;
	cout
	<< "tail.getIndex()	:   " << tail.getIndex() << " last Field and max cycle" << endl
	<< "tail.getCycle()	:   " << tail.getCycle() << endl
	;
	tail = tail + 1;
	cout
	<< "tail.getIndex()	:   " << tail.getIndex() << " first Field and first cycle" << endl
	<< "tail.getCycle()	:   " << tail.getCycle() << endl
	;
	//=================================
	// 2
	// führt zu demselben ergebnis wie bei 1
	//=================================
	Index lastTail(1, MAXCycle);
	cout
	<< "lastTail.getIndex()	:   " << lastTail.getIndex() << " last Field and max cycle" << endl
	<< "lastTail.getCycle()	:   " << lastTail.getCycle() << endl
	;
	// allowed index_value_t values are [0, 1] no duplicates!
	// tail is next write position
	// tail == Index(1, MAXCycle)
	// m_values[0] == 0, m_values[1] == invalid/free
	Index m_values[CycleLength] = {Index(0, MAXCycle), Index(0, MAXCycle-1)};
	cout
	<< endl
	<< "m_values[0]: [" << m_values[0].getIndex() << ", " << m_values[0].getCycle() << "]" << endl
	<< "m_values[1]: [" << m_values[1].getIndex() << ", " << m_values[1].getCycle() << "]"
	<< " Line: "<< __LINE__
	<< endl
	;


	//=========================================
	//push(index_value_t index){...} ab hier mehrere push und pop operationen
	//=========================================
	// 0 is already pushed, queue has one place left, cycle is max
	indexvalue_t index = 1; // call argument of push(1)

	//	cycle * CycleLength + index
	Index m_tail(1, MAXCycle);
	cout
	<< "m_tail: [" << m_tail.getIndex() << ", " << m_tail.getCycle() << "]"
	<< " Line: "<< __LINE__
	<< endl
	;
	Index oldTail = m_tail; //= m_tail.load(std::memory_order_acquire);
	auto position = oldTail.getIndex();

	Index value = m_values[position]; // = m_values[position].load(std::memory_order_relaxed);
	auto tailCycle = oldTail.getCycle();
	auto valueCycle = value.getCycle();

	cout << endl;
	cout
	<< "=== first push(index):	" << index << " Line: "<< __LINE__ << endl
	<< "position: 	" << position << endl
	<< "tailCycle:	" << tailCycle << endl
	<< "valueCycle:	" << valueCycle << endl
	;
	cout
	<< "valueCycle + static_cast<indexvalue_t>(1): "
	<< valueCycle + static_cast<indexvalue_t>(1)
	<< " Line: "<< __LINE__
	<< endl
	;
	if(valueCycle + 1 != tailCycle){
		cout << "continue" << endl;
	}
	Index newValue = Index(index, tailCycle);//Index newValue(1, tailCycle);
	m_values[position] = newValue;//m_values[position].compare_exchange_strong
	/* newValue = value, newValue wird an position gesetzt
	 * if (m_values[position].compare_exchange_strong(
			value, newValue, std::memory_order_acq_rel, std::memory_order_acquire))
	{
		break; // push successful
	}
	while(true)

	 */
	m_tail = oldTail + 1; // m_tail.compare_exchange_strong(oldTail, oldTail + 1

	cout << endl;
	cout
	<< "=== after push(1):	" << " Line: "<< __LINE__ << endl
	<< "m_tail.getIndex()	:   " << m_tail.getIndex() << endl
	<< "m_tail.getCycle()	:   " << m_tail.getCycle() << endl
	;
	cout
	<< endl
	<< "m_values[0]: [" << m_values[0].getIndex() << ", " << m_values[0].getCycle() << "]" << endl
	<< "m_values[1]: [" << m_values[1].getIndex() << ", " << m_values[1].getCycle() << "]"
	<< " Line: "<< __LINE__
	<< endl
	;

	// queue wird geleert
	cout << endl << "pop(index) index == 0" << endl;
	cout << "pop(index) index == 1" << endl;
	;
	// die values 0 und 1 können wieder gepushed werden
	//====================================
	cout << endl;
	cout << "push(0) again" << " Line: "<< __LINE__ << endl;
	index = 0;
	// oldTail = m_tail.load(std::memory_order_acquire);
	oldTail = m_tail;
	position = oldTail.getIndex();
	value = m_values[position];
	tailCycle = oldTail.getCycle();
	valueCycle = value.getCycle();

	cout
	<< "=== next push(index):	" << index << " Line: "<< __LINE__ << endl
	<< "position: 	" << position << endl
	<< "tailCycle:	" << tailCycle << endl
	<< "valueCycle:	" << valueCycle << endl
	;
	cout
		<< "valueCycle + static_cast<indexvalue_t>(1): "
		<< valueCycle + static_cast<indexvalue_t>(1)
		<< " Line: "<< __LINE__
		<< endl
	;
	if(valueCycle + 1 != tailCycle){
		cout << "continue -> endless loop if no other thread changes the queue" << endl;
	}
	newValue = Index(index, tailCycle);
	m_values[position] = newValue; //m_values[position].compare_exchange_strong(
	m_tail = oldTail + 1; // m_tail.compare_exchange_strong(oldTail, oldTail + 1
	cout
	<< endl
	<< "m_values[0]: [" << m_values[0].getIndex() << ", " << m_values[0].getCycle() << "]" << endl
	<< "m_values[1]: [" << m_values[1].getIndex() << ", " << m_values[1].getCycle() << "]"
	<< " Line: "<< __LINE__
	<< endl
	;
	//====================================
	cout << endl;
	cout << "push(1) again" << endl;
	index = 1;
	// oldTail = m_tail.load(std::memory_order_acquire);
	oldTail = m_tail;
	position = oldTail.getIndex();
	value = m_values[position];
	tailCycle = oldTail.getCycle();
	valueCycle = value.getCycle();

	cout
	<< "=== next push(index):	" << index << " Line: "<< __LINE__ << endl
	<< "position: 	" << position << endl
	<< "tailCycle:	" << tailCycle << endl
	<< "valueCycle:	" << valueCycle << endl
	;

	cout
	<< "valueCycle + static_cast<indexvalue_t>(1): "
	<< valueCycle + static_cast<indexvalue_t>(1)
	<< " Line: "<< __LINE__
	<< endl
	;
	if(valueCycle + static_cast<indexvalue_t>(1) != tailCycle){
		cout << "continue -> endless loop if no other thread changes the queue" << endl;
	}
	newValue = Index(index, tailCycle);
	m_values[position] = newValue; //m_values[position].compare_exchange_strong(
	m_tail = oldTail + 1; // m_tail.compare_exchange_strong(oldTail, oldTail + 1
	cout
	<< endl
	<< "m_values[0]: [" << m_values[0].getIndex() << ", " << m_values[0].getCycle() << "]" << endl
	<< "m_values[1]: [" << m_values[1].getIndex() << ", " << m_values[1].getCycle() << "]"
	<< " Line: "<< __LINE__
	<< endl
	;

}


