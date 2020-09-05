#!/bin/csh
# Initialize 3351 for E391 on 16 May, 2013
#

nfa 30 17 0 2040 1

# Initialize FCET for 3351
Xilinxload 1 16 /home/quser/udaq/bit/fcet/fcet_3351_2.bit -s

nfa 16 9 0 0 1
nfa 16 16 1 0xdddd 1
nfa 16 0 0 1
nfa 16 0 1 1

# Initialize 3351
# Set 3351 Status (N=17, 18, 19, 20 and 21)
# VSN Virtual Station Number     #01-#08 Bit
# N/A                            #09
# SUB Channel Subaddress         #10 Bit
# EEN ECL bus enable             #11 Bit
# OVF Over flow Indication       #12 Bit
# CCE Zero Suppresson            #13 Bit
# CSR Sequential Readout         #14 Bit
# CLE CAMAC LAM                  #15 Bit
# N/A                            #16

nfa 17 9 0 0 1
nfa 17 10 0 0 1
nfa 17 20 14 0x3c71 1

nfa 18 9 0 0 1
nfa 18 10 0 0 1
nfa 18 20 14 0x3c72 1

nfa 19 9 0 0 1
nfa 19 10 0 0 1
nfa 19 20 14 0x3c73 1

nfa 20 9 0 0 1
nfa 20 10 0 0 1
nfa 20 20 14 0x3c74 1

nfa 21 9 0 0 1
nfa 21 10 0 0 1
nfa 21 20 14 0x3c74 1

# Set 3351 Pedestal and Threshold (N=17, 18, 19, 20 and 21)
#  F(17)A(0-7)  upper threshold  0 to 255 (85% to 100%)
#  F(17)A(8-15) lower threshold  0 to 255 (0% to 10%)
#  F(20)A(0-7)  offset memory    0 to 255 (-3% to + 3%)
#  F(20)A(9)    common threshold 0 to 255

# N = 17
nfa 17  17 0  255 1 > /dev/null
nfa 17  17 1  255 1 > /dev/null
nfa 17  17 2  255 1 > /dev/null
nfa 17  17 3  255 1 > /dev/null
nfa 17  17 4  255 1 > /dev/null
nfa 17  17 5  255 1 > /dev/null
nfa 17  17 6  255 1 > /dev/null
nfa 17  17 7  255 1 > /dev/null
nfa 17  20 9    0 1
nfa 17  17 8    0 1 > /dev/null
nfa 17  17 9    0 1 > /dev/null
nfa 17  17 10   0 1 > /dev/null
nfa 17  17 11   0 1 > /dev/null
nfa 17  17 12   0 1 > /dev/null
nfa 17  17 13   0 1 > /dev/null
nfa 17  17 14   0 1 > /dev/null
nfa 17  17 15   0 1 > /dev/null
nfa 17  20  0 128 1 > /dev/null
nfa 17  20  1 128 1 > /dev/null
nfa 17  20  2 128 1 > /dev/null
nfa 17  20  3 128 1 > /dev/null
nfa 17  20  4 128 1 > /dev/null
nfa 17  20  5 128 1 > /dev/null
nfa 17  20  6 128 1 > /dev/null
nfa 17  20  7 128 1 > /dev/null


# N = 18
nfa 18  17 0  255 1 > /dev/null
nfa 18  17 1  255 1 > /dev/null
nfa 18  17 2  255 1 > /dev/null
nfa 18  17 3  255 1 > /dev/null
nfa 18  17 4  255 1 > /dev/null
nfa 18  17 5  255 1 > /dev/null
nfa 18  17 6  255 1 > /dev/null
nfa 18  17 7  255 1 > /dev/null
nfa 18  20 9    0 1
nfa 18  17 8    0 1 > /dev/null
nfa 18  17 9    0 1 > /dev/null
nfa 18  17 10   0 1 > /dev/null
nfa 18  17 11   0 1 > /dev/null
nfa 18  17 12   0 1 > /dev/null
nfa 18  17 13   0 1 > /dev/null
nfa 18  17 14   0 1 > /dev/null
nfa 18  17 15   0 1 > /dev/null
nfa 18  20  0 128 1 > /dev/null
nfa 18  20  1 128 1 > /dev/null
nfa 18  20  2 128 1 > /dev/null
nfa 18  20  3 128 1 > /dev/null
nfa 18  20  4 128 1 > /dev/null
nfa 18  20  5 128 1 > /dev/null
nfa 18  20  6 128 1 > /dev/null
nfa 18  20  7 128 1 > /dev/null

# N = 19
nfa 19  17 0  255 1 > /dev/null
nfa 19  17 1  255 1 > /dev/null
nfa 19  17 2  255 1 > /dev/null
nfa 19  17 3  255 1 > /dev/null
nfa 19  17 4  255 1 > /dev/null
nfa 19  17 5  255 1 > /dev/null
nfa 19  17 6  255 1 > /dev/null
nfa 19  17 7  255 1 > /dev/null
nfa 19  20 9    0 1
nfa 19  17 8    0 1 > /dev/null
nfa 19  17 9    0 1 > /dev/null
nfa 19  17 10   0 1 > /dev/null
nfa 19  17 11   0 1 > /dev/null
nfa 19  17 12   0 1 > /dev/null
nfa 19  17 13   0 1 > /dev/null
nfa 19  17 14   0 1 > /dev/null
nfa 19  17 15   0 1 > /dev/null
nfa 19  20  0 128 1 > /dev/null
nfa 19  20  1 128 1 > /dev/null
nfa 19  20  2 128 1 > /dev/null
nfa 19  20  3 128 1 > /dev/null
nfa 19  20  4 128 1 > /dev/null
nfa 19  20  5 128 1 > /dev/null
nfa 19  20  6 128 1 > /dev/null
nfa 19  20  7 128 1 > /dev/null

# N = 20
nfa 20  17 0  255 1 > /dev/null
nfa 20  17 1  255 1 > /dev/null
nfa 20  17 2  255 1 > /dev/null
nfa 20  17 3  255 1 > /dev/null
nfa 20  17 4  255 1 > /dev/null
nfa 20  17 5  255 1 > /dev/null
nfa 20  17 6  255 1 > /dev/null
nfa 20  17 7  255 1 > /dev/null
nfa 20  20 9    0 1
nfa 20  17 8    0 1 > /dev/null
nfa 20  17 9    0 1 > /dev/null
nfa 20  17 10   0 1 > /dev/null
nfa 20  17 11   0 1 > /dev/null
nfa 20  17 12   0 1 > /dev/null
nfa 20  17 13   0 1 > /dev/null
nfa 20  17 14   0 1 > /dev/null
nfa 20  17 15   0 1 > /dev/null
nfa 20  20  0 128 1 > /dev/null
nfa 20  20  1 128 1 > /dev/null
nfa 20  20  2 128 1 > /dev/null
nfa 20  20  3 128 1 > /dev/null
nfa 20  20  4 128 1 > /dev/null
nfa 20  20  5 128 1 > /dev/null
nfa 20  20  6 128 1 > /dev/null
nfa 20  20  7 128 1 > /dev/null

# N = 21
nfa 21  17 0  255 1 > /dev/null
nfa 21  17 1  255 1 > /dev/null
nfa 21  17 2  255 1 > /dev/null
nfa 21  17 3  255 1 > /dev/null
nfa 21  17 4  255 1 > /dev/null
nfa 21  17 5  255 1 > /dev/null
nfa 21  17 6  255 1 > /dev/null
nfa 21  17 7  255 1 > /dev/null
nfa 21  20 9    0 1
nfa 21  17 8    0 1 > /dev/null
nfa 21  17 9    0 1 > /dev/null
nfa 21  17 10   0 1 > /dev/null
nfa 21  17 11   0 1 > /dev/null
nfa 21  17 12   0 1 > /dev/null
nfa 21  17 13   0 1 > /dev/null
nfa 21  17 14   0 1 > /dev/null
nfa 21  17 15   0 1 > /dev/null
nfa 21  20  0 128 1 > /dev/null
nfa 21  20  1 128 1 > /dev/null
nfa 21  20  2 128 1 > /dev/null
nfa 21  20  3 128 1 > /dev/null
nfa 21  20  4 128 1 > /dev/null
nfa 21  20  5 128 1 > /dev/null
nfa 21  20  6 128 1 > /dev/null
nfa 21  20  7 128 1 > /dev/null



