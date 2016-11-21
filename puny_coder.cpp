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
#include <daw/daw_parser_helper.h>

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
			static size_t const PREFIX_SIZE = 4;
			static auto const DELIMITER = '-';
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

		bool begins_with_prefix( daw::range::CharRange const & input ) {
			static std::string const prefix = constants::PREFIX;
			return daw::parser::starts_with( input.begin( ), input.end( ), prefix.begin( ), prefix.end( ), []( auto c1, auto c2 ) {
				return to_lower( c1 ) == to_lower( c2 );
			} );
		}

		template<typename T>
		size_t decode_to_value( T value ) {
			if( daw::parser::in_range( value, 'a', 'z' ) ) {
				return value - 'a';
			} else if( daw::parser::in_range( value, 'A', 'Z' ) ) {
				return value - 'A';
			} else if( daw::parser::in_range( value, '0', '9' ) ) {
				return (value - '0') + 26;
			}
			throw std::runtime_error( "Unexpected character provided" );
		}

		std::u32string decode_part( daw::range::CharRange u8input ) {
			if( u8input.size( ) < 1 || u8input.size( ) > 63 ) {
				throw std::runtime_error( "The size of the part must be between 1 and 63 inclusive" );
			}
			if( !begins_with_prefix( u8input ) ) {
				std::u32string result;
				std::copy( u8input.begin( ), u8input.end( ), std::back_inserter( result ) );
				return result;
			} else {
				static std::string const prefix = constants::PREFIX;
				u8input.advance( prefix.size( ) );
			}
			auto input = u8input.to_u32string( );

			auto basic_rng = daw::parser::until_last_of( input.begin( ), input.end( ), U'-' );

			std::u32string output;
			if( basic_rng ) {
				std::transform( basic_rng.begin( ), basic_rng.end( ), std::back_inserter( output ), []( auto c ) {
					return static_cast<char32_t>( c );
				} );
			}

			auto n = constants::INITIAL_N;
			auto bias = constants::INITIAL_BIAS;

			size_t b = basic_rng ? 1 + static_cast<size_t>(std::distance( basic_rng.begin( ), basic_rng.end( ) )) : 0;

			for( size_t i=0; b < input.size( ); ++i ) {
				auto original_i = i;
				size_t w = 1;
				for( auto k = constants::BASE; ; k += constants::BASE ) {
					auto d = decode_to_value( input[b++] );

					i += d*w;
					
					auto t = calculate_threshold( k, bias );
					if( d < t ) {
						break;
					}
					w *= constants::BASE - t;
				}
				auto x = output.size( ) + 1;
				bias = adapt( i - original_i, x, 0 == original_i );

				n += i/x;

				i %= x;
				output.insert( i, 1, static_cast<char32_t>( n ) );
			}
			return output;
		}

		template<typename Delemiter>
		std::vector<boost::string_view> split( boost::string_view input, Delemiter delemiter ) {
			std::vector<boost::string_view> result;
			auto pos = input.find_first_of( delemiter );
			while( !input.empty( ) && boost::string_view::npos != pos ) {
				result.emplace_back( input.data( ), pos );
				input = input.substr( pos + 1, boost::string_view::npos );
				pos = input.find_first_of( delemiter );
			}
			result.push_back( input );
			return result;
		}
	}    // namespace anonymous

	std::string to_puny_code( boost::string_view input ) {
		std::stringstream ss;
		auto parts = split( input, '.' );
		bool is_first = true;
		for( auto const & part : parts ) {
			if( !is_first ) {
				ss << '.';
			} else {
				is_first = false;
			}
			if( !part.empty( ) ) {
				ss << encode_part( daw::range::create_char_range( part.begin( ), part.end( ) ) );
			}
		}
		return ss.str( );
	}

	std::string from_puny_code( boost::string_view input ) {
		auto parts = split( input, '.' );
		bool is_first = true;
		std::stringstream ss;
		for( auto const & part: parts ) {
			if( !is_first ) {
				ss << '.';
			} else {
				is_first = false;
			}
			if( !part.empty( ) ) {
				auto u32str = decode_part( daw::range::create_char_range( part.begin( ), part.end( ) ) );
				ss << daw::from_u32string( u32str );
			}
		}
		return ss.str( );
	}
}    // namespace daw

