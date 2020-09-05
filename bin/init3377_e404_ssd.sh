

################################################################
# Initialize FCET for 3377
Xilinxload 1 20 /home/quser/udaq/bit/fcet/fcet_fera.bit -s

nfa 20 9 0 0 1
nfa 20 16 1 0xbbbb 1
nfa 20 0 0 1
nfa 20 0 1 1

# Initialize 3377
# define 3377 mode

# Common start single word (F30 followed by F21 and F25)
for nsta in 21 22
do
  nfa ${nsta} 9 0 1
  nfa ${nsta} 30 0 1
#  nfa ${nsta} 21 0 1
  nfa ${nsta} 25 0 1
  sleep 1
  nfa ${nsta} 13 0 1
  nfa ${nsta} 9 0 1
  nfa ${nsta} 26 1 1
done


# Set 3377 Status (N=21 and 22)
#  Regsitor 0: 0x8880 at N=21, 0x8881
#  #15--14 01 Mode (Read Only)
#  #13      0 Always have header
#  #12      0 Single event buffer mode
#  #11      1 ECL Port Read out
#  #10      0 Leading edge only
#  #09--08 10 Resolution (0.5 ns/ch)
#  #07--00 0x8x Virtual station number (0x80 at N=7, 0x81 at N=8)

  nfa 21 17 0 0x0880 1
  nfa 22 17 0 0x0881 1

#  Regsitor 1: 0x0c00
#  #15--13    Event serial number (Read only)
#  #12      0 Normal FERA mode
#  #11--10 11 MPI 0 us
#  #09--00    Not Used

nfa 21 17 1 0x0000 1
nfa 22 17 1 0x0000 1

#  Regsitor 2: 0x0008
#  #15--04     Time window(Upper limit 1024+128 nsec)
#  #03--00 0xf Maximum number of hits (16 hits)

#nfa 21 17 2 0x090f 1
#nfa 22 17 2 0x090f 1
nfa 21 17 2 0x080f 1
nfa 22 17 2 0x080f 1

#  Regsitor 3: 0x1000
#  #15--04  Offset (512+128 ns)
#  #03--00  Request delay (0 ms)

#nfa 21 17 3 0x0500 1
#nfa 22 17 3 0x0500 1
nfa 21 17 3 0x0400 1
nfa 22 17 3 0x0400 1

#  Regsitor 4: 0x002a
#  #15--10       Not used
#  #09--00  0x2a Time out value (50 ns x 0x2a = 2100 ns)
#
#nfa 21 17 4 0x002a 1
#nfa 22 17 4 0x002a 1
#
#  Regsitor 5: 0x0000 (Used for test mode)
#
#nfa 21 17 5 0x0000 1
#nfa 22 17 5 0x0000 1

# Read status registor

for nsta in 21 22
do
  nfa ${nsta} 1 0 1
  nfa ${nsta} 1 1 1
  nfa ${nsta} 1 2 1
  nfa ${nsta} 1 3 1
#  nfa ${nsta} 1 4 1
#  nfa ${nsta} 1 5 1
done
