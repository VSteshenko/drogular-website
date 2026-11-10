#include "application/website_application.hpp"
#include "application/website_options.hpp"
#include "website_version.hpp"

#include <drogular/app.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string_view>

namespace {

std::string_view variable(const char* name) {
    const char* value = std::getenv(name);
    return value != nullptr ? std::string_view(value) : std::string_view{};
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        const auto options = website::parseWebsiteOptions(
            argc,
            argv,
            variable("DROGULAR_WEBSITE_PORT"),
            variable("DROGULAR_WEBSITE_ENV")
        );

        if (options.command == website::StartupCommand::Help) {
            website::printWebsiteHelp(std::cout);
            return EXIT_SUCCESS;
        }
        if (options.command == website::StartupCommand::Version) {
            std::cout << "Drogular Website " << DROGULAR_WEBSITE_VERSION << '\n';
            return EXIT_SUCCESS;
        }

        drogular::App app;
        website::configureApplication(app, options);
        std::cout << "Drogular Website " << DROGULAR_WEBSITE_VERSION
                  << " [" << website::environmentToString(options.environment)
                  << "] listening on port " << options.port << std::endl;
        app.run(options.port);
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << "drogular_website: " << error.what() << '\n';
        std::cerr << "Use --help to see available options.\n";
        return EXIT_FAILURE;
    }
}
