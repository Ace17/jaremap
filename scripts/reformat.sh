#!/usr/bin/env bash
set -euo pipefail

readonly scriptDir=$(dirname $0)
find -name "*.cpp" -or -name "*.h" | xargs -L 1 uncrustify -q -c $scriptDir/uncrustify.cfg --no-backup
