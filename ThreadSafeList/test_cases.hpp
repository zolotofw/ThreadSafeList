#pragma once
#include <iostream>
#include <thread>
#include <barrier>
#include <functional>
#include <vector>
#include "ThreadSafeList.hpp"

constexpr int num_threads = 3;

void test_out_stream(const int num_thr = num_threads)
{
    std::barrier sync_point(num_thr + 1);
    auto func = [&sync_point]<typename Type>(ThreadSafeList<Type>&list) -> void // Cant pass this to thread?
    {
        std::cout << list;
        sync_point.arrive_and_wait();
    };

    ThreadSafeList<int> safe_list{ 1,2,3 };

    std::vector<std::thread> threads;

    for (size_t i = 0; i < num_thr; ++i)
    {
        threads.emplace_back(std::thread([&sync_point, &safe_list]()
            {
                std::cout << safe_list;
                sync_point.arrive_and_wait();
            }));
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    std::cout << "counter = " << counter << std::endl;
}

template<typename Type>
void add_value(size_t amount, int value,  ThreadSafeList<Type>& list)
{
    for (size_t i = 0; i < amount; ++i)
    {
        list.push_front(value);
        list.push_back(value);
    }
}

void test_add_values_empty_list(const int num_thr = num_threads)
{
    std::barrier sync_point(num_thr + 1);
    ThreadSafeList<int> safe_list;

    std::vector<std::thread> threads;

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::thread([&sync_point, &safe_list, i]()
            {
                add_value(i+1, i+1, safe_list);
                sync_point.arrive_and_wait();
            }));
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    std::cout << "test empty: list size = " << safe_list.size() << std::endl << safe_list;
}

void test_add_values(const int num_thr = num_threads)
{
    std::barrier sync_point(num_thr + 1);
    ThreadSafeList<int> safe_list{ 0,0,0,0,0 };

    std::vector<std::thread> threads;

    for (size_t i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(std::thread([&sync_point, &safe_list, i]()
            {
                add_value(i + 1, i + 1, safe_list);
                sync_point.arrive_and_wait();
            }));
    }

    for (auto& thr : threads)
    {
        thr.detach();
    }

    sync_point.arrive_and_wait();

    std::cout << "list size = " << safe_list.size() << std::endl << safe_list;
}

void test_pop_front_back()
{
    std::barrier sync_point(3);
    ThreadSafeList<int> safe_list{ 0,0,0 };

    std::thread thread_pop_front([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.pop_front();
            }

            sync_point.arrive_and_wait();
        });

    //std::thread thread_pop_back([&sync_point, &safe_list]()
    //    {
    //        for (size_t i = 0; i < 2; ++i)
    //        {
    //            safe_list.pop_back();
    //        }

    //        sync_point.arrive_and_wait();
    //    });

    //std::thread thread_push_front([&sync_point, &safe_list]()
    //    {
    //        for (size_t i = 0; i < 2; ++i)
    //        {
    //            safe_list.push_front(10);
    //        }

    //        sync_point.arrive_and_wait();
    //    });

    std::thread thread_push_back([&sync_point, &safe_list]()
        {
            for (size_t i = 0; i < 2; ++i)
            {
                safe_list.push_back(20);
            }

            sync_point.arrive_and_wait();
        });

    thread_pop_front.detach();
    //thread_pop_back.detach();
    //thread_push_front.detach();
    thread_push_back.detach();
    
    sync_point.arrive_and_wait();

    std::cout << "list : \n" << safe_list;
}