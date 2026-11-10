#include "application/website_options.hpp"

#include <gtest/gtest.h>

#include <initializer_list>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

website::WebsiteOptions parse(
    std::initializer_list<const char*> args,
    std::string_view environmentPort = {},
    std::string_view environmentName = {}
) {
    std::vector<std::string> stored(args.begin(), args.end());
    std::vector<char*> pointers;
    pointers.reserve(stored.size());
    for (auto& item : stored) {
        pointers.push_back(item.data());
    }
    return website::parseWebsiteOptions(
        static_cast<int>(pointers.size()), pointers.data(),
        environmentPort, environmentName
    );
}

TEST(WebsiteOptionsTests, UsesDevelopmentDefaults) {
    const auto options = parse({"website"});
    EXPECT_EQ(options.port, 8080);
    EXPECT_EQ(options.environment, website::Environment::Development);
    EXPECT_EQ(options.command, website::StartupCommand::Run);
}

TEST(WebsiteOptionsTests, ReadsEnvironmentVariables) {
    const auto options = parse({"website"}, "9090", "production");
    EXPECT_EQ(options.port, 9090);
    EXPECT_EQ(options.environment, website::Environment::Production);
}

TEST(WebsiteOptionsTests, ArgumentsOverrideEnvironmentVariables) {
    const auto options = parse(
        {"website", "--port", "10001", "-e", "development"},
        "9090", "production"
    );
    EXPECT_EQ(options.port, 10001);
    EXPECT_EQ(options.environment, website::Environment::Development);
}

TEST(WebsiteOptionsTests, AcceptsValidPortsAndAliases) {
    EXPECT_EQ(parse({"website", "-p", "1"}).port, 1);
    EXPECT_EQ(parse({"website", "-p", "65535"}).port, 65535);
    EXPECT_EQ(parse({"website", "-h"}).command, website::StartupCommand::Help);
    EXPECT_EQ(parse({"website", "--help"}).command, website::StartupCommand::Help);
    EXPECT_EQ(parse({"website", "-v"}).command, website::StartupCommand::Version);
    EXPECT_EQ(parse({"website", "--version"}).command, website::StartupCommand::Version);
    EXPECT_EQ(website::environmentToString(website::Environment::Production), "production");
}

TEST(WebsiteOptionsTests, RejectsInvalidPorts) {
    for (const char* value : {"0", "65536", "abc", "12abc", "-1", "+22", ""}) {
        SCOPED_TRACE(value);
        EXPECT_THROW(parse({"website", "--port", value}), std::invalid_argument);
    }
    EXPECT_THROW(parse({"website"}, "0"), std::invalid_argument);
}

TEST(WebsiteOptionsTests, RejectsMissingAndInvalidArguments) {
    EXPECT_THROW(parse({"website", "-p"}), std::invalid_argument);
    EXPECT_THROW(parse({"website", "--env"}), std::invalid_argument);
    EXPECT_THROW(parse({"website", "--env", "staging"}), std::invalid_argument);
    EXPECT_THROW(parse({"website", "--unknown"}), std::invalid_argument);
    EXPECT_THROW(parse({"website"}, "123", "staging"), std::invalid_argument);
}

} // namespace
