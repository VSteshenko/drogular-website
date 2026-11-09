#include "application/website_application.hpp"

#include "features/home/home_routes.hpp"

#include <drogular/app.hpp>

namespace website {

void configureApplication(drogular::App& app)
{
    app.templateRoot("templates");
    app.staticFiles("/assets", "public");

    registerHomeRoutes(app);
}

} // namespace website
