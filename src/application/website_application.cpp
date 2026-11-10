#include "application/website_application.hpp"
#include "application/website_options.hpp"
#include "features/home/home_routes.hpp"

#include <drogular/app.hpp>
#include <drogular/static_file_cache_profile.hpp>

namespace website {

void configureApplication(drogular::App& app, const WebsiteOptions& options) {
    // App::run() currently listens on all interfaces. Explicitly disable
    // Developer Tools even in development, so accidental network exposure
    // cannot expose the inspection and diagnostics endpoints.
    app.disableDeveloperTools();

    if (options.environment == Environment::Production) {
        app.profile(drogular::ApplicationProfile::Production);
        app.templateCache(true);
        app.staticFileCacheProfile(drogular::StaticFileCacheProfile::Production);
    } else {
        app.profile(drogular::ApplicationProfile::Development);
        app.templateCache(false);
        app.staticFileCacheProfile(drogular::StaticFileCacheProfile::Development);
    }

    app.templateRoot("templates");
    app.staticFiles("/assets", "public");
    registerHomeRoutes(app);
}

} // namespace website
