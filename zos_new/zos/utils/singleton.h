#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#include <type_traits>
#include <iostream>
#include <mutex>
#include <memory> 

namespace zos {

template <typename T, typename D = T>
class Singleton{
    friend D;
    // static_assert(std::is_base_of_v<T, D>, "T should be a base type for D"); 
public:
    template<typename... Ts>
    static T* instance(Ts... args) {
        std::call_once(_flag, [&](){
            // Use raw pointer to create a "Leaky Singleton"
            // This prevents destruction at exit, solving the static destruction order fiasco
            _instance = new T(std::forward<Ts>(args)...);
        });
        return _instance;
    }
    
    template<typename... Ts>
    static T* GetInstance(Ts... args){ return instance(std::forward<Ts>(args)...); }
    
    template<typename... Ts>
    static T* _(Ts... args){ return instance(std::forward<Ts>(args)...); }

protected: // Use protected instead of private to allow friend D
    Singleton() = default;
    virtual ~Singleton() = default;
    Singleton( const Singleton& ) = delete;
    Singleton& operator=( const Singleton& ) = delete;

private:
   static T* _instance;
   static std::once_flag _flag;
};

template <typename T, typename D>
T* Singleton<T, D>::_instance = nullptr;

template <typename T, typename D>
std::once_flag Singleton<T, D>::_flag;

} // namespace zos
#endif // __SINGLETON_H__
