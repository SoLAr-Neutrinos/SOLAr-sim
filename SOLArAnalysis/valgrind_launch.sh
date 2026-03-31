#!/usr/bin/env bash

######################################################################
# @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
# @file        : valgrind_launch
# @created     : Friday Mar 22, 2024 10:52:51 CET
#
# @description : simple script for an easier launch of valgrind
######################################################################

export PATH=${PATH}:"/home/guff/Dune/SOLAr/SOLAr-sim/install"

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         --log-file=valgrind-out.txt \
         --num-callers=30 --suppressions=$ROOTSYS/etc/valgrind-root.supp \
         solar_sim $@

