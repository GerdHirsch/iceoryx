/*
 * DemoEndlessloopInPop.cpp
 *
 *  Created on: 15.04.2020
 *      Author: Gerd
 */

#include "../include/iox/gated_monitoring_policy.hpp"
#include "../include/iox/index_queue.hpp"
#include "../include/iox/index_queue_mk.hpp"

#include <iostream>
#include <thread>
using namespace std;

	constexpr size_t Capacity{3};

using Thread = std::thread;
using Queue = iox::IndexQueue<Capacity>;
using UniqueIdx = Queue::UniqueIndexType;
using Policy = iox::GatedMonitoringPolicy<iox::EmptyMonitoringPolicy>;
//	using Policy = iox::EmptyMonitoringPolicy;

static void print(Queue const& sut, const char* message){
	//return; // switch on/off output
	std::cout << "      demo: " << this_thread::get_id() << " "<< message << std::endl;
	return;
	sut.print();
}

void demoEndlessloopInPop(){
	cout << "demoEndlessloopInPop" << endl;

	Queue sut(Queue::ConstructFull::Policy);
	Policy sutPolicy;
	Policy testPolicy;

	print(sut, "ctor full");
//======================================================
	// SUT
	UniqueIdx idxSutToPop{100};
	bool popSUTReturnValue{true};

	auto sutTask = [&](){
		cout << "       sut: " << this_thread::get_id() << endl;
		popSUTReturnValue = sut.pop(idxSutToPop, sutPolicy);
	};
//------------------------------------------
	sutPolicy.lock(Queue::AfterLoadPosition);
	Thread sutThread(sutTask);
	sutPolicy.waitForArrival(Queue::AfterLoadPosition);

	print(sut, "sut after sutCheckpoint");
//======================================================
	// Test
	UniqueIdx idxTestToPop{100};
	bool testReturnValue{true};
	auto testTask =[&](){
		cout << "      test: " << this_thread::get_id() << endl;
		testReturnValue = sut.pop(idxTestToPop, testPolicy);
	};
//======================================================
	// drain sut
	print(sut, "drain sut");
	UniqueIdx dummy;
	while(sut.pop(dummy)){
		cout << "dummy: " << dummy.getIndex() << endl;
	}
	print(sut, "sut after drain");
//======================================================
	testPolicy.lock(Queue::EndOfMethod);
	Thread testThread(testTask);
//	testThread.join();
	testPolicy.waitForArrival(Queue::EndOfMethod);

	print(sut, "sut after test pop");
//======================================================
	sutPolicy.unlock();
	sutThread.join();

	testPolicy.unlock();
	testThread.join();

	print(sut, "sut after thread join");

	cout << "end of demoEndlessloopInPop()" << endl;
}
//============================================================================================
void demoEndlessloopInPopOld(){
	cout << "demoEndlessloopInPop" << endl;

	Policy sutPolicy;
	Policy drainPolicy;
	Queue sut(Queue::ConstructFull::Policy);

	print(sut, "ctor full");

//======================================================
	//Drain
	auto drainTask = [&](){
		UniqueIdx dummy;
		while(sut.pop(dummy, drainPolicy)){
			cout << "dummy: " << dummy.getIndex() << endl;
		}
	};
//	cout << endl << "drain sut in thread before sut task is executed" << endl;
//	drainPolicy.lock(Queue::AfterLoadPosition);
//	Thread drainThread(drainTask);
//	drainPolicy.waitForArrival(Queue::AfterLoadPosition);
//	drainThread.join();

	print(sut, "sut after drain");
//	sut.push(std::move(dummy));
//	sut.pop(dummy);
//------------------------------------------
//	cout << endl << "drain sut in f() before sut task is executed" << endl;
//	UniqueIdx dummy;
//	while(sut.pop(dummy)){
//		cout << "dummy: " << dummy.getIndex() << endl;
//	}
//		;
//	print(sut, "after drain");
//======================================================
	// SUT
	UniqueIdx idxSutToPop{100};
	bool popSUTReturnValue{true};

	auto sutTask = [&](){
		popSUTReturnValue = sut.pop(idxSutToPop, sutPolicy);
	};
//------------------------------------------
	cout << endl << "sut.pop start sutThread" << endl;
	sutPolicy.lock(Queue::AfterLoadPosition);
	Thread sutThread(sutTask);
	sutPolicy.waitForArrival(Queue::AfterLoadPosition);

	print(sut, "sut after sutcheckpoint");
//======================================================
	cout << endl << "drain sut in thread after sut task is executed" << endl;
//	drainPolicy.lock(Queue::EndOfMethod);
//	drainPolicy.waitForArrival(Queue::EndOfMethod);
	drainPolicy.unlock();
	Thread drainThread(drainTask);
	cout << endl << "drainThread.join" << endl;
	drainThread.join();
//------------------------------------------
//	cout << endl << "drain sut after sut task is executed" << endl;
//	UniqueIdx dummy;
//	while(sut.pop(dummy))
//		;
//	print(sut, "after drain");
//======================================================
	cout << endl << "sutThread.join" << endl;
	sutPolicy.unlock();
	sutThread.join();

	print(sut, "sut after join");
//======================================================
}

