/*
 *
 * k2917.h
 *   K2917 Kinetic K-BUS interface board
 *   Header file
 *                          2001/08/02   M.Uchida
 *                          2001/08/04   M.Uchida
 *   Modified for VMIVME7750
 *                          2005/05/20   M.Uchida
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>


#if 1
#define K2917_MODE  VME_A16S            /* VME trasfer mode    */
#else
#define K2917_MODE  VME_A16U            /* VME trasfer mode    */
#endif

#define K2917_SIZE  0x0071              /* VME window size     */

/* Definition of K2917 address map */

#define K2917_ADDR                0xff00           
#define K2917_CHANNEL_STAT        K2917_ADDR       
#define K2917_DEV_CONT            (K2917_ADDR+0x4)   
#define K2917_SEQ_CONT            (K2917_ADDR+0x6)   
#define K2917_MEM_TRANS           (K2917_ADDR+0xa)   
#define K2917_MEM_ADD_H           (K2917_ADDR+0xc)   
#define K2917_MEM_ADD_L           (K2917_ADDR+0xe)   
#define K2917_LAM_INT             (K2917_ADDR+0x40)   
#define K2917_DANE_INT            (K2917_ADDR+0x42)
#define K2917_DMA_BUF_EMPTY       (K2917_ADDR+0x44)
#define K2917_LIST_ABORT_INT_CONT (K2917_ADDR+0x46)
#define K2917_LAM_INT_VEC         (K2917_ADDR+0x48)
#define K2917_DANE_INT_VEC        (K2917_ADDR+0x4a)
#define K2917_BUF_EMPTY_INT       (K2917_ADDR+0x4c)
#define K2917_LIST_ABORT_INT_VEC  (K2917_ADDR+0x4e)
#define K2917_ADD_MOD             (K2917_ADDR+0x60)
#define K2917_COM_MEM             (K2917_ADDR+0x62)
#define K2917_COM_MEM_ADD         (K2917_ADDR+0x64)
#define K2917_COM_WORD_CNT        (K2917_ADDR+0x66)
#define K2917_SERVE_REQ           (K2917_ADDR+0x68)
#define K2917_DATA_LOW            (K2917_ADDR+0x6a)
#define K2917_DATA_HIGH           (K2917_ADDR+0x6c)
#define K2917_CONT_STATUS         (K2917_ADDR+0x6e)

typedef unsigned short  W;

typedef struct {
  W cse;
  W notused1;
  W doc;
  W scc;
  W notused2;
  W mtc;
  W machi;
  W maclo;
  W notused3[16];
  W lic;
  W dic;
  W dbeic;
  W laic;
  W liv;
  W div;
  W dbeiv;
  W laiv;
  W notused4[6];
  W amr;
  W cmr;
  W cma;
  W cwc;
  W srr;
  W dlr;
  W dhr;
  W csr;
} *K2917;



