// Client side C/C++ program to demonstrate Socket programming
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
#define PORT 8000
#define BSIZE 16000
void pipehandler(int signum)
{
    fprintf(stderr,"\nError happened and serever quits\n");
    exit(EXIT_FAILURE);
}
void myhandler(int signum)
{
    fprintf(stderr,"\nU cannot quit until program ends\n");
   // exit(EXIT_FAILURE);
}
void printing(long long int summ, long long int of)        //this funtion is used to print the percentage of file transfer
{
    long double perc;
    char str[100];
    perc = ((long double)summ / of) * 100;
    sprintf(str, "\rDownloading the file :%0.4Lf%%", perc);
    write(1, str, strlen(str));
}
void RecFile(int sock,char * filename)                    // this handles the recieving file part
{
    char buffer[BSIZE];
    memset(&buffer, '\0', sizeof(buffer));
    if(recv(sock,buffer,BSIZE,0)<=0)                      //after sending the file name to server server will send us the size of the file.
    {
        return;
    }
    if (strcmp(buffer, "-2") == 0)                       // -1 and -2 are the error handlings of the file in server side that we asked
    {
        printf("Some error happened at server side gonna cancel this request\n");
        return;
    }
    if(strcmp(buffer,"-1")==0)
    {
        printf("%s File Not found on server side\n\n",filename);
        return;
    }
    else
    {
        printf("%s %s\n",filename,buffer);
        long long int fsize ;
        sscanf(buffer, "%lld", &fsize);                    //taking the file size
        char filebuffer[BSIZE];
       // char *fname=(char*)malloc(sizeof(filename));
       // sprintf(fname,"./%s",filename);
        FILE *fd=fopen(filename,"wb");                     //opening the file with read write permissions if there it is over written
        if(fd==NULL)
        {
            perror("File");
            //printf("Error in creating %s file\n");
            memset(&filebuffer, '\0', sizeof(filebuffer));
            strcpy(filebuffer,"FilEnOtoPeN");
            send(sock,filebuffer,BSIZE,0);               // we will send error to the server that the file here has error to fopen 
        }
        else
        {
            memset(&filebuffer, '\0', sizeof(filebuffer));
            strcpy(filebuffer, "FilEoPeN");           // we will send file can open msg to server to further send the details
            send(sock, filebuffer, BSIZE, 0);
            long long int recsize=0;                  //recieved size 
            long long int writesize=0;             //noof bytes written
            int i=0;
            long long int recupto=0;               //no of bytes recieved upto now.
            int flag=0;
            memset(&filebuffer, '\0', sizeof(filebuffer));
            while((recsize = recv(sock,filebuffer,BSIZE,0)) >0)   //upto recv fails the while loop should run
            {
                if(i=0 && strcmp(filebuffer,"-2")==0)
                {
                    printf("Error reponce on server side\n");
                    break;
                }
                writesize=fwrite(filebuffer,1,recsize,fd);      //writing into file
                recupto+=writesize;                             //updating no of bytes written
                if(writesize<recsize)                           //if written size id less than recieved size then error happened in writing to that file
                { 
                    printf("Error occured in writing or corrupted file\n");
                    break;
                }
               // printf("\r%lld %lld\n",recupto,fsize);
                printing(recupto,fsize);                      //printing the percentage of the file
                if(recupto==fsize)                              
                {
                    break;
                }
                memset(&filebuffer, '\0', sizeof(filebuffer));
                i++;
            }
            if(recsize<0)
            {
                perror("\nin recv():");
            }
            else if(recsize==0)
            {
                printf("\nError in recv\n");
            }
            
            else
            {
                printf("\n%s File Downloaded From server\n\n",filename);
            }
            fclose(fd);
        }
      //  free(fname);
        memset(&buffer, '\0', sizeof(buffer));
    }
    return ;
}
int main(int argc, char const *argv[])
{
    signal(SIGPIPE,pipehandler);             //handling sigpipe signal to exit if we got the signal
    signal(SIGINT,myhandler);                //handling sigint signal not to exit from program
    // if(argc<2)
    // {
    //     printf("No files specified\n");
    //     return 0;
    // }
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char *hello = "exit";
    char buffer[BSIZE];
    memset(&buffer,'\0',sizeof(buffer));
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    
    memset(&serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
                                                // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)  // connect to the server address
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    char overwriting[10];
    char * sendingname;
    if(argc<2)
    {
        sendingname = (char *)malloc(BSIZE);
        // strcpy(sendingname, "get ");
        // strcat(sendingname, argv[i]);
        sprintf(sendingname, "NoFIles");
        send(sock, sendingname, BSIZE, 0);
        free(sendingname);
        printf("No files specified\n");
        return 0;
    }
    for(int i=1;i<argc;i++)                    //iterating through command line arguments 
    {
        printf("File asked: %s\n",argv[i]);
        if(access(argv[i],F_OK)>=0)                              //checking that the file is there or not in client directory
        {
            printf("%s File already exists\n",argv[i]);
            printf("If u want to overwrite then type yes or no :");
            scanf("%s",overwriting);                             //asking want to overwrite that file or not
            if(strcmp(overwriting,"no")==0)
            {
                continue;
            }
            sendingname = (char *)malloc(BSIZE);            // if want to overwrite giving this file name to RecFile() fn
            // strcpy(sendingname, "get ");
            // strcat(sendingname, argv[i]);
            sprintf(sendingname,"./%s",argv[i]);
            send(sock, sendingname, BSIZE, 0);               // sending the filename to server.
            RecFile(sock, sendingname);
            free(sendingname);
        }
        else
        {
            sendingname = (char *)malloc(BSIZE);
            // strcpy(sendingname, "get ");
            // strcat(sendingname, argv[i]);
            sprintf(sendingname,"./%s",argv[i]);
            send(sock, sendingname, BSIZE, 0); // sending the filename to server.
            RecFile(sock, sendingname);
            free(sendingname);
        }
        
    }
    
    send(sock , hello , strlen(hello) , 0 );  // send the message.
    // printf("Hello message sent\n");
    // valread = read( sock , buffer, 1024);  // receive message back from server, into the buffer
    // printf("%s\n",buffer);
    return 0;
}
