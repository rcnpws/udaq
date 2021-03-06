#!/bin/csh
# Initialize LR4300B(FERA) for E398 on 14 May, 2014
#

nfa 30 17 0 0 1

# Initialize FCET for FERA
Xilinxload 1 13 /home/quser/udaq/bit/fcet/fcet_fera.bit -s
nfa 13 9 0 0 1
nfa 13 16 1 0xcccc 1
#nfa 13 0 0 1
nfa 13 0 1 1
nfa 13 0 2 1
nfa 13 0 3 1

# Set FERA Status for TDC (N=9-10)
# Virtual Station Number     #01-08 Bit
# ECL Data Compress          #09 Bit      = 1
# ECL Pedestal Subtraction   #10 Bit      = 1
# ECL Port Enable            #11 Bit      = 1
# CAMAC Data Compress        #12 Bit      = 0
# CAMAC Pedestal Subtraction #13 Bit      = 0
# CAMAC Sequential ReadOut   #14 Bit      = 0
# CAMAC LAM Enable           #15 Bit      = 0
# Overflow Suppression       #16 Bit      = 1

nfa  9 9 0 0 1
nfa  9 16 0 0x87b1 1
nfa  9 0 0 1

nfa 10 9 0 0 1
nfa 10 16 0 0x87b2 1
nfa 10 0 0 1

# Set FERA Status for ADC (N=11-12)
# Virtual Station Number     #01-08 Bit
# ECL Data Compress          #09 Bit      = 1
# ECL Pedestal Subtraction   #10 Bit      = 1
# ECL Port Enable            #11 Bit      = 1
# CAMAC Data Compress        #12 Bit      = 0
# CAMAC Pedestal Subtraction #13 Bit      = 0
# CAMAC Sequential ReadOut   #14 Bit      = 0
# CAMAC LAM Enable           #15 Bit      = 0
# Overflow Suppression       #16 Bit      = 0

nfa 11 9 0 0 1
nfa 11 16 0 0x0731 1
nfa 11 0 0 1

nfa 12 9 0 0 1
nfa 12 16 0 0x0732 1
nfa 12 0 0 1

# Set pedestal values (N=9)
nfa  9 9 0 0 1
nfa  9 17  0  0  1
nfa  9 17  1  0  1
nfa  9 17  2  0  1
nfa  9 17  3  0  1
nfa  9 17  4  0  1
nfa  9 17  5  0  1
nfa  9 17  6  0  1
nfa  9 17  7  0  1
nfa  9 17  8  0  1
nfa  9 17  9  0  1
nfa  9 17 10  0  1
nfa  9 17 11  0  1
nfa  9 17 12  0  1
nfa  9 17 13  0  1
nfa  9 17 14  0  1
nfa  9 17 15  0  1

# Set pedestal values (N=10)
nfa 10 9 0 0 1
nfa 10 17  0  0  1
nfa 10 17  1  0  1
nfa 10 17  2  0  1
nfa 10 17  3  0  1
nfa 10 17  4  0  1
nfa 10 17  5  0  1
nfa 10 17  6  0  1
nfa 10 17  7  0  1
nfa 10 17  8  0  1
nfa 10 17  9  0  1
nfa 10 17 10  0  1
nfa 10 17 11  0  1
nfa 10 17 12  0  1
nfa 10 17 13  0  1
nfa 10 17 14  0  1
nfa 10 17 15  0  1

# Set pedestal values (N=11)
nfa 11 9 0 0 1
nfa 11 17  0  0  1
nfa 11 17  1  0  1
nfa 11 17  2  0  1
nfa 11 17  3  0  1
nfa 11 17  4  0  1
nfa 11 17  5  0  1
nfa 11 17  6  0  1
nfa 11 17  7  0  1
nfa 11 17  8  0  1
nfa 11 17  9  0  1
nfa 11 17 10  0  1
nfa 11 17 11  0  1
nfa 11 17 12  0  1
nfa 11 17 13  0  1
nfa 11 17 14  0  1
nfa 11 17 15  0  1

# Set pedestal values (N=10)
nfa 12 9 0 0 1
nfa 12 17  0  0  1
nfa 12 17  1  0  1
nfa 12 17  2  0  1
nfa 12 17  3  0  1
nfa 12 17  4  0  1
nfa 12 17  5  0  1
nfa 12 17  6  0  1
nfa 12 17  7  0  1
nfa 12 17  8  0  1
nfa 12 17  9  0  1
nfa 12 17 10  0  1
nfa 12 17 11  0  1
nfa 12 17 12  0  1
nfa 12 17 13  0  1
nfa 12 17 14  0  1
nfa 12 17 15  0  1

