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


namespace iox
{
#include <limits>
#include <stdint.h>

// a word on the target architecture, must be able to be used in a CAS operation
using word_t = uint64_t;

// a byte on the target architecture, to be used in untyped buffers
// using byte_t = unsigned char;
using byte_t = uint8_t;


/// @brief index structure that can contain logical values 0, ..., CycleLength-1
/// but also stores an internal ABA counter to be used in compare_exchange
template <uint64_t CycleLength, typename value_t = uint64_t>
class CyclicIndex
{
  public:
    static constexpr value_t MAX_INDEX = CycleLength - 1;
    static constexpr value_t MAX_VALUE = std::numeric_limits<value_t>::max();
    static constexpr value_t MAX_CYCLE = MAX_VALUE / CycleLength;

    static constexpr value_t INDEX_AT_MAX_VALUE = MAX_VALUE % CycleLength;
    static constexpr value_t OVERFLOW_START_INDEX = (INDEX_AT_MAX_VALUE + 1) % CycleLength;

    static_assert(CycleLength < MAX_VALUE / 2); // need at least one bit for the cycle

    explicit CyclicIndex(value_t value = 0) noexcept
        : m_value(value)
    {
    }

    CyclicIndex(value_t index, value_t cycle) noexcept
        : m_value(index + cycle * CycleLength)
    {
    }

    CyclicIndex(const CyclicIndex&) = default;
    CyclicIndex& operator=(const CyclicIndex&) = default;

    /// @note an implementation with bitmasks instead of the much more concise integer division
    /// and remainder arithmetics seems to provide no performance benefit (on the contrary)
    /// this will of course depend on target architecture and the implementation itself
    value_t getIndex() const noexcept
    {
        return m_value % CycleLength;
    }

    value_t getCycle() const noexcept
    {
        return m_value / CycleLength;
    }

    CyclicIndex operator+(value_t value) const
    {
        // if we were at this value, we would have no overflow, i.e. when m_value is larger there is an overflow
        auto delta = MAX_VALUE - value;
        if (delta < m_value)
        {
            // overflow, rare case (overflow by m_value - delta)
            // we need to compute the correct index and cycle we are in after overflow
            // note that we could also limit the max value to always start at OVERFLOW_START_INDEX = 0,
            // but this has other drawbacks (and the overflow will not occur often if at all with 64 bit)
            delta = m_value - delta - 1;
            return CyclicIndex(OVERFLOW_START_INDEX + delta);
        }

        // no overflow, regular case
        return CyclicIndex(m_value + value);
    }

    CyclicIndex next()
    {
        if (m_value == MAX_VALUE)
        {
            return CyclicIndex(OVERFLOW_START_INDEX);
        }
        return CyclicIndex(m_value + 1);
    }

    void print()
    {
        std::cout << "value " << m_value << " index " << getIndex() << " cycle " << getCycle() << std::endl;
    }

  private:
    value_t m_value{0};
};
} // namespace iox