/*
 * GetMinSizedType.hpp
 *
 *  Created on: Mar 30, 2020
 *      Author: user
 */

#ifndef INCLUDE_GETMINSIZEDTYPE_HPP_
#define INCLUDE_GETMINSIZEDTYPE_HPP_

#include <cstdint>

enum class Optimization{SPEED, STORAGE};

template<std::uint64_t Capacity>
struct CalculateBits{
	using this_type = CalculateBits<Capacity>;
	using CapacityType = std::uint64_t;
	static constexpr CapacityType CAPACITY{Capacity};
	static constexpr CapacityType LSBitABACounter = this_type::calculateLSBitABACounter(1);
	static constexpr CapacityType IndexBits = this_type::calculateIndexBits();
	static constexpr CapacityType ONEs = ~static_cast<CapacityType>(0);
	static constexpr CapacityType CYCLE_MASK = ONEs << IndexBits;

	static constexpr CapacityType calculateLSBitABACounter(CapacityType retVal=1) noexcept{
		return retVal < CAPACITY ? calculateLSBitABACounter(retVal<<1) : retVal;
	}
	static constexpr std::uint16_t calculateIndexBits(std::uint16_t retVal=0, CapacityType bits=1) noexcept{
		return bits < LSBitABACounter ? calculateIndexBits(retVal+1, bits<<1) : retVal;
	}
};

//template<std::uint64_t Capacity, Optimization OPTIMIZATION_ = Optimization::STORAGE>
template<std::uint64_t Capacity, Optimization = Optimization::STORAGE>
struct getMinSizedType;

template<std::uint64_t Capacity>
struct getMinSizedType<Capacity, Optimization::SPEED>:CalculateBits<Capacity>{
	using type = std::uint_fast64_t;
};

template<std::uint64_t Capacity>
struct getMinSizedType<Capacity, Optimization::STORAGE>:CalculateBits<Capacity>{
	using type = std::uint16_t;
};

template<std::uint64_t Capacity, Optimization OPTIMIZATION = Optimization::STORAGE>
using getMinSizedType_t = typename getMinSizedType<Capacity, OPTIMIZATION>::type;




#endif /* INCLUDE_GETMINSIZEDTYPE_HPP_ */
