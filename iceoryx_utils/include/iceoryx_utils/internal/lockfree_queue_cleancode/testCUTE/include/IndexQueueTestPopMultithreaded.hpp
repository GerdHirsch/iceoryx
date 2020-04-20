/*
 * IndexQueueTestPopMultithreaded.hpp
 *
 *  Created on: 14.04.2020
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
class IndexQueueTestPopMultithreaded{
public:
	using SUT  = SUTType;
	using UniqueIdx = typename SUT::UniqueIndexType;
	using NativeType = typename SUT::NativeType;
	static constexpr auto CAPACITY = SUT::CAPACITY;

	static constexpr auto sutCheckpoint = Parameter::sutCheckpoint;
	static constexpr auto testCheckpoint = Parameter::testCheckpoint;

	static void print(SUT const& sut, const char* message){
		using namespace std;
		return; // switch on/off output
		std::cout << "      demo: " << this_thread::get_id() << " "<< message << std::endl;
		return;
		sut.print();
	}
	/** tests should be made with
	 * different CAPACITIYs of the SUT
	 */
	void popFromEmptyWhilePush();
	void popFromFilledWhilePop();
	void popFromEmptyAfterFull_No_1();
	void popFromEmptyAfterFull_No_2();
	void popIfFullFromFilledBeforePop();
	void popIfFullFromFilledAfterPop();
	//=========================
	// setup
	//=========================
	using Thread = std::thread;
	using Policy = iox::GatedMonitoringPolicy<iox::EmptyMonitoringPolicy>;
	//=========================
	// register tests
	//=========================
	using this_type = IndexQueueTestPopMultithreaded<SUTType, Parameter>;
	template<class DerivedTest = this_type>
	static cute::suite make_suite(){
		cute::suite s { };
		s.push_back(CUTE_SMEMFUN(DerivedTest, popFromEmptyWhilePush));
		s.push_back(CUTE_SMEMFUN(DerivedTest, popFromFilledWhilePop));
		s.push_back(CUTE_SMEMFUN(DerivedTest, popFromEmptyAfterFull_No_1));
		s.push_back(CUTE_SMEMFUN(DerivedTest, popFromEmptyAfterFull_No_2));
		s.push_back(CUTE_SMEMFUN(DerivedTest, popIfFullFromFilledBeforePop));
		s.push_back(CUTE_SMEMFUN(DerivedTest, popIfFullFromFilledAfterPop));
		return s;
	}
};

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestPopMultithreaded<SUTType, Params>::popFromEmptyWhilePush(){
	SUT source(SUT::ConstructFull);
//	SUT source(SUT::ConstructFull);
	SUT sut;
	Policy sutPolicy;
	Policy testPolicy;

//==========================
// begin region
// this test is a first try and could be split into two tests,
//==========================
	//if(sutCheckpoint == SUT::AfterLoadPosition)
	// pop will succeed, there should be an index to pop
	// else (sutCheckpoint == SUT::AfterLoadValue)
	// pop will fail, queue seems to be empty

	//if(sutCheckpoint == SUT::AfterLoadPosition)
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
//==========================
// end region
// this test is a first try and could be split into two tests,
//==========================

	// sut will be interrupted
	auto SUTTask = [&](){
		popReturnValue = sut.pop(idxSUTToPop, sutPolicy);
	};
	auto testTask = [&](){
		sut.push(std::move(idxTestToPush), testPolicy);
	};
	{	// no assertions inside this block

		// interrupt sut
		sutPolicy.lock(sutCheckpoint);
		Thread SUTthread(SUTTask);
		sutPolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

		// interrupt test
		testPolicy.lock(testCheckpoint);
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "sut after testCheckpoint");

		//release sut to proceed
		sutPolicy.unlock();
		SUTthread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
	}
	ASSERT_EQUALM("pop not succeed", expectedPopReturnValue, popReturnValue);
	ASSERT_EQUALM("popped value", expectedPopValueSUT, idxSUTToPop.getIndex());
}
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestPopMultithreaded<SUTType, Params>::popFromFilledWhilePop(){
	SUT sut(SUT::ConstructFull);
	SUT source;
	Policy sutPolicy;
	Policy testPolicy;

	// ::max() is needed cause there is no invalid state of UniqueIndexType
	UniqueIdx idxSutToPop{std::numeric_limits<NativeType>::max()};
	constexpr NativeType expectedSUTPopValue{1};
	bool popSUTReturnValue{false};

	// sutTask will be interrupted
	auto sutTask = [&](){
		popSUTReturnValue = sut.pop(idxSutToPop, sutPolicy);
	};

	UniqueIdx idxTestToPop{std::numeric_limits<NativeType>::max()};
	constexpr NativeType expectedTestPopValue{0};
	bool popTestReturnValue{false};

	auto testTask = [&](){
		UniqueIdx dummy;
		while(sut.pop(dummy)){
			source.push(std::move(dummy));
		}
		while(source.pop(dummy)){
			sut.push(std::move(dummy));
		}
		popTestReturnValue = sut.pop(idxTestToPop, testPolicy);
	};
	constexpr bool expectedPopReturnValue{true};

	{	// no assertions inside this block

		// interrupt sut
		sutPolicy.lock(sutCheckpoint);
		Thread sutThread(sutTask);
		sutPolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

		testPolicy.lock(testCheckpoint);
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "sut after test pop");

		//release sut to proceed
		sutPolicy.unlock();
		sutThread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
	}
	ASSERT_EQUALM("sut pop not succeed", expectedPopReturnValue, popSUTReturnValue);
	ASSERT_EQUALM("sut popped value", expectedSUTPopValue, idxSutToPop.getIndex());
	ASSERT_EQUALM("test pop not succeed", expectedPopReturnValue, popTestReturnValue);
	ASSERT_EQUALM("test value", expectedTestPopValue, idxTestToPop.getIndex());
}
template<class SUTType, class Params>
inline
void IndexQueueTestPopMultithreaded<SUTType, Params>::popFromEmptyAfterFull_No_1(){
	// sut will be drained from this function
	// in No_2, sut will be drained from testThread
	SUT sut(SUT::ConstructFull);
	Policy sutPolicy;
	Policy testPolicy;

	// ::max() is needed cause there is no invalid state of UniqueIndexType
	constexpr NativeType expectedSUTPopValue{std::numeric_limits<NativeType>::max()};
	UniqueIdx idxSutToPop{expectedSUTPopValue};
	bool popSUTReturnValue{true};

	// sutTask will be interrupted
	auto sutTask = [&](){
		popSUTReturnValue = sut.pop(idxSutToPop, sutPolicy);
	};

	constexpr NativeType expectedTestPopValue{std::numeric_limits<NativeType>::max()};
	UniqueIdx idxTestToPop{expectedTestPopValue};
	bool popTestReturnValue{true};

	auto testTask = [&](){
		popTestReturnValue = sut.pop(idxTestToPop, testPolicy);
	};
	constexpr bool expectedPopReturnValue{false};

	{	// no assertions inside this block

		// interrupt sut
		sutPolicy.lock(sutCheckpoint);
		Thread sutThread(sutTask);
		sutPolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

		// drain sut
		UniqueIdx dummy;
		while(sut.pop(dummy)){
//			std::cout << "dummy: " << dummy.getIndex() << std::endl;
		}
		print(sut, "sut after drain");

//		popTestReturnValue = sut.pop(idxTestToPop);
		testPolicy.lock(testCheckpoint);
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "sut after test pop");

		//release sut to proceed
		sutPolicy.unlock();
		sutThread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
	}
	ASSERT_EQUALM("sut pop not succeed", expectedPopReturnValue, popSUTReturnValue);
	ASSERT_EQUALM("sut popped value", expectedSUTPopValue, idxSutToPop.getIndex());
	ASSERT_EQUALM("test pop not succeed", expectedPopReturnValue, popTestReturnValue);
	ASSERT_EQUALM("test value", expectedTestPopValue, idxTestToPop.getIndex());
}
template<class SUTType, class Params>
inline
void IndexQueueTestPopMultithreaded<SUTType, Params>::popFromEmptyAfterFull_No_2(){
	// sut will be drained from testThread
	// in No_1 sut will be drained from this function
	SUT sut(SUT::ConstructFull);
	Policy sutPolicy;
	Policy testPolicy;

	// ::max() is needed cause there is no invalid state of UniqueIndexType
	constexpr NativeType expectedSUTPopValue{std::numeric_limits<NativeType>::max()};
	UniqueIdx idxSutToPop{expectedSUTPopValue};
	bool popSUTReturnValue{true};

	// sutTask will be interrupted
	auto sutTask = [&](){
		popSUTReturnValue = sut.pop(idxSutToPop, sutPolicy);
	};

	constexpr NativeType expectedTestPopValue{std::numeric_limits<NativeType>::max()};
	UniqueIdx idxTestToPop{expectedTestPopValue};
	bool popTestReturnValue{true};

	auto testTask = [&](){
		UniqueIdx dummy;
		while(sut.pop(dummy)){ }
		popTestReturnValue = sut.pop(idxTestToPop, testPolicy);
	};
	constexpr bool expectedPopReturnValue{false};

	{	// no assertions inside this block

		// interrupt sut
		sutPolicy.lock(sutCheckpoint);
		Thread sutThread(sutTask);
		sutPolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

//		popTestReturnValue = sut.pop(idxTestToPop);
		testPolicy.lock(testCheckpoint);
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "sut after test pop");

		//release sut to proceed
		sutPolicy.unlock();
		sutThread.join();

		//release test to proceed
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
	}
	ASSERT_EQUALM("sut pop not succeed", expectedPopReturnValue, popSUTReturnValue);
	ASSERT_EQUALM("sut popped value", expectedSUTPopValue, idxSutToPop.getIndex());
	ASSERT_EQUALM("test pop not succeed", expectedPopReturnValue, popTestReturnValue);
	ASSERT_EQUALM("test value", expectedTestPopValue, idxTestToPop.getIndex());
}

//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestPopMultithreaded<SUTType, Params>::popIfFullFromFilledBeforePop(){
	SUT sut(SUT::ConstructFull);
	Policy sutPolicy;

	// ::max() is needed cause there is no invalid state of UniqueIndexType
	UniqueIdx idxSutToPop{std::numeric_limits<NativeType>::max()};
	constexpr NativeType expectedSUTPopValue{1};
	bool popSUTReturnValue{false};

	// sutTask will be interrupted
	auto sutTask = [&](){
		popSUTReturnValue = sut.popIfFull(idxSutToPop, sutPolicy);
	};

	constexpr bool expectedPopReturnValue{true};

	{	// no assertions inside this block
		UniqueIdx idx;
		sut.pop(idx);
		sut.push(std::move(idx));
		// interrupt sut
		sutPolicy.lock(sutCheckpoint);
		Thread sutThread(sutTask);
		sutPolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");

		//release sut to proceed
		sutPolicy.unlock();
		sutThread.join();

		print(sut, "sut after thread.join");
	}
	ASSERT_EQUALM("sut pop not succeed", expectedPopReturnValue, popSUTReturnValue);
	ASSERT_EQUALM("sut popped value", expectedSUTPopValue, idxSutToPop.getIndex());
}
//---------------------------------------------------------------------
template<class SUTType, class Params>
inline
void IndexQueueTestPopMultithreaded<SUTType, Params>::popIfFullFromFilledAfterPop(){
	SUT sut(SUT::ConstructFull);
	Policy sutPolicy;
	Policy testPolicy;

	//------------------------------------
	// ::max() is needed cause there is no invalid state of UniqueIndexType
	constexpr NativeType expectedSUTPopValue{std::numeric_limits<NativeType>::max()};
	UniqueIdx idxSutToPop{expectedSUTPopValue}; // should not be changed
	constexpr bool expectedSUTPopReturnValue{false};
	bool popSUTReturnValue{true};
	// sutTask will be interrupted
	auto sutTask = [&](){
		popSUTReturnValue = sut.popIfFull(idxSutToPop, sutPolicy);
	};
	//------------------------------------
	constexpr NativeType expectedTestPopValue{1};
	UniqueIdx idxTestToPop{std::numeric_limits<NativeType>::max()};
	constexpr bool expectedTestPopReturnValue{true};
	bool popTestReturnValue{false};
	// test runs to EndOfMethod
	auto testTask = [&](){
		popTestReturnValue = sut.pop(idxTestToPop, testPolicy);
	};
	//------------------------------------

	{	// no assertions inside this block
		UniqueIdx idx;
		sut.pop(idx);
		sut.push(std::move(idx));
		// interrupt sut
		sutPolicy.lock(sutCheckpoint);
		Thread sutThread(sutTask);
		sutPolicy.waitForArrival(sutCheckpoint);

		print(sut, "sut after sutCheckpoint");


		testPolicy.lock(testCheckpoint);
		Thread testThread(testTask);
		testPolicy.waitForArrival(testCheckpoint);

		print(sut, "sut after testCheckpoint");

		//release sut to proceed
		sutPolicy.unlock();
		sutThread.join();
		testPolicy.unlock();
		testThread.join();

		print(sut, "sut after thread.join");
	}
	ASSERT_EQUALM("sut pop succeed", expectedSUTPopReturnValue, popSUTReturnValue);
	ASSERT_EQUALM("sut popped value", expectedSUTPopValue, idxSutToPop.getIndex());
	ASSERT_EQUALM("sut pop not succeed", expectedTestPopReturnValue, popTestReturnValue);
	ASSERT_EQUALM("sut popped value", expectedTestPopValue, idxTestToPop.getIndex());
}
