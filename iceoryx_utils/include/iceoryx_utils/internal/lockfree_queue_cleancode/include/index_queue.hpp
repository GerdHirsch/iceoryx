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

#include "cyclic_index.hpp"
#include "unique_index.hpp"
#include "empty_monitoring_policy.hpp"

#include <atomic>
#include <iostream>

namespace iox
{

/// @brief lockfree queue capable of storing indices 0,1,... Capacity-1
/// @SynchronizationPolicy threadsafe, waitfree, memory neutral,
/// all members are atomic<CyclicIndex>
/// all atomic operations (load, compare_exchange_strong) are relaxed
template <uint64_t Capacity_>
class IndexQueue
{
  public:

    using NativeType = uint64_t;
	static constexpr NativeType CAPACITY{Capacity_};

    using UniqueIndexType = UniqueIndex<NativeType, CAPACITY>;
//    using indexvalue_t = word_t;
    void print();
//    template<class MonitoringPolicy=EmptyMonitoringPolicy>
//    void demoMonitoringPolicy(MonitoringPolicy const& policy=MonitoringPolicy());

//    template<class MonitoringPolicy=EmptyMonitoringPolicy>
//    void demoMonitoringPolicy(MonitoringPolicy const& policy=MonitoringPolicy()){
//    	policy.print();
//    }
  private:
    using Index = CyclicIndex<CAPACITY>;

    Index loadNextReadPosition();
    Index loadNextWritePosition();
    Index loadValueAt(Index position);
    bool tryToPublishAt(Index writePosition, Index& oldValue, Index newValue);
    bool tryToAchieveOwnership(Index& readPosition); // head
    void updateNextWritePosition(Index& oldWritePosition);// tail
//    bool isUsed(NativeType valueCycle, NativeType tailCycle);
    // @todo: a compile time check whether atomic<Index> is actually lock free would be nice (is there a solution with
    // c++11?)
    // inner call is not constexpr so we cannot do compile time check here ...
    // static_assert(std::atomic<Index>{}.is_lock_free());

    std::atomic<Index> m_values[CAPACITY];
    std::atomic<Index> m_head; // read
    std::atomic<Index> m_tail; // write

  public:
    // checkPoints for test instrumentation
    enum {AfterLoadPosition=1, AfterLoadValue, BeforeUpdatePosition };
    // just to distingish between constructors at compile time and make the
    // construction policy more explicit
    enum class ConstructFull
    {
        Policy
    };

    enum class ConstructEmpty
    {
        Policy
    };

    /// @brief constructs an empty IndexQueue
    IndexQueue(ConstructEmpty policy = ConstructEmpty::Policy);

    /// @brief constructs IndexQueue filled with all indices 0,1,...capacity-1
    IndexQueue(ConstructFull);

    /// @brief get the capacity of the IndexQueue
    /// @return capacity of the IndexQueue
    /// threadsafe, lockfree
    constexpr NativeType capacity();

    /// @brief push index into the queue in FIFO order
    /// constraint: pushing more indices than capacity is not allowed
    /// constraint: only indices in the range [0, Capacity-1] are allowed
    /// threadsafe, lockfree
    template<class MonitoringPolicy=EmptyMonitoringPolicy>
    void push(UniqueIndexType uniqueIdx, MonitoringPolicy const& = MonitoringPolicy());

    /// @brief tries to remove index in FIFO order
    /// @return true iff removal was successful (i.e. queue was not empty)
    /// value is only valid if the function returns true
    /// threadsafe, lockfree
    bool pop(UniqueIndexType& uniqueIdx);


    /// @brief tries to remove index in FIFO order iff the queue is full
    /// @return true iff removal was successful (i.e. queue was full)
    /// value is only valid if the function returns true
    /// @SynchronizationPolicy threadsafe, waitfree, memory neutral,
    /// all members are atomic<CyclicIndex>
    bool popIfFull(UniqueIndexType& uniqueIdx);

    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    /// (but it was at some point during the call)
    bool empty();
};
} // namespace iox

#include "index_queue.inl"
