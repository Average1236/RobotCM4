#ifndef __ZOS_SEMAPHORE_H__
#define __ZOS_SEMAPHORE_H__

#include "zos/config.h"

#include <mutex>
#include <condition_variable>
namespace zos{
class SemaphoreT {
public:
    SemaphoreT(unsigned int _max_capacity=100,unsigned int count = 0)
        : _max_capacity(_max_capacity),count_(count){
        #ifdef ZOS_DEBUG
        zos::log("{} SemaphoreT constructor,capa : {},count:{}\n",fmt::ptr(this),_max_capacity,count_);
        #endif
    }
    SemaphoreT(const SemaphoreT& s):count_(0){
        #ifdef ZOS_DEBUG
        zos::log("{} SemaphoreT copy ctor\n",fmt::ptr(this));
        #endif
    }
    virtual ~SemaphoreT(){
//        notify_current();
        #ifdef ZOS_DEBUG
        zos::log("{} SemaphoreT destructor\n",fmt::ptr(this));
        #endif
    }
//    virtual void notify_current() override{
//        std::scoped_lock lock(mutex_);
//        count_ = 99999;
//        cv_.notify_all();
//    }
    virtual void release() {
        std::scoped_lock lock(mutex_);
        if (count_ < _max_capacity){
            count_++;
            #ifdef ZOS_DEBUG
            zos::log("{} SemaphoreT release-c {}\n",fmt::ptr(this),count_);
            #endif
            cv_.notify_one();
        }
    }
    virtual void acquire() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return count_ > 0; });
        --count_;
        #ifdef ZOS_DEBUG
        zos::log("{} SemaphoreT acquire-c {}\n",fmt::ptr(this),count_);
        #endif
    }
    virtual bool try_acquire(){
        return try_acquire_for(0);
    }
    template<typename _Rep,typename _Period>
    bool try_acquire_for(const std::chrono::duration<_Rep, _Period>& rel_time) {
        std::unique_lock lock(mutex_);
        auto res = cv_.wait_for(lock, rel_time, [this] { return count_ > 0; });
        if(count_ > 0){
            --count_;
            #ifdef ZOS_DEBUG
            zos::log("{} SemaphoreT try_acquire_for-c {}\n",fmt::ptr(this),count_);
            #endif
            return true;
        }
        return false;
    }
    virtual bool try_acquire_for(unsigned int c){
        return try_acquire_for(c*__timestep);
    }
private:
    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int count_;
    unsigned int _max_capacity;
};
}// namespace zos
#endif // __ZOS_SEMAPHORE_H__