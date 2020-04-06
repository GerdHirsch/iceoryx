#include <iostream>
namespace iox
{

template<uint64_t Capacity>
void
IndexQueue<Capacity>::print()
{
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
template<uint64_t Capacity>
IndexQueue<Capacity>::IndexQueue(ConstructEmpty)
    : m_head(Index(Capacity))
    , m_tail(Index(Capacity))
{
}

template<uint64_t Capacity>
IndexQueue<Capacity>::IndexQueue(ConstructFull)
    : m_head(Index(0))
    , m_tail(Index(Capacity))
{
    for (NativeType i = 0; i < Capacity; ++i)
    {
        m_values[i].store(Index(i));
    }
}

template<uint64_t Capacity>
constexpr typename IndexQueue<Capacity>::NativeType IndexQueue<Capacity>::capacity()
{
    return Capacity;
}

//template<uint64_t Capacity>
//void IndexQueue<Capacity>::push(indexvalue_t index)
//{
//    Index oldTail;
//    do
//    {
////        oldTail = m_tail.load(std::memory_order_acquire);
//        oldTail = loadTail();
//
//        auto position = oldTail.getIndex();
//
////        auto value = m_values[position].load(std::memory_order_relaxed);
//        auto value = loadValueAt(position);
//        auto tailCycle = oldTail.getCycle();
//        auto valueCycle = value.getCycle();
//        // todo gh value.isUsed(oldTail),  !isFree(oldTail)...
//        // another thread has changed, we load tail before update
////        if (valueCycle == tailCycle)
//        if (isUsed(valueCycle, tailCycle))
//        {
//            // try to help moving the m_tail, can fail but we retry anyway and some enqueuer will succeed
//            m_tail.compare_exchange_strong(oldTail, oldTail + 1, std::memory_order_acq_rel, std::memory_order_acquire);
//            continue; // retry operation
//        }
//        // Todo gh !value.isBehind(oldTail)
////        if (valueCycle + 1 != tailCycle)
//        if (!value.isBehind(oldTail))
//        {
//            // valueCycle is different and m_tail not ahead by one cycle, hence oldTail is stale
//            continue; // retry operation
//        }
//
//        // we only insert a new value if m_tail is ahead by one cycle
//
//        // technically this can overwrite a value that was not yet dequeued, but in our use case
//        // this will not happen since we transfer indices back and forth from 2 identically
//        // sized queues containing only capacity (= length of a cycle) m_values in total
//
//        Index newValue(index, tailCycle);
//        // todo gh: tryToPublishAt(position, newValue),
//        if (m_values[position].compare_exchange_strong(
//                value, newValue, std::memory_order_acq_rel, std::memory_order_acquire))
//        {
//            break; // push successful
//        }
//
//    } while (true); // we leave iff the CAS was successful
//
//    // try moving the m_tail, if it fails it is no problem, another push has to help this operation
//    // before it can succeed itself
//    // todo gh moveTailByOne(oldTail)
//    m_tail.compare_exchange_strong(oldTail, oldTail + 1, std::memory_order_relaxed, std::memory_order_relaxed);
//}

template<uint64_t Capacity>
typename IndexQueue<Capacity>::Index
IndexQueue<Capacity>::loadNextReadPosition(){
	return m_head.load(std::memory_order_relaxed);
}
template<uint64_t Capacity>
typename IndexQueue<Capacity>::Index
IndexQueue<Capacity>::loadNextWritePosition(){
	return m_tail.load(std::memory_order_relaxed);
}
template<uint64_t Capacity>
typename IndexQueue<Capacity>::Index
IndexQueue<Capacity>::loadValueAt(typename IndexQueue<Capacity>::Index position){
	return m_values[position.getIndex()].load(std::memory_order_relaxed);
}
template<uint64_t Capacity>
bool
IndexQueue<Capacity>::tryToPublishAt(
		typename IndexQueue<Capacity>::Index writePosition,
		Index &oldValue,
		Index newValue){
	return m_values[writePosition.getIndex()].
			compare_exchange_strong(
				oldValue, newValue,
				std::memory_order_relaxed,
				std::memory_order_relaxed);
}
template<uint64_t Capacity>
void
IndexQueue<Capacity>::updateNextWritePosition(Index& oldWritePosition){
	// compare_exchange updates oldWritePosition
	// if not succeed
	// else oldWritePosition stays unchanged
	// and will be updated in if(succeed)
	Index newWritePosition(oldWritePosition+1);
	auto succeed = m_tail.compare_exchange_strong(
				oldWritePosition, newWritePosition,
				std::memory_order_relaxed,
				std::memory_order_relaxed);
	if(succeed){
		oldWritePosition = newWritePosition;
	}
}
template<uint64_t Capacity>
bool
IndexQueue<Capacity>::tryToAchieveOwnership(Index& oldReadPosition){
	Index newReadPosition(oldReadPosition + 1);
	return m_head.compare_exchange_strong(
			oldReadPosition, newReadPosition,
			std::memory_order_relaxed,
			std::memory_order_relaxed);
}

//template<uint64_t Capacity>
//template<class MonitoringPolicy>
//void
//IndexQueue<Capacity>::demoMonitoringPolicy(MonitoringPolicy const& policy){
//	policy.print();
//}


//void IndexQueue<Capacity>::push(indexvalue_t identity)
template<uint64_t Capacity>
template<class MonitoringPolicy>
void IndexQueue<Capacity>::push(UniqueIndexType identity, MonitoringPolicy const& policy)
{
	bool notPublished = true;
    auto writePosition = loadNextWritePosition();
    do
    {
    	policy.checkPoint(AfterLoadPosition);

    	auto oldValue = loadValueAt(writePosition);
    	policy.checkPoint(AfterLoadValue);

        auto isFree = [&](){ return oldValue.isBehind(writePosition);};

        if(isFree()){
        	Index newValue(identity, writePosition.getCycle() );
        	//if publish fails, another thread has published before
        	auto succeed = tryToPublishAt(writePosition, oldValue, newValue);
        	if(succeed){
        		break;
        	}
        }
		// try to help moving the nextWritePosition,
        // can fail but we retry anyway and some enqueuer will succeed
        updateNextWritePosition(writePosition);

    }while(notPublished);

    policy.checkPoint(BeforeUpdatePosition);
    updateNextWritePosition(writePosition);
}

template<uint64_t Capacity>
bool IndexQueue<Capacity>::pop(UniqueIndexType& identity)
{
    Index value;
    do
    {
    	auto readPosition = loadNextReadPosition();
        value = loadValueAt(readPosition);

        // we only dequeue if value and readPosition is in the same cycle
        auto isUpToDate = [&](){ return readPosition.getCycle() == value.getCycle();};
		// readPosition is ahead by one cycle, queue was empty at loadValueAt(..)
        auto isEmpty = [&](){ return value.isBehind(readPosition);};

        if(isUpToDate()){
        	auto succeed = tryToAchieveOwnership(readPosition);
			if (succeed)
			{
				break; // pop successful
			}
        }else if (isEmpty()){
			return false;
		}// else readPosition is stale, retry operation

    } while (true); // we leave if we achieve Ownership of readPosition

    identity = UniqueIndexType(value.getIndex()); // implicit move()
    return true;
}
//template<uint64_t Capacity>
//bool IndexQueue<Capacity>::pop(indexvalue_t& index)
//{
//    Index value;
//    do
//    {
////    	auto oldHead = this->loadHead();
//    	auto oldHead = m_head.load(std::memory_order_acquire);
////        value = loadValueAt(oldHead.getIndex());
//        value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed); // (value load)
//        auto headCycle = oldHead.getCycle();
//        auto valueCycle = value.getCycle();
//        //
//        if (valueCycle != headCycle)
//        {
//            if (valueCycle + 1 == headCycle)
//            {
//                return false; // m_head is ahead by one cycle, queue was empty at (value load)
//            }
//            continue; // oldHead is stale, retry operation
//        }
//
//        // we only dequeue if the cycle of value and of m_head is the same
//
//        if (m_head.compare_exchange_strong(oldHead, oldHead + 1, std::memory_order_acq_rel, std::memory_order_acquire))
//        {
//            break; // pop successful
//        }
//
//    } while (true); // we leave iff the CAS was successful
//
//    index = indexvalue_t(value.getIndex());
//    return true;
//}

template<uint64_t Capacity>
bool IndexQueue<Capacity>::popIfFull(UniqueIndexType& index)
{
	auto writePosition= loadNextWritePosition(); // tail
	auto readPosition= loadNextReadPosition(); // head
	auto value = loadValueAt(readPosition);
	auto isFull = [&](){
		return 	writePosition.getIndex() == readPosition.getIndex() &&
				readPosition.isBehind(writePosition);
	};
	if(isFull()){
		auto succeed = tryToAchieveOwnership(readPosition);
		if(succeed){
			index = value.getIndex();
			return true;
		}
	}//else someone else has popped an identity
	return false;
}
//
//template<uint64_t Capacity>
//bool IndexQueue<Capacity>::popIfFull(indexvalue_t& index)
//{
//    Index value;
//    do
//    {
//        auto oldHead = m_head.load(std::memory_order_acquire);
//        value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed); // (value load)
//        auto headCycle = oldHead.getCycle();
//        auto valueCycle = value.getCycle();
//
//        if (valueCycle != headCycle)
//        {
//            if (valueCycle + 1 == headCycle)
//            {
//                return false; // m_head is ahead by one cycle, queue was empty at (value load)
//            }
//            continue; // oldHead is stale, retry operation
//        }
//
//        // check whether the queue is full (can we do this without tail as with empty?)
//        auto oldTail = m_tail.load(std::memory_order_relaxed);
//        if (oldTail.getIndex() == oldHead.getIndex() && oldTail.getCycle() == headCycle + 1)
//        {
//            // queue is full, this can only change when m_head changes which will be detected by CAS
//
//            // we only dequeue if the cycle of value and of m_head is the same
//            if (m_head.compare_exchange_strong(
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

template<uint64_t Capacity>
bool IndexQueue<Capacity>::empty()
{
    auto oldHead = m_head.load(std::memory_order_acquire);
    auto value = m_values[oldHead.getIndex()].load(std::memory_order_relaxed);

    if (value.getCycle() + 1 == oldHead.getCycle())
    {
        // if m_head is ahead by one cycle compared to the value stored at head,
        // the queue was empty at the time of the loads ebove (but might not be anymore!)
        return true;
    }
    return false;
}
} // namespace iox
