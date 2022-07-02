#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
template<typename T>
class Singleton
{
public:
	static T* Instance();
	static void Destroy();

	friend std::ostream& operator << (std::ostream& os, Singleton const & instance) 
	{
		os << instance;
		return os;
	}

private:

	Singleton(Singleton &other) = delete; /* Singletons should not be cloneable */
	Singleton& operator=(const Singleton &) = delete; /* Singletons should not be assignable */

protected:
	static T* m_instance;

	Singleton()
    { 
        m_instance = static_cast <T*> (this);
    };
	~Singleton(){ };

};

template<typename T>
T* Singleton<T>::m_instance = nullptr; //initialization


template<typename T>
T* Singleton<T>::Instance()
{
	pthread_mutex_trylock(&lock);
	if(m_instance == nullptr)
	{
		Singleton<T>::m_instance = new T();
	}
	return m_instance;
}

template<typename T>
void Singleton<T>::Destroy()
{
    if(m_instance!=nullptr)
    {
	    delete Singleton<T>::m_instance; //undefined behavior
    }
	Singleton<T>::m_instance = nullptr;
	pthread_mutex_unlock(&lock);
}


int main()
{   
    int* s = Singleton<int>::Instance(); 
	printf("Value of instance: %d\n",*s);
	printf("Address of instance: %p\n",s);
    *s = 10;
	printf("Value of instance: %d\n",*s);
	int* p = Singleton<int>::Instance();
	Singleton<int>::Destroy();
	printf("Value of instance: %d\n",*p);
	printf("Value of instance: %d\n",*s);
	p = Singleton<int>::Instance();
	printf("Address of instance: %p\n",s);
	*s = 20;
	printf("Value of instance: %d\n",*p);
	printf("Value of instance: %d\n",*s);
    
    
    return 0;
}