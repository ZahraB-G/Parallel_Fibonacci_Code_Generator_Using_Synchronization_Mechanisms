#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include<iostream>
#include<vector>
#include<algorithm>
#include<fstream>
#include<string>
#include <semaphore.h> 
#include <sys/stat.h> 
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
using namespace std;
#define SEM_A_INIT_VALUE 0 //constant to use in semaphore 
//This function gets any number and pointer to the fibonacci table.
//It returns the closes fibonacci number to that number.

int FindClosesFibNum(int n,int*fibTable)
{
    int i = 0;
    while (n >= fibTable[i])
        i++;
    return fibTable[i - 1];
}
//This function gets fibonacci number and pointer to the fibonacci table.
//It returns fibonacci number index
int FindFibIndex(int n,int*fibTable)
{
    int i = 1;
    while (n != fibTable[i])
    {
        i++;
    }
    return i;
}
//This function gets a number and returns fibonacci numbers
int RecursiveFibFunction(int n)
{
    if (n <= 1)
        return n;
    else
        return RecursiveFibFunction(n - 2) + RecursiveFibFunction(n - 1);
}
/*this function fill out the fibonacci table*/
void FillFibTable(int*fibTable)
{
    for (int i = 1; i < 10; i++)
    {
        fibTable[i] = RecursiveFibFunction(i);
    }
}
//This function gets symbol assign integer and pointer to the fibTable and return array of index which contains all fibnocci number that we need for that interger.
int* GenerateFibonacciIndexArray(int assignInt, int* fibTable)
{  
   int reminder = assignInt;
   int x = 0;
   int j = 0;
   int n = assignInt;
   int *fibIndex = new int[10];
   int fibnumber;
    while (reminder != 0)
    {
        fibnumber =FindClosesFibNum(n,fibTable);
        reminder = n - fibnumber;
        fibIndex[j++] = FindFibIndex(fibnumber,fibTable);
        n = reminder;
    }
    return fibIndex;
}

//This function gets pointer to the fibonacci Index array of each number and returns string which is fibonacci Code word
string GenerateFibbonacciCodeWord(int *fibIndex)
{
    string result;
    
    if(fibIndex[0]==1)
    {
        result = "11";
        
        return result; 
    }
    int x = fibIndex[0] - 2;
    for (int j = 0; j <= x; j++)
    {
        result += '0';
    }
    result[x] = '1';
    int z = 1;
    while (fibIndex[z]!=0)
    {
        x = fibIndex[z] - 2;
        if (x < -1)
            break;
        if (x == -1)
        {
            result[0] = '1';
            break;
        }
        if (result[x + 1] != '1' && result[x - 1] != '1')
        {
            result[x] = '1';
        }
        z++;
    }
    result = result +'1';
    
    return result;
}
//this is the structure that contains all the information
struct node{
    char symbol;
    int frequency;
    int assignInt;
    string* fibonacciCodeWord;
    int orderedInt;
    sem_t *sem;
    pthread_mutex_t *mSem;
    string word; 
    int * turn;
    pthread_cond_t* waitTurn;
};
//This function gets a vector, array of string(which is in the file) and the inputfile array
//It returns decomposed message of the input file.
string decompossedFunction(vector<node>clientMessage, string inputFile[], int sizeOfInput)
{
    bool flag = false;
    int counter =0;
    string result;
    for(int i=0; i<sizeOfInput; i++)
    {
        for(int j=0;j< clientMessage.size(); j++)
        {
            if((clientMessage[j].word) == inputFile[i])
            {
                result =result + clientMessage[j].symbol;
            }
        }
    }
    return result;
}
//this struct is used to in sort based on frequency.
struct greater_than_key
{
    inline bool operator() (const node& node1, const node& node2)
    {
        if(node1.frequency == node2.frequency)
            return((node1.symbol)>(node2.symbol));
        return (node1.frequency > node2.frequency);
    }
};
struct greater_than_Order
{
    inline bool operator() (const node& node1, const node& node2)
    {
        return (node1.orderedInt < node2.orderedInt);
    }
};
//the function that each thread runs it
void *func_child(void *x_void_ptr)
{
    //Creating the fibonacci table 
    int* fibTable = new int[20];
    FillFibTable(fibTable);
    int *fibIndex;
    struct node TChild = (*(struct node*)x_void_ptr);
    sem_post(TChild.sem);


    pthread_mutex_lock(TChild.mSem);
    while(*(TChild.turn) < TChild.orderedInt)
    {
        pthread_cond_wait(TChild.waitTurn, TChild.mSem);
        
    }    
    pthread_mutex_unlock(TChild.mSem);
    
    fibIndex = GenerateFibonacciIndexArray(TChild.assignInt,fibTable);//create fibonacci index array
    *TChild.fibonacciCodeWord = GenerateFibbonacciCodeWord(fibIndex);//assigning the fibonaccicodeword created into fibonaccicodeword pointer
    cout<<"Symbol: "<<TChild.symbol<<", Frequency: "<<TChild.frequency<<", Code: "<<*TChild.fibonacciCodeWord<<endl;
   
    pthread_mutex_lock(TChild.mSem);
    (*TChild.turn)++;
    pthread_cond_broadcast(TChild.waitTurn);
    pthread_mutex_unlock(TChild.mSem);
     
   
    return NULL;
}
int main()
{
    string inputLine;//to hold line of input
    int numberOfSymbol;//the number of symbols
    getline(cin,inputLine);//read the first line of input
    numberOfSymbol =atoi(inputLine.c_str());//convert the string input to intger
    vector<node> childThreads; //this is the message that will be send to the server 
    node in;
    for(int i=0; i<numberOfSymbol;i++) //read the input and put it in clinetMessage vector which contins the info that needs to be send to server
    {
        getline(cin, inputLine);
        in.symbol = inputLine[0];
        in.frequency = (int)(inputLine[2])-48;
        in.orderedInt=i;
        childThreads.push_back(in);
        
    }
    string fileName;//to hold filename
    getline(cin,fileName);
    //sorting the clinetMessage based on frequency
    sort(childThreads.begin(),childThreads.end(), greater_than_key());
    int counter=1;
    //in this loop assign integer to the each symbol and also portnumber and hostname
    for(int i =0; i<=numberOfSymbol; i++)
    {
        childThreads[i].assignInt = counter++;
    }
    sort(childThreads.begin(),childThreads.end(), greater_than_Order());
    pthread_t *tid;
    tid = new pthread_t[numberOfSymbol];
    struct node Child;
    static pthread_cond_t print_cond = PTHREAD_COND_INITIALIZER;
    static int turn = 0;
    sem_t semA;
    sem_init(&semA, O_CREAT | 0600, SEM_A_INIT_VALUE); //initializing the semaphore
    static pthread_mutex_t p_sem;
    pthread_mutex_init(&p_sem, NULL); //initializing pthread_mutex created line above
    string *fibCodeArray = new string[numberOfSymbol]; //this array is gonna contained the fibonacciCodeWord created in threads
    for(int i = 0; i<numberOfSymbol ; i++)
	{
        
        Child.assignInt = childThreads[i].assignInt;
        Child.frequency = childThreads[i].frequency;
        Child.symbol = childThreads[i].symbol;
        Child.fibonacciCodeWord = &fibCodeArray[i];
        Child.turn = &turn;
        Child.sem = &semA;
        Child.mSem = &p_sem;
        Child.waitTurn = &print_cond;
        if(pthread_create(&tid[i], NULL, func_child, &Child)) 
		{
			fprintf(stderr, "Error creating thread\n");
			return 1;
        }
        //wait
        sem_wait(Child.sem);
    }
    
    for (int i = 0; i < numberOfSymbol; i++)
        	pthread_join(tid[i], NULL);

    sem_destroy(Child.sem);
    //assining the arry of the fibcode to the vector of information
    for(int i=0; i<numberOfSymbol; i++)
        childThreads[i].word = fibCodeArray[i];
   //After this we have all the information and have to decompose the input file.
    ifstream inputFile;
    string fileInput;
    inputFile.open(fileName.c_str());
    if (inputFile.is_open())
      inputFile >> fileInput;
    inputFile.close();
    int i =0;
    int c =0;
    string * fileInputArray = new string[100];
    while(fileInput.length()!=0) //This loop divide the text inside the input file and put it inside a string array.
    {
        if(fileInput[i] == '1'&& fileInput[i+1] == '1')
        {
            fileInputArray[c++]= fileInput.substr(0,i+2);
            fileInput.erase(0,i+2);
            i = 0;
        }
        else{
            i++;
        }
    }
    cout<<"Decompressed message = "<<decompossedFunction(childThreads,fileInputArray,c)<<endl;
    delete[] tid;
    return 0;
}