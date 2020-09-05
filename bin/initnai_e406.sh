#!/bin/csh
# Initialize 3351and 3377  for E406 on 5 June, 2013
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
#nfa 17 20 14 0x3c71 1
nfa 17 20 14 0x2c71 1

nfa 18 9 0 0 1
nfa 18 10 0 0 1
#nfa 18 20 14 0x3c72 1
nfa 18 20 14 0x2c72 1

nfa 19 9 0 0 1
nfa 19 10 0 0 1
#nfa 19 20 14 0x3c73 1
nfa 19 20 14 0x2c73 1

nfa 20 9 0 0 1
nfa 20 10 0 0 1
#nfa 20 20 14 0x3c74 1
nfa 20 20 14 0x2c74 1

nfa 21 9 0 0 1
nfa 21 10 0 0 1
#nfa 21 20 14 0x3c74 1
nfa 21 20 14 0x2c75 1

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
nfa 17  17 8   60 1 > /dev/null
nfa 17  17 9   60 1 > /dev/null
nfa 17  17 10  60 1 > /dev/null
nfa 17  17 11  60 1 > /dev/null
nfa 17  17 12  60 1 > /dev/null
nfa 17  17 13  60 1 > /dev/null
nfa 17  17 14  60 1 > /dev/null
nfa 17  17 15  60 1 > /dev/null
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
nfa 18  17 8   60 1 > /dev/null
nfa 18  17 9   60 1 > /dev/null
nfa 18  17 10  60 1 > /dev/null
nfa 18  17 11  60 1 > /dev/null
nfa 18  17 12  60 1 > /dev/null
nfa 18  17 13  60 1 > /dev/null
nfa 18  17 14  60 1 > /dev/null
nfa 18  17 15  60 1 > /dev/null
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
nfa 19  17 8  250 1 > /dev/null
nfa 19  17 9  250 1 > /dev/null
nfa 19  17 10 250 1 > /dev/null
nfa 19  17 11 250 1 > /dev/null
nfa 19  17 12 250 1 > /dev/null
nfa 19  17 13 250 1 > /dev/null
nfa 19  17 14 250 1 > /dev/null
nfa 19  17 15 250 1 > /dev/null
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
nfa 20  17 8  250 1 > /dev/null
nfa 20  17 9  250 1 > /dev/null
nfa 20  17 10 250 1 > /dev/null
nfa 20  17 11 250 1 > /dev/null
nfa 20  17 12 250 1 > /dev/null
nfa 20  17 13 250 1 > /dev/null
nfa 20  17 14 250 1 > /dev/null
nfa 20  17 15 250 1 > /dev/null
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


################################################################
# Initialize FCET for 3377
Xilinxload 1 6 /home/quser/udaq/bit/fcet/fcet_fera.bit -s

nfa 6 9 0 0 1
nfa 6 16 1 0xbbbb 1
nfa 6 0 0 1
nfa 6 0 1 1

# Initialize 3377
# define 3377 mode

# Common start single word (F30 followed by F21 and F25)
nfa 7 9 0 1
nfa 7 30 0 1
nfa 7 21 0 1
nfa 7 25 0 1
sleep 1
nfa 7 13 0 1
nfa 7 9 0 1

nfa 8 9 0 1
nfa 8 30 0 1
nfa 8 21 0 1
nfa 8 25 0 1
sleep 1
nfa 8 13 0 1
nfa 8 9 0 1

# Set 3377 Status (N=7 and 8)
#  Regsitor 0: 0x4a80 at N=7, 0x4a81
#  #15--14 01 Mode (Read Only)
#  #13      0 Always have header
#  #12      0 Single event buffer mode
#  #11      1 ECL Port Read out
#  #10      0 Leading edge only
#  #09--08 10 Resolution (2.0 ns/ch)
#  #07--00 0x8x Virtual station number (0x80 at N=7, 0x81 at N=8)

nfa 7 17 0 0x4a80 1
nfa 8 17 0 0x4a81 1

#  Regsitor 1: 0x0c00
#  #15--13    Event serial number (Read only)
#  #12      0 Normal FERA mode
#  #11--10 11 MPI 3.2 us
#  #09--00    Not Used

nfa 7 17 1 0x0c00 1
nfa 8 17 1 0x0c00 1

#  Regsitor 2: 0x0008
#  #15--04     Not Used
#  #03--00 0x8 Maximum number of hits

nfa 7 17 2 0x0008 1
nfa 8 17 2 0x0008 1

#  Regsitor 3: 0x1000
#  #15--04  Maximum time range (2048 ns)
#  #03--00  Request delay (0 ms)

nfa 7 17 3 0x1000 1
nfa 8 17 3 0x1000 1

#  Regsitor 4: 0x002a
#  #15--10       Not used
#  #09--00  0x2a Time out value (50 ns x 0x2a = 2100 ns)

nfa 7 17 4 0x002a 1
nfa 8 17 4 0x002a 1

#  Regsitor 5: 0x0000 (Used for test mode)

nfa 7 17 5 0x0000 1
nfa 8 17 5 0x0000 1

# Read status registor

nfa 7 1 0 1
nfa 7 1 1 1
nfa 7 1 2 1
nfa 7 1 3 1
nfa 7 1 4 1
nfa 7 1 5 1

nfa 8 1 0 1
nfa 8 1 1 1
nfa 8 1 2 1
nfa 8 1 3 1
nfa 8 1 4 1
nfa 8 1 5 1

nfa 7 26 1 1
nfa 8 26 1 1

sh init3412_e406.sh
