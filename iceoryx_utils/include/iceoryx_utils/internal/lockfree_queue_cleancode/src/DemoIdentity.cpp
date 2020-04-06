/*
 * DemoIdentity.cpp
 *
 *  Created on: Mar 30, 2020
 *      Author: user
 */

#include "../include/GetMinSizedType.hpp"

#include <iostream>
#include <cstdint>

#include "../include/unique_index.hpp"
using namespace std;

void demoIdentity(){
	cout << "demoIdentity()" << __FILE__ << endl;

	cout << "LSBitABACounter 1: " << getMinSizedType<1>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<1>::IndexBits << endl;
	cout << "LSBitABACounter 2: " << getMinSizedType<2>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<2>::IndexBits << endl;
	cout << "LSBitABACounter 3: " << getMinSizedType<3>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<3>::IndexBits << endl;
	cout << "LSBitABACounter 4: " << getMinSizedType<4>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<4>::IndexBits << endl;
	cout << "LSBitABACounter 5: " << getMinSizedType<5>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<5>::IndexBits << endl;
	cout << "LSBitABACounter 6: " << getMinSizedType<6>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<6>::IndexBits << endl;
	cout << "LSBitABACounter 7: " << getMinSizedType<7>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<7>::IndexBits << endl;
	cout << "LSBitABACounter 8: " << getMinSizedType<8>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<8>::IndexBits << endl;
	cout << "LSBitABACounter 9: " << getMinSizedType<9>::LSBitABACounter << endl;
	cout << "IndexBits : 	" << getMinSizedType<9>::IndexBits << endl;


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


