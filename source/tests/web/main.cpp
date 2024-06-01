#define CATCH_CONFIG_MAIN

#include "tests.h"

#include <catch.hpp>

class DefaultSession : public Catch::Session 
{
public:
    DefaultSession() 
    {
        Tests::SetIsTestMode(true);
    }
};

int main(int argc, char* argv[]) 
{
    // Use the custom session to initialize the variable
    DefaultSession session;
    int result = session.applyCommandLine(argc, argv);
    if (result != 0) // Indicates a command line error
    {
        return result;
    }
    
    return session.run();
}