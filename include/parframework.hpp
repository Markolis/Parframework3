#ifndef PARFRAMEWORK_H
#define PARFRAMEWORK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <queue>
#include <tuple>
#include <stack>
#include <functional>
#include <map>

namespace Parframework
{
  class sem_t_pfr
  {
    friend int sem_init_pfr(sem_t_pfr* sem, unsigned int initial_value);
    friend int sem_getvalue_pfr(sem_t_pfr* sem, int* val);
    friend int sem_destroy_pfr(sem_t_pfr* sem);
    friend int sem_wait_pfr(sem_t_pfr* sem);
    friend int sem_post_pfr(sem_t_pfr* sem);
    friend int sem_trylock_pfr(sem_t_pfr* sem);
    public:
    sem_t_pfr() : valid{false} {}


    private:
    SemaphoreHandle_t semaphore;
    bool valid;
  };

  int sem_init_pfr(sem_t_pfr* sem, unsigned int initial_value);
  int sem_getvalue_pfr(sem_t_pfr* sem, int* val);
  int sem_destroy_pfr(sem_t_pfr* sem);
  int sem_wait_pfr(sem_t_pfr* sem);
  int sem_post_pfr(sem_t_pfr* sem);
  int sem_trylock_pfr(sem_t_pfr* sem);

  class pthread_mutex_t_pfr
  {

    friend int pthread_mutex_lock_pfr(pthread_mutex_t_pfr *mut);
    friend int pthread_mutex_trylock_pfr(pthread_mutex_t_pfr *mut);
    friend int pthread_mutex_unlock_pfr(pthread_mutex_t_pfr *mut); 
    friend int pthread_mutex_destroy_pfr(pthread_mutex_t_pfr* mut);
    friend int pthread_mutex_init_pfr(pthread_mutex_t_pfr* mut);
    public:
    pthread_mutex_t_pfr() : valid{false} {}

    private:
    SemaphoreHandle_t mutex;
    bool valid;
  };

  int pthread_mutex_lock_pfr(pthread_mutex_t_pfr *mut);
  int pthread_mutex_trylock_pfr(pthread_mutex_t_pfr *mut);
  int pthread_mutex_unlock_pfr(pthread_mutex_t_pfr *mut); 
  int pthread_mutex_destroy_pfr(pthread_mutex_t_pfr* mut);
  int pthread_mutex_init_pfr(pthread_mutex_t_pfr* mut);

  class pthread_t_pfr
  {
    friend class ThreadPool;
    friend class pthread_pfr_condition_variable;


    friend int sem_wait_pfr(sem_t_pfr* sem);
    friend int pthread_create_pfr(pthread_t_pfr &thr, void *(*start_routine)(void *), void* arg);
    friend int pthread_detach_pfr(pthread_t_pfr& thr);
    friend int pthread_join_pfr(pthread_t_pfr& thr, void** retval);
    friend int pthread_cancel_pfr(pthread_t_pfr& thr);
    friend int pthread_setcancelstate_pfr(int state, int* oldstatetype);
    friend int pthread_setcanceltype_pfr(int state, int* oldstatetype);
    friend void pthread_cleanup_push_pfr(void (*routine)(void *), void *arg);
    friend void pthread_cleanup_pop_pfr(int execute);
    friend pthread_t_pfr* pthread_self();
    friend void pthread_exit_pfr(void* retval);
    friend void* cancel_handler(void* stack);
    friend void prepare_exit(pthread_t_pfr* thr);
    friend void pthread_testcancel();
    friend void* threadPoolTask(void* arg);
    friend void reinit_thread(pthread_t_pfr* thr);

    public:
      pthread_t_pfr();
      ~pthread_t_pfr();
      TaskHandle_t tHandle;

    private:
      struct threadArgStruct
      {
        pthread_t_pfr *thread;
        void *param;
        void *(*start_routine)(void *);
      };

      struct clean_up_struct
      {
        void *param;
        TaskFunction_t func;
      };

      enum cancelability_state {DISABLED, ENABLED};
      enum cancelability_type{ASYNCHRONOUS, DEFERRED};

      threadArgStruct argStruct;
      bool detached;
      bool joined;
      bool is_a_cancel_thread;
      cancelability_state cancel_state;
      cancelability_type cancel_type;
      SemaphoreHandle_t threadSem;
      std::stack<clean_up_struct> clean_up_handlers;
      bool cancelation_requested;
      pthread_t_pfr* cancel_thread;
      void* exit_status;
      inline static std::map<TaskHandle_t, pthread_t_pfr*> tHandle_thread_map = std::map<TaskHandle_t, pthread_t_pfr*>();

  };

  pthread_t_pfr* pthread_self();

  void* cancel_handler(void*);

  void prepare_exit(pthread_t_pfr* thr);
  
  int pthread_create_pfr(pthread_t_pfr &thr, void *(*start_routine)(void *), void *arg);

  int pthread_detach_pfr(pthread_t_pfr& thr);

  int pthread_join_pfr(pthread_t_pfr& thr, void **retval);

  int pthread_pfr_cancel(pthread_t_pfr& thr);

  int pthread_setcancelstate_pfr(int state, int* oldstatetype);

  int pthread_setcanceltype_pfr(int state, int* oldstatetype);

  void pthread_cleanup_push(void (*routine)(void *arg), void *arg);

  void pthread_cleanup_pop(int execute);

  void pthread_exit_pfr(void* retval);

  void pthread_testcancel();

  class pthread_cond_t_pfr
  {
    friend int pthread_cond_init_pfr(pthread_cond_t_pfr* var);
    friend int pthread_cond_signal_pfr(pthread_cond_t_pfr* var);
    friend int pthread_cond_broadcast_pfr(pthread_cond_t_pfr* var);
    friend int pthread_cond_wait_pfr(pthread_cond_t_pfr* var, pthread_mutex_t_pfr* mut);
    public:
    pthread_cond_t_pfr() : valid{false} {}
    private:
    
    bool valid;
    std::queue<TaskHandle_t> waiting_threads;
    
  };

  int pthread_cond_init_pfr(pthread_cond_t_pfr* var);
  int pthread_cond_signal_pfr(pthread_cond_t_pfr* var);
  int pthread_pfr_cond_broadcast_pfr(pthread_cond_t_pfr* var);
  int pthread_cond_wait_pfr(pthread_cond_t_pfr* var, pthread_mutex_t_pfr* mut);

template<class T> 
class promise_pfr;

template<>
class promise_pfr<void>;

template<class T>
class future_pfr
{
  friend class promise_pfr<T>;
  public:

  future_pfr() : in_shared_state{false} {}

  T get()
  {
    if (!in_shared_state)
    {
      abort();
    }
    else 
    {
      T item;
      if(xQueueReceive(queue, (void*)&item, portMAX_DELAY) != pdTRUE)
      {
        abort();
      }
      vQueueDelete(queue);
      in_shared_state = false;
      return item;  
    }
  }

  private:
  QueueHandle_t queue;
  bool in_shared_state;
};

template<>
class future_pfr<void>
{
  friend class promise_pfr<void>;

  public: 
  future_pfr() : in_shared_state{false}, sem{NULL} {}

  void get()
  {
    if (!in_shared_state)
    {
      abort();
    }
    xSemaphoreTake(sem, portMAX_DELAY);
    in_shared_state = false;
    vSemaphoreDelete(sem);
  }


  private:
  bool in_shared_state;
  SemaphoreHandle_t sem;
};

template<class T>
class promise_pfr
{
  public:
  promise_pfr() : queue{xQueueCreate(1, sizeof(T))}, set{false}{}

  future_pfr<T> get_future()
  {
    future = future_pfr<T>();
    future.queue = queue;
    future.in_shared_state = true;
    return future;
  }

  void set_value(T arg)
  {
    if (set || xQueueSend(queue, (void*)&arg, (TickType_t )0) != pdPASS)
    {
      abort();
    }
    set = true;
  }
  
  private:
  future_pfr<T> future;
  QueueHandle_t queue;
  bool set;

};

template<>
class promise_pfr<void>
{
  public:
  promise_pfr() : set{false}, sem{xSemaphoreCreateBinary()} {}

  future_pfr<void> get_future()
  {
    future = future_pfr<void>();
    future.in_shared_state = true;
    future.sem = sem;
    return future;
  }

  void set_value()
  {
    if (set)
    {
      abort();
    }
    xSemaphoreGive(sem);
    set = true;
  }

  private:
  future_pfr<void> future;
  bool set;
  SemaphoreHandle_t sem;
};

template<class Ret, class ...ArgTypes>
class packaged_task_pfr{};

template<class Ret, class ...ArgTypes>
class packaged_task_pfr<Ret(ArgTypes...)>
{
  public:
  packaged_task_pfr(Ret (*func)(ArgTypes...)) : task{func}, promise{} {}
  void operator()(ArgTypes... values)
  {
    argStruct = {std::make_tuple<ArgTypes...>(std::move(values...)), task, &promise};
    auto wrappedFunc = [](void *arg)
    {
      taskArgStruct* argStr = (taskArgStruct*)arg;
      std::tuple<ArgTypes...> tuple = argStr->args;
      Ret (*taskFunc)(ArgTypes...) = argStr->task;
      promise_pfr<Ret>* promise = argStr->promise; 

      promise->set_value(std::apply(taskFunc, tuple));

      argStr->task = NULL;
      vTaskDelete(NULL);
    };
    
  TaskHandle_t tHandle = NULL;
  xTaskCreate(wrappedFunc, "PACKAGED_TASK", 1024, &argStruct, 1, &tHandle);

  assert(tHandle);
  }

  future_pfr<Ret> get_future()
  {
    return promise.get_future();
  }

  private:
  struct taskArgStruct
  {
    std::tuple<ArgTypes...> args;
    Ret (*task)(ArgTypes...);
    promise_pfr<Ret>* promise;
  };

  taskArgStruct argStruct;
  Ret (*task)(ArgTypes...);
  promise_pfr<Ret> promise;
};

const size_t THREAD_NUMBER = 2;

void* threadPoolTask(void* arg);

class ThreadPool
{
  friend void* threadPoolTask(void* arg);
  public:
  ThreadPool(): pool{}, size{THREAD_NUMBER}, waiting_queue{}, queue{xQueueCreate(7, sizeof(threadPoolArgStruct))}
  {
    for (int i = 0; i < THREAD_NUMBER; i++)
    {
      pthread_detach_pfr(pool[i]);
    }
  }

  void start()
  {
    pthread_create_pfr(taskThr, threadPoolTask, this);
    pthread_detach_pfr(taskThr);
  }

  void queueTask(void *(*start_routine)(void *), void* arg)
  {
    threadPoolArgStruct poolArg = {.func = start_routine, .param = arg};
    threadPoolArgStruct& mes = poolArg;
    xQueueSend(queue, (void*)&mes, portMAX_DELAY);
    printf("task sent...\n");    
  }

  private:


  struct threadPoolArgStruct
  {
    void *(*func)(void *);
    void *param;
  };

  pthread_t_pfr* findFirstFreeThread()
  {
    for (int i = 0; i < THREAD_NUMBER; i++)
      if (!pool[i].tHandle) return &pool[i];

    return NULL;
  }

  std::array<pthread_t_pfr, THREAD_NUMBER> pool;
  size_t size;
  std::queue<threadPoolArgStruct> waiting_queue;
  QueueHandle_t queue;
  pthread_t_pfr taskThr;

};




}

#endif