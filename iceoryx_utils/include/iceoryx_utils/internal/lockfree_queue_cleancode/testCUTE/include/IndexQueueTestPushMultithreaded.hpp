/*
 * IndexQueueTests.h
 *
 *  Created on: 7.5.2018
 *      Author: Gerd
 */

#pragma once


#include <iox/gated_monitoring_policy.hpp>

#include "cute.h"

#include <cstddef>
#include <thread>
#include <chrono>
#include <limits>
#include <iostream>

template<class SUTType, class Parameter>
class IndexQueueTestPushMultithreaded{
public:
	using SUT  = SUTType;
	using UniqueIdx = typename SUT::value_type;
	using NativeType = typename SUT::NativeType;
	static constexpr auto CAPACITY = SUT::CAPACITY;

	static constexpr auto sutCheckpoint = Parameter::sutCheckpoint;
	static constexpr auto testCheckpoint = Parameter::testCheckpoint;

	static void print(SUT const& sut, const char* message){
		return; // switch on/off output
		std::cout << std::endl << message << std::endl;
		sut.print();
	}

	/** tests should be made with
	 * different CAPACITIYs of the SUT
	 */
	// equivalence class B & C
	void pushToEmptyQueue();
	void pushToFilledQueue();
	//=========================
	// setup
	//=========================
	using Thread = std::thread;
	using Policy = iox::GatedMonitoringPolicy<iox::EmptyMonitoringPolicy>;
	//=========================
	// register tests
	//=========================
	using this_type = IndexQueueTestPushMultithreaded<SUTType, Parameter>;
	template<class DerivedTest = this_type>
	static cute::suite make_suite(){
		cute::suite s { };
		s.push_back(CUTE_SMEMFUN(DerivedTest, pushToEmptyQueue));
		s.push_back(CUTE_SMEMFUN(DerivedTest, pushToFilledQueue));
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
void IndexQueueTestPushMultithreaded<SUTType, Params>::pushToEmptyQueue(){
	SUT source(SUT::ConstructFull);
	SUT sut;
	Policy SUTpolicy;
	Policy testPolicy;

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
		SUTpolicy.lock(sutCheckpoint); // AfterLoadPosition or AfterLoadCell
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

		// interrupt test
		testPolicy.lock(testCheckpoint);  // BeforeUpdatePosition or EndOfMethod
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "sut after testCheckpoint");

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
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
void IndexQueueTestPushMultithreaded<SUTType, Params>::pushToFilledQueue(){
	SUT source(SUT::ConstructFull);
	SUT sut;
	Policy SUTpolicy;
	Policy testPolicy;

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
		SUTpolicy.lock(sutCheckpoint); // AfterLoadPosition or AfterLoadCell
		Thread SUTthread(SUTTask);
		SUTpolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

		// interrupt test
		testPolicy.lock(testCheckpoint); // BeforeUpdatePosition or EndOfMethod
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "source after testCheckpoint");

		//release sut to proceed
		SUTpolicy.unlock();
		SUTthread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
	}
	// drain sut
	UniqueIdx dummy;
	for(NativeType i=0; i<CAPACITY-2; ++i){
		sut.pop(dummy);
		source.push(std::move(dummy));
	}
	print(sut, "sut after drain sut");
	print(source, "source after drain sut");

	succeed = sut.pop(idxTestToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("first pop", expectedTest, idxTestToPush.getIndex());

	succeed = sut.pop(idxSUTToPush);
	ASSERTM("no more unique indices available", succeed);
	ASSERT_EQUALM("second pop", expectedSUT, idxSUTToPush.getIndex());
}
