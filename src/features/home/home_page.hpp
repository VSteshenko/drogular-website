#pragma once

#include <drogular/page.hpp>

#include <string>

// Home page for the initial website foundation.
class HomePage final : public drogular::TemplatePage
{
public:
    std::string templatePath() const override
    {
        return "pages/home.html";
    }
};
