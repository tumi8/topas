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

#include <iostream>
#include <gsl/gsl_rng.h>

class Bitmap {
    friend std::ostream & operator << (std::ostream &, const Bitmap &);

public:
    Bitmap() : len_bits(0), len_octets(0), bitmap(NULL) {}

    Bitmap(uint32_t size) : bitmap(NULL)
    {
	resize(size);
    }
	
    ~Bitmap()
    {
	free(bitmap);
    }

    void resize(uint32_t size);
    void clear();
    void set(uint32_t index);
    bool test(uint32_t index) const;
    
private:
    uint8_t* bitmap;
    uint32_t len_bits;
    uint32_t len_octets;
};

std::ostream & operator << (std::ostream &, const Bitmap &);


class BloomFilter 
{

    friend std::ostream & operator << (std::ostream &, const BloomFilter &);

public:
    BloomFilter() : hf_list(NULL), hf_number(0), filter_size(0) 
    {
	r = gsl_rng_alloc (gsl_rng_fishman18);
    }

    BloomFilter(uint32_t size, unsigned hashfunctions) : hf_list(NULL), hf_number(hashfunctions), filter_size(size) 
    {
	r = gsl_rng_alloc (gsl_rng_fishman18);
	initHF();
	filter.resize(size);
    }

    ~BloomFilter()
    {
	free(hf_list);
	gsl_rng_free(r);
    }
    
    void init(uint32_t size, unsigned hashfunctions);
    void clear();
    void insert(uint8_t* input, unsigned len);
    bool test(uint8_t* input, unsigned len) const;
    bool testBeforeInsert(uint8_t* input, unsigned len);
	
private:
    void initHF();
    uint32_t hashU(uint8_t* input, uint16_t len, uint32_t max, uint32_t seed) const;
    uint32_t ggT(uint32_t m, uint32_t n);

    gsl_rng * r;
    
    struct hash_params
    {
	uint32_t seed;
    };
    
    hash_params* hf_list;
    unsigned hf_number;

    Bitmap filter;
    uint32_t filter_size;
};

std::ostream & operator << (std::ostream &, const BloomFilter &);
