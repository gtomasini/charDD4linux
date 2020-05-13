#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256             

int main(){
   int ret, fd;
   char str[BUFFER_LENGTH];
   fd = open("/dev/basicChar", O_RDWR);        
   if (fd < 0){
      perror("failed to open the device...");
      return errno;
   }

   printf("type in a string to send to my kernel module:\n");
   scanf("%[^\n]%*c", str);
   printf("writing message to the device [%s].\n", str);
   ret = write(fd, str, strlen(str)); 
   if (ret < 0){
      perror("failed to write the message to the device.");
      return errno;
   }

   printf("reading from the device...\n");
   str[0]=0;
   ret = read(fd, str, BUFFER_LENGTH); 
   if (ret < 0){
      perror("error to read the message from the device.");
      return errno;
   }
   printf("received message from LK is: [%s]\n", str);
   return 0;
}
