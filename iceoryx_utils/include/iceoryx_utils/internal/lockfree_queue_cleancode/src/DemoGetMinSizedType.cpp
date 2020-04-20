/*
 * DemoIdentity.cpp
 *
 *  Created on: Mar 30, 2020
 *      Author: user
 */

#include "../include/iox/GetMinSizedType.hpp"
#include "../include/iox/unique_index.hpp"

#include <limits>
#include <iostream>
#include <cstdint>

using namespace std;

void demoGetMinSizedType(){
	cout << "demoGetMinSizedType()" << __FILE__ << endl;

	cout << "sizeof(getMinSizedType_t<1>): " << sizeof(getMinSizedType_t<1>) << endl;
	cout << "numeric_limits<type>		:" << std::hex << numeric_limits< getMinSizedType_t<1>>::max() << std::dec << endl;
	cout << endl;
	cout << "LSBitABACounter 1: " << getMinSizedType<1>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<1>::IndexBits << endl;
	cout << "CycleMask:		" << std::hex << getMinSizedType<1>::CYCLE_MASK << std::dec << endl;

	cout << "LSBitABACounter 2: " << getMinSizedType<2>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<2>::IndexBits << endl;
	cout << "CycleMask:		" << std::hex << getMinSizedType<2>::CYCLE_MASK << std::dec << endl;

	cout << "LSBitABACounter 3: " << getMinSizedType<3>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<3>::IndexBits << endl;
	cout << "CycleMask:		" << std::hex << getMinSizedType<3>::CYCLE_MASK << std::dec << endl;

	cout << "LSBitABACounter 4: " << getMinSizedType<4>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<4>::IndexBits << endl;
	cout << "CycleMask:		" << std::hex << getMinSizedType<4>::CYCLE_MASK << std::dec << endl;

	cout << "LSBitABACounter 5: " << getMinSizedType<5>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<5>::IndexBits << endl;
	cout << "CycleMask:		" << std::hex << getMinSizedType<5>::CYCLE_MASK << std::dec << endl;

	cout << "LSBitABACounter 6: " << getMinSizedType<6>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<6>::IndexBits << endl;
	cout << "LSBitABACounter 7: " << getMinSizedType<7>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<7>::IndexBits << endl;
	cout << "LSBitABACounter 8: " << getMinSizedType<8>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<8>::IndexBits << endl;

	cout << "LSBitABACounter 9: " << getMinSizedType<9>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<9>::IndexBits << endl;
	cout << "CycleMask:		" << std::hex << getMinSizedType<9>::CYCLE_MASK << std::dec << endl;

	using MinSizedType = getMinSizedType<9>;
	using NativeType = MinSizedType::type;
//	constexpr NativeType CYCLE_MASK{MinSizedType::CYCLE_MASK};
	constexpr NativeType CYCLE_MASK(static_cast<NativeType>(MinSizedType::CYCLE_MASK));
	cout << "CycleMask:		" << std::hex << CYCLE_MASK << std::dec << endl;


}

void demoEmbeddedInts(){
	cout << "demoEmbeddedInts()" << __FILE__ <<  endl;

	cout << "uint_fast8_t: " << sizeof(uint_fast8_t) << endl;
	cout << "uint_fast16_t: " << sizeof(uint_fast16_t) << endl;
	cout << "uint_fast32_t: " << sizeof(uint_fast32_t) << endl;
	cout << "uint_fast64_t: " << sizeof(uint_fast64_t) << endl;
	cout << endl;
	cout << "uint_least8_t: " << sizeof(uint_least8_t) << endl;
	cout << "uint_least16_t: " << sizeof(uint_least16_t) << endl;
	cout << "uint_least32_t: " << sizeof(uint_least32_t) << endl;
	cout << "uint_least64_t: " << sizeof(uint_least64_t) << endl;
	cout << endl;
	cout << "uint8_t: " << sizeof(uint8_t) << endl;
	cout << "uint16_t: " << sizeof(uint16_t) << endl;
	cout << "uint32_t: " << sizeof(uint32_t) << endl;
	cout << "uint64_t: " << sizeof(uint64_t) << endl;

}


