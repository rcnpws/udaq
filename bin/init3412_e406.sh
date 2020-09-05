#!/bin/csh
# Initialize 3412 discreminator
#

# No mask
nfa 10 16 0 0x0000 1

# Threshold 
# 1024 / 4096 * x
# till 2013/06/16 0:34 
# nfa 10 16 1 150 1
# till 2013/06/18 00:08
# nfa 10 16 1 40 1
nfa 10 16 1 80 1

# Width
nfa 10 16 2 0x00b0 1

# Updating (W1) and Burst Guard (W2)
nfa 10 16 3 0x3 1

# Set Remote Mode
nfa 10 26 0 1

echo "Mask"
nfa 10 0 0 1

echo "Threshold"
nfa 10 0 1 1

echo "Width"
nfa 10 0 2 1

echo "Updating status"
nfa 10 0 3 1



