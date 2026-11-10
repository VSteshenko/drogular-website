#include "application/website_options.hpp"

#include <charconv>
#include <ostream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace website {
namespace {

unsigned short parsePort(std::string_view value) {
    unsigned int port = 0;
    const auto result = std::from_chars(
        value.data(), value.data() + value.size(), port
    );
    if (value.empty() || result.ec != std::errc{} ||
        result.ptr != value.data() + value.size() ||
        port == 0 || port > 65535
    ) {
        throw std::invalid_argument("Invalid port: " + std::string(value));
    }
    return static_cast<unsigned short>(port);
}

Environment parseEnvironment(std::string_view value) {
    if (value == "development") {
        return Environment::Development;
    }
    if (value == "production") {
        return Environment::Production;
    }
    throw std::invalid_argument(
        "Invalid environment: " + std::string(value) +
        " (expected development or production)"
    );
}

} // namespace

WebsiteOptions parseWebsiteOptions(
    int argc,
    char* const argv[],
    std::string_view environmentPort,
    std::string_view environmentName
) {
    WebsiteOptions options;
    if (!environmentPort.empty()) {
        options.port = parsePort(environmentPort);
    }
    if (!environmentName.empty()) {
        options.environment = parseEnvironment(environmentName);
    }

    for (int index = 1; index < argc; ++index) {
        const std::string_view argument(argv[index]);

        if (argument == "-h" || argument == "--help") {
            options.command = StartupCommand::Help;
            continue;
        }
        if (argument == "-v" || argument == "--version") {
            options.command = StartupCommand::Version;
            continue;
        }
        if (argument == "-p" || argument == "--port" ||
            argument == "-e" || argument == "--env") {
            if (index + 1 >= argc) {
                throw std::invalid_argument("Missing value after " + std::string(argument));
            }
            const std::string_view value(argv[++index]);
            if (argument == "-p" || argument == "--port") {
                options.port = parsePort(value);
            } else {
                options.environment = parseEnvironment(value);
            }
            continue;
        }
        throw std::invalid_argument("Unknown argument: " + std::string(argument));
    }
    return options;
}

std::string_view environmentToString(Environment environment)
{
    return environment == Environment::Production
        ? "production" : "development";
}

void printWebsiteHelp(std::ostream& output)
{
    output
        << "Drogular Website\n\n"
        << "Usage: drogular_website [options]\n\n"
        << "Options:\n"
        << "  -p, --port <1-65535>       HTTP port (default: 8080)\n"
        << "  -e, --env <mode>           development | production\n"
        << "  -v, --version              Print website version\n"
        << "  -h, --help                 Print this help\n\n"
        << "Environment variables (overridden by CLI flags):\n"
        << "  DROGULAR_WEBSITE_PORT     HTTP port\n"
        << "  DROGULAR_WEBSITE_ENV      development | production\n\n"
        << "Run from the repository root so templates/ and public/ resolve.\n";
}

} // namespace website
