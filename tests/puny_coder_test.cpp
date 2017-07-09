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

#define BOOST_TEST_MODULE puny_coder_test 

#include <boost/test/unit_test.hpp>
#include <iostream>

#include <daw/char_range/daw_char_range.h>
#include <daw/daw_parser_helper.h>
#include <daw/daw_string_view.h>
#include <daw/json/daw_json_link.h>
#include <daw/json/daw_json_link_file.h>

#include "puny_coder.h"

struct puny_tests_t : public daw::json::daw_json_link<puny_tests_t> {
	struct puny_test_t : public daw::json::daw_json_link<puny_test_t> {
		std::string in;
		std::string out;

		static void json_link_map( ) {
			link_json_string( "in", in );
			link_json_string( "out", out );
		}
	}; // puny_test_t

	std::vector<puny_test_t> tests;

	static void json_link_map( ) {
		link_json_object_array( "tests", tests );
	}
}; // puny_tests_t

bool test_puny_encode( puny_tests_t::puny_test_t test_case ) {
	std::cout << "Testing: " << test_case.in << " Expecting: " << test_case.out << " Got: ";
	auto result = daw::to_puny_code( test_case.in );
	std::cout << result << std::endl;
	return result == test_case.out;
}

BOOST_AUTO_TEST_CASE( punycode_test_encode ) {
	std::cout << "PunyCode Encoding\n";
	auto config_data = daw::json::from_file<puny_tests_t>( "../puny_coder_tests.json" ).result;
	for( auto const & puny : config_data.tests ) {
		BOOST_REQUIRE( test_puny_encode( puny ) );
	}
	std::cout << std::endl;
}

bool equal_nc( std::u32string lhs, std::u32string rhs ) {
	return std::equal( lhs.begin( ), lhs.end( ), rhs.begin( ), rhs.end( ), []( auto l, auto r ) {
		auto n = daw::parser::in_range( l, 'A', 'Z' ) ? l | 0x20 : l;
		auto m = daw::parser::in_range( r, 'A', 'Z' ) ? r | 0x20 : r;
		return n == m;
	});
}

std::u32string to_u32string( daw::string_view value ) {
	auto tmp = daw::range::create_char_range( value.begin( ), value.end( ) );
	return tmp.to_u32string( );
}

bool test_puny_decode( puny_tests_t::puny_test_t test_case ) {
	std::cout << "Testing: " << test_case.out << " Expecting: " << test_case.in << " Got: ";
	auto result = daw::from_puny_code( test_case.out );
	std::cout << result << std::endl;
	return equal_nc( to_u32string( result ), to_u32string( test_case.in ) );
}

BOOST_AUTO_TEST_CASE( punycode_test_decode ) {
	std::cout << "PunyCode Decoding\n";
	auto config_data = daw::json::from_file<puny_tests_t>( "../puny_coder_tests.json" ).result;
	for( auto const & puny : config_data.tests ) {
		BOOST_REQUIRE( test_puny_decode( puny ) );
	}
	std::cout << std::endl;
}

