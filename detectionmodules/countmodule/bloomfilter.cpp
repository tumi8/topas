/**************************************************************************/
/*    Copyright (C) 2007 Gerhard Muenz                                    */
/*                                                                        */
/*    This library is free software; you can redistribute it and/or       */
/*    modify it under the terms of the GNU Lesser General Public          */
/*    License as published by the Free Software Foundation; either        */
/*    version 2.1 of the License, or (at your option) any later version.  */
/*                                                                        */
/*    This library is distributed in the hope that it will be useful,     */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/*    Lesser General Public License for more details.                     */
/*                                                                        */
/*    You should have received a copy of the GNU Lesser General Public    */
/*    License along with this library; if not, write to the Free Software  */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA    */
/**************************************************************************/

#include "bloomfilter.h"

const uint8_t bitmask[8] =
{
    0x01, //00000001
    0x02, //00000010
    0x04, //00000100
    0x08, //00001000
    0x10, //00010000
    0x20, //00100000
    0x40, //01000000
    0x80  //10000000
};
    
void Bitmap::resize(uint32_t size)
{
    free(bitmap);
    len_bits = size;
    len_octets = (size+7)/8;
    bitmap = (uint8_t*)(malloc(len_octets));
    memset(bitmap, 0, len_octets);
}

void Bitmap::clear()
{
    memset(bitmap, 0, len_octets);
}

void Bitmap::set(uint32_t index)
{
    if(index < len_bits)
	bitmap[index/8] |= bitmask[index%8];
}

bool Bitmap::test(uint32_t index) const
{
    if(index < len_bits)
	return (bitmap[index/8] & bitmask[index%8]) != 0;
    else
	return false;
}

std::ostream & operator << (std::ostream & os, const Bitmap & b) 
{
    for(uint32_t i=0; i<b.len_bits; i++)
    {
	if(b.test(i))
	    os << '1';
	else
	    os << '0';
    }
    return os;
}


void HashFunctions::initHF()
{
    free(hf_list);
    hf_list = NULL;

    if(hf_number > 0)
    {
	hf_list = (hash_params*) calloc(sizeof(hash_params), hf_number);
	srand(time(0));
	for(unsigned i=0; i < hf_number; i++)
	{
	    hf_list[i].seed = rand();
	}
    }
}

uint32_t HashFunctions::hashU(uint8_t* input, uint16_t len, uint32_t max, uint32_t seed) const
{
    uint32_t random;
    uint32_t result = 0;
    gsl_rng_set(r, seed);
    for(unsigned i = 0; i < len; i++) {
	random = gsl_rng_get (r);
	result = (random*result + input[i]);
    }
    return result % max;
}

uint32_t HashFunctions::ggT(uint32_t m, uint32_t n)
{
    uint32_t z;
    while (n>0)
    {
	z=n;
	n=m%n;
	m=z;
    }
    return m;
}


void BloomFilter::init(uint32_t size, unsigned hashfunctions)
{
    hf_number = hashfunctions;
    initHF();
    filter_size = size;
    filter.resize(size);
}

void BloomFilter::clear()
{
    filter.clear();
}

void BloomFilter::insert(uint8_t* input, unsigned len)
{
    for(unsigned i=0; i < hf_number; i++) {
	filter.set(hashU(input, len, filter_size, hf_list[i].seed));
    }
}

bool BloomFilter::test(uint8_t* input, unsigned len) const
{
    for(unsigned i=0; i < hf_number; i++) {
	if(filter.test(hashU(input, len, filter_size, hf_list[i].seed)) == false)
	    return false;
    }
    return true;
}

bool BloomFilter::testBeforeInsert(uint8_t* input, unsigned len)
{
    bool result = true;
    uint32_t index;
    for(unsigned i=0; i < hf_number; i++) {
	index = hashU(input, len, filter_size, hf_list[i].seed);
	if(filter.test(index) == false)
	    result = false;
	filter.set(index);
    }
    return result;
}

std::ostream & operator << (std::ostream & os, const BloomFilter & b) 
{
    os << b.filter;
    return os;
}

