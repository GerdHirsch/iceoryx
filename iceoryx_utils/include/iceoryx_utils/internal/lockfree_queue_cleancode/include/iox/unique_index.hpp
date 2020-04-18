/*
 * element_type.hpp
 *
 *  Created on: Mar 23, 2020
 *      Author: user
 */

#ifndef INCLUDE_UNIQUE_INDEX_HPP_
#define INCLUDE_UNIQUE_INDEX_HPP_

namespace iox{

template<class NativeType_, NativeType_ Capacity_>
class UniqueIndex{
public:
	using NativeType = NativeType_;
	static constexpr NativeType CAPACITY = Capacity_;
	using this_type = UniqueIndex<NativeType, CAPACITY>;
	UniqueIndex():m_index(0){} // todo create invalid UniqueIndex
	explicit UniqueIndex(NativeType index):m_index(index){}
	// copy
	UniqueIndex(this_type const&) = delete;
	this_type& operator=(this_type const&) = delete;
	// move
	UniqueIndex(this_type&& rhs):m_index(rhs.m_index){}
	this_type& operator=(this_type&& rhs){
		this->m_index = rhs.m_index;
		rhs.invalidate();
		return *this;
	}
	// convert
	NativeType getIndex() const { return m_index;}
	operator NativeType() const { return m_index;}
	explicit operator bool() const {return true; } // todo implement valid bit
protected:
//	explicit Identity(NativeType index):m_index(index){}
private:
	void invalidate(){
		//todo implement validBit
	}
	NativeType m_index;
};

}//namespace


#endif /* INCLUDE_UNIQUE_INDEX_HPP_ */
