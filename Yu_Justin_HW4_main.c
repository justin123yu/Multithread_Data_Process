/**************************************************************
* Class: CSC-415-02 Fall 2022
* Name:  Justin_Yu
* Student ID: 920536640
* GitHub Name: justin123yu
* Project: Assignment 4 – Word Blast
*
* File: Yu_Justin_HW4_main.c
*
* Description:
* Taking text files and spliting it to different threads for
* procesing the information.
**************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>	
#include <pthread.h>
#include <string.h>


// You may find this Useful
char * delim = "\"\'.“”‘’?:;-,—*($%)! \t\n\x0A\r";
//Object construction
typedef struct Word{
    char name[50];
    int counter;
}  Word;

//Global Variables
int file = 0;
int process = 0;
int threadIndex = 0;
Word* wordArray = NULL;
volatile int wordCounter = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//Swaping the elements from index j to index i;
void swap(Word* wordArray, int i,  int j){
    Word tmp = wordArray[i];
    wordArray[i] = wordArray[j];
    wordArray[j] = tmp;
}

//Takes half of the data and organizes it
int partion(Word* wordArray, int low, int high){
    int pivot = wordArray[high].counter;
    int i = low - 1;
    for(int j = low; j <= high -1; j++){
        if(wordArray[j].counter < pivot){
            i++;
            swap(wordArray, i , j);
        }
    }
    swap(wordArray, i+1, high);
    return i + 1;
}
//https://www.geeksforgeeks.org/quick-sort/ 
//Used the website as reference to sort my data from least count to highest.
void quickSort(Word* wordArray, int low, int high){
        if(low < high){
            int pivot = partion(wordArray,low,high);
            quickSort(wordArray,low,pivot-1);
            quickSort(wordArray,pivot+1,high);
        }

}

void addWords(char* token){
    for(int i = 0; i < wordCounter; i++){
        if(strcasecmp(wordArray[i].name, token) == 0){
                //Critical Section: Accessing my global pointer;
                pthread_mutex_lock(&lock);
                wordArray[i].counter = wordArray[i].counter +1;
                pthread_mutex_unlock(&lock);
                return;
        }
    }
                pthread_mutex_lock(&lock);
                //Critical Section: Accessing my global pointer;
                strcpy(wordArray[wordCounter].name, token);
                wordArray[wordCounter].counter = wordArray[wordCounter].counter +1;
                wordCounter++;
                pthread_mutex_unlock(&lock);
}


void *fileReader(void *pArgs) {
    int* index = pArgs;
    char* token;
    char* saveptr;
    char* buffer = malloc(process*sizeof(char));
    if(buffer == NULL){
        printf("Buffer failed to initalize");
        exit(1);
    }
    int result = pread(file, buffer, process, *index);
    token = strtok_r(buffer, delim, &saveptr);
    //tokenizing the words
    while(token != NULL){
        if(strlen(token) >= 6){
            addWords(token);
        }
        token = strtok_r(NULL, delim, &saveptr);
    }
    //clearing buffer
    free(buffer);
    buffer = NULL;
    return NULL;
};



int main (int argc, char *argv[])
    {
    //***TO DO***  Look at arguments, open file, divide by threads
    //             Allocate and Initialize and storage structures
    char* fileName = argv[1];
    if(fileName == NULL){
        printf("No Files Found");
        exit(1);
    }
    //https://man7.org/linux/man-pages/man2/lstat.2.html
    //Helped gather the size of the files in bytes.
    struct stat stats;
    stat(fileName, &stats);
    int size =  stats.st_size;
    int threadCounter = atoi(argv[2]);
    if(threadCounter == 0){
        printf("No thread counter found");
        exit(1);
    }
    pthread_t threads [threadCounter];
    threadIndex = threadCounter;
    process = size / threadCounter;
    int offsets[threadCounter];
    int amountOfWord;
    //Split the files to chunks to be processed.
    for(int i = 0; i < threadCounter; i++){
        offsets[i] = i * process;
    }
    //Divide the process by 6 since we are looking for characters 6 or more.
    wordArray = malloc((process/6)*sizeof(Word));
    file = open(fileName,O_RDONLY);


    

    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Time stamp start
    struct timespec startTime;
    struct timespec endTime;

    clock_gettime(CLOCK_REALTIME, &startTime);
    //**************************************************************
    // *** TO DO ***  start your thread processing
    //                wait for the threads to finish
    for(int i = 0; i < threadCounter; i++){
        int error = pthread_create(&threads[i], NULL, fileReader, (void*)&offsets[i]);
        if(error != 0){
            printf("ERROR Detected");
        }
    }
    
    for(int i = 0; i < threadCounter; i++){
        pthread_join(threads[i], NULL);
    }

    // ***TO DO *** Process TOP 10 and display
    //Proccessing
    quickSort(wordArray,0,wordCounter);
    //Display
    printf("\n");
    printf("\nWord Frequency Count on %s with %d threads", fileName, threadCounter);
    printf("\nPrinting top 10 words 6 characters or more.\n");
    int index = 1;
    for(int i = wordCounter; i > wordCounter-10; i--){
        printf("Number %d is %s with a count of %d\n", index,wordArray[i].name, wordArray[i].counter);
        index++;
    }
    //**************************************************************
    // DO NOT CHANGE THIS BLOCK
    //Clock output
    clock_gettime(CLOCK_REALTIME, &endTime);
    time_t sec = endTime.tv_sec - startTime.tv_sec;
    long n_sec = endTime.tv_nsec - startTime.tv_nsec;
    if (endTime.tv_nsec < startTime.tv_nsec)
        {
        --sec;
        n_sec = n_sec + 1000000000L;
        }

    printf("Total Time was %ld.%09ld seconds\n", sec, n_sec);
    //**************************************************************


    // ***TO DO *** cleanup
    pthread_mutex_destroy(&lock);
    free(wordArray);
    wordArray = NULL;
    return 0;
    }
