#!/usr/bin/env sh

######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : setup_solarsim.sh
# @created     : Thursday Apr 10, 2025 10:39:18 CEST
#
# @description : Setup environmental variables for the solar-sim package
######################################################################

source /home/guff/Software/radsrc_v1.6/setup.sh
source /home/guff/Software/cry_v1.7/setup.sh
source /home/guff/Dune/SOLAr/SOLAr-sim/G4SOLAr/extern/install/marley/setup_marley.sh

export SOLARSIM_PREFIX="/home/guff/Dune/SOLAr/SOLAr-sim/install-v0.3"
export SOLAR_UTILS_PREFIX="/home/guff/Dune/SOLAr/SOLAr-sim/SOLArUtils/install-v03"
export PATH="${PATH}:${SOLARSIM_PREFIX}/bin:${SOLAR_UTILS_PREFIX}/bin"


