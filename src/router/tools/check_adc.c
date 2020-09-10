/*
	check_format_lib.c ... format checker
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  06-APR-1995 by A. Tamii
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef OSF1
#include <strings.h>
#endif
#include <time.h>
#include <mtformat.h>

#define CHKSUM  0       /* use check sum or not */

char *module[16] = {
	"Irregal ", "VDC     ", "NIM-IN  ", "ADC     ",
	"TDC     ", "PCOS    ", "Scaler  ", "Reserved",
	"Reserved", "VDC-NEW ", "PCOS-NEW", "ADC-LAS ",
	"TDC-LAS ", "FERA    ", "FERET   ", "CheckSum"
	};

int adc[16];
int cnt[16];

enum counter{
	CNT_NUM_BLKS = 0,       /* number of blocks */
	CNT_NUM_EVTS,           /* number of events */
	CNT_NUM_FLDS,           /* number of fields */
	CNT_NUM_RGNS,           /* number of regions */
	CNT_NUM_RGN_0,          /* number of region 0 */
	CNT_NUM_RGN_1,          /* number of region 1 */
	CNT_NUM_RGN_2,          /* number of region 2 */
	CNT_NUM_RGN_3,          /* number of region 3 */
	CNT_NUM_RGN_4,          /* number of region 4 */
	CNT_NUM_RGN_5,          /* number of region 5 */
	CNT_NUM_RGN_6,          /* number of region 6 */
	CNT_NUM_RGN_7,          /* number of region 7 */
	CNT_NUM_RGN_8,          /* number of region 8 */
	CNT_NUM_RGN_9,          /* number of region 9 */
	CNT_NUM_RGN_a,          /* number of region a */
	CNT_NUM_RGN_b,          /* number of region b */
	CNT_NUM_RGN_c,          /* number of region c */
	CNT_NUM_RGN_d,          /* number of region d */
	CNT_NUM_RGN_e,          /* number of region e */
	CNT_NUM_RGN_f,          /* number of region f */
	CNT_ERR_BLKS,           /* number of error blocks */
	CNT_ERR_EVTS,           /* number of error events */
	CNT_ERR_FLDS,           /* number of error fields */
	CNT_ERR_RGNS,           /* number of error regions */
	CNT_ERR_BLK_HEADER,     /* number of errors in block headers */
	CNT_ERR_BLK_TRAILER,    /* number of errors in block trailers */
	CNT_ERR_BLK_SIZE,       /* number of errors in block sizes */
	CNT_ERR_BLK_CONT,       /* number of errors in block contents */
	CNT_ERR_EVT_HEADER,     /* number of errors in event headers */
	CNT_ERR_EVT_SIZE,       /* number of errors in event sizes */
	CNT_ERR_EVT_CONT,       /* number of errors in event contents */
	CNT_ERR_FLD_HEADER,     /* number of errors in field headers */
	CNT_ERR_FLD_SIZE,       /* number of errors in field sizes */
	CNT_ERR_FLD_CONT,       /* number of errors in field contents */
	CNT_ERR_FLD_CHKSUM,     /* number of errors in field check sum */
	CNT_ERR_RGN_HEADER,     /* number of errors in region headers */
	CNT_ERR_RGN_SIZE,       /* number of errors in region sizes */
	CNT_ERR_RGN_CONT,       /* number of errors in region contents */
  NUMCNTS                 /* number of counters */
};

int counter[NUMCNTS];     /* counter */
int blk_num;              /* block number */

int base;

void clear_counters(){
	int      i;
	for(i=0; i<NUMCNTS; i++)
		counter[i] = 0;
	blk_num = 0;
}

void show_counters(){
	int      i;
#if 0
	printf("------------------------------------------------------------------------\n");
	printf("                    Blocks    Events    Fields   Regions\n");
	printf(" Total Number     %7d   %7d   %7d   %7d\n",
		counter[CNT_NUM_BLKS], counter[CNT_NUM_EVTS],
		counter[CNT_NUM_FLDS], counter[CNT_NUM_RGNS]);
	printf(" Error (Total)    %7d   %7d   %7d   %7d\n",
		counter[CNT_ERR_BLKS], counter[CNT_ERR_EVTS],
		counter[CNT_ERR_FLDS], counter[CNT_ERR_RGNS]);
	printf(" Error (Size)     %7d   %7d   %7d   %7d\n",
		counter[CNT_ERR_BLK_SIZE], counter[CNT_ERR_EVT_SIZE],
		counter[CNT_ERR_FLD_SIZE], counter[CNT_ERR_RGN_SIZE]);
	printf(" Error (Header)   %7d   %7d   %7d\n",
		counter[CNT_ERR_BLK_HEADER], counter[CNT_ERR_EVT_HEADER],
		counter[CNT_ERR_FLD_HEADER]);
	printf(" Error (Contents) %7d   %7d   %7d\n",
		counter[CNT_ERR_BLK_CONT], counter[CNT_ERR_EVT_CONT],
		counter[CNT_ERR_FLD_CONT]);
	printf(" Error (Trailer)  %7d\n",
		counter[CNT_ERR_BLK_TRAILER]);
#if CHKSUM
	printf(" Error (Check Sum)                    %7d\n",
		counter[CNT_ERR_FLD_CHKSUM]);
#endif
	printf("\n");
	printf("Regions\n");
	printf("   %8s %10d       %8s %10d    %8s %10d\n",
				 module[0], counter[CNT_NUM_RGN_0],
				 module[1], counter[CNT_NUM_RGN_1],
				 module[2], counter[CNT_NUM_RGN_2]);
	printf("   %8s %10d       %8s %10d    %8s %10d\n",
				 module[3], counter[CNT_NUM_RGN_3],
				 module[4], counter[CNT_NUM_RGN_4],
				 module[5], counter[CNT_NUM_RGN_5]);
	printf("   %8s %10d       %8s %10d    %8s %10d\n",
				 module[6], counter[CNT_NUM_RGN_6],
				 module[7], counter[CNT_NUM_RGN_7],
				 module[8], counter[CNT_NUM_RGN_8]);
	printf("   %8s %10d       %8s %10d    %8s %10d\n",
				 module[9], counter[CNT_NUM_RGN_9],
				 module[10], counter[CNT_NUM_RGN_a],
				 module[11], counter[CNT_NUM_RGN_b]);
	printf("   %8s %10d       %8s %10d    %8s %10d\n",
				 module[12], counter[CNT_NUM_RGN_c],
				 module[13], counter[CNT_NUM_RGN_d],
				 module[14], counter[CNT_NUM_RGN_e]);
	printf("   %8s %10d\n",
				 module[15], counter[CNT_NUM_RGN_f]);
	printf("------------------------------------------------------------------------\n");
#else
  for(i=0; i<8; i++ ){
    if(cnt[i]>0)
  		printf("%8d",adc[i]/cnt[i]);
    else
			printf("       -");
		}
	printf("\n");
  for(i=8; i<16; i++ ){
    if(cnt[i]>0)
  		printf("%8d",adc[i]/cnt[i]);
    else
			printf("       -",0);
		}
	printf("\n");
	printf("\n");
	for(i=0; i<16; i++){
		adc[i] = 0;
		cnt[i] = 0;
	}
		
#endif
}

void showOffset(ptr)
		 void *ptr;
{
	printf("  (%4x)", (int)ptr-base);
}
	
int check_rgns(buf, size, error)
		 unsigned short  *buf;
		 int             size;
		 int             *error;
{
	unsigned short *ptr, *tp,  *te;
	unsigned short *rgn;
	int rgn_num;
	int rgn_size;
	int rgn_err;
	int rgn_id;
  int  i;
	
	ptr = buf;
	te = &buf[size];
	rgn_num = 0;
	*error = 0;
	while(ptr < te){
		rgn = ptr;
		rgn_err = 0;
		rgn_num++;
		
		/* get the next region header */
		rgn_id = *rgn>>12;
		rgn_size = *rgn & DataLengthMask;

		/* check the region header */
		counter[CNT_NUM_RGNS]++;
		switch(rgn_id){
		case ID_Reserved1:
		case ID_Reserved2:
#if 0
		case ID_Reserved3:
		case ID_CHECK_SUM:
#endif
			rgn_err++;
			showOffset(rgn);
			printf("  Irregal Region ID. ID = %d\n", rgn_id);
			if(ptr < te){
				/* search the next field header */
				showOffset(ptr);
				printf("  ... Searching for the next Region Header\n");
				for(rgn++; rgn<te; rgn++){
					rgn_id = *rgn>>12;
#if 0
					if(rgn_id==ID_Reserved1 || rgn_id==ID_Reserved2 || rgn_id==ID_Reserved3)
						continue;
					if(rgn_id==ID_CHECK_SUM)
						continue;
#endif
					break;
				}
				rgn_size = ((int)rgn-(int)ptr)/2-1;
			}
			break;
		default:
			counter[CNT_NUM_RGN_0+rgn_id]++;
			if(rgn_id==3){
        for(i=0; i<rgn_size; i++){
					if(ptr[i+1]>200){
						adc[i] += ptr[i+1];
						cnt[i]++;
					}
				}
			}
		}
		tp = &ptr[rgn_size+1];
		if(tp>te){
			rgn_err++;
			counter[CNT_ERR_RGN_SIZE]++;
			showOffset(rgn);
			printf("  Region Size Error. size = %d\n", rgn_size);
		}
		/* check errors */
		if(rgn_err){
			counter[CNT_ERR_RGNS]++;
			*error += rgn_err;
		}
		
		/* advance to the next region */
		ptr += rgn_size+1;
	}
	return rgn_num;
}

int check_flds(buf, size, error)
		 unsigned short  *buf;
		 int             size;
		 int             *error;
{
	unsigned short *ptr, *tp,  *te;
	FldHeaderPtr   fld;
	int fld_num, rgn_num;
	int fld_size;
	int fld_err, rgn_err;
  unsigned short chksum, *sump;

	ptr = buf;
	te = &buf[size];
	fld_num = 0;
	*error = 0;
	while(ptr < te){
		/* search the next field header */
		if(*ptr != FldHeaderID){
			showOffset(ptr);
			printf("  Searching for the next Field Header\n");
			while(ptr < te){
				if(*ptr==FldHeaderID)
					break;
				ptr++;
			}
		}
		if(ptr >= te)
			break;

		/* check the field header */
		fld_err = 0;
		fld_num++;
		counter[CNT_NUM_FLDS]++;
		fld = (FldHeaderPtr)ptr;
		if(fld->headerID != FldHeaderID){
			fld_err++;
			showOffset(&fld->headerID);
			printf("  Field Header ID Error. ID = %x\n", fld->headerID);
		}
		if(fld->headerSize != sizeof(FldHeader)/2){
			fld_err++;
			showOffset(&fld->headerSize);
			printf("  Field Header Size Error. ID = %d\n", fld->headerSize);
		}
		if(fld->fieldID != FieldID){
			fld_err++;
			showOffset(&fld->fieldID);
			printf("  Field ID Error. ID = %x\n", fld->fieldID);
		}
		tp = &ptr[fld->headerSize+fld->fieldSize];
		if(tp>te || tp<te && *tp!=FldHeaderID ){
			fld_err++;
			counter[CNT_ERR_FLD_SIZE]++;
			showOffset(&fld->fieldSize);
			printf("  Field Size Error. size = %d\n", fld->fieldSize);
		}
		if(fld_err)
			counter[CNT_ERR_FLD_HEADER]++;
		ptr += fld->headerSize;

#if CHKSUM
		/* check sum */
		chksum = 0;
		sump = ptr;
		while(sump<te)
			chksum += *sump++;
		if(chksum!=0){
			fld_err++;
			counter[CNT_ERR_FLD_CHKSUM]++;
			showOffset(&sump[-1]);
			printf("  Field Check Sum Error. Sum = %x\n", chksum);
		}
#endif
		
		/* check the regions */
		if(ptr<=te){
			rgn_num = check_rgns(ptr, fld->fieldSize, &rgn_err);
			if(rgn_err){
				fld_err += rgn_err;
				counter[CNT_ERR_FLD_CONT]++;
			}
		}
		
		/* check errors */
		if(fld_err){
			counter[CNT_ERR_FLDS]++;
		  *error += fld_err;
		}
			
		/* advance to the next field */
		ptr += fld->fieldSize;
	}
	return fld_num;
}

int check_evts(buf, size, error)
		 unsigned short  *buf;
		 int             size;
		 int             *error;
{
	unsigned short *ptr, *tp, *te;
	EvtHeaderPtr   evt;
	int evt_num, fld_num;
	int evt_size;
	int evt_err, fld_err;

	ptr = buf;
	te = &buf[size];
	evt_num = 0;
	*error = 0;
	while(1){
		if(*ptr == BlkTrailerID)
			break;
		/* search the next event header */
		if(*ptr != EvtHeaderID){
			showOffset(ptr);
			printf("  Searching for the next Event Header\n");
			while(ptr < te){
				if(*ptr==EvtHeaderID || *ptr==BlkTrailerID)
					break;
				ptr++;
			}
		}
		if(ptr >= te || *ptr == BlkTrailerID )
			break;

		/* check the event header */
		evt_err = 0;
		evt_num++;
		counter[CNT_NUM_EVTS]++;
		evt = (EvtHeaderPtr)ptr;
		if(evt->headerID != EvtHeaderID){
			evt_err++;
			showOffset(&evt->headerID);
			printf("  Event Header ID Error. ID = %x\n", evt->headerID);
		}
		if(evt->headerSize != sizeof(EvtHeader)/2){
			evt_err++;
			showOffset(&evt->headerSize);
			printf("  Event Header Size Error. ID = %d\n", evt->headerSize);
		}
		if(evt->eventID != DataEvent && evt->eventID != BlockEndEvent){
			evt_err++;
			showOffset(&evt->eventID);
			printf("  Event ID Error. ID = %x\n", evt->eventID);
		}
		tp = &ptr[evt->headerSize+evt->eventSize];
#if FALSE
		printf("  header = %d size = %d ID=%x\n",
					 evt->headerSize, evt->eventSize, *tp);
#endif
		if(tp>te){
			evt_err++;
			counter[CNT_ERR_EVT_SIZE]++;
			showOffset(&evt->eventSize);
			printf("  Event Size Error. size = %d\n", evt->eventSize);
			break;
		}
		if(tp<te && *tp != EvtHeaderID && *tp != BlkTrailerID ){
			evt_err++;
			counter[CNT_ERR_EVT_SIZE]++;
			showOffset(&evt->eventSize);
			printf("  Event Size Error. size = %d\n", evt->eventSize);
		}
		if(evt->eventNumber != evt_num-1){
			evt_err++;
			showOffset(&evt->eventNumber);
			printf("  Event Number Error. number = %d\n", evt->eventNumber);
			evt_num = evt->eventNumber+1;
			showOffset(&evt->eventNumber);
			printf("  ... Adjust the event number.\n");
		}
		if(evt_err)
			counter[CNT_ERR_EVT_HEADER]++;
		ptr += evt->headerSize;

		/* check the fields */
		if(ptr<=te){
			fld_num = check_flds(ptr, evt->eventSize, &fld_err);
			if(fld_err){
				counter[CNT_ERR_EVT_CONT]++;
				evt_err += fld_err;
			}
			if(evt->numFields != fld_num){
				evt_err++;
				showOffset(&evt->numFields);
				printf("  Number of Fields Error. number = %d\n", evt->numFields);
			}
		}
		
		/* check errors */
		if(evt_err)
			counter[CNT_ERR_EVTS]++;
		*error += evt_err;
		
		/* advance to the next event */
		ptr += evt->eventSize;
	}
	return evt_num;
}

int check_com(buf, size, error)
		 unsigned short  *buf;
		 int             size;
		 int             *error;
{
	RunCommentPtr com;
  time_t        t;
	char str[MaxComLen+10];
	com = (RunCommentPtr)buf;
	if(size<sizeof(RunComment)/2){
		printf("Run Comment Size Error\n");
		*error++;
		return;
	}
	strncpy(str, com->comment, MaxComLen);
  t = (time_t)com->time;
	printf("    Format Ver = %d.%d\n", com->version>>8, com->version&0x00FF);
	printf("    Byte Order = %x\n", com->byte);
	printf("    Time Stamp = %s", ctime(&t));
	printf("    Run Number = %d\n", com->run);
	printf("    Comment    = '%s'\n", str);
}
	
void check_buf(buf, size)
		 unsigned short  *buf;
		 int             size;
{
	unsigned short *ptr;
	BlkHeaderPtr   blk;
	BlkTrailerPtr  trl;
	int evt_num;
	int blk_size;
	int blk_err, evt_err, trl_err;
	ptr = buf;

	/* check the block header */
	blk_err = 0;
	counter[CNT_NUM_BLKS]++;
	blk = (BlkHeaderPtr)ptr;
	if(blk->headerID != BlkHeaderID){
		blk_err++;
		showOffset(&blk->headerID);
		printf("  Block Header ID Error. ID = %x\n", blk->headerID);
	}
	if(blk->headerSize != sizeof(BlkHeader)/2){
		blk_err++;
		showOffset(&blk->headerSize);
		printf("  Block Header Size Error. size = %d\n", blk->headerSize);
	}
	switch(blk->blockID){
	case DataBlockID+0: /* Crate 0 */
	case DataBlockID+1: /* Crate 1 */
	case DataBlockID+2: /* Crate 2 */
	case DataBlockID+3: /* Crate 3 */
	case DataBlockID+4: /* Crate 4 */
	case DataBlockID+5: /* Crate 5 */
	case DataBlockID+6: /* Crate 6 */
	case DataBlockID+7: /* Crate 7 */
	case ScalerBlockID:
		break;
	case StartBlockID:
		printf("  Run Start Block\n");
		break;
	case EndBlockID:
		printf("  Run End Block\n");
		break;
	default:
		blk_err++;
		showOffset(&blk->blockID);
		printf("  Block ID Error. ID = %x\n", blk->blockID);
	}
	if(blk->blockSize > size-sizeof(BlkHeader)/2){
		blk_err++;
		counter[CNT_ERR_BLK_SIZE]++;
		showOffset(&blk->blockSize);
		printf("  Block Size Error. size = %d\n", blk->blockSize);
	}
	if((blk->blockID & ~7)==DataBlockID){
		blk_num++;
		if(blk->blockNumber != blk_num-1){
			blk_err++;
			showOffset(&blk->blockNumber);
			printf("  Block Number Error. number = %d\n", blk->blockNumber);
			blk_num = blk->blockNumber+1;
			showOffset(&blk->blockNumber);
			printf("  ... Adjust the block number.\n");
		}
	}
	if(blk_err)
		counter[CNT_ERR_BLK_HEADER]++;
	ptr += sizeof(BlkHeader)/2;
	
	/* check information block */
	switch(blk->blockID){
	case StartBlockID:
	case EndBlockID:
		check_com(ptr, size-sizeof(BlkHeader)/2, &blk_err);
		if(blk_err)
			counter[CNT_ERR_BLK_HEADER]++;
		return;
	}

	/* check events */
	evt_num = check_evts(ptr, size-(sizeof(BlkHeader)+sizeof(BlkTrailer))/2, &evt_err);
	if(evt_err){
		counter[CNT_ERR_BLK_CONT]++;
		blk_err += evt_err;
	}
	if(blk->numEvents != evt_num){
		blk_err++;
		showOffset(&blk->numEvents);
		printf("  Number of Events Error. number = %d, events = %d\n",
					 blk->numEvents, evt_num);
	}
	ptr += blk->blockSize-sizeof(BlkTrailer)/2;
	
	/* check block trailer */
	trl_err = 0;
	if(&ptr[sizeof(BlkTrailer)/2]>&buf[size]){
		trl_err++;
		printf("  Incomplete Block Trailer.\n");
	}else{
		trl = (BlkTrailerPtr)ptr;
		if(trl->trailerID != BlkTrailerID){
			trl_err++;
			showOffset(&trl->trailerID);
			printf("  Block Trailer ID Error. ID = %x\n", trl->trailerID);
		}
		if(trl->trailerSize != sizeof(BlkTrailer)/2){
			trl_err++;
			showOffset(&trl->trailerSize);
			printf("  Block Trailer Size Error. size = %d\n", trl->trailerSize);
		}
	}
	
	/* check errors */
	if(trl_err){
		blk_err += trl_err;
		counter[CNT_ERR_BLK_TRAILER]++;
	}
	if(blk_err)
		counter[CNT_ERR_BLKS]++;
}

