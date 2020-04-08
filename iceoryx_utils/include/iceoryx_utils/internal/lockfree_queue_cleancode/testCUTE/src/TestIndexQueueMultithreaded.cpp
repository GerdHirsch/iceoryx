/*
 * TestIndexQueueMultithreaded.cpp
 *
 *  Created on: 06.04.2020
 *      Author: Gerd
 */
//tests
#include "../include/IndexQueueTestMultithreaded.h"
//SUTs
#include <iox/index_queue.hpp>
//CUTE
#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

// tests & TestParameter
template<class SUT>
using ParameterPositionClassB = TestParameter<SUT::AfterLoadPosition, 0>;
template<class SUT>
using ParameterValueClassB = TestParameter<SUT::AfterLoadValue, 0>;
template<class SUT>
using ParameterPositionClassC = TestParameter<SUT::AfterLoadPosition, SUT::BeforeUpdatePosition>;
template<class SUT>
using ParameterValueClassC = TestParameter<SUT::AfterLoadValue, SUT::BeforeUpdatePosition>;

// tests for equivalence classes B and C
// IQTest IndexQueueTest Position/Value Class B/C
template<class SUT> // SRBM = SkippyRingBufferMultithreaded
using IQTestPositionClassB = IndexQueueTestMultithreaded<SUT, ParameterPositionClassB<SUT>>;
template<class SUT> // SRBM = SkippyRingBufferMultithreaded
using IQTestValueClassB = IndexQueueTestMultithreaded<SUT, ParameterValueClassB<SUT>>;
template<class SUT> // SRBM = SkippyRingBufferMultithreaded
using IQTestPositionClassC = IndexQueueTestMultithreaded<SUT, ParameterPositionClassC<SUT>>;
template<class SUT> // SRBM = SkippyRingBufferMultithreaded
using IQTestValueClassC = IndexQueueTestMultithreaded<SUT, ParameterValueClassC<SUT>>;
//==============================================
template<std::size_t MAX>
using Queue = iox::IndexQueue<MAX>;

//==============================================
void testIndexQueueMultiThreaded(int argc, char const *argv[]) {
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> listener(xmlfile.out);

	//============================================
	// IndexQueue Class B tests:
	// SUT thread runs to checkpoint,
	// test thread runs to completion
	// SUT thread runs to completion
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<1>>::make_suite(), "class B IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<2>>::make_suite(), "class B Position IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<3>>::make_suite(), "class B Position IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<5>>::make_suite(), "class B Position IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<10>>::make_suite(), "class B Position IndexQueue");
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<1>>::make_suite(), "class B IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<2>>::make_suite(), "class B Value IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<3>>::make_suite(), "class B Value IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<5>>::make_suite(), "class B Value IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<10>>::make_suite(), "class B Value IndexQueue");
	//============================================
	// IndexQueue Class C tests:
	// SUT thread runs to checkpoint,
	// test thread runs to checkpoint,
	// SUT thread runs to completion
	// test thread runs to completion
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<1>>::make_suite(), "class C IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<2>>::make_suite(), "class C Position IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<3>>::make_suite(), "class C Position IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<5>>::make_suite(), "class C Position IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<10>>::make_suite(), "class C Position IndexQueue");
//	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<1>>::make_suite(), "class C IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<2>>::make_suite(), "class C Value IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<3>>::make_suite(), "class C Value IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<5>>::make_suite(), "class C Value IndexQueue");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<10>>::make_suite(), "class C Value IndexQueue");
}



