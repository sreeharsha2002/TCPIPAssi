# Assignment 6 Sockets
## How to run the Code
* I have kept client.c in client folder and server.c in server folder.
* So for running them we first go to server folder and do
```C
gcc server.c
./a.out
``` 
* After goto client folder and do 
```C
gcc client.c
./a.out <filename> <filname> ....
```
Here I am implementing the get files thing as command line arguments passing to client.
## Implementation
* In client we are passing the filenames as command line arguments.
* If no files given then client will send the a acknowledgement to server.
* In client.c in I am handling the CTRL-C and SIGPIPE and sever.c also handles this.
* At first I am Sending the filename to server, And if the file in server is ready to transfer(i.e able to transfer) server sends filesize to client.
* After the filesize recieved the client creates the file and sends the acknowledgement to server, After this server starts sending the contents to client This sending and recieving in server and client will be done in while loop upto the readsize and recvsize reaches file size.
* After sending and recieving, client sends the filenames and do the process again upto the requested number of files.
* After files downloaded the client sends a exit string to server, When server recieves it, it disconnects from the client and starts waiting for the other client.
* Appropriate error handlings are done when file is opening or creating or sending and  recieving in client and server codes.
* According Assignment PDF I am printing the percentage downloaded thing when client is downloading and percentage transferring when the server is transfering.
* **I have wrriten the comments in both code, Please refer the comments for more explaination**  