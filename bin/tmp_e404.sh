#!/bin/sh
# Initialize 3351 for E391 on 16 May, 2013
#

nfa 30 17 0 2040 1

# Initialize FCET for 3351
# Xilinxload 1 5 /home/quser/udaq/bit/fcet/fcet_3351_2.bit -s

nfa 5 9 0 0 1
nfa 5 16 1 0xdddd 1
nfa 5 0 0 1
nfa 5 0 1 1

# Initialize 3351
# Set 3351 Status (N=6, 7, 8, 10, 11, 13, and 14)
# VSN Virtual Station Number     #01-#08 Bit
# N/A                            #09
# SUB Channel Subaddress         #10 Bit
# EEN ECL bus enable             #11 Bit
# OVF Over flow Indication       #12 Bit
# CCE Zero Suppresson            #13 Bit
# CSR Sequential Readout         #14 Bit
# CLE CAMAC LAM                  #15 Bit
# N/A                            #16

nfa 6 9 0 0 1
nfa 6 10 0 0 1
nfa 6 20 14 0x3c71 1

#nfa 7 9 0 0 1
nfa 7 2 7 0 1
nfa 7 10 0 0 1
nfa 7 20 14 0x3c72 1

nfa 8 9 0 0 1
nfa 8 10 0 0 1
nfa 8 20 14 0x3c73 1

nfa 9 9 0 0 1
nfa 9 10 0 0 1
nfa 9 20 14 0x3c74 1

nfa 10 9 0 0 1
nfa 10 10 0 0 1
nfa 10 20 14 0x3c75 1

nfa 11 9 0 0 1
nfa 11 10 0 0 1
nfa 11 20 14 0x3c76 1

nfa 13 9 0 0 1
nfa 13 10 0 0 1
nfa 13 20 14 0x3c77 1

nfa 14 9 0 0 1
nfa 14 10 0 0 1
nfa 14 20 14 0x3c78 1

# Set 3351 Pedestal and Threshold (N=17, 18, 19, 20 and 21)
#  F(17)A(0-7)  upper threshold  0 to 255 (85% to 100%)
#  F(17)A(8-15) lower threshold  0 to 255 (0% to 10%)
#  F(20)A(0-7)  offset memory    0 to 255 (-3% to + 3%)
#  F(20)A(9)    common threshold 0 to 255

for nsta in 6 7 8 9 10 11 13 14
do
  echo "Initialize N = ${nsta} ...."
  suba=0
  while [ $suba -lt 8 ]
  do
    nfa ${nsta}  17 ${suba}  255 1 > /dev/null
    nfa ${nsta}  20 ${suba}  128 1 > /dev/null
    suba=`expr $suba + 1`
  done
  nfa ${nsta}  20 9    0 1 > /dev/null
  suba=8
  while [ $suba -lt 16 ]
  do
    nfa ${nsta}  17 ${suba}  0 1 > /dev/null
    suba=`expr $suba + 1`
  done
done

