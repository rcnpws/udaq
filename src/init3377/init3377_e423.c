/* init3377_e423.c ---- initialyze LeCroy 3377                             */
/*								           */
/* Copyright (C) 1996, Atsushi Tamii				           */
/* 	This program was made at RCNP.				           */
/*								           */
/*  Version 1.00	25-MAY-1996	by Atsushi Tamii	           */
/*  Version 1.10	26-MAY-1996	by Atsushi Tamii	           */
/*  Version 1.20	10-SEP-1999	by Atsushi Tamii (for REAR Output) */
/*  Version 2.00        23-MAY-2001     by M. Uchida (for Solaris2.6)      */
/*  Version 3.00        2008-08-11      by M.Uchida (For Linux2.6)         */
/*  Version 3.10        2008-11-23      by A. Tamii (cleaned-up)           */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "camlib.h"
#include "init3377.h"

# define SLEEP 0

# define code0(a,b,c,d,e,f) \
   ((((a)&1)<<13)|(((b)&1)<<12)|(((c)&1)<<11)|\
    (((d)&1)<<10)|(((e)&3)<<8)|((f)&0x00ff))
# define code1(a,b,c,d,e,f) \
   ((((a)&3)<<13)|(((b)&1)<<12)|(((c)&3)<<10)|\
    (((d)&3)<<8)|(((e)&0x000f)<<4)|((f)&0x000f))
# define code2(a,b) \
   ((((a)&0x0fff)<<4)|((b)&0x000f))
# define code3(a,b) \
   ((((a)&0x0fff)<<4)|((b)&0x000f))
# define code4(a) \
   ((a)&0x01ff)
# define code5(a,b,c) \
   ((((a)&1)<<8)|(((b)&3)<<5)|((c)&0x001f))


int silent=0; /* silent mode */

int stop=1;   /* Common start mode(0) or common stop mode(1) */
int word=0;   /* Single word format(0) or double word format(1) */
int hdr=1;    /* always have header(0) or skip header if no data(1) */
int buf=0;    /* single(0) or multiple(1) buffer mode */
int ecl=1;    /* camac(0) or ecl(1) readout */
//int edge=0;   /* leading only(0) or both leading and trailing edge(1) */
int edge=1;   /* leading only(0) or both leading and trailing edge(1) */
int res=1;    /* resolution 0.5ns(0), 1.0ns(1), 2.0ns(2) or 4.0ns(3) */
int mpi=3;    /* measure pause interval 0us(0), 0.8us(1), */
              /* 1.6us(2) or 3.2us(3) */
int trcu=2;   /* trigger clock unit 25ns(0),50ns(1),100ns(2),external(3) */
int trdel=15; /* trigger pulse delay (0-15) */
int trwid=1;  /* trigger pulse width (0-15) */
int hits=0;   /* maximum number of hits allowed per channel */
int dly=0;    /* delay setting in units of usec */
int tmo=1023; /* common start timeout in units of nsec */
int test=0;   /* test enable(1) or disable(0) */
int tclk=0;   /* test mode clock 100ns(0), 200ns(1), 400ns(2), 800ns(3) */
int npul=0;   /* number of pulses generated in test mode */

extern int scl;  /* full scale time in units of nsec */
extern int ofs;

extern Module modules[];

char *str_mpi[4]  = {"0.0", "0.8", "1.6", "3.2"};
char *str_res[4]  = {"0.5", "1.0", "2.0", "4.0"};
char *str_tclk[4] = {"100", "200", "400", "800"};
char *str_trcu[4] = {"25 nsec","50 nsec","100 nsec","external"};

unsigned short* mem;

#define MSG       1
#define VERIFY    1
#define DEBUG     0

/*#showError---- show error and exit */

void showError( error, str )
  short		error;
  unsigned char	*str;
{
  printf( "Error in %s().\n", str );
  camClose(mem);
  exit(1);
}

void setprm(){
  char    str[256];
  int     tmp;
  int     min, max;

  printf("LeCroy 3377 Operation Mode Setting\n\n");
  
  printf("Common start(1)/stop(2) mode [%d]: ", stop+1);
  printf("sizeof(str) = %d\n", sizeof(str));
  fgets(str,sizeof(str),stdin);
  if('1'<=str[0]&&str[0]<='2') stop = str[0]-'1';

  printf("Single(1)/Double(2) word format [%d]: ", word+1);
  fgets(str,sizeof(str),stdin);
  if('1'<=str[0]&&str[0]<='2') word = str[0]-'1';

  strcpy(str, hdr ? "y":"n");
  printf("Skip header if no data words y/n [%s]: ",str);
  fgets(str,sizeof(str),stdin);
  hdr = ((str[0]|0x20)=='y') ? 1 : ((str[0]|0x20)=='n') ? 0 : hdr;

  strcpy(str, buf ? "y":"n");
  printf("Multi event buffer mode y/n [%s]: ",str);
  fgets(str,sizeof(str),stdin);
  buf = ((str[0]|0x20)=='y') ? 1 : ((str[0]|0x20)=='n') ? 0 : buf;

  strcpy(str, ecl ? "y":"n");
  printf("ECL port readout y/n [%s]: ",str);
  fgets(str,sizeof(str),stdin);
  ecl = ((str[0]|0x20)=='y') ? 1 : ((str[0]|0x20)=='n') ? 0 : ecl;

  strcpy(str, edge ? "y":"n");
  printf("Both leading/trailing edge recording y/n [%s]: ",str);
  fgets(str,sizeof(str),stdin);
  edge = ((str[0]|0x20)=='y') ? 1 : ((str[0]|0x20)=='n') ? 0 : edge;

  if(word){
    res=0;
    printf("Resolution 0.5ns(1) [1]: ");
    fgets(str,sizeof(str),stdin);
  }else{    
    printf("Resolution 0.5ns(1)/1.0ns(2)/2.0ns(3)/4.0ns(4) [%d]: ", res+1);
    fgets(str,sizeof(str),stdin);
    if('1'<=str[0]&&str[0]<='4') res = str[0]-'1';
  }

  printf("MPI 0.0us(1)/0.8us(2)/1.6us(3)/3.2us(4) [%d]: ",mpi+1);
  fgets(str,sizeof(str),stdin);
  if('1'<=str[0]&&str[0]<='4') mpi = str[0]-'1';

  printf("Trigger Clock Unit 25ns(1)/50ns(2)/100ns(3)/external(4) [%d]: ",
    trcu+1);
  fgets(str,sizeof(str),stdin);
  if('1'<=str[0]&&str[0]<='4') trcu = str[0]-'1';

  printf("Trigger Width 0<=width<=15 [%d]: ", trwid&0x0f);
  fgets(str,sizeof(str),stdin);
  if('0'<=str[0]&&str[0]<='9'){
    tmp = atoi(str);
    if(0<=tmp && tmp<=15) trwid = tmp;
  }

  printf("Trigger Delay 0<=delay<=15 [%d]: ", trdel&0x0f);
  fgets(str,sizeof(str),stdin);
  if('0'<=str[0]&&str[0]<='9'){
    tmp = atoi(str);
    if(0<=tmp && tmp<=15) trdel = tmp;
  }
   
  printf("Max number of hits per channel 1<=hits<=16 [%d]: ",
    ((hits-1)&0x0f)+1);
  fgets(str,sizeof(str),stdin);
  tmp = atoi(str);
  if(1<=tmp && tmp<=16) hits = tmp;

  min = 0;
  max = 32768;
  printf("Max full scale time %d<=time<%d nsec [%d]: ", min, max, scl);
  fgets(str,sizeof(str),stdin);
  if(str[0]!=0x00){
    tmp = atoi(str);
    if(min<=tmp && tmp<max) scl = tmp;
  }

  max = scl;
  if(word) min = 32768;
  else{
    min = edge ? 256 : 512;     /* = nsec = *2 counts */
    min <<= res;
  }
  min = max-min;
  if(ofs<=min) ofs=min+1;
  if(ofs>max) ofs=max;
  if(min<0)
    printf("Offset time %d<=offset<=%d nsec [%d]: ", 0, max, ofs);
  else
    printf("Offset time %d<offset<=%d nsec [%d]: ", min, max, ofs);
    fgets(str,sizeof(str),stdin);
  if(str[0]!=0x00){

  max = 30;
  printf("Request delay setting %d<=delay<%d usec [%d]: ", min, max, dly);
  fgets(str,sizeof(str),stdin);
  if(str[0]!=0x00){
    tmp = atoi(str);
    if(min<=tmp && tmp<=max) dly = tmp & 0x1e;
  }

  if(!stop){
    min = 0;
    max = scl;
    if(tmo<min) tmo=min;
    if(tmo>=max) tmo=max-1;
    printf("Common start timeout %d<=timeout<%d nsec [%d]: ", min, max, tmo);
    fgets(str,sizeof(str),stdin);
    if(str[0]!=0x00){
      tmp = atoi(str);
      if(min<=tmp && tmp<=max) tmo = tmp/50*50;
    }

    strcpy(str, test ? "y":"n");
    printf("Test mode y/n [%s]: ",str);
    fgets(str,sizeof(str),stdin);
    test = ((str[0]|0x20)=='y') ? 1 : ((str[0]|0x20)=='n') ? 0 : test;
    
    if(test){
      printf("Test clock 100ns(1)/200ns(2)/400ns(3)/800ns(4) [%d]: ",tclk+1);
       fgets(str,sizeof(str),stdin);
      if('1'<=str[0]&&str[0]<='4') tclk = str[0]-'1';

      min = 0;
      max = 31;
      printf("Number of test pulses %d<=pulse<%d nsec [%d]: ", min, max, npul);
      fgets(str,sizeof(str),stdin);
      if(str[0]!=0x00){
        tmp = atoi(str);
        if(min<=tmp && tmp<=max) npul = tmp;
      }
    }
  }
  }
}

int checkprm(){
  char  str[256];
  char  c;
  while(1){
    printf("\n");
    printf("Common input mode:                  %s\n",
      stop ? "Common Stop":"Common Start");
    printf("Word format:                        %s\n",
      word ? "Double Word":"Single Word");
    printf("Skip header if no data words:       %s\n",
      hdr ? "Yes":"No");
    printf("Event buffer mode:                  %s\n",
      buf ? "Multi Event":"Single Event");
    printf("Readout port:                       %s\n",
      ecl ? "ECL":"CAMAC");
    printf("Leading/trailing edge recording:    %s\n",
      edge ? "Both":"Leading Only");
    printf("Resolution:                         %s nsec\n",
      str_res[res]);
    printf("Measure Pause Interval (MPI):       %s usec\n",
      str_mpi[mpi]);
    printf("Trigger Clock Unit:                 %s\n",
      str_trcu[trcu]);
    printf("Trigger Width:                      %d\n",
      trwid);
    printf("Trigger Delay:                      %d\n",
      trdel);
    printf("Max number of hits per channel:     %d\n",
      hits);
    printf("Max full scale time:                %d nsec\n",
      scl);
    printf("Offset time:                        %d nsec\n",
      ofs);
    printf("Request delay setting:              %d usec\n",
      dly);
    if(!stop){
      printf("Common start timeout:               %d nsec\n",
        tmo);
      printf("Test mode:                          %s\n",
        test ? "Enabled":"Disabled");
      if(test){
        printf("Test mode clock:                    %s\n",
          str_tclk[tclk]);
        printf("Number of test pulses:              %d\n",
          npul);
      }
    }
    if(silent)
      return 0;
    while(1){
      printf("\nAll settings are OK? y/n/q: ");
      fgets(str,sizeof(str),stdin);
      c = str[0] | 0x20;
      if(c=='n' || c=='y' || c=='q')
        break;
    }
    if(c=='n'){
      printf("\n");
      setprm();
      continue;
    }
    break;
  }
  return(c=='q' ? 1 : 0);
}

int setup(){
  short		error;
  short		crate, n3377;
  short		i;
  unsigned long	data;
  unsigned short	code;
  unsigned long C,N,A,F,q,x;

  for(i=0;;i++){
    crate = modules[i].c;
    n3377 = modules[i].n;
    if(crate ==-1){
      fprintf(stdout,"Initialize finished successfully\n");
      exit(0);
    } else if(crate<-1){  /* modified by M.Uchida 2002/12/20 */
      fprintf(stderr,"Bad crate number!\n");
      exit(1);
    }

    C = crate;

#if DEBUG
    printf("send clear to crate %ld\n", C);
#endif
    cSetClear(mem,C);

#if MSG
    printf("%s   C=%1d N=%2d ID=%2x ... \n",
	   modules[i].name, crate, n3377, modules[i].id);
#endif


    /* Check if Module Exists */
  
#if DEBUG
    fprintf(stdout,"Check if Module exists \n");
#endif
    N = n3377;
    F = 1;
    A = 0;
    data = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);
  
    if( x == 0){   /* Check X-response */
      printf("X=%ld No Response\n",x);
      exit(1);
    }
    
    /* Enter Xilinx Program Mode */
#if DEBUG
    fprintf(stderr,"Enter Xilinx Program Mode \n");
#endif
    F = 30;
    CNAF(mem,C,N,A,F, &data, &q, &x);
    if( x == 0){
      printf("No Response\n");
      exit(1);
    }

    /* Check Data Ready */
#if DEBUG
    fprintf(stderr,"Check Data Ready \n");
#endif
    while(1){
#if SLEEP
      usleep(100000);
#endif
      F = 12;
      A = 0;
      CNAF(mem,C,N,A,F, &data, &q, &x);
    
      if(q==1){
	break;
      }
    }

    /* Set EPROM Mode */

#if DEBUG
    fprintf(stderr,"Set EPROM Mode \n");
#endif

    if(stop){
      if(word){
	F = 22;
	A = 0;
	CNAF(mem,C,N,A,F, &data, &q, &x);
      }
    }else{
      if(word){
	F = 21;
	A = 0;
	CNAF(mem,C,N,A,F, &data, &q, &x);
      } else {
	F = 23;
	A = 0;
	CNAF(mem,C,N,A,F, &data, &q, &x);
      }
	
    }
    /* Do nothing = Common Stop, Single Word */

    /* Start Programming */

#if DEBUG
    fprintf(stderr,"Start Programming \n");
#endif

    F = 25;
    A = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);

    /* Check the End of the Programming */

#if DEBUG
    fprintf(stderr,"Check the end of the programming \n");
#endif

    while(1){
      usleep(0x100000);
      F = 13;
      A = 0;
      CNAF(mem,C,N,A,F, &data, &q, &x);
      if(q==1) {
	break;
      }
    }
    
    /* Exit Program Mode */

#if DEBUG
    fprintf(stderr,"Exit program mode \n");
#endif

    F = 28;
    A = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);    
     
    /* Clear and Start Program */    

#if DEBUG
    fprintf(stderr,"Clear and start program \n");
#endif

    F = 9;
    A = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);    
        
    /* Set On-Board Register #0 */
    if(modules[i].id==0x00){
      /* Always send header */
      code = code0(0, buf, ecl, edge, res, modules[i].id);
    }else{
      code = code0(hdr, buf, ecl, edge, res, modules[i].id);
    }
    while(1){

#if DEBUG
      fprintf(stdout,"code0:0x%x\n",code);
#endif
      F = 17;
      A = 0;
      data = code;
      CNAF(mem,C,N,A,F, &data, &q, &x);      
  
      if(x == 1) break;
    }

#if 0
    if(error) showError( error, "k3922Write16" );
#endif

#if VERIFY

#if SLEEP
    usleep(0x100000);
#endif
    F = 1;
    A = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);    
    
#if DEBUG
    fprintf(stdout,"code0:0x%lx\n",data);
#endif 

    if(code!=data)
      printf("Error in verifying Register #0\n");
#endif

    /* Set On-Board Register #1 */

#if 1
    code = code1(0, 0, mpi, trcu, trdel, trwid);
#else  /* Fast FERA Mode Test */
    code = code1(0, 1, mpi, trcu, trdel, trwid);
#endif

#if DEBUG
    fprintf(stdout,"code1:0x%x\n",code);
#endif
    F = 17;
    A = 1;
    data = code;
    CNAF(mem,C,N,A,F, &data, &q, &x);

#if VERIFY
#if SLEEP
    usleep(0x100000);
#endif

    F = 1;
    A = 1;
    CNAF(mem,C,N,A,F, &data, &q, &x);    
    
#if DEBUG
    fprintf(stdout,"code1:0x%lx\n",data);
#endif

    if(code!=data)
      printf("Error in verifying Register #1\n");
#endif
        

    /* Set On-Board Register #2 */
    code = code2(scl/8, hits&0x0f);

#if DEBUG
    fprintf(stdout,"code2:0x%x\n",code);
#endif
    F = 17;
    A = 2;
    data = code;
    CNAF(mem,C,N,A,F, &data, &q, &x);    

#if VERIFY
#if SLEEP
    usleep(0x100000);
#endif

    F = 1;
    A = 2;
    CNAF(mem,C,N,A,F, &data, &q, &x);    

#if DEBUG
    fprintf(stdout,"code2:0x%lx\n",data);
#endif

    if(code!=data)
      printf("Error in verifying Register #2\n");
#endif

    /* Set On-Board Register #3 */
    code = code3(ofs/8, 0);

#if DEBUG
    fprintf(stdout,"code3:0x%x\n",code);
#endif

    F = 17;
    A = 3;
    data = code;
    CNAF(mem,C,N,A,F, &data, &q, &x);    

#if VERIFY
#if SLEEP
    usleep(0x100000);
#endif
    F = 1;
    A = 3;
    CNAF(mem,C,N,A,F, &data, &q, &x);    

#if DEBUG
    fprintf(stdout,"code3:0x%lx\n",data);
#endif

    if(code!=data)
      printf("Error in verifying Register #3\n");
#endif

    if(!stop){
      /* Set On-Board Register #4 */

      code = code4(tmo/50);
      F = 17;
      A = 4;
      data = code;
      CNAF(mem,C,N,A,F, &data, &q, &x);    
      
      /* Set On-Board Register #5 */

      code = code5(test,tclk,npul);

      F = 17;
      A = 5;
      data = code;
      CNAF(mem,C,N,A,F, &data, &q, &x);    
      
      if(error) showError( error, "k3922Write16" );
    }

    /* Clear */

    F = 9;
    A = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);    

    /* Enable Acquisition Mode */

    F = 26;
    A = 1;
    CNAF(mem,C,N,A,F, &data, &q, &x);    

    /* Enable LAM */
    F = 26;
    A = 0;
    CNAF(mem,C,N,A,F, &data, &q, &x);    
  }
  return 0;
} 

/*#main--- main function */
 int main( argc, argv )
     short		argc;
     unsigned char	*argv[];
{
  short		error;
  char          str[256];
  unsigned char c;

  if( argc<1 || (argc>1 && !strcmp(argv[1],"-h")) ){
    printf( "%s --- Initialize LeCroy 3377\n", argv[0] );
    printf( "Usage : %s [-y]\n", argv[0] );
    printf( "   -y : Ignore the confirmation of parameters\n" );
    exit(0);
  }
  
  silent = argc>=2 && !strcmp(argv[1], "-y");

  /* Open CC device */

  mem = camOpen();
  if(mem == NULL) showError( error, "camOpen" );
  //fprintf(stdout,"Set Crate address \n");
  //cSetClear(mem,6);
  //cSetClear(mem,7);
  //  CGENZ();
  //  CGENC();
  printf( "Initialize LeCroy 3377 (Drift Chamber TDC) for GR-VDC\n" );

  while(1){
    if(checkprm())
      exit(0);

#if DEBUG
    printf("Enter setup \n");
#endif

    setup();
    if(silent)
      break;
    printf("\nTry again? y/n/q: ");
    fgets(str,strlen(str),stdin);
    c = str[0] | 0x20;
    if(c=='y')
      continue;
    break;
  }
  camClose(mem);
  return 0;
}

