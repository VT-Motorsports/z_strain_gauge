#pragma once
#include <zephyr/kernel.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/sys/mem_stats.h>
#include <zephyr/debug/cpu_load.h>
#include "hardware.h"

class System {
private:
    static constexpr size_t HEAP_SIZE = 2048;
    static constexpr int DIAG_THREAD_STACK_SIZE = 1024;
    static constexpr int DIAG_THREAD_PRIORITY = 5;
    static constexpr int DIAG_PERIOD_MS = 1000;
    
    struct k_heap heap_;
    struct k_thread diag_thread_;
    k_thread_stack_t* diag_stack_;
    
    Hardware* hardware_;
    bool initialized_;
    bool diagnostics_running_;
    
    static void diagnostics_thread_entry(void* p1, void* p2, void* p3);
    void diagnostics_loop();
    
public:
    System();
    
    int init();
    int start_diagnostics(Hardware* hw);
    void stop_diagnostics();
    
    void* heap_alloc(size_t size, k_timeout_t timeout);
    void heap_free(void* ptr);
    int get_heap_stats(struct sys_memory_stats* stats);
    
    uint64_t get_uptime_ms() const;
    uint8_t get_cpu_load() const;
    
    bool is_initialized() const { return initialized_; }
};