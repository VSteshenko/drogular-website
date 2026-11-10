#include "support/website_test_support.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <string_view>
#include <thread>

namespace {

void verifyHttp(std::string_view mode) {
    const auto port = website::test::availableLoopbackPort();
    website::test::ServerProcess server(port, mode);

    std::optional<website::test::HttpResponse> homepage;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(15);
    while (std::chrono::steady_clock::now() < deadline) {
        ASSERT_TRUE(server.running());
        homepage = website::test::httpGet(port, "/");
        if (homepage.has_value()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    ASSERT_TRUE(homepage.has_value()) << "HTTP server did not respond: " << mode;
    ASSERT_EQ(homepage->status, 200);
    EXPECT_NE(homepage->body.find("Hello Drogular"), std::string::npos);
    EXPECT_NE(homepage->body.find("Your Drogular application is running."),
              std::string::npos);

    const auto missing = website::test::httpGet(port, "/missing-website-page");
    ASSERT_TRUE(missing.has_value());
    EXPECT_EQ(missing->status, 404);
}

TEST(WebsiteHttpTests, DevelopmentServesHomepageAndMissingRoute) {
    verifyHttp("development");
}

TEST(WebsiteHttpTests, ProductionServesHomepageAndMissingRoute) {
    verifyHttp("production");
}

TEST(WebsiteCliTests, HelpPrintsUsage) {
    const auto result = website::test::runCli({"--help"});
    EXPECT_EQ(result.exitCode, 0) << result.output;
    EXPECT_NE(result.output.find("Usage: drogular_website"), std::string::npos);
}

TEST(WebsiteCliTests, VersionPrintsProjectVersion) {
    const auto result = website::test::runCli({"--version"});
    EXPECT_EQ(result.exitCode, 0) << result.output;
    EXPECT_NE(result.output.find("Drogular Website " WEBSITE_TEST_VERSION),
              std::string::npos);
}

TEST(WebsiteCliTests, InvalidPortExitsWithError) {
    const auto result = website::test::runCli({"--port", "0"});
    EXPECT_NE(result.exitCode, 0);
    EXPECT_NE(result.output.find("Invalid port"), std::string::npos);
}

} // namespace
