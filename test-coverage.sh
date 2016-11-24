#!/bin/sh

./ctyml_test
cd CMakeFiles/ctyml_lib.dir/src
lcov -c -d . -o coverage-report.info
genhtml coverage-report.info
xdg-open index.html
