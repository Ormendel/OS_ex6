#include "guard.cpp"

// Global variable
int counter = 0;
pthread_mutex_t lock  = PTHREAD_MUTEX_INITIALIZER;

void* update_count( void *ptr)
{

        Guard g(&lock);
    
        for ( int i = 0; i< 10; i++)
        {

            printf("%s - %d\n", ptr, ++counter);
            sleep(1);
        }
}

int main (){
        pthread_t thread1, thread2;

        char *messages1 = "Thread1";
        char *messages2 = "Thread2";
       
        pthread_create(&thread1, NULL, update_count, messages1);
        pthread_create(&thread2, NULL, update_count, messages2);
        
 
        pthread_join(thread1,NULL);
        pthread_join(thread2,NULL);
        
        pthread_mutex_destroy(&lock);

        return 0;
}