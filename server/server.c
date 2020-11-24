#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#define PORT 8000
#define BSIZE 16000
void printing(long long int summ, long long int of)         //printing the percentage of transfering
{
    long double perc;
    char str[100];
    perc = ((long double)summ / of) * 100;
    sprintf(str, "\rTransfering the file :%0.4Lf%%", perc);
    write(1, str, strlen(str));
}
void pipehandler(int signum)
{
    printf("\nError occured\n");
}
void myhandler(int signum)
{
    fprintf(stderr, "\nServer is shutting down\n");
    exit(EXIT_FAILURE);
}
void sendf(int new_socket,char * filename,long long int fsize)     //handles sending file
{
    char filebuffer[BSIZE]; 
    FILE * fd=fopen(filename,"r");                  //opening the asked file with read permission
    memset(&filebuffer, '\0', sizeof(filebuffer));
    long long int sendsize = 0;                  //noof bytes sending
    long long int readsize = 0;                 //noof bytes reading
    //int i = 0;
    long long int sendupto = 0;                     //noof bytes sent upto now
    memset(&filebuffer, '\0', sizeof(filebuffer));
    int flag=0;
    while(1)                                     //we have to send data until breaking
    {
        if((readsize=fread(filebuffer,1,BSIZE,fd))<0)           //if fread failed then breaking from the loop
        {
            printf("\nFiled to read\n");
            flag=1;
            break;
        }
        if(send(new_socket,filebuffer,readsize,0)<0)             //if send failed then break th eloop
        {
            printf("\nFailed to send\n");
            flag=1;
            break;
        }
        sendupto+=readsize;                              //updating the noof bytes sent 
        memset(&filebuffer, '\0', sizeof(filebuffer));
        printing(sendupto,fsize);
        if(sendupto==fsize)
        {
            break;
        }
    }
    if(flag==0)
        printf("\n%s File transfer completed\n",filename);
    fclose(fd);
    return ;
   
}
int main(int argc, char const *argv[])
{
    signal(SIGPIPE, pipehandler);
    signal(SIGINT, myhandler);
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BSIZE];
    memset(&buffer, '\0', sizeof(buffer));
    // char *hello = "Hello from server";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address,
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1)      //while loop for connecting multiple clients
    {
    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                        (socklen_t*)&addrlen))<0)
        {
            perror("accept");
            continue;
        }
        printf("Client connected to server\n");
        while(1)                                //each client asking one or more files
        {
            if(recv(new_socket,buffer,BSIZE,0)<0)       //receving the file name
            {
                perror("recv failed");
                break;
            }
            if(strlen(buffer)==0)
            {
                printf("Disconnecting client\n");
                break;
            }
            char filebuffer[BSIZE];
            memset(&filebuffer, '\0', sizeof(filebuffer));
            if(strcmp(buffer,"NoFIles")==0)
            {
                printf("No file specified\n");
                printf("Disconnecting client\n");
                break;
            }
            if (strcmp(buffer, "exit") == 0)
            {
                printf("Disconnecting client\n");
                break;
            }
            printf("\nFile giving : %s\n", buffer);
            if(access(buffer,R_OK)<0)                       //checking that the file asked have read permission 
            {

                perror("File");
                strcpy(filebuffer,"-1");
                send(new_socket,filebuffer,BSIZE,0);         //sending -1 as error occured 
            }
            else
            {
                struct stat st;
                long long int fsize;
                memset(&filebuffer, '\0', sizeof(filebuffer));
                if(stat(buffer, &st)==0)                  //finding file size
                {
                    fsize=st.st_size;
                    sprintf(filebuffer,"%lld",fsize);
                    send(new_socket,filebuffer,BSIZE,0);        //sending the file size to client
                    memset(&filebuffer, '\0', sizeof(filebuffer));
                    recv(new_socket,filebuffer,BSIZE,0);       //server will recieve that the file in client has an error or not
                    if (strcmp(filebuffer, "FilEnOtoPeN")==0)
                    {
                        printf("File is not opened in client so leaving\n");
                    }
                    else if (strcmp(filebuffer, "FilEoPeN")==0)
                    {
                        sendf(new_socket, buffer,fsize);     //if it is fileopen then we will pass it to sendf()
                    }
                }
                else                                //if the stat becomes and erro then send this error to client
                {
                    perror("Filesize thing");
                    strcpy(filebuffer, "-2");
                    send(new_socket, filebuffer, BSIZE, 0);
                }
                
            }
            
            memset(&buffer, '\0', sizeof(buffer));
        }

    }
    
    // valread = read(new_socket , buffer, 1024);  // read infromation received into the buffer
    // printf("%s\n",buffer);
    // send(new_socket , hello , strlen(hello) , 0 );  // use sendto() and recvfrom() for DGRAM
    // printf("Hello message sent\n");
    return 0;
}
