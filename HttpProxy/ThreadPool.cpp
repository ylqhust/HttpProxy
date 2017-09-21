#include "ThreadPool.h"
#include "Utils.h"
using namespace ylq;

std::mutex ThreadPool::m_singleMutex;

ThreadPool &ThreadPool::getInstance()
{
    static ThreadPool instance;
    return instance;
}

unsigned int ThreadPool::threadCount()
{
	static unsigned int count = 0;
	if (count != 0)
		return count;
	count = 8;//at this time ,just use 8
	return count;
}

bool ThreadPool::addTask(std::shared_ptr<Runnable> t_run)
{

	{
		std::unique_lock<std::mutex> loc(m);
		if (m_stop)
			return false;
		m_taskQueue.emplace(t_run);
	}
	m_waitForTask.notify_one();
	return true;
}

unsigned int ThreadPool::getTaskCount()const
{
    return (unsigned int)m_taskQueue.size();
}

ThreadPool::ThreadPool():m_stop(false)
{
	unsigned int tc = threadCount();
	for (int i = 0; i < tc; ++i)
	{
		//create tc thread
		m_workers.emplace_back([&]() {
			while (!m_stop)
			{
				std::shared_ptr<Runnable> run;
				{
					std::unique_lock<std::mutex> loc(m);
					m_waitForTask.wait(loc, [&]() {
						return m_stop || (!m_taskQueue.empty());
					});
					if (m_stop&&m_taskQueue.empty())
						break;
					run = m_taskQueue.front();
					m_taskQueue.pop();
				}
				run->operator()();
			}

		});
	}
}


ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> loc(m);
		m_stop = true;
	}
	m_waitForTask.notify_all();
	for (std::thread &worker : m_workers)
        if(worker.joinable())
			worker.join();
}

void Runnable::operator()()
{

}
