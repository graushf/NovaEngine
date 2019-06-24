#pragma once

// concurrent_queue was grabbed from 
// http://www.justsoftwaresolutions.co.uk/threading/implementing-a-thread-safe-queue-using-condition-variables.html
// and was written by Anthony Williams
//
// class concurrent_queue					- Chapter 18, page 669
//

template<typename Data>
class concurrent_queue
{
private:
	std::queue<Data> the_queue;
	CriticalSection m_cs;
public:
	concurrent_queue()
	{
		m_dataPushed = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	void push(Data const& data)
	{
		//boost::mutex::scoped_lock lock(the_mutex);
		{
			ScopedCriticalSection locker(m_cs);
			the_queue.push(data);
		}
		//lock.unlock();
		//the_condition_variable.notify_one();
		PulseEvent(m_dataPushed);
	}

	bool empty() const
	{
		//boost::mutex::scoped_lock lock(the_mutex);
		ScopedCriticalSection locker(m_cs);
		return the_queue.empty();
	}

	bool try_pop(Data& popped_value)
	{
		//boost::mutex::scoped_lock lock(the_mutex);
		ScopedCriticalSection locker(m_cs);
		if (the_queue.empty())
		{
			return false;
		}

		popped_value = the_queue.front();
		the_queue.pop();
		return true;
	}

	void wait_and_pop(Data& popped_value)
	{
		//boost::mutex::scoped_lock lock(the_mutex);
		ScopedCriticalSection locker(m_cs);
		while (the_queue.empty())
		{
			//the_condition_variable.wait(lock);
			WaitForSingleObject(m_dataPushed);
		}

		popped_value = the_queue.front();
		the_queue.pop();
	}
};
