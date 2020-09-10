#include "tcplib.h"

int main(int argc, char **argv)
{
  int sockfd;

  if(argc != 3){
	err_quit("usage:cli <host> <port#>\n");
  }

  sockfd = Tcp_connect(argv[1],argv[2]);

  Write(sockfd,"123",3);
  printf("wrote 3byte of normal data\n");

  sleep(1);

  Send(sockfd,"4",1,MSG_OOB);
  
  

  
	
  
