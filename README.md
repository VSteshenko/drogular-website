# Drogular Website

Official website for [Drogular](https://github.com/VSteshenko/drogular).
This is an **independent application** built using Drogular's public API.

## Status

Website **0.1**, Stage A.2 — application foundation. The home page is
intentionally a placeholder; layout, documentation, demos, and Playground
are implemented in later stages.

## Requirements

- CMake 3.24 or newer and a C++20 compiler
- Drogon installed and discoverable by CMake (`find_package(Drogon REQUIRED)`)
- Git access during the first configure, to retrieve Drogular

CMake pins Drogular to the `v0.25.0` Git tag. Website changes are deployed
independently from framework releases; upgrades require a deliberate tag change.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

## Run

Run from the **repository root**: template and static asset paths are currently
relative to the process working directory.

```bash
./build/drogular_website
```

Open <http://localhost:8080/>. Static files in `public/` are served from
`/assets`.

## Source layout

```text
src/
  main.cpp                             Process entry point only
  application/
    website_application.hpp/.cpp      Shared setup and feature registration
  features/
    home/
      home_routes.hpp/.cpp            Home route + component registration
      home_page.hpp                   Home page implementation
      home_component.hpp              Home-specific component
templates/
  pages/home.html                     Home page markup
  components/home.html                Home component markup
public/                                Static resources served at /assets
```

New sections (`documentation`, `examples`, `playground`, `releases`) should
register their routes in their own feature modules rather than adding logic to
`main.cpp`. Shared layout components will be introduced in Stage B.

## Local smoke check

After the app starts, run:

```bash
curl --fail --silent --show-error http://localhost:8080/ | grep 'Hello Drogular'
```

This confirms that the root page is routed and rendered. Full tests and CI
will be added in subsequent foundation and release stages.
