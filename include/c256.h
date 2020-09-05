#include <vme/vme.h>
#include <vme/vme_api.h>



/*
unsigned short *c256_open(vme_bus_handle_t bus_handle,int c);
void c256_close(unsigned short *mem,vme_bus_handle_t bus_handle);
void c256_reset(unsigned short *mem,int c);
void c256_read_dbuf_mode(unsigned short *mem, unsigned short sbuf[], int ch);
*/


struct K_REG *c256_open(vme_bus_handle_t bus_handle,int c);
void c256_close(struct K_REG *k2917,vme_bus_handle_t bus_handle);
void c256_reset(struct K_REG *k2917,int c);
void c256_read_dbuf_mode(struct K_REG *k2917, unsigned short sbuf[], int ch);
