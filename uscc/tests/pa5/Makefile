# Maintenance targets for the SpecLICM test suite assets.
#
# The 26 speclicm*.usc sources are the authoritative test programs.
# The expected/ directory contains two kinds of committed test data:
#
#   *.profile   -- edge-count text produced by running the PA2 -EN
#                  instrumented build through lli.  GENERATED — can
#                  be regenerated after editing a .usc source via
#                  `make profiles` (delegates to gen_profiles.sh).
#
#   *.output    -- textual LLVM IR produced by `uscc -speclicm -p`.
#                  Committed golden fixtures compared directly by
#                  testSpecLICM.py.  NEVER touched by this Makefile.
#
# Typical workflows
#   make profiles    (re)generate all expected/speclicm*.profile
#   make clean       remove generated profiles + stray .bc/.raw,
#                    leave committed expected/*.output untouched
#   make regen       clean then regenerate profiles

USCC ?= ../bin/uscc
LLI  ?= ../../bin/lli

# All speclicm test programs present in this directory.
SpecLICM_SRCS  := $(sort $(wildcard speclicm*.usc))
SpecLICM_NAMES := $(basename $(notdir $(SpecLICM_SRCS)))

.PHONY: profiles regen clean clean-profiles clean-bc help

help:
	@echo "SpecLICM test-asset targets:"
	@echo "  make profiles        (re)generate all expected/speclicm*.profile"
	@echo "  make clean-profiles  remove expected/speclicm*.profile"
	@echo "  make clean-bc        remove all stray *.bc / *.raw"
	@echo "  make clean           clean-profiles + clean-bc"
	@echo "  make regen           clean then profiles"
	@echo ""
	@echo "NOTE: expected/speclicm*.output files are committed golden IR"
	@echo "      fixtures and are NEVER touched by this Makefile."

# ------------------------------------------------------------------
# Profile regeneration (delegates to the canonical shell helper)
# ------------------------------------------------------------------

profiles:
	@sh gen_profiles.sh

regen: clean profiles

# ------------------------------------------------------------------
# Cleanup targets (expected/*.output files are NOT touched — they
# are hand-curated golden fixtures, same as the classic expected/
# outputs in cs502-Spring24/tests/)
# ------------------------------------------------------------------

clean-profiles:
	@rm -f expected/speclicm*.profile
	@echo "removed expected/speclicm*.profile"

clean-bc:
	@rm -f *.bc *.raw
	@echo "removed all stray *.bc / *.raw"

clean: clean-profiles clean-bc
