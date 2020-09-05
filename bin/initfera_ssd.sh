#!/bin/csh
# Initialize 3351 for E391 on 16 May, 2013
#

nfa 30 17 0 2040 1

# Initialize FCET for 3351
Xilinxload 1 6 /home/quser/udaq/bit/fcet/fcet_fera.bit -s

nfa 6 9 0 0 1
nfa 6 16 1 0xcccc 1
nfa 6 0 0 1
nfa 6 0 1 1

# Set FERA Status (N=7, 8)
# Virtual Station Number     #01-08 Bit
# ECL Data Compress          #09 Bit      = 1
# ECL Pedestal Subtraction   #10 Bit      = 1
# ECL Port Enable            #11 Bit      = 1
# CAMAC Data Compress        #12 Bit      = 0
# CAMAC Pedestal Subtraction #13 Bit      = 0
# CAMAC Sequential ReadOut   #14 Bit      = 0
# CAMAC LAM Enable           #15 Bit      = 0
# Overflow Suppression       #16 Bit      = 1

nfa 7 9 0 0 1
nfa 7 16 0 0x87b2 1
nfa 7 0 0 1

nfa 8 9 0 0 1
nfa 8 16 0 0x87b1 1
nfa 8 0 0 1

# Set pedestal values (N=7)
nfa 7 9 0 0 1
nfa 7 17  0  0  1
nfa 7 17  1  0  1 > /dev/null
nfa 7 17  2  0  1 > /dev/null
nfa 7 17  3  0  1 > /dev/null
nfa 7 17  4  0  1 > /dev/null
nfa 7 17  5  0  1 > /dev/null
nfa 7 17  6  0  1 > /dev/null
nfa 7 17  7  0  1 > /dev/null
nfa 7 17  8  0  1 > /dev/null
nfa 7 17  9  0  1 > /dev/null
nfa 7 17 10  0  1 > /dev/null
nfa 7 17 11  0  1 > /dev/null
nfa 7 17 12  0  1 > /dev/null
nfa 7 17 13  0  1 > /dev/null
nfa 7 17 14  0  1 > /dev/null
nfa 7 17 15  0  1 > /dev/null

# Set pedestal values (N=8)
nfa 8 9  0  0  1
nfa 8 17  0  0  1
nfa 8 17  1  0  1 > /dev/null
nfa 8 17  2  0  1 > /dev/null
nfa 8 17  3  0  1 > /dev/null
nfa 8 17  4  0  1 > /dev/null
nfa 8 17  5  0  1 > /dev/null
nfa 8 17  6  0  1 > /dev/null
nfa 8 17  7  0  1 > /dev/null
nfa 8 17  8  0  1 > /dev/null
nfa 8 17  9  0  1 > /dev/null
nfa 8 17 10  0  1 > /dev/null
nfa 8 17 11  0  1 > /dev/null
nfa 8 17 12  0  1 > /dev/null
nfa 8 17 13  0  1 > /dev/null
nfa 8 17 14  0  1 > /dev/null
nfa 8 17 15  0  1 > /dev/null
