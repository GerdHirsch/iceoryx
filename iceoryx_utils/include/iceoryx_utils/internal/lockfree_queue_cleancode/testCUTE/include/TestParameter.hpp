/*
 * TestParameter.hpp
 *
 *  Created on: 14.04.2020
 *      Author: Gerd
 */

#pragma once
#include <cstdint>

template<std::uint64_t sutCheckpoint_, uint64_t testCheckpoint_>
struct TestParameter{
	static constexpr auto sutCheckpoint = sutCheckpoint_;
	static constexpr auto testCheckpoint = testCheckpoint_;
};

