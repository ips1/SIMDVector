// du1simd.hpp
// Petr Kubát NPRG051 2013/2014

//
// std::ptrdiff_t is used as an offset of an iterator
//
// On some platforms, ptrdiff_t is represented by signed 32bit integer, which is not large enough 
// to hold all possible offsets for memory blocks of 2GB or more (if the block consists of 1B primitive 
// types) and can lead to undefined results.
//
// This behaviour is not compliant with C++11 standard, as section 18.2 paragraph 5 explicitly states:
//
// "The type ptrdiff_t is an implementation-defined signed integer type that can hold the difference of two
// subscripts in an array object, as described in 5.7."
//

#include <iterator>
#include <memory>
#include <exception>
#include <cstdlib>

#ifndef DU1SIMD_HPP
#define DU1SIMD_HPP

// Exception to be thrown if memory alignment fails in simd_vector constructor
class alignment_exception : std::exception
{

};

// simd_iterator

template< typename T, typename S>
class simd_vector_simd_iterator {
	// Static check of type parameters
	static_assert(sizeof(S) % sizeof(T) == 0, "Incompatible type parameters!");

public:
	typedef simd_vector_simd_iterator<T, S> self;

	// Random access iterator category
	typedef std::random_access_iterator_tag iterator_category;

	// Iterator types
	typedef S value_type;
	typedef std::ptrdiff_t difference_type;
	typedef S* pointer;
	typedef S& reference;

private:
	// Pointer to beginning of correspondent simd_vector
	S* base;

	// Iterator offset
	difference_type offset;

	simd_vector_simd_iterator(S* origin, difference_type off) : base(origin), offset(off) { }

	template<typename U, typename K> friend class simd_vector_iterator;

public:
	// Operator overloads
	S& operator*() const { return base[offset]; }
	S& operator[](const difference_type& n) const { return base[offset + n]; }
	bool operator==(const self& v) const { return ((base == v.base) && (offset == v.offset)); }
	bool operator!=(const self& v) const { return ((base != v.base) || (offset != v.offset)); }
	bool operator<(const self& v) const { return ((base + offset) < (v.base + v.offset)); }
	bool operator>(const self& v) const { return ((base + offset) > (v.base + v.offset)); }
	bool operator<=(const self& v) const { return ((base + offset) <= (v.base + v.offset)); }
	bool operator>=(const self& v) const { return ((base + offset) >= (v.base + v.offset)); }
	self& operator++()
	{
		++offset;
		return (*this);
	}
	self operator++(int)
	{
		self tmp = (*this);
		++offset;
		return tmp;
	}
	self& operator--()
	{
		--offset;
		return (*this);
	}
	self operator--(int)
	{
		self tmp = (*this);
		--offset;
		return tmp;
	}
	self& operator+=(const difference_type& n)
	{
		offset += n;
		return (*this);
	}
	self& operator-=(const difference_type& n)
	{
		offset -= n;
		return (*this);
	}
	difference_type operator-(const self& a) const
	{
		return (offset - a.offset);
	}

	// Default constructor
	simd_vector_simd_iterator() : base(nullptr), offset(0) { }

	// Copy constructor
	simd_vector_simd_iterator(const simd_vector_simd_iterator<T, S>& v) : base(v.base), offset(v.offset) { }

	// Copy assignment operator
	simd_vector_simd_iterator<T, S>& operator=(const simd_vector_simd_iterator<T, S>& v)
	{
		base = v.base;
		offset = v.offset;
		return (*this);
	}
};

// Addition and subtraction operators
template< typename T, typename S>
simd_vector_simd_iterator< T, S> operator+(simd_vector_simd_iterator< T, S> a, std::ptrdiff_t b)
{
	return a += b;
}

template< typename T, typename S>
simd_vector_simd_iterator< T, S> operator+(std::ptrdiff_t b, simd_vector_simd_iterator< T, S> a)
{
	return a += b;
}

template< typename T, typename S>
simd_vector_simd_iterator< T, S> operator-(simd_vector_simd_iterator< T, S> a, std::ptrdiff_t b)
{
	return a -= b;
}


// iterator

template< typename T, typename S>
class simd_vector_iterator {
	// Static check of type parameters
	static_assert(sizeof(S) % sizeof(T) == 0, "Incompatible type parameters!");

public:
	typedef simd_vector_iterator<T, S> self;
	typedef simd_vector_simd_iterator<T, S> simd_it;

	// Random access iterator category
	typedef std::random_access_iterator_tag iterator_category;

	// Iterator types
	typedef T value_type;
	typedef std::ptrdiff_t difference_type;
	typedef T* pointer;
	typedef T& reference;

private:
	// Pointer to beginning of correspondent simd_vector
	T* base;

	// Iterator offset
	difference_type offset;

	// Constructor, should be called only from begin() and end() methods
	simd_vector_iterator(T* origin, difference_type off) : base(origin), offset(off) {  }

	template<typename U, typename K> friend class simd_vector;

public:
	// simd_iterator related methods
	simd_it lower_block() const
	{
		int k = sizeof(S) / sizeof(T);
		return simd_it(reinterpret_cast<S*>(base), offset / k);
	}

	simd_it upper_block() const
	{
		int k = sizeof(S) / sizeof(T);
		return simd_it(reinterpret_cast<S*>(base), (offset / k) + 1);
	}

	difference_type lower_offset() const
	{
		int k = sizeof(S) / sizeof(T);
		return (offset % k);
	}

	difference_type upper_offset() const
	{
		int k = sizeof(S) / sizeof(T);
		return ((offset % k) - (k - 1));
	}

	// Operator overloads
	T& operator*() const { return base[offset]; }
	T& operator[](const difference_type& n) const { return base[offset + n]; }
	bool operator==(const self& v) const { return ((base == v.base) && (offset == v.offset)); }
	bool operator!=(const self& v) const { return ((base != v.base) || (offset != v.offset)); }
	bool operator<(const self& v) const { return ((base + offset) < (v.base + v.offset)); }
	bool operator>(const self& v) const { return ((base + offset) > (v.base + v.offset)); }
	bool operator<=(const self& v) const { return ((base + offset) <= (v.base + v.offset)); }
	bool operator>=(const self& v) const { return ((base + offset) >= (v.base + v.offset)); }
	self& operator++()
	{
		++offset;
		return (*this);
	}
	self operator++(int)
	{
		self tmp = (*this);
		++offset;
		return tmp;
	}
	self& operator--()
	{
		--offset;
		return (*this);
	}
	self operator--(int)
	{
		self tmp = (*this);
		--offset;
		return tmp;
	}
	self& operator+=(const difference_type& n)
	{
		offset += n;
		return (*this);
	}
	self& operator-=(const difference_type& n)
	{
		offset -= n;
		return (*this);
	}
	difference_type operator-(const self& a) const
	{
		return (offset - a.offset);
	}

	// Default constructor
	simd_vector_iterator() : base(nullptr), offset(0) {	}

	// Copy constructor
	simd_vector_iterator(const simd_vector_iterator<T, S>& v) : base(v.base), offset(v.offset) { }

	// Copy assignment operator
	simd_vector_iterator<T, S>& operator=(const simd_vector_iterator<T, S>& v)
	{
		base = v.base;
		offset = v.offset;
		return (*this);
	}
};

// Addition and subtraction operators
template< typename T, typename S>
simd_vector_iterator< T, S> operator+( simd_vector_iterator< T, S> a, std::ptrdiff_t b)
{
	return a += b;
}

template< typename T, typename S>
simd_vector_iterator< T, S> operator+(std::ptrdiff_t b, simd_vector_iterator< T, S> a)
{
	return a += b;
}

template< typename T, typename S>
simd_vector_iterator< T, S> operator-(simd_vector_iterator< T, S> a, std::ptrdiff_t b)
{
	return a -= b;
}


// Vector class

template< typename T, typename S>
class simd_vector {
	// Static check of type parameters
	static_assert(sizeof(S) % sizeof(T) == 0, "Incompatible type parameters!");

public:
	typedef simd_vector<T, S> self;

private:
	// k = sizeof(S) / sizeof(T)
	int k;
	// Raw pointer to block of allocated data
	T* raw_block;
	// Pointer to beginning of the aligned data block
	T* aligned_begin;
	// Pointer to end of the aligned data block
	T* aligned_end;
	// Size of the container
	std::size_t content_size;

	void swap(self& other)
	{
		std::swap(k, other.k);
		std::swap(raw_block, other.raw_block);
		std::swap(aligned_begin, other.aligned_begin);
		std::swap(aligned_end, other.aligned_end);
		std::swap(content_size, other.content_size);
	}

public:
	typedef simd_vector_iterator< T, S> iterator;

	typedef simd_vector_simd_iterator<T, S> simd_iterator;

public:
	explicit simd_vector( std::size_t s)
	{
		content_size = s;

		k = sizeof(S) / sizeof(T);

		// Round size to be divisible by k (we need all bigger blocks to be allocated and accesible)
		auto rounded_count = (s % k == 0) ? s : ((s / k) + 1) * k;

#ifdef __GNUC__
		// GCC ONLY CODE

		// Directly attempting to allocate aligned memory block
		void* raw_pointer = aligned_alloc(sizeof(S), rounded_count * sizeof(T));
		if (raw_pointer == NULL)
		{
			throw alignment_exception();
		}
		aligned_begin = reinterpret_cast<T*>(raw_pointer);
		raw_block = reinterpret_cast<T*>(raw_pointer);

#else
		// MSVC AND OTHER COMPILERS CODE

		// First allocating bigger block and then performing aligning in it

		// Required size must be higher because of alignment overhead
		auto required_count = rounded_count + k;

		// Allocating space for data block
		raw_block = new T[required_count];

		// Getting aligned pointer for the data block
		void* tmp_begin = reinterpret_cast<void*>(raw_block);
		std::size_t buffer_size = required_count * sizeof(T);


		if (!std::align(sizeof(S), rounded_count * sizeof(T), tmp_begin, buffer_size))
		{
			throw alignment_exception();
		}

		aligned_begin = reinterpret_cast<T*>(tmp_begin);

#endif
		aligned_end = aligned_begin + content_size;

	}

	// Move constructor
	simd_vector(self&& v) : k(v.k), raw_block(v.raw_block), aligned_begin(v.aligned_begin), aligned_end(v.aligned_end), content_size(v.content_size)
	{
		v.raw_block = nullptr;
		v.aligned_begin = nullptr;
		v.aligned_end = nullptr;
	}

	// Move assignment operator
	simd_vector<T, S>& operator=(self&& v)
	{
		swap(v);
		return (*this);
	}

	~simd_vector()
	{
#ifdef __GNUC__
		// GCC code
		// The memory was malloc-ed
		free(raw_block);
#else
		// MSVC and other compilers code
		// The memory was new-ed
		delete raw_block;
#endif
	}

	iterator begin()
	{
		return iterator(aligned_begin, 0);
	}

	iterator end()
	{
		return iterator(aligned_begin, content_size);
	}

	std::size_t size()
	{
		return content_size;
	}
};


#endif // DU1SIMD_HPP
