#!/bin/bash

BN=pipwatch_zero
TGD=$BN-hw02-pragoboard

mkdir -p $TGD

cp -f $BN-B_Cu.gbl  $TGD/bot.gbr
cp -f $BN-F_Cu.gtl  $TGD/top.gbr
cp -f $BN-F_Mask.gts  $TGD/smt.gbr
cp -f $BN-B_Mask.gbs  $TGD/smb.gbr
cp -f $BN-Edge_Cuts.gbr  $TGD/mil.gbr
cp -f $BN.drl         $TGD/pth.exc
