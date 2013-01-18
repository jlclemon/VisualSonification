#pragma once

#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H


#include <pthread.h>
#include <vector>
#include <string>
using namespace std;
struct ThreadStruct
{
	void * (*threadFunction)(void *);
	void * threadParam;
	void * threadData;
	pthread_t threadId;
	int threadLogicalId;
	pthread_cond_t wakeupSignalConditionVariable;
	pthread_mutex_t wakeupLock;
	bool wakeupSet;
	pthread_barrier_t * barrierPtr;




};
struct ThreadSpecificDataStructBase
{
	int threadLogicalId;
	void * appSepcificDataStruct;

};

class ThreadManager;

class Thread
{


	public:
		Thread();
		~Thread();
		Thread(const Thread & oldThread); 
		Thread & operator=(const Thread & p);

		pthread_t getThreadId( )
		{
			return this->threadId;

		}

		void setThreadId(pthread_t newThreadId)
		{

			this->threadId = newThreadId;

		}
		pthread_t * getThreadIdPtr()
		{

			return &(this->threadId);

		}



		int getThreadLogicalId()
		{
			return this->threadLogicalId;
		}

		void setThreadLogicalId(int newThreadLogicalId)
		{
			this->threadLogicalId = newThreadLogicalId;

		}


		//Return the function pointer
		void * (*getThreadFunction())(void *) 
		{

			return threadFunction;
		}

		void setThreadFunction( void * (*newThreadFunction)(void *) )
		{
				
			threadFunction = newThreadFunction;	
				
		}


		void * getThreadParam()
		{
			return this->threadParam;

		}

		void setThreadParam(void * newThreadParam)
		{
			this->threadParam = newThreadParam;

		}


		void * getThreadData(int index =0)
		{
			return this->threadDataVec[index];

		}

		void setThreadData(void * newThreadData, int index=0)
		{
			this->threadDataVec[index] = newThreadData;

		}
		int addToThreadData(void * newThreadData)
		{
			this->threadDataVec.push_back(newThreadData);
			return this->threadDataVec.size();
		}
		int delFromThreadData()
		{
			this->threadDataVec.pop_back();
			return this->threadDataVec.size();
		}

		void setThreadDataVecSize(int newSize)
		{
			this->threadDataVec.resize(newSize);
		}

		int  getThreadDataVecSize()
		{
			return this->threadDataVec.size();
		}


		void setBarrierPtr(pthread_barrier_t * newBarrierPtr)
		{

			this->barrierPtr = newBarrierPtr;

		}

		pthread_barrier_t * getBarrierPtr()
		{

			return this->barrierPtr;

		}





		int waitAtBarrier();
		int wakeup();

		int waitForWakeup();
		int lanuchThread(pthread_attr_t * attribute);

		void * getThreadSpecificData(void);


		void setThreadSpecificData(void *);
		pthread_key_t * getThreadSpecificDataKeyPtr();
		static pthread_key_t * getStaticThreadSpecificDataKeyPtr();
		void setThreadSpecificDataKeyPtr(pthread_key_t * newThreadSpecificDataKey);
		static void setStaticThreadSpecificDataKeyPtr(pthread_key_t * newThreadSpecificDataKey);
		static void * getThreadSpecificData(pthread_key_t * currentThreadSpecificDataKey);
		void setThreadSpecificData(pthread_key_t * currentThreadSpecificDataKey,void * threadSpecificData);
		int  createThreadSpecificDataKey();
		int  destroyThreadSpecificDataKey();


	ThreadManager * getThreadManagerPtr()
	{
		return this->threadManagerPtr;
	}
	void setThreadManagerPtr(ThreadManager * _threadManagerPtr)
	{
		this->threadManagerPtr = _threadManagerPtr;
	}


	protected:
		void * (*threadFunction)(void *);
		void * threadParam;

		vector<void *> threadDataVec;

		ThreadManager * threadManagerPtr;
		void * threadData;
		pthread_t threadId;
		int threadLogicalId;
		pthread_cond_t wakeupSignalConditionVariable;
		pthread_mutex_t wakeupLock;
		bool wakeupSet;
		pthread_barrier_t * barrierPtr;
		pthread_key_t * threadSpecificDataKeyPtr;
		static pthread_key_t * staticThreadSpecificDataKeyPtr;
};

class ThreadManager
{
public:
	ThreadManager(void);
	ThreadManager(int numThreads);
	~ThreadManager(void);
	Thread * getThread(int index)
	{
		return &threadList[index];

	}
	int addThread(Thread & newThread)
	{
		this->threadList.push_back(newThread);
		return this->threadList.size();

	}
	int removeThread(int index)
	{
		this->threadList.erase(this->threadList.begin()+index);		
		return	this->threadList.size();
	}



	int getNumberOfThreads()
	{
			
		return this->threadList.size();		
			
	};

	int wakeupThread(int threadId);
	int setupBarrier();
	int setupBarrier(int numberOfThreads);
	void setThreadBarrier(int threadId);
	int launchThread(int threadId,void * (*newThreadFunction)(void *), pthread_attr_t * attribute, bool setThreadPtrAsParam=false);
	int launchThread(int threadId, pthread_attr_t * attribute, bool setThreadPtrAsParam=false);
	void * getThreadSpecificData(void);
	void setThreadSpecificData(void *threadSpecificData);
	int  setupThreadSpecificDataKey();

	void setStaticThreadThreadSpecificDataKeyPtrToMine();
	void setThreadThreadSpecificDataKeyPtrToMine(int threadId);
	pthread_key_t * getThreadThreadSpecificDataKeyPtr(int threadId);
	pthread_key_t * getThreadSpecificDataKeyPtr(void);

protected:


	vector<Thread> threadList;
	pthread_barrier_t barrier;
	pthread_key_t threadSpecificDataKey;


};

class BaseConfig
{
	public:
	BaseConfig()
	{

	}

	virtual ~BaseConfig()
	{


	}

	protected:
	string configId;

};

class BaseData
{
	public:
	BaseData()
	{

	}

	virtual ~BaseData()
	{


	}
	virtual void clearData() = 0;


	protected:
	string dataId;

};


class GeneralWorkerThreadDataHolder
{
	GeneralWorkerThreadDataHolder()
	{
		this->sharedConfig = NULL;
		this->sharedData = NULL;

		this->privateConfig = NULL;
		this->privateData = NULL;

		this->threadManager = NULL;

		this->numberOfHorizontalThreads = 0;
		this->numberOfVerticalThreads = 0;
		this->totalNumberOfThreads = 0;
		this->coordinator = false;
	}
	GeneralWorkerThreadDataHolder(BaseConfig* _sharedConfig,BaseData*  _sharedData,BaseConfig * _privateConfig,BaseData * _privateData,ThreadManager *_threadManager,int _numberHThreads,int _numberVThreads,bool _coordinator):sharedConfig(_sharedConfig),sharedData(_sharedData),privateConfig(_privateConfig),privateData(_privateData),threadManager(_threadManager),numberOfHorizontalThreads(_numberHThreads),numberOfVerticalThreads(_numberVThreads), coordinator(_coordinator)
	{
			this->totalNumberOfThreads = this->numberOfHorizontalThreads * this->numberOfVerticalThreads;
	}



	virtual ~GeneralWorkerThreadDataHolder()
	{

	}

public:

	BaseConfig * sharedConfig;
	BaseData * sharedData;



	BaseConfig * privateConfig;
	BaseData * privateData;

	//Thread * myThread;
	ThreadManager * threadManager;

	int numberOfVerticalThreads;
	int numberOfHorizontalThreads;
	int totalNumberOfThreads;

	bool coordinator;

};



#endif
