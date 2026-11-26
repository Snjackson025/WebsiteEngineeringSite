# WebsiteEngineeringSite

**A website-building engine: a customer describes the site they need, and
an AI character called Architect builds it for them.**

## What it does

- **Customer briefs** — business name, site type (Business, Portfolio, Shop, Blog), tagline, theme, pages, and features
- **Site builder** — generates real HTML pages (header, navigation, hero, features, footer) in the chosen theme
- **Program builder** — writes a C++ program scaffold and README for the project
- **Architect** — the AI character that confirms the brief and presents the finished site or program
- **Project memory** — every build is recorded, so past projects can be listed

Menu: `1` describe a site, `2` show the brief, `3` build the site,
`4` build a program, `5` list projects, `6` talk to Architect,
`7` run the demo, `0` exit. The demo builds a full site for
Rosewood Bakery. `WebsiteEngineeringSite.exe --demo` runs the demo with
no keyboard input.

## Build and run

Requires C++17. With Visual Studio, open the solution and build
Debug x64. Or with g++:

```sh
g++ -std=c++17 -Wall -Wextra -o WebsiteEngineeringSite WebsiteEngineeringSite.cpp
./WebsiteEngineeringSite --demo
```

## Continuous integration

`.github/workflows/website.yml` builds Debug x64 with MSBuild and runs
the demo, checking that the site files are written. It also runs daily
at 06:00 UTC.
