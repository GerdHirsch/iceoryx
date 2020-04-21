/*
 * GetMinSizedType.hpp
 *
 *  Created on: Mar 30, 2020
 *      Author: user
 */

#ifndef INCLUDE_GETMINSIZEDTYPE_HPP_
#define INCLUDE_GETMINSIZEDTYPE_HPP_

#include <cstdint>
#include <type_traits>

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

template<bool condition, class TrueType, class FalseType>
using IF = typename std::conditional<condition, TrueType, FalseType>::type;

//template<std::uint64_t Capacity, Optimization OPTIMIZATION_ = Optimization::STORAGE>
template<std::uint64_t Capacity, Optimization = Optimization::STORAGE>
struct getMinSizedType;

template<std::uint64_t Capacity>
struct getMinSizedType<Capacity, Optimization::SPEED>:CalculateBits<Capacity>{
	using base_type = CalculateBits<Capacity>;
	static constexpr std::uint64_t IdxB_8{base_type::IndexBits+8};
	using type =
			IF<IdxB_8 <= sizeof(std::uint8_t)*8,
				std::uint_fast8_t,
			IF<IdxB_8 <= sizeof(std::uint16_t)*8,
				std::uint_fast16_t,
			IF<IdxB_8 <= sizeof(std::uint32_t)*8,
				std::uint_fast32_t,
				std::uint_fast64_t
			>>>;
};

template<std::uint64_t Capacity>
struct getMinSizedType<Capacity, Optimization::STORAGE>:CalculateBits<Capacity>{
	using base_type = CalculateBits<Capacity>;
	static constexpr std::uint64_t IdxB_8{base_type::IndexBits+8};
	using type =
			IF<IdxB_8 <= sizeof(std::uint8_t)*8,
				std::uint_least8_t,
			IF<IdxB_8 <= sizeof(std::uint16_t)*8,
				std::uint_least16_t,
			IF<IdxB_8 <= sizeof(std::uint32_t)*8,
				std::uint_least32_t,
				std::uint_least64_t
			>>>;
};

template<std::uint64_t Capacity, Optimization OPTIMIZATION = Optimization::STORAGE>
using getMinSizedType_t = typename getMinSizedType<Capacity, OPTIMIZATION>::type;




#endif /* INCLUDE_GETMINSIZEDTYPE_HPP_ */
