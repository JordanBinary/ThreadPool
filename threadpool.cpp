#include "stdafx.h"

ThreadPool* ThreadPool::instance = NULL;
CRITICAL_SECTION ThreadPool::instanceCS;

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    InitializeCriticalSection(&queueCS);
    workEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    workers.resize(threads);
    for (size_t i = 0; i < threads; ++i) {
        workers[i] = CreateThread(NULL, 0, WorkerThread, this, 0, NULL);
    }
}

ThreadPool::~ThreadPool() {
    EnterCriticalSection(&queueCS);
    stop = true;
    LeaveCriticalSection(&queueCS);

    SetEvent(workEvent);
    for (size_t i = 0; i < workers.size(); ++i) {
        WaitForSingleObject(workers[i], INFINITE);
        CloseHandle(workers[i]);
    }

    CloseHandle(workEvent);
    DeleteCriticalSection(&queueCS);
}

DWORD WINAPI ThreadPool::WorkerThread(LPVOID arg) {
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    while (true) {
        WaitForSingleObject(pool->workEvent, INFINITE);

        EnterCriticalSection(&pool->queueCS);
        if (pool->stop && pool->tasks.empty()) {
            LeaveCriticalSection(&pool->queueCS);
            break;
        }

        void (*task)(void*) = NULL;
        if (!pool->tasks.empty()) {
            task = pool->tasks.front();
            pool->tasks.pop();
        }
        LeaveCriticalSection(&pool->queueCS);

        if (task) {
            task(NULL);
        }
    }
    return 0;
}

ThreadPool* ThreadPool::GetInstance(size_t threads) {
    EnterCriticalSection(&instanceCS);
    if (instance == NULL) {
        instance = new ThreadPool(threads);
    }
    LeaveCriticalSection(&instanceCS);
    return instance;
}

void ThreadPool::Enqueue(void (*f)(void*)) {
    EnterCriticalSection(&queueCS);
    tasks.push(f);
    LeaveCriticalSection(&queueCS);
    SetEvent(workEvent);
}

void ThreadPool::Initialize() {
    InitializeCriticalSection(&instanceCS);
}

void ThreadPool::Cleanup() {
    DeleteCriticalSection(&instanceCS);
    if (instance) {
        delete instance;
        instance = NULL;
    }
}
