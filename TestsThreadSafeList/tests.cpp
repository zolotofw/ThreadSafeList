#include "pch.h"
#include "../ThreadSafeList/ThreadSafeList.hpp"
#include <barrier>


TEST(TestThreadSafeList, TestPushFrontAndBackIfEmpty)
{
    int num_threads = 4;

    std::barrier sync_point(num_threads + 1);
    ThreadSafeList<int> safe_list;

    std::vector<std::thread> threads;

    int value = 0;
    bool if_push_front_or_back = true;

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::thread([&if_push_front_or_back, &sync_point, &safe_list, &value]()
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    if (if_push_front_or_back)
                    {
                        safe_list.push_front(++value);
                    }
                    else
                    {
                        safe_list.push_back(++value);
                    }
                }
                sync_point.arrive_and_wait();
            }));

        if_push_front_or_back = !if_push_front_or_back;
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    EXPECT_TRUE(Testing::check_if_any<int>(safe_list, { 1,2,3,4,5,6,7,8,9,10,11,12 }));

    _CrtDumpMemoryLeaks();
}

TEST(TestThreadSafeList, TestPushFrontAndBackIfNotEmpty)
{
    int num_threads = 4;
    std::barrier sync_point(num_threads + 1);
    ThreadSafeList<int> safe_list{ 0,0,0,0,0 };

    std::vector<std::thread> threads;

    int value = 0;
    bool switch_push_front_or_back = true;

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::thread([&switch_push_front_or_back, &sync_point, &safe_list, &value]()
            {
                for (size_t i = 0; i < 3; ++i)
                {
                    if (switch_push_front_or_back)
                    {
                        safe_list.push_front(++value);
                    }
                    else
                    {
                        safe_list.push_back(++value);
                    }
                }
                sync_point.arrive_and_wait();
            }));

        switch_push_front_or_back = !switch_push_front_or_back;
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    EXPECT_TRUE(Testing::check_if_any<int>(safe_list, { 1,2,3,4,5,0,0,0,0,0,6,7,8,9,10,11,12 }));
}

TEST(TestThreadSafeList, TestPopFrontAllElements)
{
    std::barrier sync_point(3);

    ThreadSafeList<int> safe_list{ 1,2,3,4,5 };

    std::thread thread_pop_front_1([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.pop_front();
            }

            sync_point.arrive_and_wait();
        });

    std::thread thread_pop_front_2([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 3; ++i)
            {
                safe_list.pop_front();
            }

            sync_point.arrive_and_wait();
        });

    thread_pop_front_1.detach();
    thread_pop_front_2.detach();

    sync_point.arrive_and_wait();

    EXPECT_EQ(safe_list.size(), 0);
}

TEST(TestThreadSafeList, TestPopFrontLeaveOneElement)
{
    std::barrier sync_point(3);

    ThreadSafeList<int> safe_list{ 1,2,3,4,5 };

    std::thread thread_pop_front_1([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.pop_front();
            }

            sync_point.arrive_and_wait();
        });

    std::thread thread_pop_front_2([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.pop_front();
            }

            sync_point.arrive_and_wait();
        });

    thread_pop_front_1.detach();
    thread_pop_front_2.detach();

    sync_point.arrive_and_wait();

    EXPECT_EQ(safe_list.size(), 1);
}

TEST(TestThreadSafeList, TestPopBackAllElements)
{
    std::barrier sync_point(3);

    ThreadSafeList<int> safe_list{ 1,2,3,4,5 };

    std::thread thread_pop_front_1([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 5; ++i)
            {
                safe_list.pop_back();
            }

            sync_point.arrive_and_wait();
        });

    std::thread thread_pop_front_2([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 3; ++i)
            {
                safe_list.pop_back();
            }

            sync_point.arrive_and_wait();
        });

    thread_pop_front_1.detach();
    thread_pop_front_2.detach();

    sync_point.arrive_and_wait();

    EXPECT_TRUE(safe_list.empty());
}

TEST(TestThreadSafeList, TestPopBackLeaveOneElement)
{
    std::barrier sync_point(3);

    ThreadSafeList<int> safe_list{ 1,2,3,4,5 };

    std::thread thread_pop_front_1([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.pop_back();
            }

            sync_point.arrive_and_wait();
        });

    std::thread thread_pop_front_2([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.pop_back();
            }

            sync_point.arrive_and_wait();
        });

    thread_pop_front_1.detach();
    thread_pop_front_2.detach();

    sync_point.arrive_and_wait();

    EXPECT_EQ(safe_list.size(), 1);
}