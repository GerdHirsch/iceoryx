// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <stdint.h>

namespace iox
{

// a word on the target architecture, must be able to be used in a CAS operation
using word_t = uint64_t;
//using word_t = uint16_t;

// a byte on the target architecture, to be used in untyped buffers
// using byte_t = unsigned char;
using byte_t = uint8_t;

/// @todo: try to make the operations more efficient (i.e. no modulo)
/// e.g. store the internal state differently, use bitoperations in updates/retrievals

/// @brief index structure that can contain logical values 0, ..., CycleLength-1
/// but also stores an internal ABA counter to be used in compare_exchange
template <uint64_t CycleLength_>
class CyclicIndex
{
  public:
	static constexpr uint64_t CycleLength = CycleLength_;
	using this_type = CyclicIndex<CycleLength>;
    /// @brief index structure that can contain logical values 0, ..., CycleLength-1
    explicit CyclicIndex(word_t value = 0) noexcept
        : m_value(value)
    {
    }

    /// @brief create CyclicIndex using the actual index and a specified cycle
    CyclicIndex(word_t index, word_t cycle) noexcept
        : m_value(cycle * CycleLength + index)
    {
    }

    CyclicIndex(const CyclicIndex&) = default;
    CyclicIndex& operator=(const CyclicIndex&) = default;

    /// @brief get the logical index
    /// @return logical index
    word_t getIndex() const
    {
        return m_value % CycleLength;
    }

    /// @brief get the cycle stored internally
    /// @return cycle
    word_t getCycle() const
    {
        return m_value / CycleLength;
    }
    bool isBehind(this_type const& rhs){
    	return getCycle() + 1 == rhs.getCycle();
    }
    /// @brief increment the internal index by one
    /// @return stored internal index value after increment
    word_t operator++()
    {
        return ++m_value;
    }

    /// @brief return the CyclicIndex incremented by value
    /// @return current CyclicIndex incremented by value
    CyclicIndex operator+(word_t value)
    {
        return CyclicIndex(m_value + value);
    }

  private:
    word_t m_value{0};
};
} // namespace iox
