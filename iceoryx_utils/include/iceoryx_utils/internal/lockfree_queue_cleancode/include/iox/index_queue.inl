#include <iostream>
namespace iox
{

template<uint64_t Capacity, class NativeType>
void
IndexQueue<Capacity, NativeType>::printThreadInfo(const char* message) const
{
	return;// switch on/off output

	using namespace std;
	cout << endl << "IndexQueue: " << this_thread::get_id() << " " << message << endl;
}
template<uint64_t Capacity, class NativeType>
void
IndexQueue<Capacity, NativeType>::print() const
{
	//return;// switch on/off output

	using namespace std;
	cout << endl << "IndexQueue::print()" << endl;
	auto rPos = loadNextReadPosition();
	auto wPos = loadNextWritePosition();
	cout
	<< "rPos head[" << rPos.getCycle() << ", " << rPos.getIndex() << "]" << endl
	<< "wPos tail[" << wPos.getCycle() << ", " << wPos.getIndex() << "]" << endl
	;

	for(NativeType i=0; i<Capacity; ++i){
		auto idx = loadValueAt(Index(i));
		cout
		<< "m_values[" << i << "]: ["
		<< idx.getCycle() << ", "
		<< idx.getIndex() << "]"
		<< endl
		;
	}
}
template<uint64_t Capacity, class NativeType>
IndexQueue<Capacity, NativeType>::IndexQueue(ConstructEmpty_t)
    : m_nextReadPosition(Index(Capacity))
    , m_nextWritePosition(Index(Capacity))
{
}

template<uint64_t Capacity, class NativeType>
IndexQueue<Capacity, NativeType>::IndexQueue(ConstructFull_t)
    : m_nextReadPosition(Index(0)) // nextReadPosition
    , m_nextWritePosition(Index(Capacity)) // nextWritePosition
{
    for (NativeType i = 0; i < Capacity; ++i)
    {
        m_values[i].store(Index(i));
    }
}



template<uint64_t Capacity, class NativeType>
typename IndexQueue<Capacity, NativeType>::Index
IndexQueue<Capacity, NativeType>::loadNextReadPosition() const{
	return m_nextReadPosition.load(std::memory_order_relaxed);
//	return m_nextReadPosition.load(std::memory_order_acquire);
}
template<uint64_t Capacity, class NativeType>
typename IndexQueue<Capacity, NativeType>::Index
IndexQueue<Capacity, NativeType>::loadNextWritePosition() const{
	return m_nextWritePosition.load(std::memory_order_relaxed);
//	return m_nextWritePosition.load(std::memory_order_acquire);
}
template<uint64_t Capacity, class NativeType>
typename IndexQueue<Capacity, NativeType>::Index
IndexQueue<Capacity, NativeType>::loadValueAt(typename IndexQueue<Capacity, NativeType>::Index position) const{
	return m_values[position.getIndex()].load(std::memory_order_relaxed);
//	return m_values[position.getIndex()].load(std::memory_order_acquire);
}
template<uint64_t Capacity, class NativeType>
bool
IndexQueue<Capacity, NativeType>::tryToPublishAt(
		typename IndexQueue<Capacity, NativeType>::Index writePosition,
		Index &oldValue,
		Index newValue){
	return m_values[writePosition.getIndex()].
			compare_exchange_strong(
				oldValue, newValue,
				std::memory_order_relaxed,
				std::memory_order_relaxed
//				std::memory_order_release,
//				std::memory_order_acquire
				);
}
template<uint64_t Capacity, class NativeType>
typename IndexQueue<Capacity, NativeType>::Index
IndexQueue<Capacity, NativeType>::updateNextWritePosition(Index oldWritePosition){
	// compare_exchange updates oldWritePosition
	// if not succeed
	// else oldWritePosition stays unchanged
	// and will be updated in if(succeed)
	Index newWritePosition(oldWritePosition+1);
	auto succeed = m_nextWritePosition.compare_exchange_strong(
				oldWritePosition, newWritePosition,
				std::memory_order_relaxed,
				std::memory_order_relaxed
//				std::memory_order_release,
//				std::memory_order_acquire
				);
	if(succeed){
		oldWritePosition = newWritePosition;
	}
	return oldWritePosition; // updated
}
template<uint64_t Capacity, class NativeType>
bool
IndexQueue<Capacity, NativeType>::tryToAchieveOwnershipAt(Index& oldReadPosition){
	Index newReadPosition(oldReadPosition + 1);
	return m_nextReadPosition.compare_exchange_strong(
			oldReadPosition, newReadPosition,
			std::memory_order_relaxed,
			std::memory_order_relaxed);
}

/**
 * push: not waitfree, starvation may occur in the surrounding Queue,
 * in the following rare cases, P_SUT may wait forever
 * P_SUT (System Under Test) is the process to be considered
 * if P_SUT will be interrupted each time before tryToPublishAt
 * and another thread publishes at this position, P_SUT will loop forever
 * hint: there must be pops also between the pushes, otherwise
 * the queue will be full and no more uniqueIdx are available to push
 */template<uint64_t Capacity, class NativeType>
template<class MonitoringPolicy>
void IndexQueue<Capacity, NativeType>::push(UniqueIndexType uniqueIdx, MonitoringPolicy const& policy)
{
//	 to consider:
//	 all statements of this algorithm can be relaxed!
//	 they can not be reordered, because of
//	 data dependencies between the statements
//	 loadNextWritePosition->loadValueAt->newValue->tryToPublishAt->updateNextWritePosition
//	 reordering would violate the "as if rule"
//	 see: en.cppreference.com/w/cpp/language/as_if
//	 only checkPoints could be reordered,
//	 but this would only make the test fail
	constexpr bool notPublished = true;
    auto writePosition = loadNextWritePosition();
    do
    {
    	policy.checkPoint(AfterLoadPosition);

    	auto oldValue = loadValueAt(writePosition);
    	policy.checkPoint(AfterLoadValue);

        auto isFree = [&](){ return oldValue.isBehind(writePosition);};

        if(isFree()){
        	Index newValue(uniqueIdx, writePosition.getCycle() );
        	//if publish fails, another thread has published before
        	auto published = tryToPublishAt(writePosition, oldValue, newValue);
        	if(published){
        		break;
        	}
        }
		// try to help moving the nextWritePosition,
        // can fail but we retry anyway and some enqueuer will succeed
        writePosition = updateNextWritePosition(writePosition);

    }while(notPublished);

    policy.checkPoint(BeforeUpdatePosition);
    updateNextWritePosition(writePosition);
    policy.checkPoint(EndOfMethod);
}
/**
 * pop: not waitfree, starvation may occur in the surrounding Queue,
 * in the following rare cases, P_SUT may wait forever
 * P_SUT (System Under Test) is the process to be considered
 * if P_SUT will be interrupted each time before tryToAchieveOwnership
 * and another thread achieves ownership, P_SUT will loop forever
 * hint: there must be pushes also between the pops, otherwise
 * the queue will be empty and P_SUT.pop returns false
 */
template<uint64_t Capacity, class NativeType>
template<class MonitoringPolicy>
bool IndexQueue<Capacity, NativeType>::pop(UniqueIndexType& uniqueIdx, MonitoringPolicy const& policy)
{
    Index value;
    bool notEmpty=true;
    do
    {
    	auto readPosition = loadNextReadPosition();
    	policy.checkPoint(AfterLoadPosition);

    	value = loadValueAt(readPosition);
        policy.checkPoint(AfterLoadValue);

        // we only dequeue if value and readPosition is in the same cycle
        auto isUpToDate = [&](){ return readPosition.getCycle() == value.getCycle();};
		// readPosition is ahead by one cycle, queue was empty at loadValueAt(..)
        auto isEmpty = [&](){ return value.isBehind(readPosition);};

        if(isUpToDate()){
        	auto ownershipAchieved = tryToAchieveOwnershipAt(readPosition);
			if (ownershipAchieved)
			{
				uniqueIdx = UniqueIndexType(value.getIndex()); // implicit move()
				break; // pop successful
			}
        }else if (isEmpty()){
			notEmpty = false;
		}// else readPosition is stale, retry operation

    } while (notEmpty); // we leave if we achieve Ownership of readPosition

    policy.checkPoint(EndOfMethod);
    return notEmpty;
}

/**
 * popIfFull: not waitfree, starvation may occur in the surrounding Queue,
 * in the following rare cases, P_SUT may wait forever,
 * P_SUT (System Under Test) is the process to be considered
 * The possibility depends on the relationship between the number of
 * elements, the number of threads/processes and the degree of
 * filling
 * number of Elements == 4 in the surrounding queue
 * freeIdx == [3]
 * usedIdx == [0, 1, 2]
 * Px push ownership 3, freeIdx == [empty]
 * P_SUT push
 * 	freeIdx.pop -> false
 * 	usedIdx.popIfFull -> false
 * Px transfer ownership usedIdx == [0, 1, 2, 3]
 * Py pop 0 ... usedIdx == [1, 2, 3]
 * P_SUT
 * 	freeIdx.pop -> false
 * 	useIdx.popIfFull -> false
 * Py
 * 	transfer ownership freeIdx == [0]
 * Pz
 * 	push ownership 0, freeIdx == [empty]
 * P_SUT
 * 	freeIdx.pop -> false
 * 	usedIdx.popIfFull -> false
 * Pz etc
 */
/**
 * popIfFull no longer works, if we loose a UniqueIndex,
 * cause isFull() will return always false! even though,
 * no other threads hold the ownership of a UniqueIndex
 * and freeIdx == [empty]
 */
template<uint64_t Capacity, class NativeType>
template<class MonitoringPolicy>
bool IndexQueue<Capacity, NativeType>::popIfFull(UniqueIndexType& uniqueIdx, MonitoringPolicy const& policy)
{

	auto writePosition= loadNextWritePosition(); // tail
	auto readPosition= loadNextReadPosition(); // head
	policy.checkPoint(AfterLoadPosition);

	auto value = loadValueAt(readPosition);
	policy.checkPoint(AfterLoadValue);

	bool returnValue = false;

	auto isFull = [&](){
		return 	writePosition.getIndex() == readPosition.getIndex() &&
				readPosition.isBehind(writePosition);
	};
	if(isFull()){
		auto ownershipAchieved = tryToAchieveOwnershipAt(readPosition);
		if(ownershipAchieved){
			uniqueIdx = value_type(value.getIndex()); // implizit move
			returnValue = true;
		}
	}//else someone else has popped an identity
	policy.checkPoint(EndOfMethod);
	return returnValue;
}
//
//template<uint64_t Capacity, class NativeType>
//bool IndexQueue<Capacity, NativeType>::popIfFull(indexvalue_t& index)
//{
//    Index value;
//    do
//    {
//        auto oldHead = m_nextReadPosition.load(std::memory_order_acquire);
//        value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed); // (value load)
//        auto headCycle = oldHead.getCycle();
//        auto valueCycle = value.getCycle();
//
//        if (valueCycle != headCycle)
//        {
//            if (valueCycle + 1 == headCycle)
//            {
//                return false; // m_nextReadPosition is ahead by one cycle, queue was empty at (value load)
//            }
//            continue; // oldHead is stale, retry operation
//        }
//
//        // check whether the queue is full (can we do this without tail as with empty?)
//        auto oldTail = m_nextWritePosition.load(std::memory_order_relaxed);
//        if (oldTail.getIndex() == oldHead.getIndex() && oldTail.getCycle() == headCycle + 1)
//        {
//            // queue is full, this can only change when m_nextReadPosition changes which will be detected by CAS
//
//            // we only dequeue if the cycle of value and of m_nextReadPosition is the same
//            if (m_nextReadPosition.compare_exchange_strong(
//                    oldHead, oldHead + 1, std::memory_order_acq_rel, std::memory_order_acquire))
//            {
//                break; // pop successful
//            }
//        }
//        else
//        {
//            return false; // queue is not full
//        }
//
//    } while (true); // we leave iff the CAS was successful
//
//    index = value.getIndex();
//    return true;
//}

template<uint64_t Capacity, class NativeType>
bool IndexQueue<Capacity, NativeType>::empty()
{
    auto oldHead = m_nextReadPosition.load(std::memory_order_acquire);
    auto value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed);

    if (value.getCycle() + 1 == oldHead.getCycle())
    {
        // if m_nextReadPosition is ahead by one cycle compared to the value stored at head,
        // the queue was empty at the time of the loads ebove (but might not be anymore!)
        return true;
    }
    return false;
}
//template<uint64_t Capacity, class NativeType>
//constexpr typename IndexQueue<Capacity, NativeType>::NativeType IndexQueue<Capacity, NativeType>::capacity()
//{
//    return Capacity;
//}
} // namespace iox
