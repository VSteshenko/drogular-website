#include "application/website_application.hpp"

#include <drogular/app.hpp>

int main()
{
    drogular::App app;
    website::configureApplication(app);
    app.run(8080);
    return 0;
}
