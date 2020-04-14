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
#include <limits>
#include <iostream>

template<uint64_t sutCheckpoint_, uint64_t testCheckpoint_>
struct TestParameter{
	static constexpr auto sutCheckpoint = sutCheckpoint_;
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
	void popFromEmptyWhilePush();
	//=========================
	// setup
	//=========================
	using Thread = std::thread;
	using Policy = iox::GatedMonitoringPolicy<iox::EmptyMonitoringPolicy>;
	//=========================
	// register tests
	//=========================
	using this_type = IndexQueueTestMultithreaded<SUTType, Parameter>;
	template<class DerivedTest = this_type>
	static cute::suite make_suite(){
		cute::suite s { };
		s.push_back(CUTE_SMEMFUN(DerivedTest, pushToEmptyQueue));
		s.push_back(CUTE_SMEMFUN(DerivedTest, pushToFilledQueue));
		s.push_back(CUTE_SMEMFUN(DerivedTest, popFromEmptyWhilePush));
		return s;
	}
};
//---------------------------------------------------------------------
// SUT will be interrupted AfterLoad Position/Value
// test will be interrupted BeforeUpdatePosition or
// runs to completion (EndOfMethod)
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
	constexpr auto sutCheckpoint = Params::sutCheckpoint;
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
	ASSERT_EQUALM("second pop", expectedTest, idxTestToPush.getIndex());

	// sut will be interrupted
	auto SUTTask = [&sut, &idxSUTToPush, &SUTpolicy](){
		sut.push(std::move(idxSUTToPush), SUTpolicy);
	};
	auto testTask = [&sut, &idxTestToPush, &testPolicy](){
		sut.push(std::move(idxTestToPush), testPolicy);
	};
	{	// no assertions inside this block

		// interrupt sut
		SUTpolicy.lock(sutCheckpoint);
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(sutCheckpoint);

		std::cout<< std::endl << "sut after sutCheckpoint" << std::endl;
		sut.print();

		// interrupt test
		testPolicy.lock(testCheckpoint); // 0 == no checkpoint
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		std::cout<< std::endl << "sut after testCheckpoint" << std::endl;
		sut.print();

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		std::cout<< std::endl << "sut after thread.join" << std::endl;
		sut.print();
	}
	succeed = sut.pop(idxTestToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

	succeed = sut.pop(idxSUTToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("second pop", expectedSUT, idxSUTToPush.getIndex());
}
//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestMultithreaded<SUTType, Params>::pushToFilledQueue(){
	using namespace std; //numerical literals

	SUT source(SUT::ConstructFull::Policy);
	SUT sut;
	Policy SUTpolicy;
	Policy testPolicy;
	constexpr auto sutCheckpoint = Params::sutCheckpoint;
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

	// sut will be interrupted at sutCheckpoint
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
	{	// no assertions inside this block

		// interrupt sut
		SUTpolicy.lock(sutCheckpoint);
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(sutCheckpoint);

		std::cout<< std::endl << "sut after sutCheckpoint" << std::endl;
		sut.print();

		// interrupt test only after publishValueAt
		testPolicy.lock(testCheckpoint); // 0 == no checkpoint
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		std::cout<< std::endl << "source after testCheckpoint" << std::endl;
		source.print();

		std::cout<< std::endl << "sut after testCheckpoint" << std::endl;
		sut.print();

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed if testCheckpoint was set
		testPolicy.unlock();
		testThread.join();

		std::cout << std::endl << "sut after thread.join" << std::endl;
		sut.print();
	}
	// drain sut
	UniqueIdx dummy;
	for(NativeType i=0; i<CAPACITY-2; ++i){
		sut.pop(dummy);
		source.push(std::move(dummy));
	}
	std::cout<< std::endl << "sut after drain sut" << std::endl;
	sut.print();
	std::cout << std::endl << "source after drain sut" << std::endl;
	source.print();

	succeed = sut.pop(idxTestToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

	succeed = sut.pop(idxSUTToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("second pop", expectedSUT, idxSUTToPush.getIndex());
}
//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestMultithreaded<SUTType, Params>::popFromEmptyWhilePush(){
	using namespace std; //numerical literals
	using UniqueIdx = typename SUT::UniqueIndexType;
	using NativeType = typename SUT::NativeType;

	SUT source(SUT::ConstructFull::Policy);
	SUT sut;
	Policy SUTpolicy;
	Policy testPolicy;

	constexpr auto sutCheckpoint = Params::sutCheckpoint;
	constexpr auto testCheckpoint = Params::testCheckpoint;

	//---------------------------
	// begin region
	// this test is a first try and must be split into two tests,
	// no cases in tests!
	//---------------------------
	//if(sutCheckpoint == SUT::AfterLoadPosition)
	// pop will succeed, there should be an index to pop
	// else (sutCheckpoint == SUT::AfterLoadValue)
	// pop will fail, queue seems to be empty

	// ::max() is needed cause there is no invalid state of UniqueIndexType
	UniqueIdx idxSUTToPop{std::numeric_limits<NativeType>::max()};
	NativeType expectedPopValueSUT{0};
	bool expectedPopReturnValue{true};
	bool popReturnValue{false};

	UniqueIdx idxTestToPush;
	bool succeed = source.pop(idxTestToPush);

	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("expected pop value", expectedPopValueSUT, idxTestToPush.getIndex());

	if(sutCheckpoint == SUT::AfterLoadValue){
		// pop will fail, queue seems to be empty
		expectedPopValueSUT = std::numeric_limits<NativeType>::max();
		expectedPopReturnValue = false;
		popReturnValue = true; // will be set to false
	}
	//---------------------------
	// end region
	// this test is a first try and must be split into two tests,
	// no cases in tests!
	//---------------------------

	// sut will be interrupted
	auto SUTTask = [&](){
		popReturnValue = sut.pop(idxSUTToPop, SUTpolicy);
	};
	auto testTask = [&](){
		sut.push(std::move(idxTestToPush), testPolicy);
	};
	{	// no assertions inside this block

		// interrupt sut
		SUTpolicy.lock(sutCheckpoint);
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(sutCheckpoint);

		std::cout<< std::endl << "sut after sutCheckpoint" << std::endl;
		sut.print();

		// interrupt test
		testPolicy.lock(testCheckpoint); // 0 == no checkpoint
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		std::cout<< std::endl << "sut after testCheckpoint" << std::endl;
		sut.print();

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		std::cout<< std::endl << "sut after thread.join" << std::endl;
		sut.print();
	}
	ASSERT_EQUALM("pop not succeed", expectedPopReturnValue, popReturnValue);
	ASSERT_EQUALM("popped value", expectedPopValueSUT, idxSUTToPop.getIndex());
}
