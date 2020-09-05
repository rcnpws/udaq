/*   
 *
 *  grdump_local.c 
 *     GR dump at local FORCE machine
 *   Ver0.1        2003/08/19 M.Uchida
 *
 */

#include <stdio.h>
#include <time.h>

#include "hsm8170.h"
#include "hsmlib.h"
#include "lr1190.h"
#include "rpv130.h"
#include "lr1151.h"
#include "grlib.h"


#ifndef BLK_BUF_SIZE
#define BLK_BUF_SIZE 0x2000
#endif

char version[30]="GR DUMP LOCAL Ver 0.1";

#define MAX_LRM_NUM 12               /* Number of CES HSM            */
#define MAX_SCL_NUM  4               /* Number of LR1151(Scaler)     */

Lrm lrm[MAX_LRM_NUM];                /* LRM modules                  */
Rpv rpv;                             /* RPV130 module                */
unsigned short buf[BLK_BUF_SIZE];    /* Local buffer                 */
unsigned short sbuf[BLK_BUF_SIZE];   /* Local buffer for sclaer      */

unsigned long scl_sum[MAX_SCL_NUM][16];
unsigned long scl_sum_p[MAX_SCL_NUM][16];
unsigned long scl_cur[MAX_SCL_NUM][16];
int scl_count=0,scl_err_count=0;     /* Scaler error count           */
int sflag = 0,dflag = 0,cflag = 0;
int sampling_rate;
unsigned long bsize;
unsigned short sca_word;
int sca_index;

void usage()
{
  fprintf(stdout,"Usage: grdump_local [option] \n");
  fprintf(stdout,"option:-h help\n");
  fprintf(stdout,"       -s show scaler\n");
  fprintf(stdout,"       -d <n>  show n words data\n");
  fprintf(stdout,"       -c <n>  show ADC (1/n sampling)\n");
  fprintf(stdout,"       -a <n>  show scaler and n words data\n");
  exit(0);
}

void show_scaler(void ){
  int i,j;
  unsigned long sum;
  int len;

#if 1
  printf("\n              Scaler data                  Rate Meter     \n");
  printf(" ch    spin-up  spin-down   spin-sum    spin-up  spin-down\n");
  for(i=0;i<16;i++){
    printf("%3d ",i);
    sum =0;
    for(j=0;j<2;j++){
      sum += scl_sum[j][i];
      printf("%10d ",scl_sum[j][i]);
    }
    printf("%10d ",sum);
   
    for(j=0;j<2;j++){
      printf("%10d ",scl_sum[j][i]-scl_sum_p[j][i]);
    }
    printf("\n");
  }
#else

  for(j=0;j<16;j++){
    for(i=0;i<MAX_SCL_NUM;i++){
      printf("%10ld ",scl_cur[i][j]);
    }
    printf("\n");
  }
#endif

}

void check_buf(unsigned short *s, int rate);

void sighdl( arg )              /* Signal handler                 */
{
  int i,j;
  unsigned long ldata1,ldata2;
  int scl_err = 0;
  printf("\n   ******** Final Result ********\n");
  rpv->level |= RPV_OREG_VETO;


  /* Read Scaler */

  read_c256(sbuf);

  i = j = 0;
  sca_index = 0;
  while( sca_index < MAX_SCL_NUM ){
#if 1
    printf("i=%d sbuf[%d] = 0x%x\n",i, i, sbuf[i]);
#endif
    if((sbuf[i] & 0x6000) > 0){ /* Scaler header */
      sca_word = (0xff & sbuf[i++]);
      for(j=0;j<(sca_word>>1);j++){
	scl_cur[sca_index][j] = (unsigned long)  (0xffffff & (sbuf[i+2*j] | (sbuf[i+2*j+1]<<16)));
      }
      i += sca_word;
      sca_index++;
    }
  }

  for(j=0;j<2;j++){
    for(i=0;i<16;i++){
      scl_sum_p[j][i] = scl_sum[j][i];
      scl_sum[j][i] += scl_cur[j][i] + scl_cur[j+2][i];
    }
  }
      

  if(sflag == 1){
    show_scaler();
  }

  if(dflag == 1){
    pr_buf(buf,0,bsize,8);
  }

  if(cflag == 1){
    check_buf(buf,sampling_rate);     /* ADC check (sampling)  */
  }
  
  exit(1);
}


int main(int argc, char **argv)
{
  int fd;  
  int i,j,count,word;
  int vmedev1,vmedev2,vmedev3;    /* file descripter                */
  int index;

  long lrm_addr;                  /* HSM base address               */
  unsigned short rpv_addr;        /* RPV130 base address            */
  int            scl_addr;        /* LR1151 base address            */

  int      i_buf,p_buf=0;         /* Buffer index                   */

  struct timespec t1,t2;          /* For nanosleep                  */
  int buf_loop=0;

  unsigned int  dummy;            /* dummy data (unsigned long)     */
  unsigned short dat;             /* temporary data                 */
  unsigned long ldata1,ldata2;    /* unsigned long data             */


  int scl_err=0;


#if 1
  show_version(version);
#endif

  if(argc == 1){
    usage();
  }
  
  argc--;
  argv++;

  while(argc>0){
    if(**argv != '-') break;
    if(strlen(*argv) == 2){
      switch((*argv)[1]){
      case 'h':
        usage();
        break;
      case 's':  /* Show scaler */
        sflag=1;
	argc--;
        argv++;

        break;
      case 'd':  /* Show HSM    */
        dflag=1;
	argc--;
        argv++;

	
	if(argc == 0){
	  bsize = 100;
	} else {
	  bsize = atoi(*argv);
	}
	printf("dflag=%d, bsize=%d\n",dflag,bsize);

        break;
      case 'c':  /* Show ADC signal      */
	cflag = 1;
	argc--;
        argv++;

	if(argc == 0){
	  sampling_rate = 1;
	} else {
	  sampling_rate = atoi(*argv);
	}
	
	printf("cflag=%d, sampling_rate=%d\n",cflag,sampling_rate);

	break;
      case 'a':  /* Show Scaler & HSM    */
        dflag=1;
        sflag=1;
	argc--;
        argv++;

	if(argc == 0){
	  bsize = 100;
	} else {
	  bsize = atoi(*argv);
	}

        break;
      default:
        fprintf(stderr,"Illegal argument %s\n",*argv);
        usage();
      }
    } else {
      usage();
    }
    argc--;
    argv++;
  }

#if 0
	bsize = atoi(argv[1]);          /*  dump size                     */
#endif
  /*  bsize = 100; */
  /*   dflag = 1;  */


  
  t1.tv_sec  = 0;                 /*  Loop interval = 1usec         */
  t1.tv_nsec = 1000;            

  /* Open VME device(A32D16)   for HSM      */
  if((vmedev1 = open(LRM_MODE, O_RDWR)) < 0){
    fprintf(stderr,"Can't open VME device:hsm8170\n");
    exit(1);
  }




  /* Open VME device(A16D16)  for RPV130    */
  
  if((vmedev2 = open(RPV_MODE, O_RDWR)) < 0){
    fprintf(stderr,"Can't open VME device:rpv130\n");
    exit(1);
  }


#if 0
  /* Open VME device(A24D32) for LR1151     */

  if((vmedev3 = open(SCL_MODE_S, O_RDWR)) < 0){
    fprintf(stderr,"Can't open VME device:lr1151\n");
    exit(1);
  }
#endif
  
  /* Initialize all modules  */

  fprintf(stderr,"\n Now mapping RPV130 module \n");

  rpv_addr = RPV_ADDR;

  rpv = rpv_map(vmedev2,rpv_addr);
 
  /* Initialize RPV130              */

  rpv_init(rpv);       



  
  /* Load and Initialize each LRM module */  

  fprintf(stderr,"\n Now mapping LRM modules \n");
  lrm_nmap(vmedev1, LRM_ADDR, lrm, MAX_LRM_NUM, 0x100000);

  for(i=0;i<MAX_LRM_NUM;i++){

#if 1
    printf("%d-th LRM addr = 0x%x\n",i,lrm[i]->mem_ptr);    
#endif

#if 0    
    lrm_init(lrm[i]);
#else
    lrm[i]->mem[0] = 0;
#endif

    /* Check HSM status           */

#if VERBOSE
    fprintf(stderr,"\n Load module base_addr=0x%lx\n",hsm_addr);
    hsm_reg_dump(hsm[i]);
#endif

  }

  for(i=0;i<MAX_LRM_NUM;i++){

    printf("%d-th LRM addr = 0x%x\n",i,lrm[i]->mem_ptr);    

  }

#if 0
  fprintf(stderr,"\n Now mapping LR1151 modules \n");

  scl_addr = SCL_ADDR;

  for(i=0;i<MAX_SCL_NUM;i++){

    /* Mapping VME module to local memory */

    scl[i] = (struct lr1151 *) mmap(0,0xff, PROT_READ | PROT_WRITE,
				    MAP_SHARED, vmedev3, scl_addr);
    if (scl[i] == MAP_FAILED) {
      perror("Mapping: ");
      exit(1);
    }




#if VERBOSE
  fprintf(stderr,"\n Load module base_addr=0x%x\n",rpv_addr);
  rpv_dump(rpv);       
#endif


    /* Initialize each LR1151 module */

#if 0
    printf("scl init %d\n",i);

#endif

    scl_init(scl[i]);

    for(j=0;j<15;j++){
      ldata1 = scl[i]->rrwp[j];
    }

    scl_addr += 0x10000;
    
  }
#endif

  /* Enable DAQ             */

  for(i=0;i<MAX_LRM_NUM;i++){
    lrm_ena(lrm[i]);
  }

  /* Main routine                         */
  /* 1.Check OVF signal from bufchanger   */
  /* 2.Clear Ireg.                        */
  /* 3.Buf disable signal                 */
  /* 4.Buffer transfer                    */
  /* 5.Buf enable signal                  */

  /* Set counter           */

  count=0;

  /* Set signal            */

  sigset(SIGINT, sighdl);

  dat = RPV_OREG_VETO | RPV_OREG_BUF1 | RPV_OREG_BUF2;


  /* Initialize scaler sum      */
  for(j=0;j<MAX_SCL_NUM;j++){
    for(i=0;i<16;i++){     
      scl_sum[j][i]   = 0;
      scl_sum_p[j][i] = 0;
      scl_cur[j][i]   = 0;
    }
  }

  
  printf("\n DAQ Ready? Press Enter:");
  getchar();                      

  /*  DAQ Start                            */

  fprintf(stdout,"\n ****** now collector is running ****** \n\n");

#if 0 /* Old version */
  rpv_ena(rpv,dat);  /* VETO off */
#else 
  rpv_ena(rpv);     /* VETO off */
#endif
  printf("show scaler\n");
  show_scaler();
  
  while(1){

    ldata1 = ldata2 = 0;

    /* Check the status of input register */
    /* 1: Buf1 overflow                   */
    /* 2: Buf2 overflow                   */
    /* 3: Buf1,2 overflow                 */

    if(HSM_BUF_OVF(i_buf,rpv)){

      for(i=0;i<BLK_BUF_SIZE;i++){
	buf[i]=0;
      }

      rpv_dis(rpv);                      /* DAQ VETO ON          */

      rpv_clr(rpv);                      /* CLEAR RPV130         */

      rpv_dis_buf(rpv,i_buf);            /* Disable buffer       */

      /* Read Scaler */

      read_c256(sbuf);
      rpv_ena(rpv);                    /* DAQ VETO OFF         */


      i = j = 0;
      sca_index = 0;

      while( sca_index < MAX_SCL_NUM ){
#if 0
	printf("i=%d sbuf[%d] = 0x%x\n",i, i, sbuf[i]);
#endif
	if((sbuf[i] & 0x6000) > 0){ /* Scaler header */
	  sca_word = (0xff & sbuf[i++]);
	  for(j=0;j<(sca_word>>1);j++){
	    scl_cur[sca_index][j] = (unsigned long)  (0xffffff & (sbuf[i+2*j] | (sbuf[i+2*j+1]<<16)));
	  }
	  i += sca_word;
	  sca_index++;
	}
      }


      for(j=0;j<2;j++){
	for(i=0;i<16;i++){
	  scl_sum_p[j][i] = scl_sum[j][i];
	  scl_sum[j][i] += scl_cur[j][i] + scl_cur[j+2][i];
	}
      }
      
      if(dflag == 1){
	pr_buf(sbuf,0,bsize,8);                    /* Sampling output       */
      }

      if(SINGLE_BUF_OVF(i_buf)){

	count++;
	printf(" Single buf ovf status : %d  counter : %d \n",i_buf,count);

	/* Read Buffer */


	printf("show scaler \n");
	if(sflag == 1){
	  show_scaler();    /* Show scaler data      */
	}
	printf("end \n");	

	i_buf--;

	for(i=i_buf;i<MAX_LRM_NUM;i += 2){

	  if(lrm[i]->mem_ptr > 0){
	    lrm_dis(lrm[i]);                            /* Disable HSM           */

#if 1	
	    word = lrm_get2(lrm[i],buf,BLK_BUF_SIZE);    /* Read LRM data         */
#else 
	    for(index=0;index<bsize;index++){
	      buf[index] = lrm[i]->mem[index];
	    }
#endif

#if 1	    
	    printf("%d-th mem word: 0x%x, mem_ptr: 0x%x\n",i,word, lrm[i]->mem_ptr);
#else
	    printf("word = 0x%x\n",lrm[i]->mem_ptr);
#endif
	    
	    if(dflag == 1){
	      pr_buf(buf,0,bsize,8);                    /* Sampling output       */
	    }

	    if(cflag == 1){
	      check_buf(buf,sampling_rate);             /* ADC check (sampling)  */
	    }

	    lrm_reset(lrm[i]);                          /* Reset LRM             */

	    lrm_ena(lrm[i]);                            /* Enable LRM            */
	  } else {
	    lrm_reset(lrm[i]);                          /* Reset LRM             */
	  }


	}

	p_buf = i_buf++;                              /* Set current buf index */

	rpv_ena_buf(rpv,i_buf);                       /* Enable buffer         */

      } else if(DOUBLE_BUF_OVF(i_buf)){      /* Buf1,2 OVF            */

	if(sflag == 1){
	  show_scaler();
	}

	
	for(buf_loop=0;buf_loop<2;buf_loop++){


	  p_buf = TOGGLE_BUF(p_buf);         /* Toggle buffer    */

	  count++;
	  printf(" Buf status : %d(%d) count=%d \n",i_buf,p_buf+1,count);

	  for(i=p_buf;i<MAX_LRM_NUM;i += 2){
	    lrm_dis(lrm[i]);                          /* Disable LRM      */  
	    word = lrm_get2(lrm[i],buf,BLK_BUF_SIZE);  /* Get LRM data     */
	    printf("word = 0x%x\n",word);

	    if(dflag == 1){
	      pr_buf(buf,0,bsize,8);             /* Sampling output  */
	    }

	    if(cflag == 1){
	      check_buf(buf,sampling_rate);      /* ADC check (sampling)  */
	    }

	    lrm_reset(lrm[i]);                   /* Initialize HSM        */
	    lrm_ena(lrm[i]);                     /* Enable HSM            */
	  }

	}

	rpv_ena_buf(rpv,i_buf);                  /* Enable buffer         */

      } else {
	fprintf(stderr,"Undefined flag\n");
      }
    }

#if 0
    if(nanosleep(&t1,&t2) != 0){            /* Interval 1usec    */
      fprintf(stderr,"Nanosleep: Error occured!\n");
      exit(1);
    }
#endif

  }

  return 0;
  
}
  
#define ST_ADC 1
#define ST_TDC 2
#define ST_REG 3

void check_buf(unsigned short *s, int rate)
{
  int i=0,j=0;
  int st,bs,ch,cnt=0;
  unsigned short adc[16],tdc[16],reg[16];


  while( i < BLK_BUF_SIZE ){
    if( (s[i] & 0x8000) ){ /* header */
      st = s[i] & 0xf;   /* station ID */
      bs = (s[i] & 0x7800) >> 11; /* block size */
      if(bs == 0 ){
	bs = 16;
      }

#if 0
      printf("header: %x st=%d,  bs=%d\n",s[i],st,bs);
#endif
      
      switch(st){
      case ST_ADC: /* ADC */
	for(j=0;j<bs;j++){
	  ch = (s[i+j+1] & 0x7800)>>11;
	  if(ch != j){
	    printf("no event: ch %d \n",j);
	  }
	  adc[ch] = s[i+j+1] & 0x7ff;
	}
	i += bs+1;
	if(cnt%rate == 0){
	  printf(" CH   ");
	  for(j=0;j<16;j++){
	    printf("%5d",j);
	  }
	  
	  printf("\nADC%d : ",st);
	  for(j=0;j<16;j++){
	    printf("%4x ",adc[j]);
	  }
	  printf("\n");
	} 
	break;
      case ST_TDC: /* TDC */
	for(j=0;j<bs;j++){
	  tdc[j] = s[i+j+1] & 0x7ff;
	}
	i += bs+1;
	if(cnt%rate == 0){
	  printf("ADC%d : ",st);
	  for(j=0;j<bs;j++){
	    printf("%4x ",tdc[j]);
	  }
	  printf("\n");
	} 

	break;
      case ST_REG: /* REG */
	for(j=0;j<bs;j++){
	  reg[j] = s[i+j+1] & 0x7ff;
	}
	i += bs+1;

	if(cnt%rate == 0){
	  printf("ADC%d : ",st);
	  for(j=0;j<bs;j++){
	    printf("%4x ",reg[j]);
	  }
	  printf("\n\n");
	} 

	cnt++;
	
	break;
      default:
	break;
      }
    } else {
      i++;
    }
  }

}






