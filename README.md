# punycoder
Puny enoder/decoder in C++

To use, call the to_puny_code or from_puny_code functions with a variable that is a string or a char * that has the domain name to encode.
#Example
    std::string domain1 = "Bücher.ch";
    char const * domain2 = "example.com";
    
    auto enc1 = daw::to_puny_code( domain1 );
    auto enc2 = daw::to_puny_code( domain2 );
    auto enc3 = daw::to_puny_code( "Bücher.ch" );
    
    auto dec1 = daw::from_puny_code( enc1 );
    
