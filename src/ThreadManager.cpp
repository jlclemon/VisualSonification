#include "ThreadManager.h"
#include <iostream>
#include <cstring>
//**********************************************************************
//
//	ThreadManager.cpp
//	Overview:  A pair of pthread wrapper classes for my own personal use
//			   The thread class contains basic features for multithreaded
//			   programming including barriers, condition variables, and locks
//			   Thread manager is for a group of threads and allows communication
//			   General Worker Thread Data for thread data value
//			   Version .1
//			   Date: Jan 17, 2013
//
//
//
//*************************************************************************



pthread_key_t * Thread::staticThreadSpecificDataKeyPtr;

Thread::Thread(void)
{
	#ifdef VERBOSE
	cout << "Thread Created" << endl;
	#endif


	threadSpecificDataKeyPtr = NULL;
	threadFunction = NULL;
	threadParam = NULL;
	threadData = NULL;
	threadLogicalId = -1;
	barrierPtr = NULL;
	wakeupSet = false;
	threadDataVec.resize(5);
	this->threadManagerPtr = NULL;
	int err = pthread_mutex_init(&wakeupLock, NULL);
	if(err != 0)
	{
		cout << "Error initializing mutex: " << strerror(err) << endl;
	}


	//int err = pthread_mutex_lock(&wakeupLock);
	//int err = pthread_mutex_unlock(&wakeupLock);

	err = pthread_cond_init(&wakeupSignalConditionVariable,NULL);
	if(err != 0)
	{
		cout << "Error initializing condition variable: " << strerror(err) << endl;
	}


	#ifdef VERBOSE
		cout << "Err in creation is " << err << endl;
	#endif
	//int err = pthread_mutex_lock(&wakeupLock);
	//int returnVale = pthread_cond_wait(&wakeupSignalConditionVariable,&wakeupLock);
	//wakeupSet = false;
	//err = pthread_mutex_unlock(&wakeupLock);
	//
	//int err = pthread_mutex_lock(&wakeupLock);
	//wakeupSet = true;
	//pthread_cond_signal(&wakeupSignalConditionVariable);
	//pthread_cond_broadcast(&wakeupSignalConditionVariable);
	//err = pthread_mutex_unlock(&wakeupLock);
}

Thread::Thread(const Thread & oldThread)
{
	#ifdef VERBOSE
	cout << "Thread Created" << endl;
	#endif

	this->threadSpecificDataKeyPtr = oldThread.threadSpecificDataKeyPtr;
	this->threadData = oldThread.threadData;
	this->barrierPtr = oldThread.barrierPtr;
	this->threadFunction = oldThread.threadFunction;
	this->threadId = oldThread.threadId;
	this->threadLogicalId = oldThread.threadLogicalId;
	this->threadParam = oldThread.threadParam;
	this->threadDataVec = oldThread.threadDataVec;
	this->threadManagerPtr = oldThread.threadManagerPtr;
	wakeupSet = false;
	int err = pthread_mutex_init(&wakeupLock, NULL);
	if(err != 0)
	{
		cout << "Error initializing mutex: " << strerror(err) << endl;
	}


	//int err = pthread_mutex_lock(&wakeupLock);
	//int err = pthread_mutex_unlock(&wakeupLock);

	err = pthread_cond_init(&wakeupSignalConditionVariable,NULL);
	if(err != 0)
	{
		cout << "Error initializing condition variable: " << strerror(err) << endl;
	}

}


Thread::~Thread(void)
{
	#ifdef VERBOSE
	cout << "Thread Destroyed" << endl;
	#endif

	int err = pthread_mutex_destroy(&wakeupLock);
	if(err != 0)
	{
		cout << "Error destroying mutex: " << strerror(err) << endl;
	}



	pthread_cond_destroy(&wakeupSignalConditionVariable);
	if(err != 0)
	{
		cout << "Error destroying condition variable: " << strerror(err) << endl;
	}


}

Thread & Thread::operator=(const Thread & oldThread)
{
	#ifdef VERBOSE
	cout << "Thread Assigned" << endl;
	#endif

	this->threadSpecificDataKeyPtr = oldThread.threadSpecificDataKeyPtr;
	this->threadData = oldThread.threadData;
	this->barrierPtr = oldThread.barrierPtr;
	this->threadFunction = oldThread.threadFunction;
	this->threadId = oldThread.threadId;
	this->threadLogicalId = oldThread.threadLogicalId;
	this->threadParam = oldThread.threadParam;
	this->threadDataVec = oldThread.threadDataVec;
	this->threadManagerPtr = oldThread.threadManagerPtr;

	return *this;
}

int Thread::wakeup()
{

	int err = pthread_mutex_lock(&wakeupLock);
	if(err != 0)
	{
		cout << "Error locking mutex: " << strerror(err) << endl;
	}


	if(!wakeupSet)
	{
		#ifdef VERBOSE
			cout << "Thread "<< this->threadLogicalId << " has been told to wake up" << endl;
		#endif
		wakeupSet = true;
		err = pthread_cond_signal(&wakeupSignalConditionVariable);
		if(err != 0)
		{
			cout << "Error signaling condition variable: " << strerror(err) << endl;
		}


		#ifdef VERBOSE
			cout << "Err on wakeup " << err << endl;
		#endif
		//pthread_cond_broadcast(&wakeupSignalConditionVariable);

	}
	err = pthread_mutex_unlock(&wakeupLock);
	if(err != 0)
	{
		cout << "Error unlocking mutex: " << strerror(err) << endl;
	}


	return err;
}

int Thread::waitForWakeup()
{
	int err = pthread_mutex_lock(&wakeupLock);
	if(err != 0)
	{
		cout << "Error locking mutex: " << strerror(err) << endl;
	}



	#ifdef VERBOSE
		cout << "Thread "<< this->threadLogicalId << " has gone to sleep." << endl;
	#endif
	while(!wakeupSet) 
	{
		err = pthread_cond_wait(&wakeupSignalConditionVariable,&wakeupLock);
		if(err != 0)
		{
			cout << "Error waiting on condition variable: " << strerror(err) << endl;
		}


		#ifdef VERBOSE
			cout << "Thread "<<this->threadLogicalId <<" Woke up for a second" << endl;
		#endif
	}
	wakeupSet = false;

	#ifdef VERBOSE
		cout << "Thread "<< this->threadLogicalId << " has woke up" << endl;
	#endif
	err = pthread_mutex_unlock(&wakeupLock);
	if(err != 0)
	{
		cout << "Error unlocking mutex: " << strerror(err) << endl;
	}


	return err;


}
int Thread::waitAtBarrier()
{

	int returnVal = pthread_barrier_wait(barrierPtr);
	if(!((returnVal == 0)|| (returnVal ==PTHREAD_BARRIER_SERIAL_THREAD)))
	{
		cout << "Error waiting at barrier: " << strerror(returnVal) << endl;
	}



	if(returnVal == PTHREAD_BARRIER_SERIAL_THREAD)
	{
		returnVal = 0;
	}
	return returnVal;
}

int Thread::lanuchThread(pthread_attr_t * attribute)
{



	int errcode = pthread_create(this->getThreadIdPtr(),attribute,this->getThreadFunction(),this);
	if(errcode != 0)
	{
		cout << "Error creating thread: " << strerror(errcode) << endl;
		cerr << "Error creating thread: " << strerror(errcode);
	}

	return errcode;


}


void * Thread::getThreadSpecificData(void)
{
	return pthread_getspecific(*(this->threadSpecificDataKeyPtr));
}
void Thread::setThreadSpecificData(void * threadSpecificData)
{
	pthread_setspecific(*(this->threadSpecificDataKeyPtr),threadSpecificData);


}

void * Thread::getThreadSpecificData(pthread_key_t * currentThreadSpecificDataKey)
{
	return pthread_getspecific(*(currentThreadSpecificDataKey));
}
void Thread::setThreadSpecificData(pthread_key_t * currentThreadSpecificDataKey,void * threadSpecificData)
{
	pthread_setspecific(*(currentThreadSpecificDataKey),threadSpecificData);


}



void Thread::setStaticThreadSpecificDataKeyPtr(pthread_key_t * newThreadSpecificDataKeyPtr)
{
	Thread::staticThreadSpecificDataKeyPtr = newThreadSpecificDataKeyPtr;
}
void Thread::setThreadSpecificDataKeyPtr(pthread_key_t * newThreadSpecificDataKeyPtr)
{
	this->threadSpecificDataKeyPtr = newThreadSpecificDataKeyPtr;
}


pthread_key_t * Thread::getThreadSpecificDataKeyPtr()
{

	return this->threadSpecificDataKeyPtr;

}

pthread_key_t * Thread::getStaticThreadSpecificDataKeyPtr()
{

	return Thread::staticThreadSpecificDataKeyPtr;

}


int  Thread::createThreadSpecificDataKey()
{
	int returnVal = 0;
	if(this->threadLogicalId == 0)
	{
#ifdef VERBOSE
	cout << "Thread "<< this->threadLogicalId << " is creating the data key." << endl;
#endif

		returnVal = pthread_key_create(this->threadSpecificDataKeyPtr, NULL);
	}
#ifdef VERBOSE
	cout << "Thread "<< this->threadLogicalId << " is waiting on data key creation." << endl;
#endif


	this->waitAtBarrier();
	return returnVal;

}

int Thread::destroyThreadSpecificDataKey()
{
	int returnVal = 0;
	this->waitAtBarrier();
	if(this->threadLogicalId == 0)
	{
		returnVal = pthread_key_delete(*(this->threadSpecificDataKeyPtr));

	}
	return returnVal;

}







ThreadManager::ThreadManager(void)
{
	#ifdef VERBOSE
	cout << "Thread Manager Created" << endl;
	#endif
}

ThreadManager::ThreadManager(int numThreads)
{
	#ifdef VERBOSE
	cout << "Thread Manager Created" << endl;
	#endif
	this->threadList.resize(numThreads);
	this->setupBarrier();
}



ThreadManager::~ThreadManager(void)
{
	#ifdef VERBOSE
	cout << "Thread Manager Destroyed" << endl;
	#endif
}

int ThreadManager::wakeupThread(int threadId)
{

	return this->threadList[threadId].wakeup();


}

int ThreadManager::setupBarrier()
{

	
	for(int i = 0; i<threadList.size(); i++)
	{
		threadList[i].setBarrierPtr(&barrier);

	}
	return pthread_barrier_init(&barrier, NULL, threadList.size());


}

int ThreadManager::setupBarrier(int numberOfThreads)
{

	
	return pthread_barrier_init(&barrier, NULL, numberOfThreads);


}

void ThreadManager::setThreadBarrier(int threadId)
{

	threadList[threadId].setBarrierPtr(&barrier);
	


}

int ThreadManager::launchThread(int threadId,void * (*newThreadFunction)(void *), pthread_attr_t * attribute, bool setThreadPtrAsParam)
{
	Thread * currentThreadPtr = (this->getThread(threadId));

	currentThreadPtr->setThreadManagerPtr(this);
	currentThreadPtr->setThreadLogicalId(threadId);

	if(setThreadPtrAsParam)
	{
		currentThreadPtr->setThreadParam(currentThreadPtr);
	}
	currentThreadPtr->setThreadFunction(newThreadFunction);
	this->setThreadBarrier(threadId);
	this->setThreadThreadSpecificDataKeyPtrToMine(threadId);

	return currentThreadPtr->lanuchThread(attribute);




}

int ThreadManager::launchThread(int threadId,pthread_attr_t * attribute, bool setThreadPtrAsParam)
{
	Thread * currentThreadPtr = (this->getThread(threadId));

	currentThreadPtr->setThreadManagerPtr(this);
	currentThreadPtr->setThreadLogicalId(threadId);
	if(setThreadPtrAsParam)
	{
		currentThreadPtr->setThreadParam(currentThreadPtr);
	}

	this->setThreadBarrier(threadId);
	this->setThreadThreadSpecificDataKeyPtrToMine(threadId);

	return currentThreadPtr->lanuchThread(attribute);




}




int  ThreadManager::setupThreadSpecificDataKey()
{
	int returnVal = 0;


	for(int i = 0; i<threadList.size(); i++)
	{
		setThreadThreadSpecificDataKeyPtrToMine(i);

	}

	setStaticThreadThreadSpecificDataKeyPtrToMine();
	return returnVal;

}




void * ThreadManager::getThreadSpecificData(void)
{

	return pthread_getspecific(this->threadSpecificDataKey);

}
void ThreadManager::setThreadSpecificData(void *threadSpecificData)
{
	pthread_setspecific(this->threadSpecificDataKey,threadSpecificData);


}


void ThreadManager::setThreadThreadSpecificDataKeyPtrToMine(int threadId)
{


	threadList[threadId].setThreadSpecificDataKeyPtr(&(this->threadSpecificDataKey));


}

pthread_key_t *  ThreadManager::getThreadThreadSpecificDataKeyPtr(int threadId)
{


	return threadList[threadId].getThreadSpecificDataKeyPtr();


}



void ThreadManager::setStaticThreadThreadSpecificDataKeyPtrToMine()
{

	Thread::setStaticThreadSpecificDataKeyPtr(&(this->threadSpecificDataKey));

}


pthread_key_t * ThreadManager::getThreadSpecificDataKeyPtr(void)
{
	return &(this->threadSpecificDataKey);

}
