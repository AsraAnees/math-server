#include <netinet/in.h> //structure for storing address information
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h> //for socket APIs
#include <sys/types.h>
#include <sys/time.h> //select datastructure
#include <math.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_SIZE 4096


#define MAX_POINTS 4096
#define MAX_CLUSTERS 32
 

pthread_mutex_t pl = PTHREAD_MUTEX_INITIALIZER;

typedef struct point
{
    float x; // The x-coordinate of the point
    float y; // The y-coordinate of the point
    int cluster; // The cluster that the point belongs to
} point;
bool something_changed;
int	N;		// number of entries in the data
int k;      // number of centroids
point data[MAX_POINTS];		// Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also called centroid)

void read_data(int i,char* name)
{
    N = 1797;
    k = i;
    FILE* fp = fopen(name, "r");
    if (fp == NULL) {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }
   
    // Initialize points from the data file
    float temp;
    for (int i = 0; i < N; i++)
    {
        fscanf(fp, "%f %f", &data[i].x, &data[i].y);
        data[i].cluster = -1; // Initialize the cluster number to -1
    }
    printf("Read the problem data!\n");
    // Initialize centroids randomly
    srand(0); // Setting 0 as the random number generation seed
    for (int i = 0; i < k; i++)
    {
        int r = rand() % N;
        cluster[i].x = data[r].x;
        cluster[i].y = data[r].y;
    }
    fclose(fp);
}

int get_closest_centroid(int i, int k)
{
    /* find the nearest centroid */
    int nearest_cluster = -1;
    double xdist, ydist, dist, min_dist;
    min_dist = dist = INT_MAX;
    for (int c = 0; c < k; c++)
    { // For each centroid
        // Calculate the square of the Euclidean distance between that centroid and the point
        xdist = data[i].x - cluster[c].x;
        ydist = data[i].y - cluster[c].y;
        dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
        //printf("%.2lf \n", dist);
        if (dist <= min_dist)
        {
            min_dist = dist;
            nearest_cluster = c;
        }
    }
    //printf("-----------\n");
    return nearest_cluster;
}

void* clusterPointAssign(void *arg)
{
     
    int old_cluster = -1, new_cluster = -1;
    something_changed = false;
    for (int i = 0; i < N; i++)
    { // For each data point
        old_cluster = data[i].cluster;
        new_cluster = get_closest_centroid(i, k);
        data[i].cluster = new_cluster; // Assign a cluster to the point i
        if (old_cluster != new_cluster)
        {
            something_changed = true;
        }
    }
    return (void*) &something_changed;
}

void* update_cluster_centers(void *arg)
{
    /* Update the cluster centers */
    int c;
    int count[MAX_CLUSTERS] = { 0 }; // Array to keep track of the number of points in each cluster
    point temp[MAX_CLUSTERS] = { 0.0 };

    for (int i = 0; i < N; i++)
    {
        c = data[i].cluster;
        count[c]++;
        temp[c].x += data[i].x;
        temp[c].y += data[i].y;
    }
    for (int i = 0; i < k; i++)
    {
        cluster[i].x = temp[i].x / count[i];
        cluster[i].y = temp[i].y / count[i];
    }
    return NULL;
}

int kmeans(int k)
{
 pthread_t assignThread;
 pthread_t Pthread;

    int i = 0;
    bool* somechange;
    
   

    do {
    
        i++; 
      
           pthread_create(&assignThread,NULL,clusterPointAssign,NULL);
        
              pthread_join(assignThread,(void**) &somechange);
          
        pthread_create(&Pthread,NULL,update_cluster_centers,NULL);
        
    pthread_mutex_lock(&pl);
        
           pthread_join(Pthread,NULL);
         
      pthread_mutex_unlock(&pl);
       
    } while (*somechange);
    
  
    printf("iterations = %d\n", i);
    printf("cluster evaluate successfully..!\n");
  
}






typedef double matrix[MAX_SIZE][MAX_SIZE];


pthread_mutex_t l = PTHREAD_MUTEX_INITIALIZER;

int	N;		/* matrix size		*/
int	maxnum;		/* max number of element*/
char* Init;		/* matrix init type	*/
int	PRINT;		/* print switch		*/
matrix	A;		/* matrix A		*/
matrix I = {0.0};  /* The A inverse matrix, which will be initialized to the identity matrix */

/* forward declarations */
void find_inverse(void);
void Init_Matrix(void);
void Print_Matrix(matrix M, char name[]);
void Init_Default(void);




int port;   //To assign port from argument
char *strat;
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
            case 'h':
                printf("HELP: This server calculate Matrix Inverse and K mean Clusters.\n");
                printf("-p : to specify Port of this server.\n");
                printf("-d : to run this server as a daemon.\n");
                printf("-s : to specify the strategy to use while running this server ex. fork, muxbasic, or muxscale\n");
                exit(1);
            case 's':
                --argc;
                strat = *++argv;
                break;
            case 'd':
                --argc;
            	pid_t process_id = fork();
                
                if(process_id>0){
                	printf("Program Process ID is : %d\n",process_id);
                	exit(0);
                }
                close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
                break;
            default:
                
              	printf("%s: ignored option: -%s\n", prog, *argv);
                printf("HELP: try %s -h \n\n", prog);
                break;
            }
}

int main(int argc, char const* argv[])
{
    // Create a socket
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    port = 9001; // default server port
    strat = "fork"; //by default using fork
    Read_Options(argc,argv); //reading server options
    // define server address and bind!
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = INADDR_ANY;
    // bind socket to the specified IP and port
    bind(sd, &servAddr, sizeof(servAddr));
    
    // listen for connections
    listen(sd, 10);
    int count =0;
    int cd;
    pid_t childProcess;
    printf("Listening for Clients...\n");
    printf("Strategy used is %s\n",strat);
    if(strcmp(strat,"fork")==0){ 
    	while(1){
    		char cmd[255];
    		// file descriptor referring to the client socket
    		cd = accept(sd, NULL, NULL);
    		if (cd < 0) {
        	    exit(1);
        	}
    		count++;
    		childProcess = fork();
    		if (childProcess == 0) {
 	
        	    // Closing the server socket id
        	    close(sd);
        	    printf("Connected with client %d \n",count);
        	    int matNum=0,kmeanNum=0;
        	    for(;;){
 		    	
 		    	bzero(cmd,sizeof(cmd));
        	    	
 		    	read(cd,cmd,sizeof(cmd));
        	    	printf("Client %d commanded : %s \n",count,cmd);
        	    	
        	    	char *parts = strtok(cmd, " ");
        	    	char *cmdArr[255];
        	    	char outfile_name[255];
        	    	int c=0;
        	    	while(parts!=NULL){
        	    		cmdArr[c++]=parts;
        	    		parts=strtok(NULL," ");
        	    	}
        	    	
        	    	
        	    	if(strcmp(cmdArr[0],"matinvpar")==0){
        	    		matNum++;
        	    		sprintf(outfile_name, "Matinv_client%d_soln%d.txt",count,matNum);
        	    		
        	    		freopen(outfile_name, "a+", stdout); 
        	    		
        	    		printf("Matrix Inverse\n");
    				int i, timestart, timeend, iter;
    
    				Init_Default();		/* Init default values	*/
    				int xx=0;
    				/*while(cmdArr[xx]!=NULL&&cmdArr[xx]!=""){
    					if(strcmp(cmdArr[xx],"-n")==0){
    						N = atoi(cmdArr[xx+1]);
    					}else if(strcmp(cmdArr[xx],"-h")==0){
    						printf("\nHELP: try matinv -u \n\n");
                				
    					}else if(strcmp(cmdArr[xx],"-u")==0){
    						printf("\nUsage: matinv [-n problemsize]\n");
                				printf("           [-D] show default values \n");
                				printf("           [-h] help \n");
                				printf("           [-I init_type] fast/rand \n");
                				printf("           [-m maxnum] max random no \n");
                				printf("           [-P print_switch] 0/1 \n");
                				
    					}else if(strcmp(cmdArr[xx],"-D")==0){
    						printf("\nDefault:  n         = %d ", N);
                				printf("\n          Init      = fast");
                				printf("\n          maxnum    = 5 ");
                				printf("\n          P         = 0 \n\n");
                				
    					}else if(strcmp(cmdArr[xx],"-I")==0){
    						Init = cmdArr[xx+1];
    					}else if(strcmp(cmdArr[xx],"-P")==0){
    						PRINT = atoi(cmdArr[xx+1]);
    					}else if(strcmp(cmdArr[xx],"-m")==0){
    						maxnum = atoi(cmdArr[xx+1]);
    					}
    					xx++;
    				}*/
    				
    				
    				
    				
    				
    				Init_Matrix();		/* Init the matrix	*/
    				find_inverse();

    				if (PRINT == 1)
    				{
        				//Print_Matrix(A, "End: Input");
        				Print_Matrix(I, "Inversed");
    				}
        	    		freopen("/dev/tty", "w", stdout);
        	    	}else if(strcmp(cmdArr[0],"kmeanspar")==0){
        	    		kmeanNum++;
        	    		sprintf(outfile_name, "Kmeans_client%d_soln%d.txt",count,kmeanNum);
        	    		freopen(outfile_name, "a+", stdout);
        	    		k=9;
        	    		char file[255];
        	    		strcpy(file, "kmeans-data.txt");
        	    		int xx=0;
        	    		/*while(cmdArr[xx]!=NULL&&cmdArr[xx]!=""){
    					if(strcmp(cmdArr[xx],"-k")==0){
    						k = atoi(cmdArr[xx+1]);
    					}else if(strcmp(cmdArr[xx],"-f")==0){
    						strcpy(file,cmdArr[xx+1]);
                			}
                			xx++;
                		}*/
        	    		read_data(k,file); 
    				kmeans(k);
    				for (int i = 0; i < N; i++)
        			{
        				printf("%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
        			}
        			
    				freopen("/dev/tty", "w", stdout);
        	    	}else{
        	    		strcpy(outfile_name, "Invalid Command.");
        	    	}
        	    	bzero(cmd,sizeof(cmd));
        	    	strcpy(cmd, outfile_name);
        	    	
        	    	write(cd,cmd,sizeof(cmd));
        	    	printf("Sending Solution : %s \n",cmd);
        	    }
        	    
        	}
    	}
    	close(cd);
    }
    //using max client connect as 20
    if(strcmp(strat,"muxbasic")==0){
    	fd_set reads;
    	int cs[20];
    	int maxsd,soc,conn,val;
    	for(int i=0;i<20;i++){
    		cs[i]=0;	
    	}
    	while(1){
    		FD_ZERO(&reads);
    		FD_SET(sd,&reads);
    		char cmd[255];
    		maxsd = sd;
    		//for now max clients setting default 20
    		for(int i=0;i<20;i++){
    			soc=cs[i];
    			if(soc>0){
    				FD_SET(soc,&reads);
    			}
    			if(soc>maxsd){
    				maxsd=soc;
    			}
    		}
    		conn = select(maxsd+1,&reads,NULL,NULL,NULL);
    		if(conn<0){
    			printf("MUXBASIC ERROR!");
    		}
    		if(FD_ISSET(sd,&reads)){
    			cd = accept(sd, NULL, NULL);
    			if (cd < 0) {
        	    		exit(1);
        		}
    			count++;
    			
    			for(int i=0;i<20;i++){
    				if(cs[i]==0){
    					cs[i]=cd;
    					break;
    				}
    			}
    			printf("Connected with client %d \n",count);
    			bzero(cmd,sizeof(cmd));
        		read(cd,cmd,sizeof(cmd));
        		printf("Client %d commanded : %s\n",count,cmd);
        	    	bzero(cmd,sizeof(cmd));
        	    	write(cd,cmd,sizeof(cmd));
        	    	printf("Sending Solution : %s \n",cmd);
        	    
        	}else{
        	for(int i=0;i<20;i++){
        		soc = cs[i];
        		if(FD_ISSET(soc,&reads)){
        			if((val=read(cd,cmd,sizeof(cmd)))==0){
        				getpeername(soc,NULL,NULL);
        				close(soc);
        				cs[i]=0;
        			}else{
        				bzero(cmd,sizeof(cmd));
        				read(cd,cmd,sizeof(cmd));
        				printf("Client %d commanded : %s\n",count,cmd);
        	    			bzero(cmd,sizeof(cmd));
        	    			write(cd,cmd,sizeof(cmd));
        	    			printf("Sending Solution : %s \n",cmd);
        			}
        		}
        	}
    	}}
    	close(cd);
    }
    if(strcmp(strat,"muxscale")==0){
    	exit(3);
    }
    return 0;
}

void* eliminationFun(void *arg){

 int row, col;
 double pivalue;
 
 int p = (int *)arg;
 
  double multiplier;
        for (row = 0; row < N; row++) {
            multiplier = A[row][p];
            if (row != p) // Perform elimination on all except the current pivot row 
            {
                for (col = 0; col < N; col++)
                {
                    A[row][col] = A[row][col] - A[p][col] * multiplier; /* Elimination step on A */
                    I[row][col] = I[row][col] - I[p][col] * multiplier; /* Elimination step on I */
                }      
                assert(A[row][p] == 0.0);
            }
        }
        
          pthread_exit(NULL);
}

void* identityFun(void *arg){


 int row, col; // 'p' stands for pivot (numbered from 0 to N-1)
    double pivalue; // pivot value
    int p = (int *)arg;

    /* Bringing the matrix A to the identity form */
     /* Outer loop */
        pivalue = A[p][p];
        for (col = 0; col < N; col++)
        {
            A[p][col] = A[p][col] / pivalue; /* Division step on A */
            I[p][col] = I[p][col] / pivalue; /* Division step on I */
        }
        assert(A[p][p] == 1.0);


  pthread_exit(NULL);

}

void find_inverse()
{
      pthread_t matrixElimination;
      
    pthread_t matrixIdentity;
    
    int row, col, p; // 'p' stands for pivot (numbered from 0 to N-1)
    double pivalue; // pivot value

    /* Bringing the matrix A to the identity form */
    for (p = 0; p < N; p++) { /* Outer loop */
     pthread_create(&matrixIdentity,NULL,identityFun,p);
     pthread_join(matrixIdentity, NULL);
     
     
      pthread_create(&matrixElimination,NULL,eliminationFun,p);
      
       pthread_mutex_lock(&l);
      
      pthread_join(matrixElimination, NULL);
      
        pthread_mutex_unlock(&l);
       
     
     
        }
    }




void
Init_Matrix()
{
    int row, col;

    // Set the diagonal elements of the inverse matrix to 1.0
    // So that you get an identity matrix to begin with
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++) {
            if (row == col)
                I[row][col] = 1.0;
        }
    }

    printf("\nsize      = %dx%d ", N, N);
    printf("\nmaxnum    = %d \n", maxnum);
    printf("Init	  = %s \n", Init);
    printf("Initializing matrix...");

    if (strcmp(Init, "rand") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = (double)(rand() % maxnum) + 5.0;
                else
                    A[row][col] = (double)(rand() % maxnum) + 1.0;
            }
        }
    }
    if (strcmp(Init, "fast") == 0) {
        for (row = 0; row < N; row++) {
            for (col = 0; col < N; col++) {
                if (row == col) /* diagonal dominance */
                    A[row][col] = 5.0;
                else
                    A[row][col] = 2.0;
            }
        }
    }

    printf("done \n\n");
    if (PRINT == 1)
    {
        //Print_Matrix(A, "Begin: Input");
        //Print_Matrix(I, "Begin: Inverse");
    }
}

void
Print_Matrix(matrix M, char name[])
{
    int row, col;

    printf("%s Matrix:\n", name);
    for (row = 0; row < N; row++) {
        for (col = 0; col < N; col++)
            printf(" %5.2f", M[row][col]);
        printf("\n");
    }
    printf("\n\n");
}

void
Init_Default()
{
    N = 5;
    Init = "fast";
    maxnum = 15.0;
    PRINT = 1;
}
