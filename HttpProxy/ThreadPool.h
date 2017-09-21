#pragma once
#include <mutex>
#include <queue>
#include <vector>
#include <condition_variable>
#include <thread>

namespace ylq {
	class Runnable
	{
	public:
		virtual void operator()();
	};


	class ThreadPool
	{
	public:
		static ThreadPool &getInstance();
		static unsigned int threadCount();
		bool addTask(std::shared_ptr<Runnable>);//add a task to taskQueue
        unsigned int getTaskCount()const;
		~ThreadPool();
	private:
		ThreadPool();
	private:
		std::condition_variable m_waitForTask;
		std::queue <std::shared_ptr<Runnable> > m_taskQueue;
		std::vector<std::thread> m_workers;
		bool m_stop;
		std::mutex m;
		static std::mutex m_singleMutex;
	};
}
