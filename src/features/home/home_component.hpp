#pragma once

#include <drogular/component.hpp>

#include <string>

// Introductory component for the home page.
class HomeComponent final : public drogular::TemplateComponent
{
public:
    static constexpr auto tag = "Home";

    std::string templatePath() const override
    {
        return "components/home.html";
    }
};
