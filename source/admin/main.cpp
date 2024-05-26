#include "config.h"
#include "mongo.h"

int main(int argc, char** argv)
{
    Options options;
    Config::LoadOrCreateConfig(options);
    if (!Config::ParseCommandLineOptions(argc, argv, options, Config::ParsingType::Admin))
    {
        return -1;
    }

    if (!options.admin.userToCreate.empty() || !options.admin.userPassword.empty())
    {
        if (options.admin.userToCreate.empty())
        {
            std::cerr << "error: Username was not provided" << std::endl;
        }
        else if (options.admin.userPassword.empty())
        {
            std::cerr << "error: Password was not provided" << std::endl;
        }
    }

    if (options.admin.direct)
    {
        if (!options.admin.userToCreate.empty() && !options.admin.userPassword.empty())
        {
            User user;
            user.SetPassword(options.admin.userPassword);
            user.m_Username = options.admin.userToCreate;
            user.m_IsAdmin = options.admin.userIsAdmin;

            Mongo mongo(options);
            mongo.CreateUser(user);
        }
    }

    return 0;
}