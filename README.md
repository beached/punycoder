# punycoder
Puny enoder/decoder in C++

Currently only encode is complete.  To use call to_puny_code( ... ) with a variable that is a string or a char * that has the domain name to encode.
#Example
    std::string domain1 = "Bücher.ch";
    char const * domain2 = "example.com";
    
    auto enc1 = to_puny_code( domain1 );
    auto enc2 = to_puny_code( domain2 );
    auto enc3 = to_puny_code( "Bücher.ch" );
