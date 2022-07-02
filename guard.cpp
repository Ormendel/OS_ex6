#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

class Guard{

    private:

        pthread_mutex_t *lock;
    
    public:

        /*Constructor that gets a mutex_lock and lock it.
          We will create this instance in start of each function we want 
          to lock with Guard
        */
        Guard(pthread_mutex_t * lock )
        {
            this->lock = lock;
            pthread_mutex_lock(this->lock);
            printf("Guard LOCK sucessfully\n");
        }

        // And the distructor will unlock the mutex_lock
        ~Guard()
        {
            pthread_mutex_unlock(this->lock);
            printf("Guard UNLOCK sucessfully\n");
        }
};

/* ================ SECTION 4 ================
Answer to theoretical question:

In the first call to strtok, you supply the string and the delimiters.
In subsequent calls, the first parameter is NULL, and you just supply the delimiters. 
strtok remembers the string that you passed in.

In a multithreaded environment,
this is dangerous because many threads may be calling strtok with different strings. 
It will only remember the last one and return the wrong result.

Therefore using Guard design pattern can't help.

================ END OF SECTION 4 ================
*/