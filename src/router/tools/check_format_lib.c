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
#include <string.h>
#include <time.h>

#include "mtformat.h"
#include "fera.h"
#include "lr3377.h"
#include "pcos.h"

#define HILITE    "\033[34m\033[1m"
#define UNHILITE  "\033[m"
#define BOLD      "\033[1m"
#define UNBOLD    "\033[m"

#define CHKSUM  0       /* use check sum or not */

char *module[16] = {
	"Irregal ", "VDC     ", "Inp.Reg.", "ADC     ",
	"TDC     ", "PCOS    ", "Scaler  ", "LR3377  ",
	"Reserved", "VDC-NEW ", "PCOS-NEW", "ADC-LAS ",
	"TDC-LAS ", "FERA    ", "FERET   ", "CheckSum"
	};

char *fera_name[16] = {
	"GR--QDC", "LAS-QDC", "GSO-QDC", "SSD-QDC",
	"GR--ADC", "LAS-ADC", "GSO-ADC", "SSD-ADC",
	"GR--TDC", "LAS-TDC", "GSO-TDC", "SSD-TDC",
	"N/U    ", "N/U    ", "N/U    ", "N/U    "
};

#if 0
char *lr3377_name[16] = {
	"GR  X-1", "NPMWDC1", "GR  U-1", "GR  V-1",
	"GR  X-2", "Reserve", "GR  U-2", "GR  V-2",
	"LAS X-1", "LMWDC-X", "LAS U-1", "LAS V-1",
	"LAS X-2", "LMWDC-Y", "LAS U-2", "LAS V-2"
};
#else
char *lr3377_name[16] = {
	"GR  X-1", "BaseSig", "GR  U-1", "GR  V-1",
	"GR  X-2", "Reserve", "GR  U-2", "GR  V-2",
	"LAS X-1", "LMWDC-X", "LAS U-1", "LAS V-1",
	"LAS X-2", "LMWDC-Y", "LAS U-2", "LAS V-2"
};
#endif

char *pcos_name[16] = {
	"X-1    ", "X-2    ", "X-3    ", "X-4    ",
	"U-1    ", "U-2    ", "U-3    ", "U-4    ",
	"V-1    ", "V-2    ", "V-3    ", "V-4    ",
	"N/U    ", "N/U    ",	"N/U    ", "N/U    "
};

char *inpreg_nam0[16] = {
	"Not Used",              /* Event #1  Bit #0 */
	"! Spin Up",             /* Event #2  Bit #1 */
	"! Spin Down",           /* Event #3  Bit #2 */
	"! Hata In (CAVE)",      /* Event #4  Bit #3 */
	"! Hata In (WN)",        /* Event #5  Bit #4 */
	"GR FP Event",           /* Event #6  Bit #5 */
	"SSD Event",             /* Event #7  Bit #6 */
	"LAS FP Event",          /* Event #8  Bit #7 */
	"Coin. Event",           /* Event #9  Bit #8 */
#if 1
	"LAS C.P. VETO Samp."   ,/* Event #10 Bit #9 */
#else
	"LAS FPP Event",         /* Event #10 Bit #9 */
	"Not Used",              /* Event #10 Bit #9 */
#endif
	"2nd Level Event",       /* Event #11 Bit #10 */
	"FPP Event",             /* Event #12 Bit #11 */
	"2nd Level Accept",      /* Event #13 Bit #12 */
	"2nd Level Reject",      /* Event #14 Bit #13 */
	"Not Used",              /* Event #15 Bit #14 */
	"Block End Event"        /* Event #16 Bit #15 */
};

char *inpreg_range_nam[16] = {
  "NONE", "2E-2", "6E-3", "2E-3",
  "6E-4", "2E-4", "6E-5", "2E-5",
  "6E-6", "2E-6", "6E-7", "2E-7",
  "6E-8", "2E-8", "6E-9", "2E-9"
};

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
int show_inpreg_itvl = 0x7FFFFFFF;
int show_fera_itvl = 0x7FFFFFFF;
int show_3377_itvl = 0x7FFFFFFF;
int show_pcos_itvl = 0x7FFFFFFF;
int dump_flag = 0;
int wire_flag = 0;
int pcos_flag = 0;

void clear_counters(){
	int      i;
	for(i=0; i<NUMCNTS; i++)
		counter[i] = 0;
	blk_num = 0;
}

void show_counters(){
	int      i;
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
}

void showOffset(ptr)
		 void *ptr;
{
	printf("  (%4x)", (long)ptr-base);
}
	
/*** Dump the data ***/
static int show_dump(ptr, len)
		 unsigned short *ptr;
		 int len;
{
	int  i;
	if(!dump_flag)
		return;
	printf("Dump Data\n");
	while(len>0){
		for(i=0; i<len && i<8; i++){
			printf(" %.4x", *ptr++);
		}
		printf("\n");
		len -= i;
	}
	printf("\n");
}

/*** Input Register ***/

#define MAXINPREGMODS 64
#define MAXNINPREGDAT 64
#define NCHINPREG     16
static struct INPREG {
		int   data;
		int   bits[NCHINPREG];
		int   diff[NCHINPREG];
		int   sum[NCHINPREG];
	} inpreg[MAXINPREGMODS];
static int ninpreg=0;
static int cinpreg=0;
static unsigned short inpreg_dat[MAXNINPREGDAT];
static int ninpreg_dat;

int check_inpreg(ptr, len)
		 unsigned short *ptr;
		 int  len;
{
	int  i, data;
	for(i=0; i<len && ninpreg_dat<MAXNINPREGDAT; i++)
		inpreg_dat[ninpreg_dat++] = ptr[i];
	while(len>0){
		if(cinpreg>=MAXINPREGMODS){
			printf("Too many input register modules (%d).\n", cinpreg);
			return(-1);
		}
		data = *ptr++;
		len--;
		inpreg[cinpreg].data = data;
		for(i=0; i<NCHINPREG; i++){
			if(data&1){
				inpreg[cinpreg].bits[i] = 1;
				inpreg[cinpreg].diff[i]++;
				inpreg[cinpreg].sum[i]++;
			}else{
				inpreg[cinpreg].bits[i] = 0;
			}
			data >>= 1;
		}
		ninpreg = ++cinpreg;
	}
}

int check_inpreg_init(){
	int  i, j, k;
	/* clear the data */
	ninpreg_dat = 0;
	cinpreg = 0;
}

int show_inpreg()
{
	static int show_inpreg_count=0;
	int   i, j;
	if((++show_inpreg_count)<show_inpreg_itvl)
		return;
	show_inpreg_count = 0;
  printf("---------------------------- Input Register DATA ---------------------------\n\n");
  printf("                            Bit       Diff        Sum\n");
	show_dump(inpreg_dat, ninpreg_dat);
	if(cinpreg>=1){
		for(j=NCHINPREG-1; j>=0; j--){
			printf(" %2d  %-20s %5d %10d %10d\n", j+1,
						 inpreg_nam0[j], inpreg[0].bits[j], 
						 inpreg[0].diff[j], inpreg[0].sum[j]);
		}
	}
	if(cinpreg>=2){
		printf("\n");
		printf(" Cur. Int. Range: %s\n",
					 inpreg_range_nam[inpreg[1].data&0x0F]);
		printf(" Reserved Bits:   %.3x\n", inpreg[1].data>>4);
	}
	if(cinpreg>=3){
		printf(" Reserved Words:  ");
		for(i=3; i<=cinpreg; i++){
			printf("%.4x", inpreg[i].data);
		}
		printf("\n");
	}
	for(i=0; i<ninpreg; i++){
		for(j=0; j<NCHINPREG; j++){
			inpreg[i].diff[j] = 0;
		}
	}
}

/*** End of Input Register ***/

/*** FERA ***/

#define MAXFERAMODS 64
#define MAXNFERADAT 4096
#define NCHFERA     16
static struct FERA {
		int   id;
		int   data[16];
	} fera[MAXFERAMODS];
static int nfera=0;
static unsigned short fera_dat[MAXNFERADAT];
static int nfera_dat;

int check_fera(ptr,len)
		 unsigned short *ptr;
		 int  len;
{
	int  i, j;
	int  vsn;
	int  ch, dat;
	fera_header header;
	fera_data   data;
	

	for(i=0; i<len && nfera_dat<MAXNFERADAT; i++)
		fera_dat[nfera_dat++] = ptr[i];

	*(unsigned short*)&header = *ptr++;
	vsn = header.vsn;
	len--;

	for(i=0; i<nfera; i++){
		if(fera[i].id==vsn)
			break;
	}
	if(i>=MAXFERAMODS){
		printf("Too many FERA modules (%d).\n", nfera);
		return(-1);
	}
	if(i>=nfera){
		for(j=0; j<NCHFERA; j++)
			fera[i].data[j]=-1;
		fera[i].id = vsn;
		nfera++;
	}
	for(j=0; j<len; j++){
		*(unsigned short*)&data = *ptr++;
		ch  = data.ch;
		dat = data.data;
#if 0
		if(vsn==0x12) printf("j=%2d, ch=%2d\n", j, ch);
#endif
		fera[i].data[ch] = dat;
	}
}

int check_fera_init(){
	int  i, j, k;
	/* clear the data */
	nfera_dat = 0;
	for(i=0; i<nfera; i++){
		for(j=0; j<NCHFERA; j++){
			fera[i].data[j] = -1;
		}
	}
	/* (bubble) sort */
	for(i=0; i<nfera-1; i++){
		for(j=i+1; j<nfera; j++){
			if(fera[i].id > fera[j].id){
				k = fera[i].id;
				fera[i].id = fera[j].id;
				fera[j].id = k;
			}
		}
	}
}

int show_fera()
{
	static int show_fera_count=0;
	int   i, j, id;
	if((++show_fera_count)<show_fera_itvl)
		return;
	show_fera_count = 0;
  printf("------------------------------ FERA/FERET DATA -----------------------------\n\n");
	show_dump(fera_dat, nfera_dat);
  printf("VSN   MODULE   ");
  for(i=0; i<NCHFERA; i++){
    printf("#%2d ", i);
	}
  printf("\n");
	for(i=0; i<nfera; i++){
		id = fera[i].id;
		printf("%3x %-6s-%-2d ", id, fera_name[id>>4], id&0x0F);
		for(j=0; j<NCHFERA; j++){
			if(fera[i].data[j]<0)
				printf("--- ");
			else
			  printf("%.3x ", fera[i].data[j]);
		}
		printf("\n");
	}
	printf("\n");
}

/*** FERA END ***/

/*** 3377 ***/

#define MAX3377MODS 256
#define MAXN3377DAT 4096
#define NCH3377     32

static struct LR3377 {
		int   id;
		int   evt;
		int   data[NCH3377];
		int   nhit[NCH3377];
	} lr3377[MAX3377MODS];
static int n3377=0;
static unsigned short lr3377_dat[MAXN3377DAT];
static int n3377_dat;

int check_lr3377(ptr,len)
		 unsigned short *ptr;
		 int  len;
{
	int  i, j;
	int  vsn;
	int  ch, dat;
	lr3377_header header;
	lr3377_data   data;

	for(i=0; i<len && n3377_dat<MAXN3377DAT; i++)
		lr3377_dat[n3377_dat++] = ptr[i];

	i=-1;
	while(len>0){
		*(unsigned short*)&header = *ptr;
		*(unsigned short*)&data   = *ptr++;
		len--;
		if(header.hdr){
			if(header.word)
				printf("Warning: double word format is not supported\n");
			if(header.edge){
				//20-OCT-2006
				//printf("Warning: trailing edge is not supported\n");
			}

  		vsn = header.id;
			//printf("header = %.4x\n", *(unsigned short*)&header);
			//printf("header.id = %.4x\n", header.id);

			for(i=0; i<n3377; i++){
				if(lr3377[i].id==vsn)
					break;
			}
			if(i>=MAX3377MODS){
				printf("Too many 3377 modules (%d).\n", n3377);
				return(-1);
			}
			if(i>=n3377){
				for(j=0; j<NCH3377; j++){
					lr3377[i].data[j]=-1;
					lr3377[i].nhit[j] = 0;
				}
				lr3377[i].id = vsn;
				n3377++;
			}
			lr3377[i].evt = header.evt;
		}else{
			ch   = data.ch;
			dat =  data.data;
			if(i>=0){
				lr3377[i].data[ch] = dat;
				lr3377[i].nhit[ch]++;
			}
		}
	}
}

int check_lr3377_init(){
	int  i, j, k;
	int  tmp;
	/* clear the data */
	for(i=0; i<n3377; i++){
		lr3377[i].evt = -1;
		for(j=0; j<NCH3377; j++){
			lr3377[i].data[j] = -1;
		}
	}
	n3377_dat = 0;

	/* (bubble) sort */
	for(i=0; i<n3377-1; i++){
		for(j=i+1; j<n3377; j++){
			if(lr3377[i].id > lr3377[j].id){
				tmp = lr3377[i].id;
				lr3377[i].id = lr3377[j].id;
				lr3377[j].id = tmp;
				for(k=0; k<NCH3377; k++){
					tmp = lr3377[i].nhit[k];
					lr3377[i].nhit[k] = lr3377[j].nhit[k];
					lr3377[j].nhit[k] = tmp;
				}
			}
		}
	}
}

int show_lr3377_tdc()
{
	int   i, j, id;
	show_dump(lr3377_dat, n3377_dat);
  printf("VSN   MODULE   ");
  for(i=0; i<16; i++){
    printf("#%2d ", i);
	}
  printf("\n               ");
  for(i=16; i<NCH3377; i++){
    printf("#%2d ", i);
	}
  printf("\n");
	for(i=0; i<n3377; i++){
		id = lr3377[i].id;
		printf("%3x %-6s-%-2d ", id, lr3377_name[id>>4], id&0x0F);
		for(j=0; j<16; j++){
			if(lr3377[i].data[j]<0)
				printf("--- ");
			else
			  printf("%.3x ", lr3377[i].data[j]);
		}
		printf("\n               ");
		for(j=16; j<NCH3377; j++){
			if(lr3377[i].data[j]<0)
				printf("--- ");
			else
			  printf("%.3x ", lr3377[i].data[j]);
		}
		printf("\n");
	}
	printf("\n");
}

int show_lr3377_hit()
{
	int   i, j, id, k, n, nhit, evt;
	char  c;

	show_dump(lr3377_dat, n3377_dat);
  printf("VSN   MODULE                  WIRE HITS                  EVT NHITS");
  printf("\n");
	for(i=0; i<n3377; i++){
		id  = lr3377[i].id;
		evt = lr3377[i].evt;
		printf("%3x %-6s-%-2d ", id, lr3377_name[id>>4], id&0x0F);
		n = 0;
		for(j=0; j<NCH3377; j++){
			nhit = lr3377[i].nhit[j];
			if(nhit<=0){
				c='-';
				printf("%c", c);
			}	else if(nhit<10){
				c='0'+nhit;
				if(lr3377[i].data[j]>0)
					printf(HILITE"%c"UNHILITE, c);
				else
  				printf("%c", c);
			}else{
        c='A';
				nhit >>= 4;
				for(;nhit>0;nhit>>=1)
					c++;
				if(lr3377[i].data[j]>=0)
					printf(BOLD""HILITE"%c"UNHILITE""UNBOLD, c);
				else
					printf(BOLD"%c"UNBOLD, c);
			}
			if((j&0x03)==0x03) printf(" ");
			if((j&0x0F)==0x0F) printf(" ");
			if(lr3377[i].data[j]>=0) n++;
		}
		if(n>0){
			printf(" %c  %2d\n", evt<0 ? ' ':'0'+evt, n);
		}else{
			printf(" %c    \n", evt<0 ? ' ':'0'+evt);
		}
	}
	printf("\n");
}

int show_lr3377()
{
	static int show_3377_count=0;
	if((++show_3377_count)<show_3377_itvl)
		return;
	show_3377_count = 0;

  printf("-------------------------------- LR3377 DATA -------------------------------\n\n");
	if(wire_flag) show_lr3377_hit();
	else show_lr3377_tdc();
}


/*** 3377 END ***/

/*** PCOS ***/

#define MAXPCOSMODS 256
#define MAXNPCOSDAT 4096
#define NCHPCOS     32

static struct PCOS {
		int   addr;
		int   hit[NCHPCOS];
		int   nhit[NCHPCOS];
	} pcos[MAXPCOSMODS];
static int npcos=0;
static unsigned short pcos_dat[MAXNPCOSDAT];
static int npcos_dat;

int check_pcos(ptr,len)
		 unsigned short *ptr;
		 int  len;
{
	int  i, j, k, w;
	int  addr, ch, h;
	pcos_delim   delim;
	pcos_width   width;
	pcos_mod     mod;

	for(i=0; i<len && npcos_dat<MAXNPCOSDAT; i++)
		pcos_dat[npcos_dat++] = ptr[i];

	i=-1;
	w=1;
	while(len>0){
		*(unsigned short*)&delim =
		*(unsigned short*)&width =
		*(unsigned short*)&mod   = *ptr++;
		len--;
		if(delim.hdr){
			if(delim.delim){  /* Delimiter Word */
#if 0
				printf("Delimiter (PCOS=%d)\n", delim.pcos);
#endif
			}else{            /* Width Word */
				w = width.width;
			}
		}else{              /* Data Word */
#if 0
			printf("data = %4x\n", *(unsigned short*)&mod);
			printf("mod = %4x, addr=%4x, w=%d\n", *(unsigned short*)&mod,
						 mod.addr, w);
#endif
			(*(unsigned short*)&mod) -= w-1;
			for(k=0; k<w; k++){   /* Width Loop */
				addr = mod.addr;
				for(i=0; i<npcos; i++){
					if(pcos[i].addr==addr)
						break;
				}
				if(i>=MAXPCOSMODS){
					printf("Too many PCOS modules (%d).\n", npcos);
					return(-1);
				}
				if(i>=npcos){
					for(j=0; j<NCHPCOS; j++){
						pcos[i].hit[j] = 0;
						pcos[i].nhit[j] = 0;
					}
					pcos[i].addr = addr;
					npcos++;
				}
				ch   = mod.ch;
				h    = mod.h;
				if(h){
					//20-OCT-2006
					//printf("PCOS half bit inconsistency.\n");
				}else{
#if 0
					printf("addr = %4x, ch=%3d\n", pcos[i].addr, ch);
#endif
					pcos[i].hit[ch]++;
					pcos[i].nhit[ch]++;
				}
				(*(unsigned short*)&mod) += 2;
			}
			w = 1;
		}
	}
}

int check_pcos_init(){
	int  i, j, k;
	int  tmp;
	/* clear the data */
	for(i=0; i<npcos; i++){
		for(j=0; j<NCHPCOS; j++){
			pcos[i].hit[j] = 0;
		}
	}
	npcos_dat = 0;

	/* (bubble) sort */
	for(i=0; i<npcos-1; i++){
		for(j=i+1; j<npcos; j++){
			if(pcos[i].addr > pcos[j].addr){
				tmp = pcos[i].addr;
				pcos[i].addr = pcos[j].addr;
				pcos[j].addr = tmp;
				for(k=0; k<NCHPCOS; k++){
					tmp = pcos[i].nhit[k];
					pcos[i].nhit[k] = pcos[j].nhit[k];
					pcos[j].nhit[k] = tmp;
				}
			}
		}
	}
}

int show_pcos_hit()
{
	int   i, j, addr, k, n, nhit, evt;
	char  c;

	show_dump(pcos_dat, npcos_dat);
  printf("ADDR  MODULE                  WIRE HITS                  NHITS");
  printf("\n");
	for(i=0; i<npcos; i++){
		addr = pcos[i].addr;
		printf("%3x %-6s-%-2d ", addr, pcos_name[addr>>5], addr&0x1F);
		n = 0;
		for(j=0; j<NCHPCOS; j++){
			nhit = pcos[i].nhit[j];
			if(nhit<=0) c='-';
			else if(nhit<10) c='0'+nhit;
			else{
        nhit /= 10;
        c='A';
				while(nhit>9){
          nhit /= 10;
          c++;
				}
			}
			printf("%c", c);
			if((j&0x03)==0x03) printf(" ");
			if((j&0x0F)==0x0F) printf(" ");
			if(pcos[i].hit[j]) n++;
		}
		if(n>0){
			printf("%2d\n", n);
		}else{
			printf("  \n" );
		}
	}
	printf("\n");
}

int show_pcos()
{
	static int show_pcos_count=0;
	if((++show_pcos_count)<show_pcos_itvl)
		return;
	show_pcos_count = 0;

  printf("--------------------------------- PCOS DATA --------------------------------\n\n");
	show_pcos_hit();
}


/*** PCOS END ***/

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

	ptr = buf;
	te = &buf[size];
	rgn_num = 0;
	*error = 0;
	while(ptr < te){
		rgn = ptr;
		rgn_err = 0;
		rgn_num++;
		
		/* get the next region header */
		rgn_id   = *rgn & ModuleIDMask;
		rgn_size = *rgn & DataLengthMask;

		/* check the region header */
		counter[CNT_NUM_RGNS]++;
		counter[CNT_NUM_RGN_0+(rgn_id>>12)]++;
		switch(rgn_id){
		case ID_Reserved:
			rgn_err++;
			showOffset(rgn);
			printf("  Irregal Region ID. ID = %d\n", rgn_id>>12);
			if(ptr < te){
				/* search the next field header */
				showOffset(ptr);
				printf("  ... Searching for the next Region Header\n");
				for(rgn++; rgn<te; rgn++){
					rgn_id = *rgn>>12;
					break;
				}
				rgn_size = ((long)rgn-(long)ptr)/2-1;
			}
			break;
		case ID_FERA_ADC:
		case ID_FERA_TDC:
			check_fera(&ptr[1], (*ptr & DataLengthMask));
			break;
		case ID_3377:
			check_lr3377(&ptr[1], (*ptr & DataLengthMask));
			break;
		case ID_NimIn:
			check_inpreg(&ptr[1], (*ptr & DataLengthMask));
			break;
		case ID_4299_MWPC:
			check_pcos(&ptr[2], (*ptr & DataLengthMask)-1);
			break;
		case ID_CHKSUM:
		case ID_4299_VDC_Q:
		case ID_ADC:
		case ID_TDC:
/*	case ID_4299_MWPC_Q:   --> disposed */
		case ID_3351:
		case ID_Scaler:
		case ID_4299_VDC:
		case ID_ADC_LAS:
		case ID_TDC_LAS:
		default:
			break;
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

	check_inpreg_init();
	check_fera_init();
	check_lr3377_init();
	check_pcos_init();
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
		if((fld->fieldID & ~7)!= FieldID){
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
	show_inpreg();
	show_fera();
	show_lr3377();
	show_pcos();
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
	unsigned short t[2], tmp;
	char str[MaxComLen+10];
	com = (RunCommentPtr)buf;
	if(size<sizeof(RunComment)/2){
		printf("Run Comment Size Error\n");
		*error++;
		return;
	}
	strncpy(str, com->comment, MaxComLen);
  *(unsigned long*)t = com->time;
	if(0x01020304 != ntohl(0x01020304)){
		tmp=t[0]; t[0]=t[1]; t[1]=tmp;
	}

	printf("    Format Ver = %d.%d\n", com->version>>8, com->version&0x00FF);
	printf("    Byte Order = %x\n", com->byte);
	printf("    Time Stamp = %s", ctime((time_t*)t));
#if 0
	printf("    Time Stamp = %.8x\n", *(unsigned long*)t);
	time(t);
	printf("    Time       = %.8x\n", *(unsigned long*)t);
#endif
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
	if((blk->headerSize==6 && blk->blockSize > size-sizeof(BlkHeader)/2)
		 ||(blk->headerSize==8 && (blk->blockSize32_l | (blk->blockSize32_u<<16)) > size-sizeof(BlkHeader)/2)
		 ){
		blk_err++;
		counter[CNT_ERR_BLK_SIZE]++;
		showOffset(&blk->blockSize);
		printf("  Block Size Error. size = %d\n", blk->blockSize);
	}
	if((blk->blockID & ~7)==DataBlockID){
		blk_num++;
		if(blk->blockNumber != blk_num-1){
			blk_err++;
#if 0
			showOffset(&blk->blockNumber);
			printf("  Block Number Error. number = %d\n", blk->blockNumber);
#endif
			blk_num = blk->blockNumber+1;
#if 0
			showOffset(&blk->blockNumber);
			printf("  ... Adjust the block number.\n");
#endif
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
	ptr += (blk->headerSize==6 ? blk->blockSize : (blk->blockSize32_l | (blk->blockSize32_u<<16)))
		 -sizeof(BlkTrailer)/2;
	
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

/*
Local Variables:
mode: C
tab-width: 2
End:
*/
