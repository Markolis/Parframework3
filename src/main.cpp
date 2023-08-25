#include "parframework.hpp"
#include "stdio.h"
#include <sys/time.h>
#include "pthread.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

using namespace Parframework;

int totalNumberOfThreads = 0;
UBaseType_t totalNumberOfThreads_pthread = 0;
UBaseType_t unusedStack = 0;

/* void* fibonacci(void* num)
{
    ++totalNumberOfThreads;
    if (num == (void*)0)
    {
        return (void*)0;
    }
    else if (num == (void*)1)
    {
        return (void*)1;
    }
    else
    {
        void* result1 = NULL;
        void* result2 = NULL;
        pthread_t_pfr one, two;
        pthread_create_pfr(one, fibonacci, num-2);
        pthread_create_pfr(two, fibonacci, num-1);
        pthread_join_pfr(one, &result1);
        pthread_join_pfr(two, &result2);

        int a = (int)result1;
        int b = (int)result2;
        return (void*)(a+b);
    }
} */

/* void* fibonacci_pthread(void* num)
{
    if (num == (void*)0)
    {
        return (void*)0;
    }
    else if (num == (void*)1)
    {
        return (void*)1;
    }
    else
    {
        void* result1 = NULL;
        void* result2 = NULL;
        pthread_t one, two;
        pthread_create(&one, NULL, fibonacci_pthread, num-2);
        pthread_create(&two, NULL, fibonacci_pthread, num-1);
        pthread_join(one, &result1);
        pthread_join(two, &result2);

        int a = (int)result1;
        int b = (int)result2;
        //totalNumberOfThreads_pthread = uxTaskGetStackHighWaterMark(NULL);
        return (void*)(a+b);
    }
} */

int fibonacci_packaged_task(int num)
{
    if (num == 0)
    {
        return 0;
    }
    else if (num == 1)
    {
        return 1;
    }
    else
    {
        packaged_task_pfr<int(int)> pac1{fibonacci_packaged_task};
        packaged_task_pfr<int(int)> pac2{fibonacci_packaged_task};
        future_pfr<int> fut1 = pac1.get_future();
        future_pfr<int> fut2 = pac2.get_future();
        pac1(num-1);
        pac2(num-2);

        int res1 = fut1.get();
        int res2 = fut2.get();
        return res1+res2;
    }
}

extern "C" void app_main() 
{
    packaged_task_pfr<int(int)> mainTask{fibonacci_packaged_task};
    future_pfr<int> fut = mainTask.get_future();
    mainTask(10);
    int res = fut.get();
    printf("%d\n", res);
    /* struct timeval tv_now_start;
    gettimeofday(&tv_now_start, NULL);
    int64_t time_us_start = (int64_t)tv_now_start.tv_sec * 1000000L + (int64_t)tv_now_start.tv_usec;
    printf("Time: %lld\n", time_us_start);
    
    void* a = NULL;
    /* pthread_t_pfr thr;
    TickType_t start = xTaskGetTickCount();
    pthread_create_pfr(thr, fibonacci, (void*)11);
    pthread_join_pfr(thr,  &a);
    printf("the number is %d\n", a);
    printf("total number of threads is: %d\n", totalNumberOfThread );*/
    /* pthread_t thr;
    pthread_create(&thr, NULL, fibonacci_pthread, (void*)12);
    pthread_join(thr, &a);
    printf("the number is %d\n", a);
    printf("total number of threads is: %d\n", totalNumberOfThreads_pthread); */
    //printf("unsused stack: %d\n", )

    /* struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
    printf("Time: %lld\n", time_us);

    double ms = (time_us-time_us_start)/1000;

    printf("Time dif ms: %f\n", ms);

    vTaskDelete(NULL);  */

}