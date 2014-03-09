#include "du1simd.hpp"

#include <memory>
#include <algorithm>
#include <xmmintrin.h>
#include <cassert>
#include <string>
#include <iostream>
#include <chrono>

#include <cstdint>


namespace du1simd {

	template< typename value_type, typename simd_carrier_type> 
	struct simd;

	template<>
	struct simd< float, float> {
		static float broadcast( float x)
		{
			return x;
		}
		static float zero()
		{
			return 0.0F;
		}
		static float add( float a, float b)
		{
			return a + b;
		}
		static float sub( float a, float b)
		{
			return a - b;
		}
		static float mul( float a, float b)
		{
			return a * b;
		}
		static float sum( float a)
		{
			return a;
		}
		static float mask_lower( float a, std::ptrdiff_t lgap)
		{
			assert( lgap == 0);
			return a;
		}
		static float mask_upper( float a, std::ptrdiff_t ugap)
		{
			assert( ugap == 0);
			return a;
		}
		static float mask_both( float a, std::ptrdiff_t lgap, std::ptrdiff_t ugap)
		{
			assert( lgap == 0);
			assert( ugap == 0);
			return a;
		}
	};

	template<>
	struct simd< float, __m128> {
		static __m128 broadcast( float x)
		{
			__m128 a = _mm_load_ss( & x);
			return _mm_shuffle_ps( a, a, 0x00);
		}
		static __m128 zero()
		{
			return _mm_setzero_ps();
		}
		static __m128 add( __m128 a, __m128 b)
		{
			return _mm_add_ps( a, b);
		}
		static __m128 sub( __m128 a, __m128 b)
		{
			return _mm_sub_ps( a, b);
		}
		static __m128 mul( __m128 a, __m128 b)
		{
			return _mm_mul_ps( a, b);
		}
		static float sum( __m128 a)
		{
			float x;
			__m128 b = _mm_hadd_ps( a, a);
			__m128 c = _mm_hadd_ps( b, b);
			_mm_store_ss( & x, c);
			return x;
		}

		static __m128 mask_lower( __m128 a, std::ptrdiff_t lgap)
		{
			assert( lgap >= 0);
			assert( lgap < 4);
			return _mm_and_ps( a, mask_data_.lmask_[ lgap]);
		}
		static __m128 mask_upper( __m128 a, std::ptrdiff_t ugap)
		{
			assert( ugap > -4);
			assert( ugap <= 0);
			return _mm_and_ps( a, mask_data_.umask_[ ugap + 3]);
		}
		static __m128 mask_both( __m128 a, std::ptrdiff_t lgap, std::ptrdiff_t ugap)
		{
			return mask_upper( mask_lower( a, lgap), ugap);
		}
	private:
		struct mask_data {
			__m128 lmask_[ 4];
			__m128 umask_[ 4];
			mask_data()
			{
				lmask_[ 0] = _mm_castsi128_ps( _mm_set_epi32( -1, -1, -1, -1));
				lmask_[ 1] = _mm_castsi128_ps( _mm_set_epi32( -1, -1, -1,  0));
				lmask_[ 2] = _mm_castsi128_ps( _mm_set_epi32( -1, -1,  0,  0));
				lmask_[ 3] = _mm_castsi128_ps( _mm_set_epi32( -1,  0,  0,  0));
				umask_[ 0] = _mm_castsi128_ps( _mm_set_epi32(  0,  0,  0, -1));
				umask_[ 1] = _mm_castsi128_ps( _mm_set_epi32(  0,  0, -1, -1));
				umask_[ 2] = _mm_castsi128_ps( _mm_set_epi32(  0, -1, -1, -1));
				umask_[ 3] = _mm_castsi128_ps( _mm_set_epi32( -1, -1, -1, -1));
			}
		};
		static const mask_data mask_data_;
	};

	const simd< float, __m128>::mask_data simd< float, __m128>::mask_data_;
};

namespace du1example {

	template< typename F>
	double measure_time( F f)
	{
		auto tb = std::chrono::steady_clock::now();
		f();
		auto te = std::chrono::steady_clock::now();
		return std::chrono::duration_cast<std::chrono::microseconds>( te - tb).count() / 1000000.0;
	}

	template< typename simd_carrier_type>
	struct tester
	{
		typedef simd_vector< float, simd_carrier_type> vector_type;

		typedef typename vector_type::iterator simd_iterator;

		typedef du1simd::simd< float, simd_carrier_type> simd_op;

		static float sum( simd_iterator b, simd_iterator e)
		{
			float acc = 0;

			for (; b != e; ++ b)
			{
				acc = acc + * b;
			}

			return acc;
		}

		static float simd_sum( simd_iterator b, simd_iterator e)
		{
			auto bb = b.lower_block();
			auto ee = e.upper_block();

			if ( bb == ee )
			{
				return 0;
			}

			-- ee;

			if ( bb == ee )
			{
				return simd_op::sum( simd_op::mask_both( * bb, b.lower_offset(), e.upper_offset()));
			}

			simd_carrier_type acc = simd_op::mask_lower( * bb, b.lower_offset());

			for ( ++ bb; bb != ee; ++ bb)
			{
				acc = simd_op::add( acc, * bb);
			}

			return simd_op::sum( simd_op::add( acc, simd_op::mask_upper( * bb, e.upper_offset())));
		}

		static void test( const std::string & name)
		{
#ifdef _DEBUG
			std::size_t K1 = 111, K2 = 700666, K3 = 729000;
#else
			std::size_t K1 = 111, K2 = 700000666, K3 = 729000000;
#endif
			float X1 = 0.0F, X2 = 1.00F;

			vector_type vec( K3);

			float gen = X1;

			std::generate( vec.begin(), vec.end(), [ & gen, X2](){
				return gen += X2;
			});

			auto b = vec.begin() + K1;
			auto e = vec.begin() + K2;

			float exp = (K2 - K1) * (X1 + (K2 + K1 + 1) * X2 / 2);
			float s1; 
			double t1 = measure_time( [ & s1, b, e](){
				s1 = sum( b, e);
			});
			float s2;
			double t2 = measure_time( [ & s2, b, e](){
				s2 = simd_sum( b, e);
			});

			assert( std::abs(s1 - s2) / std::abs(s1 + s2) < 0.001);
			assert( std::abs(s1 - exp) / std::abs(s1 + exp) < 0.001);

			std::cout << name << "/sum: " << (1000000000.0 * t1 / (K2-K1)) << " ns" << std::endl;
			std::cout << name << "/simd_sum: " << (1000000000.0 * t2 / (K2-K1)) << " ns" << std::endl;
		}
	};

	void test()
	{
		tester< float>::test( "float");
		tester< __m128>::test( "__m128");
	}
};


void iterator_test() {
	simd_vector<uint8_t, uint32_t> my_vector(20);
	char i = 0;
	for (auto it = my_vector.begin(); it != my_vector.end(); it++)
	{
		(*it) = i++;
	}
	
	simd_vector<uint8_t, uint32_t>::iterator::difference_type dif = (my_vector.end() - my_vector.begin());
	std::cout << dif << std::endl;

	simd_vector<uint8_t, uint32_t>::iterator my_it = my_vector.begin();

	simd_vector<uint8_t, uint32_t>::simd_iterator my_simd_it = my_it.lower_block();
	simd_vector<uint8_t, uint32_t>::simd_iterator end = my_vector.end().lower_block();

	std::cout << (my_simd_it < end) << std::endl;

	std::cout << std::hex << my_simd_it[0] << std::endl;
	std::cout << std::hex << my_simd_it[1] << std::endl;

	for (auto it = my_simd_it; it != end; it++)
	{
		std::cout << std::hex << it[0] << std::endl;
	}

	my_simd_it[0] = 0x05050505;

	for (auto it = my_vector.begin(); it != my_vector.end(); it++)
	{
		std::cout << std::dec << (uint32_t)it[0] << std::endl;
	}

	simd_vector<uint8_t, uint32_t>::iterator other_it;

	other_it = my_it + 5;

	std::cout << (uint32_t)my_it[0] << std::endl;
	std::cout << (uint32_t)(*other_it) << std::endl;

	(*other_it) = 55;
	std::cout << (uint32_t)my_it[5] << std::endl;
}

// 8B structure
struct s1 
{
	uint32_t first;
	uint32_t second;
};

// 12B structure
struct s2
{
	uint32_t first;
	uint32_t second;
	uint32_t third;
};

// Uncomment following function to test static assertion
/*
void static_assert_test()
{
	simd_vector<s1, s2> my_vector(3);

}
*/

int main(int argc, char* *argv)
{
	iterator_test();
	du1example::test();
	return 0;
}

