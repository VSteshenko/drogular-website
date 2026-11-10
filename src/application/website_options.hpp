#pragma once

#include <iosfwd>
#include <string_view>

namespace website {

enum class Environment {
    Development,
    Production
};

enum class StartupCommand {
    Run,
    Help,
    Version
};

struct WebsiteOptions {
    unsigned short port = 8080;
    Environment environment = Environment::Development;
    StartupCommand command = StartupCommand::Run;
};

// Environment values are supplied by the caller so parsing is deterministic
// and can be exercised without modifying process-global environment variables.
WebsiteOptions parseWebsiteOptions(
    int argc,
    char* const argv[],
    std::string_view environmentPort = {},
    std::string_view environmentName = {}
);

std::string_view environmentToString(Environment environment);
void printWebsiteHelp(std::ostream& output);

} // namespace website
