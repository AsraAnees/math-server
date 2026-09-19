#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <string.h>
#include <strings.h>
int port;   //port to connect with
	    
char *ip_address;   //IP of the server to connect with
	    

int Read_Options(int argc, char** argv)
{
    char* prog;

    prog = *argv;
    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++ * argv) {
            case 'p':
                --argc;
                port = atoi(*++argv);
                break;

            case 'ip':
                --argc;
                ip_address = *++argv;
                break;

            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -u \n\n", prog);
                break;
            }
}



int main(int argc, char const* argv[])
{
    int cd = socket(AF_INET, SOCK_STREAM, 0);
    port = 9001; //default port to connect with
    ip_address="127.0.0.1";	 //Default IP address to connect with
    Read_Options(argc,argv);   //Read argument Values
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port); // use some unused port number
    servAddr.sin_addr.s_addr = inet_addr(ip_address);
    
    int connectStatus = connect(cd, &servAddr, sizeof(servAddr));
  
    if (connectStatus == -1) {
        printf("Error Connecting to server...\n");
    }
    else {
        printf("Connected to server\n\n");
        char strCmd[255];
        int n;
        for(;;){
        	bzero(strCmd,sizeof(strCmd));
        	printf("Enter a Command for the server : ");
        	n=0;
        	while((strCmd[n++]=getchar())!='\n');
        	write(cd,strCmd,sizeof(strCmd));
        	bzero(strCmd,sizeof(strCmd));
        	read(cd,strCmd,sizeof(strCmd));
        	printf("Received the solution : %s \n",strCmd);
        }
    }
    return 0;
}

