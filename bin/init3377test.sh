

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
nfa 7 26 1 1

nfa 8 9 0 1
nfa 8 30 0 1
nfa 8 21 0 1
nfa 8 25 0 1
sleep 1
nfa 8 13 0 1
nfa 8 9 0 1
nfa 8 26 1 1

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


