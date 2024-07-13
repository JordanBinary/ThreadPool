#pragma once

class ThreadPool {
private:
    static ThreadPool* instance;
    static CRITICAL_SECTION instanceCS;

    std::vector<HANDLE> workers;
    std::queue<void (*)(void*)> tasks;
    CRITICAL_SECTION queueCS;
    HANDLE workEvent;
    bool stop;

    ThreadPool(size_t threads);
    ~ThreadPool();

    static DWORD WINAPI WorkerThread(LPVOID arg);

public:
    static ThreadPool* GetInstance(size_t threads = 3);
    void Enqueue(void (*f)(void*));
    static void Initialize();
    static void Cleanup();

private:
    ThreadPool(const ThreadPool&);
    ThreadPool& operator=(const ThreadPool&);
};
