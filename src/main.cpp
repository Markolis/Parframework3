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

void* taskOne(void*)
{
    printf("task1\n");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    printf("task1 ends\n");
    return NULL;
}

void* taskTwo(void*)
{
    printf("task2\n");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    printf("task2 ends\n");
    return NULL;
}

void* taskThree(void*)
{
    printf("task3\n");
    vTaskDelay(1000/portTICK_PERIOD_MS);
    printf("task3 ends\n");
    return NULL;
}

extern "C" void app_main() 
{
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

    ThreadPool pool;
    pool.start();
    pool.queueTask(taskOne, NULL);
    pool.queueTask(taskTwo, NULL);
    pool.queueTask(taskThree, NULL);
    vTaskDelete(NULL); 

    /* struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
    printf("Time: %lld\n", time_us);

    double ms = (time_us-time_us_start)/1000;

    printf("Time dif ms: %f\n", ms);

    vTaskDelete(NULL);  */

}