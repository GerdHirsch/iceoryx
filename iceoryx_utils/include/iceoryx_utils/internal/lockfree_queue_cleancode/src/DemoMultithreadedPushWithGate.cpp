/*
 * DemoMultithreadedPushWithGate.cpp
 *
 *  Created on: Apr 2, 2020
 *      Author: user
 */

#include "../include/iox/unique_index.hpp"
#include "../include/iox/index_queue.hpp"

#include "../include/iox/gated_monitoring_policy.hpp"


#include <iostream>
#include <thread>
using namespace std;

void demoEquivalenceclassA(){ //NoConcurrencyPush
	cout << endl << "===== demoEquivalenceclassA" << endl;

	constexpr size_t Capacity{4};
	using Queue = iox::IndexQueue<Capacity>;
	using IdxType = Queue::UniqueIndexType;
	Queue queue(Queue::ConstructFull::Policy);
	IdxType idx;
	queue.print();
	queue.pop(idx);
	queue.push(std::move(idx));
	queue.pop(idx);
	queue.push(std::move(idx));
	while(queue.pop(idx)){
		cout << "idx: " << idx << endl;
	}
}
void demoEquivalenceclassB(){ // test thread is not interrupted
	cout << endl << "===== demoEquivalenceclassB" << endl;
	using Policy = iox::GatedMonitoringPolicy<>;
	Policy SUTPolicy;

	constexpr size_t Capacity{4};
	using Queue = iox::IndexQueue<Capacity>;
	using IdxType = Queue::UniqueIndexType;

	Queue queue(Queue::ConstructFull::Policy);
	IdxType idx, idx0, idx1;

	queue.pop(idx0);// 0
	queue.pop(idx1);// 1

	cout << "idx0: " << idx0 << endl;
	cout << "idx1: " << idx1 << endl;

	auto SUTpush = [&](){
		queue.push(std::move(idx0), SUTPolicy);
	};
	auto checkPoint = Queue::AfterLoadPosition;
//	auto checkPoint = Queue::AfterLoadValue;

	cout << "SUTPolicy.lock(checkPoint)" << endl;
	SUTPolicy.lock(checkPoint);
	cout << "SUTThread(SUTpush)" << endl;
	thread SUTThread(SUTpush);
	SUTPolicy.waitForArrival(checkPoint);

	cout << "queue.push(idx1);" << endl;
	queue.push(std::move(idx1));

	cout << "SUTPolicy.unlock()" << endl;
	SUTPolicy.unlock();
	SUTThread.join();

	while(queue.pop(idx)){
		cout << "idx: " << idx << endl;
	}

}
void demoEquivalenceclassC(){ // test thread is interrupted
	cout << endl << "===== demoEquivalenceclassC" << endl;
	using Policy = iox::GatedMonitoringPolicy<>;
	Policy SUTPolicy;
	Policy TestPolicy;

	constexpr size_t Capacity{4};
	using Queue = iox::IndexQueue<Capacity>;
	using IdxType = Queue::UniqueIndexType;

	Queue queue(Queue::ConstructFull::Policy);
	IdxType idx, idx0, idx1;

	queue.pop(idx0);// 0
	queue.pop(idx1);// 1

	cout << "idx0: " << idx0 << endl;
	cout << "idx1: " << idx1 << endl;

	auto SUTpush = [&](){
		queue.push(std::move(idx0), SUTPolicy);
	};
	auto testPush = [&](){
		queue.push(std::move(idx1), TestPolicy);
	};
	auto checkPoint = Queue::AfterLoadPosition;
//	auto checkPoint = Queue::AfterLoadValue;
	auto testCheckPoint = Queue::BeforeUpdatePosition;

	cout << "SUTPolicy.lock(checkPoint);" << endl;
	SUTPolicy.lock(checkPoint);
	cout << "SUTThread(SUTpush)" << endl;
	thread SUTThread(SUTpush);
	SUTPolicy.waitForArrival(checkPoint);

	cout << endl << "after sut checkpoint " << endl;
	queue.print();

	cout << "testThread(testPush)" << endl;
	TestPolicy.lock(testCheckPoint);
	thread testThread(testPush);
	TestPolicy.waitForArrival(testCheckPoint);

	cout << endl << "after test checkpoint " << endl;
	queue.print();

	cout << "SUTPolicy.unlock()" << endl;
	SUTPolicy.unlock();
	SUTThread.join();

	cout << "TestPolicy.unlock()" << endl;
	TestPolicy.unlock();
	testThread.join();

	cout << endl << "after thread join " << endl;
	queue.print();

	while(queue.pop(idx)){
		cout << "idx: " << idx << endl;
	}
}
void demoMultithreadedPushWithGate(){
	cout << "demoMultithreadedPushWithGate" << endl;
	demoEquivalenceclassA();
	demoEquivalenceclassB();
	demoEquivalenceclassC();
}



