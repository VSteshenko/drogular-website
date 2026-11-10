#include "application/website_application.hpp"
#include "application/website_options.hpp"

#include <drogular/app.hpp>
#include <drogular/developer_tools/diagnostics_page.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string_view>

namespace {

bool registeredRoute(const drogular::App& app, std::string_view path) {
    const auto inspection = app.inspect();
    return std::any_of(inspection.routes.begin(), inspection.routes.end(),
        [path](const auto& route) {
            return route.path == path;
        });
}

TEST(WebsiteApplicationTests, DevelopmentConfiguresRoutesAndDisablesCaching) {
    drogular::App app;
    website::WebsiteOptions options;
    options.environment = website::Environment::Development;
    website::configureApplication(app, options);

    EXPECT_EQ(app.profile(), drogular::ApplicationProfile::Development);
    EXPECT_EQ(app.options().templateRoot(), std::filesystem::path("templates"));
    EXPECT_FALSE(app.options().templateCacheEnabled());
    EXPECT_FALSE(app.options().staticFileCacheEnabled());
    ASSERT_EQ(app.options().staticFiles().size(), 1U);
    EXPECT_EQ(app.options().staticFiles()[0].routePrefix, "/assets");
    EXPECT_EQ(app.options().staticFiles()[0].directory, std::filesystem::path("public"));
    EXPECT_TRUE(registeredRoute(app, "/"));
    EXPECT_FALSE(registeredRoute(app, drogular::DiagnosticsPage::Path));
}

TEST(WebsiteApplicationTests, ProductionUsesCacheAndDisablesDeveloperTools) {
    drogular::App app;
    website::WebsiteOptions options;
    options.environment = website::Environment::Production;
    website::configureApplication(app, options);

    EXPECT_EQ(app.profile(), drogular::ApplicationProfile::Production);
    EXPECT_TRUE(app.options().templateCacheEnabled());
    EXPECT_TRUE(app.options().staticFileCacheEnabled());
    EXPECT_TRUE(app.options().staticFileEtagEnabled());
    EXPECT_TRUE(app.options().staticFileLastModifiedEnabled());
    EXPECT_EQ(app.options().staticFileCacheMaxAge(), std::chrono::hours(24));
    EXPECT_TRUE(registeredRoute(app, "/"));
    EXPECT_FALSE(registeredRoute(app, drogular::DiagnosticsPage::Path));
}

} // namespace
