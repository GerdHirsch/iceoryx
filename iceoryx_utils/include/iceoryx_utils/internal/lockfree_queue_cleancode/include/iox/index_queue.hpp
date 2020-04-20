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
#include "GetMinSizedType.hpp"

#include <atomic>
#include <thread>
#include <iostream>

namespace iox
{

/// @brief lockfree queue capable of storing indices 0,1,... Capacity-1
/// @SynchronizationPolicy threadsafe, waitfree, memory neutral,
/// all members are atomic<CyclicIndex>
/// all atomic operations (load, compare_exchange_strong) are relaxed
template <uint64_t Capacity_, class NativeType_ = getMinSizedType_t<Capacity_>>
class IndexQueue
{
  public:

    using NativeType = NativeType_;
	static constexpr NativeType CAPACITY{Capacity_};

    using UniqueIndexType = UniqueIndex<NativeType, CAPACITY>;
    using value_type = UniqueIndexType; // for standard conformmity

    // just to distingish between constructors at compile time and make the
    // construction policy more explicit
    class ConstructFull_t
    {
    };

    class ConstructEmpty_t
    {
    };
    static  constexpr ConstructFull_t ConstructFull{};
    static  constexpr ConstructEmpty_t ConstructEmpty{};
    /// @brief constructs an empty IndexQueue
    IndexQueue(ConstructEmpty_t = ConstructEmpty);

    /// @brief constructs IndexQueue filled with all indices 0,1,...capacity-1
    IndexQueue(ConstructFull_t);

    /// @brief push index into the queue in FIFO order
    /// constraint: pushing more indices than capacity is not allowed (and not possible with UniqueIndex)
    /// constraint: only indices in the range [0, Capacity-1] are allowed
    /// threadsafe, lockfree
    template<class MonitoringPolicy=EmptyMonitoringPolicy>
    void push(UniqueIndexType uniqueIdx, MonitoringPolicy const& = MonitoringPolicy());

    /// @brief tries to remove index in FIFO order
    /// @return true iff removal was successful (i.e. queue was not empty)
    /// value is only valid if the function returns true
    /// threadsafe, lockfree
    template<class MonitoringPolicy=EmptyMonitoringPolicy>
    bool pop(UniqueIndexType& uniqueIdx, MonitoringPolicy const& = MonitoringPolicy());


    /// @brief tries to remove index in FIFO order iff the queue is full
    /// @return true iff removal was successful (i.e. queue was full)
    /// value is only valid if the function returns true
    /// @SynchronizationPolicy threadsafe, waitfree, memory neutral,
    /// all members are atomic<CyclicIndex>
    template<class MonitoringPolicy=EmptyMonitoringPolicy>
    bool popIfFull(UniqueIndexType& uniqueIdx, MonitoringPolicy const& = MonitoringPolicy());

    /// @brief get the capacity of the IndexQueue
   /// @return capacity of the IndexQueue
   /// threadsafe, lockfree
   constexpr NativeType capacity(){ return CAPACITY; }


    /// @brief check whether the queue is empty
    /// @return true iff the queue is empty
    /// note that if the queue is used concurrently it might
    /// not be empty anymore after the call
    /// (but it was at some point during the call)
    bool empty();

  private:
    using Index = CyclicIndex<CAPACITY>;

    Index loadNextReadPosition() const;
    Index loadNextWritePosition() const;
    Index loadValueAt(Index position) const;
    bool tryToPublishAt(Index writePosition, Index& oldValue, Index newValue);
    bool tryToAchieveOwnershipAt(Index& readPosition); // head
    Index updateNextWritePosition(Index oldWritePosition);// tail
//    bool isUsed(NativeType valueCycle, NativeType tailCycle);
    // @todo: a compile time check whether atomic<Index> is actually lock free would be nice (is there a solution with
    // c++11?)
    // inner call is not constexpr so we cannot do compile time check here ...
    // static_assert(std::atomic<Index>{}.is_lock_free());

    std::atomic<Index> m_values[CAPACITY];
    std::atomic<Index> m_nextReadPosition; // aka head
    std::atomic<Index> m_nextWritePosition; // aka tail
  public:
    // helper functions for development
    void print() const;
    void printThreadInfo(const char* message) const;
    // checkPoints for test instrumentation
    enum {AfterLoadPosition=1, AfterLoadValue=2, BeforeUpdatePosition=4, EndOfMethod };


};
} // namespace iox

#include "index_queue.inl"
