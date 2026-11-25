# WebsiteEngineeringSite — Workflow

## What it is

WebsiteEngineeringSite is a C++ engine that builds websites and programs
for customers. A customer describes what they need, the Architect character
turns the description into a brief, and the builder engines generate the
real site files (one HTML page per page) and program scaffolds.

Systems: HomeEngine, BriefEngine (customer briefs), SiteBuilderEngine
(generates HTML sites), ProgramBuilderEngine (generates program scaffolds),
ArchitectCharacter, ProjectMemory (`website_projects.txt`), EventLogger
(`website_events.txt`). Menu 1–7, 0 Exit. Option 7 runs the Rosewood Bakery
demo; option 1 takes a new customer brief; option 3 builds the site;
option 4 builds the program.

## Workflow — `.github/workflows/website.yml`

- Triggers: push / pull request to `master` touching the code, the check
  script, or the workflow file; plus a daily run at 06:00 UTC.
- Job `build-and-test` on `windows-latest`:
  1. Checks out the repo.
  2. Builds the project in Debug x64 with MSBuild (no `|| echo` — a
     failed build fails the run).
  3. Runs the smoke test: feeds the demo session (demo, list projects,
     exit) to `x64\Debug\WebsiteEngineeringSite.exe` and fails unless the
     exe exits 0, prints `[HomeEngine] Initialized`, reports
     `Site built for 'Rosewood Bakery': 4 pages`, and the generated
     `rosewood_bakery_home.html` exists and contains `Rosewood Bakery`.

## Local check

From the repo root:

```sh
sh scripts/website-check.sh
```

It compiles with `g++ -std=c++17 -Wall -Wextra`, runs a scripted session
(demo + a custom Harper Gym shop brief), verifies the output, the
generated HTML/program files, the theme colour, and the project memory,
then cleans up the generated files. It also compiles and runs the
generated program scaffold.

## Adding the workflow

Muse cannot push workflow files through the GitHub API. Add
`.github/workflows/website.yml` via the GitHub web UI (repo
Snjackson025/WebsiteEngineeringSite, branch `master`, path
`.github/workflows/website.yml`), pasting the file from this repo, then
verify the run on the Actions tab.
