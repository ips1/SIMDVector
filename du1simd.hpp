#include <iterator>


template< typename T, typename S>
class simd_vector_iterator {
public:
	/*...*/
};

template< typename T, typename S>
simd_vector_iterator< T, S> operator+( simd_vector_iterator< T, S> a, std::ptrdiff_t b)
{
	return a += b;
}

/*...*/

template< typename T, typename S>
class simd_vector {
public:
	typedef simd_vector_iterator< T, S> iterator;

	typedef /*...*/ simd_iterator;

public:
	explicit simd_vector( std::size_t s)
	{
		/*...*/
	}

	iterator begin()
	{
		/*...*/
	}

	iterator end()
	{
		/*...*/
	}

	std::size_t size()
	{
		/*...*/
	}

	/*...*/
};
