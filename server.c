/**
 * @Author: Atul Sahay <atul>
 * @Date:   2018-09-24T19:32:51+05:30
 * @Email:  atulsahay01@gmail.com
 * @Last modified by:   atul
 * @Last modified time: 2018-10-06T19:54:44+05:30
 */

 /*

   1. MAXITERATIONS = macro used to control the numbers to produce by the
                       producer. For our case 10K.
   2. MAXBUFFERSIZE = buffer length at max
   3. SLEEP_NANOSEC_CONS, SLEEP_NANOSEC_PROD = to provide extra delay in the
                     producer and consumer routine, so that the other can do
                     their routine more frequently
   4. mutex = for buffer lock, at any time only 1 thread is allowed to work on it
   5. condp = condition variable, controlling the routine when buffer gets full
   6. condc = condition variable, controlling the routine when buffer gets Empty
   7. queue_count = size of the list at any timespec
   8.
 */


  #include <stdio.h>
  #include <pthread.h>
  #include <stdbool.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
  #include <signal.h>
  #include <netdb.h>
  #include <netinet/in.h>

  #define MAXITERATIONS 10001			/* Numbers to produce */
  #define MAXBUFFERSIZE 3
  #define SLEEP_NANOSEC_PROD 100
  #define SLEEP_NANOSEC_CONS 100
  #define CONSTHREAD 100

  /////////////////////      Global definitions
  pthread_mutex_t mutex;
  pthread_cond_t condc, condp;

  // for data processing
  pthread_mutex_t Pmutex;
  pthread_cond_t reader_can_enter, writer_can_enter;
  bool writer_present;
  int readcount,writer_waiting;
  // Ends here

  // Buffer vars are written here...............
  int buffer[MAXBUFFERSIZE];
  int front,rear;
  int written;
  int queue_count;
  int numT;
  /* Hashtable is used for global access of the key value pairs
   pointer to pointer is used to have char array of pointers so that dynamic allocation
  of keys can be done*/
  char **hashTable;
  int *bitmap; /* Whether that particular key is present or not */

  // Networking Things (Socket Programming essentials)
  int sockfd, newsockfd, portno, clilen;
  struct sockaddr_in serv_addr, cli_addr;
  //////////////////////////////////////////ENDS:::: Global definitions

  //******************************** Funtion declarations*************************//

  //********************** Read Instruction is performed here *******************//
  void readInstruction(char buffer[], int sock, int thread_id);
  //******************* All data base modification is done *********************//
  void modifyInstruction(char buffer[], int sock, int thread_id);

  // *****************  Locks and Unlocks Methods*********************/
  void readLock(int thread_id);
  void readUnLock(int thread_id);
  void writeLock(int thread_id);
  void writeUnLock(int thread_id);

  //-------------- Query Processing --------------------------------//
  void doprocessing (int sock, int thread_id);
  // Buffer insertion and deletion is defined here
  void insert(int value);
  int release();
  // Clean up routine is presented however it is incomplete
  void cleanup_routine();
  /* producer stores the incomming connection in the buffer so that consumer thread cleanup_routine
  act upon it */
  void* producer(void *ptr);
  /* pops the socket from the queue and do the necessary operations on the request of user*/
  void* consumer(void *ptr);


  //******************************* Driver function
  int main(int argc, char **argv) {
    front = 0;
    rear = -1;
    queue_count = 0;
    written=0;
    numT=0;
    //for data  Processing
    writer_present = false;
    writer_waiting = 0;
    readcount = 0; // how many readers are present
    // Initialize the mutex and condition variables
    pthread_mutex_init(&Pmutex, NULL);
    pthread_cond_init(&reader_can_enter, NULL);		/* Initialize reader condition variable */
    pthread_cond_init(&writer_can_enter, NULL);		/* Initialize writer condition variable */
    // Ends
    pthread_t pro, con[CONSTHREAD]; //con1,con2,con3,con4; // Pthreads

    // Networking things

     /* First call to socket() function */
     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
     }

     int enable = 1;
     if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");
     /* Initialize socket structure */
     bzero((char *) &serv_addr, sizeof(serv_addr));
     // portno = 5002;
     // Command line address and portno is passed
     char address[1024];
     char portStr[1024];
     strcpy(address,argv[1]);
     strcpy(portStr,argv[2]);
     // for string to int
     int port;
     sscanf(portStr, "%d", &port);
     // int address;
     // sscanf(addressStr, "%d", &address);

     serv_addr.sin_family = AF_INET;
     // address.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_addr.s_addr = inet_addr(address);
     serv_addr.sin_port = htons(port);

     /* Now bind the host address using bind() call.*/
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
     }

     /* Now start listening for the clients, here
        * process will go in sleep mode and will wait
        * for the incoming connection
     */

     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     hashTable = (char **)malloc(10000008*sizeof(char *));
     bitmap = (int *)malloc(10000008*sizeof(int *));
     memset(bitmap, 0, sizeof(bitmap));
    // Ends here

    // Ids for every pthread
    int prod_thread_id = 0;
    // int con1_thread_id = 1;
    // int con2_thread_id = 2;
    // int con3_thread_id = 3;
    // int con4_thread_id = 4;
    int con_thread_id[CONSTHREAD];
    int i = 0;
    for(i = 0 ; i < CONSTHREAD ; i++)
    {
        con_thread_id[i] = i+1;
    }
    // Initialize the mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condc, NULL);		/* Initialize consumer condition variable */
    pthread_cond_init(&condp, NULL);		/* Initialize producer condition variable */
    // Creation of threads
    pthread_create(&pro, NULL, producer, (void *)&prod_thread_id);
    for(i = 0 ; i < CONSTHREAD ; i++)
    {
        pthread_create(&con[i], NULL, consumer, (void *)&con_thread_id[i]);
    }
    // pthread_create(&con3, NULL, consumer, (void *)&con3_thread_id);
    // pthread_create(&con4, NULL, consumer, (void *)&con4_thread_id);
    // pthread_create(&con2, NULL, consumer, (void *)&con2_thread_id);
    // pthread_create(&con1, NULL, consumer, (void *)&con1_thread_id);

    sleep(1000);
    printf("Captured Error: Resources not allocated\n");
    // // For threads to join (Once they complete their tasks)
    // pthread_join(&con1, NULL);
    // pthread_join(&con2, NULL);
    // pthread_join(&con3, NULL);
    // pthread_join(&con4, NULL);
    // pthread_join(&pro, NULL);


    pthread_mutex_destroy(&mutex);	/* Free up the_mutex */
    pthread_cond_destroy(&condc);		/* Free up consumer condition variable */
    pthread_cond_destroy(&condp);		/* Free up producer condition variable */

  }


  // Functions definition are written Here

  //********************** Read Instruction is performed here *******************//
  void readInstruction(char buffer[], int sock, int thread_id)
  {
      int key;
      int n;
      char bufferText[1024];
      bool present = false;
      snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **READ** command\n", thread_id);
      n = write(sock,bufferText,strlen(bufferText)+1);
      bzero(buffer,256);
      n = read(sock,buffer,255);
      sscanf(buffer, "%d", &key);
      printf("%s size--->%d\n",buffer,n-1);
      if(bitmap[key]==0)
          snprintf(bufferText, sizeof(bufferText), "not\n");
      else{
          snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
          present = true;
      }
      printf("%s\n",bufferText);
      n = write(sock,bufferText,strlen(bufferText)+1);

      if(present){
          // value size
          int size = strlen(hashTable[key]);
          snprintf(bufferText, sizeof(bufferText), "%d\n",size);
          printf("%s\n",bufferText);
          n = write(sock,bufferText,strlen(bufferText)+1);
          char dummy[100];
          n = read(sock,dummy,strlen(dummy)+1);
          n = write(sock,hashTable[key],size+1);
          printf("%s\n",hashTable[key]);
      }
  }

  //******************* All data base modification is done *********************//
  void modifyInstruction(char buffer[], int sock, int thread_id)
  {
      // commnon vars
      char bufferText[100000];
      int n;

      // Choices start here ...............................
      if(strncmp(buffer,"create",6)==0)
      {
          int key, size;
          bool present = false;
          snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **CREATE** command\n", thread_id);
          n = write(sock,bufferText,strlen(bufferText)+1);
          bzero(buffer,256);
          n = read(sock,buffer,255);
          sscanf(buffer, "%d", &key);
          printf("%s size--->%d\n",buffer,n-1);
          if(bitmap[key]==0)
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
          else{
              snprintf(bufferText, sizeof(bufferText), "present\n");
              present = true;
          }
          printf("%s\n",bufferText);
          n = write(sock,bufferText,strlen(bufferText)+1);

          if(!present){
              // value size
              bzero(buffer,256);
              n = read(sock,buffer,255);
              sscanf(buffer, "%d", &size);
              printf("%s size--->%d\n",buffer,n-1);
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
              printf("%s\n",bufferText);
              n = write(sock,bufferText,strlen(bufferText)+1);

              int bytesRead = 0;
              int bytesToRead =size+1;
              int readThisTime;
              char *buffer = (char *)malloc(bytesToRead*sizeof(char));

              while (bytesToRead != bytesRead)
              {
                  do
                  {
                       readThisTime = read(sock, buffer + bytesRead, (bytesToRead - bytesRead));
                  }
                  while(readThisTime == -1);

                  if (readThisTime == -1)
                  {
                      /* Real error. Do something appropriate. */
                      return;
                  }
                  bytesRead += readThisTime;
              }
              printf("Buffer :->%s\n",buffer);
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
              printf("%s\n",bufferText);
              n = write(sock,bufferText,strlen(bufferText)+1);
              hashTable[key] = buffer;
              bitmap[key] = 1;
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Ok: added\n", thread_id,buffer);
              n = write(sock,bufferText,strlen(bufferText)+1);
          }
      }

      else if(strncmp(buffer,"update",6)==0)
      {
          int key, size;
          bool present = true;
          snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **UPDATE** command\n", thread_id);
          n = write(sock,bufferText,strlen(bufferText)+1);
          bzero(buffer,256);
          n = read(sock,buffer,255);
          sscanf(buffer, "%d", &key);
          printf("%s size--->%d\n",buffer,n-1);
          if(bitmap[key]==1)
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
          else{
              snprintf(bufferText, sizeof(bufferText), "not\n");
              present = false;
          }
          printf("%s\n",bufferText);
          n = write(sock,bufferText,strlen(bufferText)+1);

          if(present){
              // value size
              bzero(buffer,256);
              n = read(sock,buffer,255);
              sscanf(buffer, "%d", &size);
              printf("%s size--->%d\n",buffer,n-1);
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
              printf("%s\n",bufferText);
              n = write(sock,bufferText,strlen(bufferText)+1);

              int bytesRead = 0;
              int bytesToRead =size+1;
              int readThisTime;
              char *buffer = (char *)malloc(bytesToRead*sizeof(char));

              while (bytesToRead != bytesRead)
              {
                  do
                  {
                       readThisTime = read(sock, buffer + bytesRead, (bytesToRead - bytesRead));
                  }
                  while(readThisTime == -1);

                  if (readThisTime == -1)
                  {
                      /* Real error. Do something appropriate. */
                      return;
                  }
                  bytesRead += readThisTime;
              }
              printf("Buffer :->%s\n",buffer);
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
              printf("%s\n",bufferText);
              n = write(sock,bufferText,strlen(bufferText)+1);
              hashTable[key] = buffer;
              bitmap[key] = 1;
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Ok: updated\n", thread_id,buffer);
              n = write(sock,bufferText,strlen(bufferText)+1);
          }
      }

      else if(strncmp(buffer,"delete",6)==0)
      {
          int key;
          bool present = true;
          snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **DELETE** command\n", thread_id);
          n = write(sock,bufferText,strlen(bufferText)+1);
          bzero(buffer,256);
          n = read(sock,buffer,255);
          sscanf(buffer, "%d", &key);
          printf("%s size--->%d\n",buffer,n-1);
          if(bitmap[key]==1)
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Recieved **%s** \n", thread_id,buffer);
          else{
              snprintf(bufferText, sizeof(bufferText), "not\n");
              present = false;
          }
          printf("%s\n",bufferText);
          n = write(sock,bufferText,strlen(bufferText)+1);

          if(present){
              free(hashTable[key]);
              hashTable[key]=NULL;
              bitmap[key]=0;
              snprintf(bufferText, sizeof(bufferText), "FROM THREAD :%d Ok: Deleted\n", thread_id,buffer);
              n = write(sock,bufferText,strlen(bufferText)+1);
          }
      }

  }

  // *****************  Locks and Unlocks Methods*********************/
  void readLock(int thread_id)
  {
      pthread_mutex_lock(&Pmutex); // mutex lock to access buffer
      while(writer_present || writer_waiting>0){
             // If buffer gets full time for producer to rest
             printf("Thread %d goes to sleep: Reading\n",thread_id);
             pthread_cond_wait(&reader_can_enter, &Pmutex);
             printf("Thread %d start executing again; Reading\n");
      }
      readcount+=1;
      pthread_mutex_unlock(&Pmutex);
  }

  void readUnLock(int thread_id)
  {
      pthread_mutex_lock(&Pmutex); // mutex lock to access buffer
      readcount-=1;
      if(readcount==0)
              pthread_cond_broadcast(&writer_can_enter);
      pthread_mutex_unlock(&Pmutex);
  }

  void writeLock(int thread_id)
  {
      pthread_mutex_lock(&Pmutex); // mutex lock to access buffer
      writer_waiting+=1;
      while(writer_present || readcount>0){
             // If buffer gets full time for producer to rest
             printf("Thread %d goes to sleep: Modifying\n",thread_id);
             pthread_cond_wait(&writer_can_enter, &Pmutex);
             printf("Thread %d start executing again; Modifying\n");
      }
      writer_waiting-=1;
      writer_present = true;
      pthread_mutex_unlock(&Pmutex);
  }

  void writeUnLock(int thread_id)
  {
      pthread_mutex_lock(&Pmutex); // mutex lock to access buffer
      writer_present = false;
      if(writer_waiting==0)
              pthread_cond_broadcast(&reader_can_enter);
      else
              pthread_cond_broadcast(&writer_can_enter);
      pthread_mutex_unlock(&Pmutex);
  }


  //-------------- Query Processing --------------------------------//
  void doprocessing (int sock, int thread_id) {
        int n;
        char buffer[256];
        bzero(buffer,256);
        int tokenCount = 0;
        while(1){
            n = read(sock,buffer,255);
            tokenCount+=1;
            char bufferText[1024];
            //snprintf(bufferText, sizeof(bufferText), "I got your message from thread %d\n", thread_id);

            // For disconnecting the active action /////////////
            if(strncmp(buffer,"bye",3)==0)
            {
                printf("%s from client %d\n","Connnection closed",(int)(sock-3));
                n = write(sock,"Connection Terminated\n",100);
                break;
            }
            if (n < 0) {
               perror("ERROR reading from socket");
               break;
            }

            printf("Thread_id :%d From Client :%d Here is the message: %s\n",thread_id,(int)(sock-3),buffer);
            //n = write(sock,bufferText,100);
            if(strncmp(buffer,"create",6)==0 || strncmp(buffer,"delete",6)==0 || strncmp(buffer,"update",6)==0)
            {
                writeLock(thread_id);
                modifyInstruction(buffer, sock, thread_id);
                writeUnLock(thread_id);
            }

            else if(strncmp(buffer,"read",4)==0)
            {
                readLock(thread_id);
                readInstruction(buffer,sock,thread_id);
                readUnLock(thread_id);
            }
         }
         close(sock);
    }
  //
  // Networking Ends here

  // Buffer insertion and deletion is defined here
  void insert(int value)
  {
     rear=(rear+1)%MAXBUFFERSIZE;
     buffer[rear] = value;
     // printf("Insert: front = %d, rear = %d, buffer[rear]=%d\n",front,rear,buffer[rear]);
     queue_count+=1;
  }

  int release()
  {
     written+=1;
     int data = buffer[front];
     front = (front+1)%MAXBUFFERSIZE;
     // printf("Release: front = %d, rear = %d, buffer[front-1]=%d\n",front,rear,data);
     queue_count-=1;
     return data;
  }

  // Clean up routine is presented however it is incomplete
  void cleanup_routine()
  {
    pthread_mutex_destroy(&mutex);	/* Free up the_mutex */
    pthread_cond_destroy(&condc);		/* Free up consumer condition variable */
    pthread_cond_destroy(&condp);		/* Free up producer condition variable */
    printf("\n\n**************DONE CLEANING RESOURCES******************\n");
    pid_t pid = getpid();
    printf("**************DONE COLLECTING THREADS******************\n");
    kill(pid,SIGTERM);
  }

  /* producer stores the incomming connection in the buffer so that consumer thread cleanup_routine
  act upon it */
  void* producer(void *ptr) {
    int newsockfd;
    int thread_id = *((int *)ptr); // for thread id

    while(1){
      pthread_mutex_lock(&mutex); // mutex lock to access buffer
      while(queue_count==MAXBUFFERSIZE){
             // If buffer gets full time for producer to rest
             printf("Producer goes to sleep\n");
             pthread_cond_wait(&condp, &mutex);
             printf("Producer start executing\n");
      }
      pthread_mutex_unlock(&mutex);
      // // inserts the number in buffer
      // if(written==MAXITERATIONS-1){
      //   pthread_cond_broadcast(&condc);
      //   pthread_mutex_unlock(&mutex);
      //   break;
      // }
      newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

      if (newsockfd < 0) {
         perror("ERROR on accept");
         exit(1);
      }
      pthread_mutex_lock(&mutex);
      insert(newsockfd);

      printf("Producer: Written %d Current Buffer size %d\n",newsockfd,queue_count);
      //close(newsockfd);
      // broadcast signal for the consumer to print number
      // while(queue_count==2){
      //     pthread_cond_broadcast(&condc);
      //     pthread_mutex_unlock(&mutex);
      // }
      pthread_cond_broadcast(&condc);
      pthread_mutex_unlock(&mutex);
      // Optional sleep for the master, so that worker can get extra time for
      // processing
      struct timespec delay;
      delay.tv_sec = 0.2;
      delay.tv_nsec = SLEEP_NANOSEC_PROD;
      nanosleep(&delay, NULL);
    }
    // printf("Here Producer\n");
    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condc);
    numT+=1;
    if(numT==5)
    {
        cleanup_routine();
    }
    //pthread_exit(NULL);
  }

  /* pops the socket from the queue and do the necessary operations on the request of user*/
  void* consumer(void *ptr) {
    int newsockfd,data;
    int thread_id = *((int *)ptr);
    printf("Thread comes to play id: %d\n",thread_id);
    while(1){
      pthread_mutex_lock(&mutex);
      while (queue_count==0){
              printf("Thread %d goes to sleep\n",thread_id);
        pthread_cond_wait(&condc, &mutex);
        printf("Thread %d starts executing\n",thread_id);
      }
      // if(written==MAXITERATIONS-1){
      //   pthread_cond_signal(&condp);
      //   pthread_cond_broadcast(&condc);
      //   pthread_mutex_unlock(&mutex);
      //   break;
      // }
      newsockfd = release();
      printf("Thread %d Consumed %d Current Buffer size %d\n",thread_id,newsockfd,queue_count);
      // doprocessing(newsockfd,thread_id); //{ when nothing works do this}
      pthread_cond_broadcast(&condc);
      pthread_cond_signal(&condp);
      pthread_mutex_unlock(&mutex);
      doprocessing(newsockfd,thread_id);

      // optional sleep to allow workers to run
      struct timespec delay;
      delay.tv_sec = 0;
      delay.tv_nsec = SLEEP_NANOSEC_CONS;
      //nanosleep(&delay, NULL);
    }
    // printf("Here Thread %d\n",thread_id);
    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condc);
    pthread_cond_broadcast(&condc);
    pthread_cond_signal(&condp);
    numT+=1;
    if(numT==5)
    {
      cleanup_routine();
    }
    //pthread_exit(NULL);
  }
