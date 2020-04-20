/*
 * DemoCyclicQueue.cpp
 *
 *  Created on: Mar 23, 2020
 *      Author: user
 */

#include "../include/iox/index_queue.hpp"
#include "../include/iox/unique_index.hpp"
#include "../include/iox/gated_monitoring_policy.hpp"

#include <iostream>
#include "../include/iox/mk/index_queue_mk_old.hpp"

using namespace std;

void demoIndexQueue(){
	cout << "demoIndexQueue" << endl;

	constexpr size_t Capacity{4};

	using Queue = iox::IndexQueue<Capacity>;
	using IdxType = Queue::UniqueIndexType;

	Queue freeIndices(Queue::ConstructFull);
	Queue usedIndices(Queue::ConstructEmpty);

	cout << "==== full Queue freeIndices.print() " << endl;
	freeIndices.print();
	cout << endl;
	cout << "==== empty Queue usedIndices.print()" << endl;
	usedIndices.print();
	cout << endl;

	IdxType idx{0};

	if(usedIndices.pop(idx)){
		cout << "usedIndices not empty idx: "
				<< static_cast<IdxType::NativeType>(idx) << endl;
	}
	else{
		cout << "usedIndices empty" << endl;
	}

	double data[] = { 3.14, 2.71, 42, 665 };

	cout << "freeIndices.pop(idx)" << endl;
	while(freeIndices.pop(idx)){
		cout << "idx: " << static_cast<IdxType::NativeType>(idx) << endl;
		cout << "data[idx]: " << data[idx] << endl;
		usedIndices.push(std::move(idx));
	}
//	usedIndices.pop(idx);
//	IdxType wrongIdx{2};
//	usedIndices.push(wrongIdx);// endless loop

	cout << "usedIndices.pop(idx)" << endl;
	while(usedIndices.pop(idx)){
		cout << "idx: " << static_cast<IdxType::NativeType>(idx)<< endl;
		freeIndices.push(std::move(idx));
	}

}


