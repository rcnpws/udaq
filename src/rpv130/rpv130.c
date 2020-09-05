/*
 *
 * rpv130.c
 * Sample program to access RPV130
 * VMIC Pentium SBC version
 *              2005/05/20 M.Uchida
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rpv130.h"

typedef unsigned long  DWORD;
typedef DWORD  *DWORDP;

int main(int argc, char **argv){

  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;
  /*  uint8_t *ptr; */
  char *phys_addr;
  unsigned short command, dat;
  int j,k,select;

  Rpv rpv;

  /*  int ii; */

  if(vme_init(&bus_handle)){
    perror("Error initializing the VMEbus");
    return -1;
  }

  if (vme_master_window_create(bus_handle, &window_handle,
                               RPV_ADDR, RPV_MODE, RPV_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    return -1;
  }

#if 0
  phys_addr = (char *) vme_master_window_phys_addr(bus_handle, window_handle);
  fprintf(stdout,"VME master window addrress: 0x%lx\n",(unsigned long)phys_addr);
#endif

  rpv = (struct rpv130 *) vme_master_window_map(bus_handle, window_handle, 0);
  if(!rpv){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }


  while(1){
    printf("*********** menu **********\n");
    printf("Output register test(P) : 1\n");
    printf("Output register test(L) : 2\n");
    printf("Interrupt register test : 3\n");
    printf("Read  a register        : 4\n");
    printf("Write a register        : 5\n");
    printf("Exit                    : 6\n");
    printf("Input ?>");
    scanf ("%d",&select);
    switch(select){
    case 1:
      printf("input data>");
      scanf("%hx",&dat);
      outpuls( rpv, dat);
      break;
    case 2:
      printf("input data>");
      scanf("%hx",&dat);
      outlevel( rpv, dat);
      break;
    case 3:
      fprintf(stdout,"Not available\n");
      /*      test_funct(fd); */
      break;
    case 4:
      printf("Which register do you wanna read form?\n");
      printf("Latch1 =0  Latch2 =1  R/S FF =2  THROUGH =3  CSR1 =6  CSR2 =7\n");
      printf(">");
      scanf("%d",&k);

      switch(k){
      case 0:
             dat = rpv->latch1;
             break;
      case 1:
             dat = rpv->latch2;
             break;
      case 2:
             dat = rpv->ff;
	     printf("Data: 0x%x .\n",rpv->ff&0xff);
             break;
      case 3:
             dat = rpv->thr;
             break;
      case 6:
             dat = rpv->csr1;
             break;
      case 7:
             dat = rpv->csr2;
             break;
      default:
        printf("Invalid number!\n");
        exit(1);
      }
      printf("Data: 0x%x .\n",dat&0xff);
      break;
    case 5:
      printf("Which register do you wanna write on?\n");
      printf("Pulse:4  Level:5  CSR1:6  CSR2:7\n");
      printf(">");
      scanf("%d",&j);

      if(j<=3 || j>=8) {
        printf("Invalid number!\n");
        exit(1);
      } else {
        printf("Please write a command. >0x");
        scanf("%hx",&command);
        command = command & 0xff;
        *((unsigned short *)rpv+j) = command;
      }

      break;
    case 6:
      printf("Thank you!\n");
      exit(0);
    default:
      exit(1);
    }
    printf(".......done\n");
  }


  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("Error unmapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle)){
    perror("Error releasing the window");
    vme_term(bus_handle);
    return -1;
  }

  if(vme_term(bus_handle)){
    perror("Error terminating");
    return -1;
  }

  return 0;
}

void outpuls( Rpv rpv, unsigned short dat)
{
  int i;

  printf("Now Pulsing %X\n", dat);
  while(1) {
    rpv->pulse = dat;
    for(i=0;i<10000;i++){}
    for(i=0;i<10000;i++){}
  }
}

void outlevel( Rpv rpv, unsigned short dat)
{

  printf("Now Pulsing %X\n", dat);
  rpv->level = dat;
}
