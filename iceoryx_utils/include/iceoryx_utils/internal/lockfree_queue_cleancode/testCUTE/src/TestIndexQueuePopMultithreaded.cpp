/*
 * TestIndexQueuePopMultithreaded.cpp
 *
 *  Created on: 14.04.2020
 *      Author: Gerd
 */



#include "../include/TestParameter.hpp"
#include "../include/IndexQueueTestPopMultithreaded.hpp"

//tests
#include <iox/index_queue.hpp>
//#include <iox/mk/index_queue_mk_old.hpp>
//#include <iox/mk/index_queue.hpp>

#include "cute.h"
#include "ide_listener.h"
#include "xml_listener.h"
#include "cute_runner.h"

// tests for equivalence classes B and C
// IQTest IndexQueueTest Position/Value Class B/C
template<class SUT>
using IQTestPosition =
		IndexQueueTestPopMultithreaded<SUT,
		TestParameter<SUT::AfterLoadPosition, SUT::EndOfMethod>>;
template<class SUT>
using IQTestValue =
		IndexQueueTestPopMultithreaded<SUT,
		TestParameter<SUT::AfterLoadValue, SUT::EndOfMethod>>;
//==============================================
template<std::size_t MAX>
using Queue = iox::IndexQueue<MAX>; //switch between different implementations
//using Queue = iox::mk::IndexQueue<MAX>;

//==============================================
void testIndexQueuePopMultiThreaded(int argc, char const *argv[]) {
	cute::xml_file_opener xmlfile(argc, argv);
	cute::xml_listener<cute::ide_listener<>> listener(xmlfile.out);

	//============================================
	// IndexQueue Class B tests:
	// SUT thread runs to checkpoint,
	// test thread runs to completion checkPoint(EndOfMethod)
	// SUT thread runs to completion
	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestPosition<Queue<1>>::make_suite(), "pop CheckPoint AfterLoadPosition");
//	cute::makeRunner(listener,argc,argv)(IQTestPosition<Queue<2>>::make_suite(), "pop CheckPoint AfterLoadPosition");
	cute::makeRunner(listener,argc,argv)(IQTestPosition<Queue<3>>::make_suite(), "pop CheckPoint AfterLoadPosition");
	cute::makeRunner(listener,argc,argv)(IQTestPosition<Queue<5>>::make_suite(), "pop CheckPoint AfterLoadPosition");
	cute::makeRunner(listener,argc,argv)(IQTestPosition<Queue<10>>::make_suite(), "pop CheckPoint AfterLoadPosition");
//	//============================================
//	cute::makeRunner(listener,argc,argv)(IQTestValue<Queue<1>>::make_suite(), "pop CheckPoint AfterLoadValue");
//	cute::makeRunner(listener,argc,argv)(IQTestValue<Queue<2>>::make_suite(), "pop CheckPoint AfterLoadValue");
//	cute::makeRunner(listener,argc,argv)(IQTestValue<Queue<3>>::make_suite(), "pop CheckPoint AfterLoadValue");
//	cute::makeRunner(listener,argc,argv)(IQTestValue<Queue<5>>::make_suite(), "pop CheckPoint AfterLoadValue");
//	cute::makeRunner(listener,argc,argv)(IQTestValue<Queue<10>>::make_suite(), "pop CheckPoint AfterLoadValue");
}


