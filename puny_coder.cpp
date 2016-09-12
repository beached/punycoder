// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include <sstream>

#include <daw/char_range/daw_char_range.h>

#include "puny_coder.h"

namespace daw {
	namespace {
		struct constants final {
			static size_t const BASE = 36;
			static size_t const TMIN = 1;
			static size_t const TMAX = 26;
			static size_t const SKEW = 38;
			static size_t const DAMP = 700;
			static size_t const INITIAL_BIAS = 72;
			static size_t const INITIAL_N = 128;
			constexpr static auto const PREFIX = "xn--";
			static char const DELIMITER = '-';
		};	// constants

		template<typename CP>
		auto to_lower( CP cp ) {
			return cp | 32;
		}

		template<typename T, typename U>
		auto adapt( T delta, U n_points, bool is_first ) {
			// scale back, then increase delta
			delta /= is_first ? constants::DAMP : 2;
			delta += delta / n_points;

			auto const s = constants::BASE - constants::TMIN;
			auto const t = (s * constants::TMAX)/2;

			uint32_t k = 0;
			for( ; delta > t; k += constants::BASE ) {
				delta /= s;
			}

			auto const a = (constants::BASE - constants::TMIN + 1) * delta;
			auto const b = (delta + constants::SKEW);

			return k + (a / b);
		}


		template<typename Iterator>
		auto sort_uniq( Iterator first, Iterator last ) {
			using value_type = std::decay_t<decltype(*first)>;
			std::vector<value_type> result;
			std::copy( first, last, std::back_inserter( result ) );
			std::sort( result.begin( ), result.end( ), std::greater<value_type>( ) );
			auto new_end = std::unique( result.begin( ), result.end( ) );
			result.erase( new_end, result.end( ) );
			return result;
		}


		template<typename T, typename U>
		auto calculate_threshold( T k, U bias ) {
			if( k <= bias + constants::TMIN ) {
				return constants::TMIN;
			} else if( k >= bias + constants::TMAX ) {
				return constants::TMAX;
			}
			return k - bias;
		}

		template<typename T>
		auto encode_digit( T d ) {
			if( d < 26 ) {
				return d + 97;
			}
			return d + 22;
		}

		template<typename T, typename U>
		auto encode_int( T bias, U delta ) {
			std::string result;
		
			auto k = constants::BASE;
			auto q = delta;

			while( true ) {
				auto t = calculate_threshold( k, bias );
				if( q < t ) {
					result += encode_digit( q );
					break;
				} else {
					result += encode_digit( t + ((q - t) % (constants::BASE - t)) );
					q = (q - t)/(constants::BASE - t);
				
				}
				k += constants::BASE;
			}

			return result;
		}

		std::string encode_part( daw::range::CharRange input ) {
			std::string output;
			std::vector<uint32_t> non_basic;
			
			for( auto c : input ) {
				if( c < 128 ) {
					output += static_cast<char>( to_lower( c ) );
				} else {
					non_basic.push_back( c );
				}
			}

			if( non_basic.empty( ) ) {
				return output;
			}


			auto b = output.size( );
			auto h = b;

			if( !output.empty( )) {
				output += constants::DELIMITER;
			}

			auto n = constants::INITIAL_N;
			auto bias = constants::INITIAL_BIAS;
			uint32_t delta = 0;

			non_basic = sort_uniq( non_basic.begin( ), non_basic.end( ) );

			for( auto len = input.size( ); h < len; ++n, ++delta ) {
				auto m = non_basic.back( );
				non_basic.pop_back( );

				delta += (m - n) * (h + 1);
				n = m;

				for( auto it = input.begin( ); it != (input.begin( ) + len); ++it ) {
					if( *it < n && ++delta == 0 ) {
						throw std::runtime_error( "delta overflow" );
					} else if( *it == n ) {
						output += encode_int( bias, delta );
						bias = adapt( delta, h + 1, b == h );
						delta = 0;
						++h;
					}
				}
			}
			return constants::PREFIX + output;
		}

		template<typename Delemiter>
		std::vector<boost::string_ref> split( boost::string_ref input, Delemiter delemiter ) {
			std::vector<boost::string_ref> result;
			auto pos = input.find_first_of( delemiter );
			while( !input.empty( ) && boost::string_ref::npos != pos ) {
				result.emplace_back( input.data( ), pos );
				input = input.substr( pos + 1, boost::string_ref::npos );
				pos = input.find_first_of( delemiter );
			}
			result.push_back( input );
			return result;
		}
	}    // namespace anonymous

	std::string to_puny_code( boost::string_ref input ) {
		std::stringstream ss;
		auto parts = split( input, '.' );
		bool is_first = true;
		for( auto const & part : parts ) {
			if( !is_first ) {
				ss << ".";
			} else {
				is_first = false;
			}
			if( !part.empty( ) ) {
				ss << encode_part( daw::range::create_char_range( part.begin( ), part.end( )));
			}
		}
		return ss.str( );
	}
}    // namespace daw

