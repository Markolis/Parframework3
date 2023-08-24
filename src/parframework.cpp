#include "parframework.hpp"
#include <functional>
#include <array>
#include <vector>
#include <functional>
#include <tuple>
#include <stack>

namespace Parframework
{

int sem_init_pfr(sem_t_pfr* sem, unsigned int val)
{
  if (sem->valid)
  {
    return -1;
  }
  sem->semaphore = xSemaphoreCreateCounting(val, val);
  if (sem->semaphore)
  {
    sem->valid = true;
    return 0;
  }
  else
  {
    return -1;
  }
}

int sem_getvalue_pfr(sem_t_pfr* sem, int* val)
{
  if (!sem->valid)
  {
    return -1;
  }
  *val = uxSemaphoreGetCount(sem->semaphore);

  return 0;
}

int sem_destroy_pfr(sem_t_pfr* sem)
{
  if (!sem->valid)
  {
    return -1;
  }
  vSemaphoreDelete(sem->semaphore);
  return 0;
}

pthread_t_pfr* pthread_self()
{
  TaskHandle_t curHandle = xTaskGetCurrentTaskHandle();
  pthread_t_pfr* thread = pthread_t_pfr::tHandle_thread_map.at(curHandle);
  return thread;
}

int sem_wait_pfr(sem_t_pfr* sem)
{
  pthread_t_pfr* self = pthread_self();
  if (self->cancel_state == pthread_t_pfr::cancelability_state::ENABLED 
  && self->cancel_type== pthread_t_pfr::cancelability_type::DEFERRED
  && self->cancelation_requested)
  {
    pthread_t_pfr handler;
    pthread_create_pfr(handler, cancel_handler, (void*)self);
    pthread_detach_pfr(handler);
    vTaskSuspend(NULL);
  }

  if (!sem->valid)
  {
    return -1;
  }
  BaseType_t ret = xSemaphoreTake(sem->semaphore, portMAX_DELAY);
  if (ret == pdTRUE)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

int sem_post_pfr(sem_t_pfr* sem)
{
  if (!sem->valid)
  {
    return -1;
  }
  
  BaseType_t ret = xSemaphoreGive(sem->semaphore);
  if (ret == pdTRUE)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

int sem_trylock_pfr(sem_t_pfr* sem)
{

  if (xSemaphoreTake(sem->semaphore, 0) == pdTRUE)
  {
  return 0;
  }
  else
  {
    return -1;
  }
  
}

int pthread_mutex_lock_pfr(pthread_mutex_t_pfr *mut)
{
  if (!mut->valid)
  {
    return -1;
  }
  else
  {
    xSemaphoreTake(mut->mutex, portMAX_DELAY);
    return 0;
  }
}

int pthread_mutex_trylock_pfr(pthread_mutex_t_pfr *mut)
{
  if (xSemaphoreTake(mut->mutex, 0) == pdTRUE)
  {
  return 0;
  }
  else
  {
    return -1;
  }
}

int pthread_mutex_unlock_pfr(pthread_mutex_t_pfr* mut)
{
  if (!mut->valid)
  {
    return -1;
  }
  BaseType_t ret = xSemaphoreGive(mut->mutex);
  if (ret == pdTRUE)
  {
    return 0;
  }
  else
  {
    return -1;
  }
}

int pthread_mutex_init_pfr(pthread_mutex_t_pfr* mut)
{
  if (mut->valid)
  {
    return -1;
  }
  mut->mutex = xSemaphoreCreateMutex();
  if (mut->mutex)
  {
    mut->valid = true;
    return 0;
  }
  else
  {
    return -1;
  }
}

int pthread_mutex_destroy_pfr(pthread_mutex_t_pfr* mut)
{
  if (!mut->valid)
  {
    return -1;
  }
  vSemaphoreDelete(mut->mutex);
  return 0;
}


pthread_t_pfr::pthread_t_pfr() : tHandle{NULL}, 
                                 detached{false}, 
                                 joined{false},
                                 is_a_cancel_thread{false},
                                 cancel_state{ENABLED}, 
                                 cancel_type{DEFERRED}, 
                                 threadSem{xSemaphoreCreateBinary()},
                                 clean_up_handlers{}, 
                                 cancelation_requested{false},
                                 cancel_thread{NULL},
                                 exit_status{NULL} {}
pthread_t_pfr::~pthread_t_pfr()
{
  vSemaphoreDelete(threadSem);
  if (!(joined || detached))
  {
    abort();
  }
}

int pthread_create_pfr(pthread_t_pfr &thr, void *(*start_routine)(void *), void *arg)
{
  thr.argStruct = {&thr, arg, start_routine};
  auto wrappedFunc = [](void *arg)
  {
    pthread_t_pfr::threadArgStruct* argStr = (pthread_t_pfr::threadArgStruct*)arg;

    void *taskArgument = argStr->param;
    pthread_t_pfr *calledThread = argStr->thread;
    void *(*start_routine)(void *) = argStr->start_routine;

    void* ret = start_routine(taskArgument);
    calledThread->exit_status = ret;

    prepare_exit(calledThread);
    calledThread->tHandle = NULL;
    if (calledThread->is_a_cancel_thread)
    {
      delete calledThread;
    }
    vTaskDelete(NULL);
  };
  xTaskCreate(wrappedFunc, "STANDARD_TASK", 1024, &thr.argStruct, 0, &thr.tHandle);
  pthread_t_pfr::tHandle_thread_map.insert({thr.tHandle, &thr});
  vTaskPrioritySet(thr.tHandle, 1);
  if (!thr.tHandle)
  {
    return -1;
  }
  //printf("task created\n");
  return 0;
}

int pthread_detach_pfr(pthread_t_pfr& thr)
{
  if (thr.detached || thr.joined)
  {
    abort();
  }
  thr.detached = true;
  return 0;
}

int pthread_join_pfr(pthread_t_pfr& thr, void **retval)
{
  if (thr.detached || thr.joined)
  {
    printf("same thread\n");
    abort();
  }
  xSemaphoreTake(thr.threadSem, portMAX_DELAY);
  if (retval)
  {
  *retval = thr.exit_status;
  }
  thr.joined = true;
  return 0;
}


int pthread_setcancelstate_pfr(int state, int* oldstatetype)
{
  if (!(state == pthread_t_pfr::cancelability_state::ENABLED || state == pthread_t_pfr::cancelability_state::DISABLED) || !oldstatetype)
  {
    return-1;
  }
  pthread_t_pfr* curThread = pthread_self();
  *oldstatetype = curThread->cancel_state;
  curThread->cancel_state = (pthread_t_pfr::cancelability_state)state;
  return 0;
}

int pthread_setcanceltype_pfr(int type, int* oldstatetype)
{
  if (!(type == pthread_t_pfr::cancelability_type::DEFERRED || type == pthread_t_pfr::cancelability_type::ASYNCHRONOUS))
  {
    return -1;
  }
  pthread_t_pfr* curThread = pthread_self();
  *oldstatetype = curThread->cancel_type;
  curThread->cancel_type = (pthread_t_pfr::cancelability_type)type;
  return 0;
}

void pthread_cleanup_push_pfr(void (*routine)(void *), void *arg)
{
  pthread_t_pfr* curThread = pthread_self();
  pthread_t_pfr::clean_up_struct cleaner = {.param = arg, .func = routine};
  curThread->clean_up_handlers.push(cleaner);
}

void pthread_cleanup_pop_pfr(int execute)
{
  pthread_t_pfr* curThread = pthread_self();
  std::stack<pthread_t_pfr::clean_up_struct>* clean_up_handlers = &curThread->clean_up_handlers;
  if (clean_up_handlers->empty())
  {
    return;
  }
  pthread_t_pfr::clean_up_struct cleaner = clean_up_handlers->top();
  clean_up_handlers->pop();
  if (execute > 0)
  {
    cleaner.func(cleaner.param);
  }
}

void pthread_exit_pfr(void* retval)
{
  pthread_t_pfr* current_thread = pthread_self();
  current_thread->exit_status = retval;
  prepare_exit(current_thread);
  vTaskDelete(NULL);

}

void* cancel_handler(void* thread)
{
  pthread_t_pfr* thr = (pthread_t_pfr*)thread;
  prepare_exit(thr);
  thr->exit_status = (void*)-1;
  vTaskDelete(thr->tHandle);
  thr->tHandle = NULL;
  return NULL;
}

void prepare_exit(pthread_t_pfr* thr)
{
    std::stack<pthread_t_pfr::clean_up_struct>* clean_up = &thr->clean_up_handlers;
    while(!clean_up->empty())
    {
      pthread_t_pfr::clean_up_struct cleaner = clean_up->top();
      clean_up->pop();
      cleaner.func(cleaner.param);
    }

    pthread_t_pfr::tHandle_thread_map.erase(thr->tHandle);
    xSemaphoreGive(thr->threadSem);
}


int pthread_cancel_pfr(pthread_t_pfr& thr)
{
  thr.cancelation_requested = true;
  if (thr.cancel_state == pthread_t_pfr::cancelability_state::DISABLED)
  {
    return 0;
  }
  else if (thr.cancel_type == pthread_t_pfr::cancelability_type::ASYNCHRONOUS)
  {
    if (thr.tHandle == xTaskGetCurrentTaskHandle())
    {
      pthread_exit_pfr((void*)-1);
    }
    vTaskSuspend(thr.tHandle);  
    thr.cancel_thread = new pthread_t_pfr();
    thr.cancel_thread->is_a_cancel_thread = true;
    pthread_create_pfr(*thr.cancel_thread, cancel_handler, (void*)&thr);
    pthread_detach_pfr(*thr.cancel_thread);
    printf("handler called\n");
    return 0;
  }

  return -1;
}

void pthread_testcancel()
{
  pthread_t_pfr* thr = pthread_self();
  if (thr->cancelation_requested 
  && thr->cancel_state == pthread_t_pfr::cancelability_state::ENABLED 
  && thr->cancel_type == pthread_t_pfr::cancelability_type::DEFERRED)
  {
    pthread_exit_pfr((void*)-1);
  }
  else
  {
    return;
  }
}

int pthread_cond_init_pfr(pthread_cond_t_pfr* var)
{
  if (var->valid)
  {
    return -1;
  }
  var->waiting_threads = std::queue<TaskHandle_t>();
  var->valid = true;
  return 0;
}

int pthread_cond_wait_pfr(pthread_cond_t_pfr* var, pthread_mutex_t_pfr* mut)
{
  if (!var->valid)
  {
    return -1;
  }
  var->waiting_threads.push(xTaskGetCurrentTaskHandle());
  pthread_mutex_unlock_pfr(mut);
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  pthread_mutex_lock_pfr(mut);
  return 0;
}


int pthread_cond_signal_pfr(pthread_cond_t_pfr* var)
{
  if (var->valid)
  {
    if (!var->waiting_threads.empty())
    {
      TaskHandle_t thread = var->waiting_threads.front();
      var->waiting_threads.pop();
      xTaskNotifyGive(thread);
    }
    return 0;
  }
  else
  {
    return -1;
  }
}

int pthread_cond_broadcast_pfr(pthread_cond_t_pfr* var)
{
  if (var->valid)
  {
    while(!var->waiting_threads.empty())
    {
      TaskHandle_t thread = var->waiting_threads.front();
      var->waiting_threads.pop();
      xTaskNotifyGive(thread);
    }
    return 0;
  }
  else
  {
    return -1;
  }
}

void* threadPoolTask(void* arg)
{
  ThreadPool* pool = (ThreadPool*)arg;
  ThreadPool::threadPoolArgStruct task;
  while(1)
  {
    pthread_t_pfr* thr = pool->findFirstFreeThread();
    if (thr && !pool->waiting_queue.empty())
    {
      task = pool->waiting_queue.front();
      pool->waiting_queue.pop();
      reinit_thread(thr);
      pthread_create_pfr(*thr, task.func, task.param);
    }
    else if(xQueueReceive(pool->queue, (void*)&task, 50 / portTICK_PERIOD_MS) == pdTRUE)
    {
      printf("received a task\n");
      pthread_t_pfr* thr = pool->findFirstFreeThread();
      if (thr)
      {
        printf("available thread found\n");
        reinit_thread(thr);
        pthread_create_pfr(*thr, task.func, task.param);
      }
      else
      {
        printf("no available thread found\n");
        pool->waiting_queue.push(task);
      }
    }
  }

  return NULL;
}

void reinit_thread(pthread_t_pfr* thr)
{
  thr->detached = false;
  thr->joined = false;
  vSemaphoreDelete(thr->threadSem);
  thr->threadSem = NULL;
  thr->threadSem = xSemaphoreCreateBinary();
  thr->cancelation_requested = false;
  thr->cancel_state = pthread_t_pfr::cancelability_state::DISABLED;
  thr->cancel_type = pthread_t_pfr::cancelability_type::DEFERRED;
}


}