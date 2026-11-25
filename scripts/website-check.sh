#!/bin/sh
# website-check.sh — local check for WebsiteEngineeringSite.
# Compiles the engine, runs a scripted session (demo + custom brief),
# and verifies the output and the generated site/program files.
# Run from the repo root: sh scripts/website-check.sh
set -e
cd "$(dirname "$0")/.."

CXX="${CXX:-g++}"
echo "== compile =="
"$CXX" -std=c++17 -Wall -Wextra -o /tmp/wes_check WebsiteEngineeringSite.cpp
echo "compile OK"

rm -f website_projects.txt website_events.txt rosewood_bakery_*.html rosewood_bakery_program* harper_gym_*.html harper_gym_program*

echo "== run scripted session =="
printf '7\n1\nHarper Gym\nShop\nTrain hard, feel strong.\nmodern\nClasses, Membership, Merch\n3\n4\n5\n0\n' | /tmp/wes_check > /tmp/wes_check.log 2>&1
echo "run exit 0"

check() {
  if ! grep -q "$1" /tmp/wes_check.log; then
    echo "FAIL: output missing: $1"
    exit 1
  fi
}
check "\[HomeEngine\] Initialized"
check "Site built for 'Rosewood Bakery': 4 pages"
check "Site built for 'Harper Gym': 4 pages"
check "Program built for 'Harper Gym': 2 files"
check "Got it — a Shop site for 'Harper Gym', 4 pages, modern theme"
echo "output checks OK"

for f in rosewood_bakery_home.html rosewood_bakery_about.html harper_gym_home.html harper_gym_shop.html harper_gym_program.cpp; do
  [ -f "$f" ] || { echo "FAIL: missing generated file $f"; exit 1; }
done
grep -q "Rosewood Bakery" rosewood_bakery_home.html || { echo "FAIL: bakery name missing from home page"; exit 1; }
grep -q "Harper Gym" harper_gym_home.html || { echo "FAIL: gym name missing from home page"; exit 1; }
grep -q "#2563eb" harper_gym_home.html || { echo "FAIL: modern theme color missing"; exit 1; }
grep -q "site | Harper Gym | Shop | 4 files" website_projects.txt || { echo "FAIL: project memory missing Harper Gym"; exit 1; }
echo "generated-file checks OK"

echo "== generated program compiles and runs =="
"$CXX" -std=c++17 -o /tmp/harper_prog harper_gym_program.cpp
/tmp/harper_prog | grep -q "Welcome to Harper Gym!" || { echo "FAIL: generated program output wrong"; exit 1; }
echo "generated program OK"

rm -f website_projects.txt website_events.txt rosewood_bakery_*.html rosewood_bakery_program* harper_gym_*.html harper_gym_program*
echo "ALL CHECKS PASSED"
