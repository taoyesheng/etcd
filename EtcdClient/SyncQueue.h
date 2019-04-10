#pragma once

#include <iostream>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <memory>
#include <atomic>
#include <string>
//#include <functional>

using namespace std;

template<typename T>
class SyncQueue
{
public:
	SyncQueue(int maxSize):m_maxSize(maxSize),m_needStop(false){}

	void Put(const T& input)
	{
		Add(input);
	}
	void Put(T && input)
	{
		Add(forward<T>(input));
	}
	
	void Take(list<T>& list)
	{
		unique_lock<mutex> locker(m_mutex);
		m_notEmpty.wait(locker, [this] {return m_needStop || NotEmpty(); });
		
		if (m_needStop)
			return;

		list = move(m_queue);
		m_notFull.notify_one();
	}

	void Take(T& t)
	{
		unique_lock<mutex> locker(m_mutex);
		m_notEmpty.wait(locker, [this] {return m_needStop || NotEmpty(); });

		if (m_needStop)
			return;

		t = m_queue.front();
		m_queue.pop_front();
		m_notFull.notify_one();
	}

	void Stop()
	{
		{
			lock_guard<mutex> locker(m_mutex);
			m_needStop = true;
		}
		m_notFull.notify_all();
		m_notEmpty.notify_all();
	}

	bool Empty()
	{
		lock_guard<mutex> locker(m_mutex);
		return m_queue.empty();
	}

	bool Full()
	{
		lock_guard<mutex> locker(m_mutex);
		return m_queue.size()==m_maxSize;
	}
	
	size_t Size()
	{
		lock_guard<mutex> locker(m_mutex);
		return m_queue.size();
	}

	int Count()
	{
		return m_queue.size();
	}
private:
	bool NotFull() const
	{
		bool bfull = static_cast<int>(m_queue.size()) >= m_maxSize;
		if (bfull)
			cout << "full, waiting, thread id: " << this_thread::get_id() << endl;
		
		return !bfull;
	}

	bool NotEmpty() const
	{
		bool bEmpty = m_queue.empty();
		if (bEmpty)
			cout << "empty, waiting, thread id: " << this_thread::get_id() << endl;

		return !bEmpty;
	}

	template<typename F>
	void Add(F&& input)
	{
		unique_lock<mutex> locker(m_mutex);
		m_notFull.wait(locker, [this] {return m_needStop || NotFull(); });

		if (m_needStop)
			return;

		m_queue.push_back(forward<F>(input));
		m_notEmpty.notify_one();
	}

	list<T>            m_queue    ; 
	mutex              m_mutex    ; 
	condition_variable m_notEmpty ; 
	condition_variable m_notFull  ; 
	int                m_maxSize  ; 
	bool               m_needStop ; 
};

