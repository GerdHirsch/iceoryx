/*
 * TestIndexQueueMultithreaded.cpp
 *
 *  Created on: 06.04.2020
 *      Author: Gerd
 */
#include "../include/TestParameter.hpp"
#include "../include/IndexQueueTestPushMultithreaded.hpp"
//tests
#include <iox/index_queue.hpp>
#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

// tests for equivalence classes B and C
// IQTest IndexQueueTest Position/Value Class B/C
template<class SUT>
using IQTestPositionClassB =
		IndexQueueTestPushMultithreaded<SUT,
		TestParameter<SUT::AfterLoadPosition, SUT::EndOfMethod>>;
template<class SUT>
using IQTestValueClassB =
		IndexQueueTestPushMultithreaded<SUT,
		TestParameter<SUT::AfterLoadValue, SUT::EndOfMethod>>;
template<class SUT>
using IQTestPositionClassC =
		IndexQueueTestPushMultithreaded<SUT,
		TestParameter<SUT::AfterLoadPosition, SUT::BeforeUpdatePosition>>;
template<class SUT>
using IQTestValueClassC =
		IndexQueueTestPushMultithreaded<SUT,
		TestParameter<SUT::AfterLoadValue, SUT::BeforeUpdatePosition>>;
//==============================================
template<std::size_t MAX>
using Queue = iox::IndexQueue<MAX>; //switch between different implementations
//using Queue = iox::mk::IndexQueue<MAX>;

//==============================================
void testIndexQueuePushMultiThreaded(int argc, char const *argv[]) {
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> listener(xmlfile.out);

	//============================================
	// IndexQueue Class B tests:
	// SUT thread runs to checkpoint,
	// test thread runs to completion checkPoint(EndOfMethod)
	// SUT thread runs to completion
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<1>>::make_suite(), "push CheckPoint AfterLoadPosition EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<2>>::make_suite(), "push CheckPoint AfterLoadPosition EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<3>>::make_suite(), "push CheckPoint AfterLoadPosition EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<5>>::make_suite(), "push CheckPoint AfterLoadPosition EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassB<Queue<10>>::make_suite(), "push CheckPoint AfterLoadPosition EndOfMethod");
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<1>>::make_suite(), "push CheckPoint AfterLoadValue EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<2>>::make_suite(), "push CheckPoint AfterLoadValue EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<3>>::make_suite(), "push CheckPoint AfterLoadValue EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<5>>::make_suite(), "push CheckPoint AfterLoadValue EndOfMethod");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassB<Queue<10>>::make_suite(), "push CheckPoint AfterLoadValue EndOfMethod");
	//============================================
	// IndexQueue Class C tests:
	// SUT thread runs to checkpoint,
	// test thread runs to checkpoint(BeforeUpdatePosition),
	// SUT thread runs to completion
	// test thread runs to completion
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<1>>::make_suite(), "push CheckPoint AfterLoadPosition BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<2>>::make_suite(), "push CheckPoint AfterLoadPosition BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<3>>::make_suite(), "push CheckPoint AfterLoadPosition BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<5>>::make_suite(), "push CheckPoint AfterLoadPosition BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestPositionClassC<Queue<10>>::make_suite(), "push CheckPoint AfterLoadPosition BeforeUpdatePosition");
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<1>>::make_suite(), "push CheckPoint AfterLoadValue BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<2>>::make_suite(), "push CheckPoint AfterLoadValue BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<3>>::make_suite(), "push CheckPoint AfterLoadValue BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<5>>::make_suite(), "push CheckPoint AfterLoadValue BeforeUpdatePosition");
	cute::makeRunner(listener,argc,argv)(IQTestValueClassC<Queue<10>>::make_suite(), "push CheckPoint AfterLoadValue BeforeUpdatePosition");
}



