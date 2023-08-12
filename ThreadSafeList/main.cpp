#include <iostream>
#include "test_cases.hpp"
#include "example_thread_safe_list.h"




int main()
{
	std::barrier bar(4, []() noexcept
		{
			std::cout << "all threads finished\n"; 
		});

	threadsafe_list<int> exp_list;
	std::thread thr1([&bar, &exp_list]()
		{
			for (size_t i = 0; i < 5; ++i)
			{
				exp_list.push_front(i+1);
			}

			bar.arrive_and_wait();
		});

	std::thread thr2([&bar, &exp_list]()
		{
			for (size_t i = 0; i < 3; ++i)
			{
				exp_list.remove_if([](int val) 
					{
						return val % 2 == 0;
					});
			}

			bar.arrive_and_wait();
		});

	std::thread thr3([&bar, &exp_list]()
		{
			for (size_t i = 0; i < 3; ++i)
			{
				exp_list.find_first_if([](int val)
					{
						return val % 2 != 0;
					});
			}

			bar.arrive_and_wait();
		});
	
	thr1.detach();
	thr2.detach();
	thr3.detach();

	bar.arrive_and_wait();

	//test_out_stream();
	//test_add_values_empty_list();
	//test_add_values();
	//test_pop_front_back();
}
