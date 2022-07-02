#include <pthread.h>
#include <iostream>
#include <unistd.h>
   
using namespace std;

// Reactor struct
typedef struct reactor 
{

        int file_d;
        pthread_t thread_id;
        void*(*func)(void*);

}reactor;

// Request struct
typedef struct requsets{

        int file_d;
        reactor * r;
}req;

// First function - newReactor()
reactor* newReactor(){

      reactor *r = (reactor*)(malloc(sizeof(reactor)));
      return r;
} 

// Second function - InstallHandler()
void InstallHandler(reactor * r, void*(func)(void*), int file_d)
{

        r->func = func;
        r->file_d = file_d;

        // Make memory for requests
        req *request = (req*)(malloc(sizeof(req)));
        // Assign the fd to this request
        request->file_d = file_d;
        // Assign our Reactor to the request
        request->r = r;
        // Create a thread for this request with the func
        pthread_create(&r->thread_id,NULL,func,request);
}

// Third function - RemoveHandler()
void RemoveHandler(reactor * r, int file_d){

    // First needs to wait for the thread that handle right now is done
    pthread_join(r->thread_id,NULL);
    r->file_d = -1; // Change back the file descriptor
    r->func = NULL;

}


