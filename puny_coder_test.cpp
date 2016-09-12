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

#include <daw/json/daw_json_link.h>

#include "puny_coder.h"

struct puny_tests_t: public daw::json::JsonLink<puny_tests_t> {
	struct puny_test_t: public daw::json::JsonLink<puny_test_t> {
		std::string in;
		std::string out;

		puny_test_t( ):
				daw::json::JsonLink<puny_test_t>{ },
				in{ },
				out{ } {

			link_string( "in", in );
			link_string( "out", out );
		}
	};	// puny_test_t

	std::vector<puny_test_t> tests;

	puny_tests_t( ):
			daw::json::JsonLink<puny_tests_t>{ },
			tests{ } {

		link_array( "tests", tests );
	}
};	// puny_tests_t

bool test_puny( puny_tests_t::puny_test_t test_case ) {
	std::cout << "Testing: " << test_case.in << " Expecting: " << test_case.out << " Got: ";
	auto result = daw::to_puny_code( test_case.in );
	std::cout << result << std::endl;
	return result == test_case.out;
}

BOOST_AUTO_TEST_CASE( punycode_test ) {
	std::cout << "PunyCode\n";
	auto config_data = puny_tests_t{ }.decode_file( "../puny_tests.json" );
	for( auto const & puny : config_data.tests ) {
		BOOST_REQUIRE( test_puny( puny ) );
	}
	std::cout << std::endl;
}
