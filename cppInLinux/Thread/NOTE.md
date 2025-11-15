# 1. 线程库的基本使用
## 创建一个线程
```c++
std::thread t1(printHello, "World"); // 创建一个线程，并传入参数
t1.detach(); // 线程分离，主线程不等待子线程结束
t1.join(); // 等待子线程结束
```
`joinable` 表示：这个 thread 对象 当前是否表示一个可用的执行线程。
换句话说：
- true → 这个线程对象有一个正在运行或已运行完但还没被回收的线程。
- false → 这个线程对象是“空的”，没有管理任何线程。
```c++
bool isJoin = t1.joinable(); 
```
# 2. 线程函数中的数据未定义错误
`std::ref()`传递引用类型的变量
共享指针解决类的示例提前释放问题
```c++
std::shared_ptr<A> a = std::make_shared<A>();
std::thread t3(&A::foo, a);
t3.join();
```
# 3. 互斥量解决多线程数据共享问题
```c++
std::mutex mtx;
mtx.lock(); // 加锁
// 线程安全的代码
mtx.unlock(); // 解锁
```
# 4. lock_guard与unique_lock
`lock_guard`是c++标准库中的互斥量封装类，用于保护共享数据，防止多个线程同时访问同一资源而导致数据竞争问题。
特点：
- 当构造函数被调用时，该互斥量会被自动锁定
- 当析构函数被调用时，该互斥量会被自动解锁
- std::lock_guard对象不能复制或移动，因此它只能在局部作用域中使用

`unique_lock`是c++标准库中提供的一个互斥量封装类，用于在多线程中对互斥量进行加锁和解锁操作。
它的主要特点是可以对互斥量进行更灵活的管理，包括延迟加锁、条件变量、超时等。
成员函数：
- `lock()`:尝试对互斥量进行加锁操作，如果当前互斥量已经被其他线程持有，则当前线程会被阻塞，直到互斥量被解锁后成功加锁
- `try_lock()`:尝试对互斥量进行加锁操作，如果当前互斥量已经被其他线程持有，则立即返回false，否则成功加锁并返回true
- `try_lock_for(const std::chrono::duration<Rep, Period>& rel_time)`：尝试对互斥量进行加锁操作，如果当前互斥量已经被其他线程持有，则在rel_time时间内等待，直到互斥量被解锁后成功加锁，或者超时返回false
- `try_lock_until(const std::chrono::time_point<Clock, Duration>& abs_time)`：尝试对互斥量进行加锁操作，如果当前互斥量已经被其他线程持有，则在abs_time时间点之前等待，直到互斥量被解锁后成功加锁，或者超时返回false
- `unlock()`:解锁互斥量

# 5. call_once与其使用场景
单例设计模式需要确保某个类中只能创建一个实例，由于是全局唯一的，所以在使用单例模式的时候需要考虑线程安全问题。
`call_once`是c++11中提供的一个函数模板，用于确保某个函数只被调用一次，并且在多线程环境下也能保证线程安全。
只能在多线程中使用，在main中使用会报错
```c++
tmeplate<class Callable, class... Args>
void call_once(std::once_flag& flag, Callable&& func, Args&&... args)
```
`std::once_flag`是一个类型，用于标识某个函数是否已经被调用过。
`func`是被调用的函数，`args`是函数的参数。

# 6. condition_variable与其使用场景
常使用在生产者消费者模型
`condition_variable`是c++11中提供的一个类，用于实现线程间的同步。
步骤如下：
> 1. 创建一个condition_variable对象
2. 创建一个互斥锁std::mutex对象，用来保护共享资源的访问
3. 在需要等待条件变量的地方，使用std::unique_lock<std::mutex> 对象锁定互斥锁，并调用
std::condition_variable::wait()、std::condition_variable::wait_for()等函数等待条件变量
4. 在其他线程中需要同时等待的线程时，调用std::condition_vatriable::notify_one()
或std::condition_variable::notify_all()函数

# 7. 异步并发--async future packaged_task promise
1. async、future
是c++11中引入的函数模板，用于异步执行一个函数，并返回一个std::future对象，表示异步操作的结束。
使用std::async可以方便进行异步编程，避免手动创建线程和管理线程的麻烦。
2. packaged_task
在c++中，packaged_task是一个类模板，用于讲一个可调用对象（如函数，函数对象，或lambda表达式）封装成一个可调用的对象。
并返回一个std::future对象，表示异步操作的结果。packaged_task可以方便将一个函数或可调用对象转换成一个异步操作。
3. promise
在c++中，promise是一个类模板，用于在一个线程中产生一个值，并在另一个线程中获取这个值。promise通常与future和async一起使用。

# 8. 原子操作--atomic
`std::atomic`是c++11中的一个模板类，用于实现多线程环境下的原子操作。它提供了一种线程安全的方式来访问和修改共享变量，
可以避免多线程环境中的数据竞争问题。
`std::atomic`的使用方式类似于c++中的普通变量，但是它的操作是原子性的。也就是说，在多线程环境下，多个线程同时对同一个
`std::atomic`变量进行操作时，不会出现数据竞争问题。
以下是一些常见的`std::atomic`操作
1. `load()`:将原子变量的值加载到当前线程的本地缓存中，并返回这个值。
2. `store()`: 将`val`的值存储到原子变量中，并保证这个操作时原子性的。
3. `exchange()`: 将`val`的值存储原子变量中，并返回修改前的旧值。
4. `compare_exchange_strong(expected, desired)`: 如果原子变量当前值 == expected，则将其更新为 desired，并返回 true；
否则返回 false，并将 expected 更新为当前值。
