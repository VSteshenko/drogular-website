#include "features/home/home_routes.hpp"

#include "features/home/home_component.hpp"
#include "features/home/home_page.hpp"

#include <drogular/app.hpp>

namespace website {

void registerHomeRoutes(drogular::App& app)
{
    app.component<HomeComponent>();
    app.page<HomePage>("/");
}

} // namespace website
