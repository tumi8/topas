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

#include "agebloomfilter.h"

void AgeArray::resize(uint32_t size)
{
    free(array);
    array_size = size;
    array = (uint16_t*)(malloc(size*sizeof(uint16_t)));
    clear();
}

void AgeArray::clear()
{
    memset(array, 0, array_size*sizeof(uint16_t));
}

inline uint16_t AgeArray::getAndSet(uint32_t index, uint16_t time)
{
    uint16_t ret = 0;
    if(index < array_size)
	ret = array[index];
    array[index] = time;
    return ret;
}

inline uint16_t AgeArray::get(uint32_t index) const
{
    if(index < array_size)
	return array[index];
    else
	return 0;
}

std::ostream & operator << (std::ostream & os, const AgeArray & a) 
{
    for(uint32_t i=0; i<a.array_size; i++)
    {
	os << a.get(i) << " ";
    }
    return os;
}


void AgeBloomFilter::init(uint32_t size, unsigned hashfunctions)
{
    hf_number = hashfunctions;
    initHF();
    filter_size = size;
    filter.resize(size);
}

void AgeBloomFilter::clear()
{
    filter.clear();
}

uint16_t AgeBloomFilter::getLastTime(uint8_t* input, unsigned len, uint16_t time) const
{
    uint16_t ret = 0;
    uint16_t current, diff, maxdiff = 0;
    for(unsigned i=0; i < hf_number; i++) 
    {
	current = filter.get(hashU(input, len, filter_size, hf_list[i].seed));
	diff = time - current;
	if(diff > maxdiff)
	{
	    ret = current;
	    maxdiff = diff;
	}	    
    }
    return ret;
}

uint16_t AgeBloomFilter::getAndSetLastTime(uint8_t* input, unsigned len, uint16_t time)
{
    uint16_t ret = 0;
    uint16_t current, diff, maxdiff = 0;
    for(unsigned i=0; i < hf_number; i++) 
    {
	current = filter.getAndSet(hashU(input, len, filter_size, hf_list[i].seed), time);
	diff = time - current;
	if(diff > maxdiff)
	{
	    ret = current;
	    maxdiff = diff;
	}	    
    }
    return ret;
}


std::ostream & operator << (std::ostream & os, const AgeBloomFilter & b) 
{
    os << b.filter;
    return os;
}

