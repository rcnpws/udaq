/*
	assign_j11.c ... Assign J11 Buffer Type
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  09-JUL-1995 by A. Tamii
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "mtformat.h"
#include "assign_j11.h"

int assign_j11(buf, size, cnt, str)
		 unsigned short *buf;
		 int            size;
		 int            *cnt;
		 char           *str;
{
	BlkHeaderPtr   blk;
	RunCommentPtr  com;
  unsigned short *ptr;
  unsigned short *end;
	unsigned int   ver;
  unsigned short temp;
	ptr = buf;
	end = &buf[size/2];
	if(size<sizeof(BlkHeader)+
		 sizeof(EvtHeader)+sizeof(FldHeader)+sizeof(BlkTrailer))
		return BLK_UNKNOWN;
  blk = (BlkHeaderPtr)ptr;
	if(blk->headerID!=BlkHeaderID)
		return BLK_UNKNOWN;
	if(blk->headerSize<S_BH_MIN || S_BH_MAX<blk->headerSize)
		return BLK_UNKNOWN;
	switch(blk->blockID){
	case DataBlockID+0: /* Crate 0 */
	case DataBlockID+1: /* Crate 1 */
	case DataBlockID+2: /* Crate 2 */
	case DataBlockID+3: /* Crate 3 */
	case DataBlockID+4: /* Crate 4 */
	case DataBlockID+5: /* Crate 5 */
	case DataBlockID+6: /* Crate 6 */
	case DataBlockID+7: /* Crate 7 */
		return BLK_DATA;
	case ScalerBlockID:
		return BLK_SCALER;
	case StartBlockID:
		com = (RunCommentPtr)&buf[blk->headerSize];
		if((unsigned short*)&com[1]>end)
			return BLK_UNKNOWN;
		ver = com->version;
		if(ntohl(0x01020304)!=0x01020304){
      temp = ((unsigned short*)&ver)[0];
      ((unsigned short*)&ver)[0] = ((unsigned short*)&ver)[1];
      ((unsigned short*)&ver)[1] = temp;
		}
		//		if(ver!=ComVar1_0)
		if(ver!=ComVar4_0)
			return BLK_UNKNOWN;
		*cnt = com->run;
		strncpy(str, com->comment, MaxComLen);
		return BLK_START;
	case EndBlockID:
		com = (RunCommentPtr)&buf[blk->headerSize];
		if((unsigned short*)&com[1]>end)
			return BLK_UNKNOWN;
		ver = com->version;
		if(ntohl(0x01020304)!=0x01020304){
      temp = ((unsigned short*)&ver)[0];
      ((unsigned short*)&ver)[0] = ((unsigned short*)&ver)[1];
      ((unsigned short*)&ver)[1] = temp;
		}
		//		if(ver!=ComVar1_0)
		if(ver!=ComVar4_0)
			return BLK_UNKNOWN;
		*cnt = com->run;
		strncpy(str, com->comment, MaxComLen);
		return BLK_END;
	case MiddleBlockID:
		return BLK_MIDDLE;
	}
	return BLK_UNKNOWN;
}
					 
/*
  Local Variables:
  tab-width: 2
  End:
*/
