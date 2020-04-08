/*
 * IndexQueueTests.h
 *
 *  Created on: 7.5.2018
 *      Author: Gerd
 */

//#pragma once

#include <iox/gated_monitoring_policy.hpp>

#include "cute.h"

#include <cstddef>
#include <thread>
#include <chrono>
#include <iostream>

template<uint64_t SUTcheckpoint_, uint64_t testCheckpoint_>
struct TestParameter{
	static constexpr auto SUTcheckpoint = SUTcheckpoint_;
	static constexpr auto testCheckpoint = testCheckpoint_;
};

template<class SUTType, class Parameter>
class IndexQueueTestMultithreaded{
public:
	using SUT  = SUTType;
	using UniqueIdx = typename SUT::UniqueIndexType;
	using NativeType = typename SUT::NativeType;
	static constexpr auto CAPACITY = SUT::CAPACITY;


	/** tests should be made with
	 * different CAPACITIYs of the SUT
	 */
	// equivalence class B & C
	void pushToEmptyQueue();
	// equivalence class B & C
	void pushToFilledQueue();
	//=========================
	// setup
	//=========================
	using Thread = std::thread;
	using Policy = iox::GatedMonitoringPolicy<iox::EmptyMonitoringPolicy>;
	enum {
		AfterLoadPosition = SUT::AfterLoadPosition
		, AfterLoadValue = SUT::AfterLoadValue
		, BeforeUpdatePosition = SUT::BeforeUpdatePosition
	};

	enum { expectedIsEmpty /*pop returns false*/, expectedIsSkipped/*push returns true*/};

	//=========================
	// register tests
	//=========================
	using this_type = IndexQueueTestMultithreaded<SUTType, Parameter>;
	template<class DerivedTest = this_type>
	static cute::suite make_suite(){
		cute::suite s { };
		s.push_back(CUTE_SMEMFUN(DerivedTest, pushToEmptyQueue));
		s.push_back(CUTE_SMEMFUN(DerivedTest, pushToFilledQueue));
		return s;
	}
};
//---------------------------------------------------------------------
// SUT interrupted AfterLoadPosition test runs to completion: equivalence class B
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestMultithreaded<SUTType, Params>::pushToEmptyQueue(){
	using namespace std; //numerical literals
	using UniqueIdx = typename SUT::UniqueIndexType;
	using NativeType = typename SUT::NativeType;

	SUT source(SUT::ConstructFull::Policy);
	SUT sut;
	Policy SUTpolicy;
	Policy testPolicy;
	constexpr auto SUTcheckpoint = Params::SUTcheckpoint;
	constexpr auto testCheckpoint = Params::testCheckpoint;

	UniqueIdx idxSUTToPush;
	constexpr NativeType expectedSUT{0};
	bool succeed = source.pop(idxSUTToPush);

	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedSUT, idxSUTToPush.getIndex());

	UniqueIdx idxTestToPush;
	NativeType expectedTest{1};
	succeed = source.pop(idxTestToPush);

	ASSERTM("no more unique indices available, Capacity must be greater 1", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

//	sut.print();

	// sut will be interrupted
	auto SUTTask = [&sut, &idxSUTToPush, &SUTpolicy](){
		sut.push(std::move(idxSUTToPush), SUTpolicy);
	};
	auto testTask = [&sut, &idxTestToPush, &testPolicy](){
		sut.push(std::move(idxTestToPush), testPolicy);
	};
	{
		// interrupt sut
		SUTpolicy.lock(SUTcheckpoint);
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(SUTcheckpoint);

		std::cout<< std::endl << "after SUTcheckpoint" << std::endl;
		sut.print();

		// interrupt test
		testPolicy.lock(testCheckpoint); // 0 == no checkpoint
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		std::cout<< std::endl << "after testCheckpoint" << std::endl;
		sut.print();

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		std::cout<< std::endl << "after thread.join" << std::endl;
		sut.print();
	}
	//todo sometimes this test fails !!! reason: memory_order_relaxed in IndexQueue ???
	succeed = sut.pop(idxTestToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

	succeed = sut.pop(idxSUTToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("second pop", expectedSUT, idxSUTToPush.getIndex());
}
//---------------------------------------------------------------------
// SUT interrupted at SUTcheckpoint test runs to testCheckpoint, SUT runs to completion
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestMultithreaded<SUTType, Params>::pushToFilledQueue(){
	using namespace std; //numerical literals

	SUT source(SUT::ConstructFull::Policy);
	SUT sut;
	Policy SUTpolicy;
	Policy testPolicy;
	constexpr auto SUTcheckpoint = Params::SUTcheckpoint;
	constexpr auto testCheckpoint = Params::testCheckpoint;

	UniqueIdx idxSUTToPush;
	constexpr NativeType expectedSUT{0};
	bool succeed = source.pop(idxSUTToPush);

	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedSUT, idxSUTToPush.getIndex());

	UniqueIdx idxTestToPush;
	succeed = source.pop(idxTestToPush);
	constexpr NativeType expectedTest{1};
	ASSERTM("no more unique indices available, Capacity must be greater 1", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

	// sut will be interrupted at SUTcheckpoint
	auto SUTTask = [&sut, &idxSUTToPush, &SUTpolicy](){
		sut.push(std::move(idxSUTToPush), SUTpolicy);
	};
	// test will be interrupted or runs to end depending on testCheckpoint
	auto testTask = [&sut, &source, &idxTestToPush, &testPolicy](){
		UniqueIdx dummy;
		while(source.pop(dummy)){
			sut.push(std::move(dummy));
		}
		sut.push(std::move(idxTestToPush), testPolicy);
	};
	{
		// interrupt sut
		SUTpolicy.lock(SUTcheckpoint);
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(SUTcheckpoint);

		std::cout<< std::endl << "after SUTcheckpoint" << std::endl;
		sut.print();

		// interrupt test only after publishValueAt
		testPolicy.lock(testCheckpoint); // 0 == no checkpoint
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		std::cout<< std::endl << "after testCheckpoint" << std::endl;
		sut.print();

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed if testCheckpoint was set
		testPolicy.unlock();
		testThread.join();

		std::cout << std::endl << "after thread.join" << std::endl;
		sut.print();
	}
	// drain sut
	UniqueIdx dummy;
	for(NativeType i=0; i<CAPACITY-2; ++i){
		sut.pop(dummy);
	}
	std::cout<< std::endl << "after drain sut" << std::endl;
	sut.print();

	succeed = sut.pop(idxTestToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

	succeed = sut.pop(idxSUTToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("second pop", expectedSUT, idxSUTToPush.getIndex());
}
