#!/bin/sh
# gen_profiles.sh
# Regenerate expected/profile_split*.profile using the PA2 -EN + lli
# + awk pipeline.  The generated .profile files feed the profile-split
# register allocator via `uscc -s -profile-file ...`.
#
# Pipeline:
#   1. Compile each profile_split*.usc with `uscc -EN -b` to produce
#      an instrumented .bc that counts every CFG edge at run time.
#   2. Run it through lli; the instrumented binary writes the
#      program's own stdout first, then an `=== NAIVE EDGE PROFILING`
#      block with EDGE_PROFILE / EDGES / END_PROFILE records.
#   3. awk strips the program output and writes the profile block to
#      expected/<name>.profile.
#
# Re-run after editing any profile_split*.usc.
set -e
cd "$(dirname "$0")"
mkdir -p expected
for usc in profile_split*.usc; do
  name="${usc%.usc}"
  rm -f "${name}.bc"
  ../../bin/uscc -EN -b "${usc}"
  ../../../bin/lli "${name}.bc" > "${name}.raw" || true
  awk '/=== NAIVE EDGE PROFILING ===/{p=1} p' "${name}.raw" > "expected/${name}.profile"
  rm -f "${name}.bc" "${name}.raw"
  echo "wrote expected/${name}.profile"
done
