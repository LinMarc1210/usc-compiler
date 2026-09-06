# Maintenance targets for the PA6 test suite assets.
#
# Two kinds of committed test data live under expected/:
#
#   *.ra.output        -- stderr emitted by `uscc -s` on the baseline
#                         PA6 Chaitin allocator.  Committed golden
#                         fixture compared by testRegAlloc.py.
#                         NEVER touched by this Makefile.
#
#   *.profile_split.ra.output -- stderr emitted by `uscc -s -profile-file ...`
#                         on the PA6 extension allocator (profile-aware
#                         + live-range splitting).  Committed golden
#                         fixture compared by testRegAlloc.py.
#                         NEVER touched by this Makefile.
#
#   *.profile          -- edge-count text produced by running the
#                         PA2 -EN instrumented build through lli.
#                         GENERATED -- regenerate via `make profiles`,
#                         which delegates to gen_profiles.sh.
#
# Typical workflows
#   make profiles   (re)generate all expected/*.profile
#   make clean      remove generated profiles + stray .bc/.raw/out
#   make regen      clean then profiles

USCC ?= ../bin/uscc
LLI  ?= ../../bin/lli

.PHONY: profiles regen clean clean-profiles clean-bc clean-out help

help:
	@echo "PA6 test-asset targets:"
	@echo "  make profiles        (re)generate all expected/*.profile"
	@echo "  make clean-profiles  remove expected/*.profile"
	@echo "  make clean-bc        remove stray *.bc / *.raw / *.s"
	@echo "  make clean-out       remove out/"
	@echo "  make clean           clean-profiles + clean-bc + clean-out"
	@echo "  make regen           clean then profiles"
	@echo ""
	@echo "NOTE: expected/*.ra.output and *.profile_split.ra.output are committed"
	@echo "      golden fixtures and are NEVER touched by this Makefile."

# ------------------------------------------------------------------
# Profile regeneration (delegates to the canonical shell helper)
# ------------------------------------------------------------------

profiles:
	@sh gen_profiles.sh

regen: clean profiles

# ------------------------------------------------------------------
# Cleanup targets (expected/*.output files are NOT touched)
# ------------------------------------------------------------------

clean-profiles:
	@rm -f expected/*.profile
	@echo "removed expected/*.profile"

clean-bc:
	@rm -f *.bc *.raw *.s
	@echo "removed stray *.bc / *.raw / *.s"

clean-out:
	@rm -rf out
	@echo "removed out/"

clean: clean-profiles clean-bc clean-out
