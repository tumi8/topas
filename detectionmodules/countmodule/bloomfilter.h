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
#include <stdint.h>

/* GenericKey class holding uint8_t* input for BloomFilter hash functions */
template<unsigned size> class GenericKey
{
    public:
	unsigned len;
	uint8_t data[size];

	GenericKey() : len(0) {
	    memset(data,0,sizeof(data));
	}

	void set(uint8_t *input, unsigned inputlen)
	{
	    len = inputlen<sizeof(data)?inputlen:sizeof(data);
	    memcpy(&data, input, len);
	}

	void reset()
	{
	    len = 0;
	    memset(data,0,sizeof(data));
	}

	void append(uint8_t *input, unsigned inputlen)
	{
	    if(len<sizeof(data)) {
		if((len+inputlen) < sizeof(data)) {
		    memcpy(&(data[len]), input, inputlen);
		    len = len + inputlen;
		} else {
		    memcpy(&(data[len]), input, sizeof(data)-len);
		    len = sizeof(data);
		}
	    }
	}

	bool operator<(const GenericKey& other) const 
	{
	    if(len < other.len)
		return true;
	    else if(len > other.len)
		return false;
	    else
		return memcmp(data, other.data, len)<0?true:false;
	}

};


/* QuintupleKey class holding input for BloomFilter hash functions */
class QuintupleKey
{
    public:
	uint8_t data[15];
	const unsigned len;

	struct Quintuple
	{
	    uint32_t srcIp;
	    uint32_t dstIp;
	    uint8_t proto;
	    uint16_t srcPort;
	    uint16_t dstPort;
	};

	QuintupleKey() : len(15)
	{
	    memset(data,0,sizeof(data));
	}

	inline Quintuple* getQuintuple()
	{
	    return (Quintuple*)data;
	}

	void reset()
	{
	    memset(data,0,sizeof(data));
	}

	bool operator<(const QuintupleKey& other) const 
	{
	    return memcmp(data, other.data, len)<0?true:false;
	}

};


/* Bitmap vector for normal BloomFilter */
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


/* HashFunctions provides hash functions for filters */
class HashFunctions
{
    public:
	HashFunctions() : hf_list(NULL), hf_number(0) 
	{
	    r = gsl_rng_alloc (gsl_rng_fishman18);
	}

	HashFunctions(unsigned hashfunctions) : hf_list(NULL), hf_number(hashfunctions)
	{
	    r = gsl_rng_alloc (gsl_rng_fishman18);
	    initHF();
	}

	~HashFunctions()
	{
	    free(hf_list);
	    gsl_rng_free(r);
	}

    protected:
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
};


/* BloomFilter normal bloom filter */
class BloomFilter : public HashFunctions
{
    friend std::ostream & operator << (std::ostream &, const BloomFilter &);

    public:
	BloomFilter() : HashFunctions(), filter_size(0) {}

	BloomFilter(uint32_t size, unsigned hashfunctions) : HashFunctions(hashfunctions), filter_size(size) 
	{
	    filter.resize(size);
	}

	~BloomFilter() {}

	void init(uint32_t size, unsigned hashfunctions);
	void clear();
	void insert(uint8_t* input, unsigned len);
	bool test(uint8_t* input, unsigned len) const;
	bool testBeforeInsert(uint8_t* input, unsigned len);

    private:
	Bitmap filter;
	uint32_t filter_size;
};

std::ostream & operator << (std::ostream &, const BloomFilter &);



