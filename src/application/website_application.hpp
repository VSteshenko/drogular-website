#pragma once

namespace drogular {

class App;

}

namespace website {

struct WebsiteOptions;

// Configures caching, resources, and feature routes for one environment.
void configureApplication(drogular::App& app, const WebsiteOptions& options);

} // namespace website
