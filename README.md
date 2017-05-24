# thread-ext
some useful primitives for multithreaded programming that are not included into std (since C++11)

## Supported Compilers
* TDM-GCC 4.9.2 integrated with [improved fork of Bloodshed Dev-C++](https://sourceforge.net/projects/orwelldevcpp/?source=typ_redirect)
* Microsoft (R) C/C++ Optimizing Compiler Version 19.00.xyz ([Visual Studio 2015](https://www.visualstudio.com/vs/visual-studio-express/))

## Building notes
This framework is deployed with a bundle of unit tests for the each particular primitive.
Corresponding projects (.dev; .vcxproj) are located in <root>/src/test/
In order to build them, you will require the following:

* [TUT framework](https://github.com/mrzechonek/tut-framework), to be cloned under the 'my_root_tut' directory in your file system structure 
* specify a path 'my_root_tut/include' in your project global settings

## te_async.h
the function that acts like std::async, but that automatically uses std::launch::async as the launch policy

```cpp
// the following invocations below are identical
int calculate() { ... return result; }

std::future<int> f1= std::async(std::launch::async, calculate);
std::future<int> f2= thread_ex::call_async(calculate);
```
### related link
* [Effective Modern C++, item 36](http://shop.oreilly.com/product/0636920033707.do) by Scott Meyers

## te_block_lock.h
lock (mutex) {...} block constuction like a language feature

```cpp
	std::mutex m;
	my_local_data data;
	
    lock( m, [&]{
    ... use data ...
    });		
```
### related link
* [Elements of Modern C++ Style](https://herbsutter.com/elements-of-modern-c-style/) by Herb Sutter

## te_hierarchical_mutex.h
Lock hierarchy is really a particular case of defining lock ordering, 
a lock hierarchy can provide a means of checking that the convention is adhered to at runtime. 
The idea is that you divide your application into layers and identify all the mutexes
that may be locked in any given layer.
When code tries to lock a mutex, it isn’t permitted to lock that mutex if it already holds a lock from a lower layer. 
You can check this at runtime by assigning layer numbers to each mutex 
and keeping a record of which mutexes are locked by each thread

```cpp
	hierarchical_mutex m1000   {1000};
	hierarchical_mutex m100    {100};
	hierarchical_mutex m10     {10};

	void low_level()
	{
	 lock_guard l(m10);
	 // ...
	}

	void middle_level()
	{
	 lock_guard l(m100);
	 // ...
	 low_level();   // <-- OK, higher layer calls lower one
	}

	void high_level()
	{
	 lock_guard l(m1000);
	 // ...
	 middle_level();   // <-- OK, higher layer calls lower one
	}

	void rude_level() // <-- bad specified by design
	{
	 lock_guard l(m100);
	 // ...
	 high_level();   // <-- not allowed, lower layer calls higher one
	}
```

### related link
* [C++ Concurrency in Action", chapter 3.2.5](https://www.amazon.com/C-Concurrency-Action-Practical-Multithreading/dp/1933988770) by Anthony Williams

## te_unique_pair.h
Calling of thread_ex::lock_unique_pair() locks the two mutexes, and two std::unique_lock instances are constructed, one for each mutex.
These two instances are packed up (by means 'move' semantics) into std::pair instance and returned to the client 

```cpp
container_type a, b;
mutex ma,mb;

{	// synchronized operation
	auto l = lock_unique_pair(ma,mb);
	std::copy_if(a.begin(),a.end(),b.end(),predicate);
	std::remove_if(a.begin(),a.end(),predicate);	
}
```
### related link
* [C++ Concurrency in Action", chapter 3.2.4](https://www.amazon.com/C-Concurrency-Action-Practical-Multithreading/dp/1933988770) by Anthony Williams

## te_runtime_concurrency.h
The number of threads to run is the minimum of calculated maximum (total_num/min_num) and
the number of hardware threads. You don’t want to run more threads than the
hardware can support (which is called oversubscription), because the context switching
will mean that more threads will decrease the performance

```cpp
	const size_t total_work      = 100;
	const size_t work_per_thread = 33;
	const size_t calculated_thread_no = runtime_concurrency(total_work, work_per_thread);

	// output
	// if hardware core=1,  then calculated_thread_no=1, i.e. 4 sequential tasks: 33+33+33+1
	// if hardware core=16, then calculated_thread_no=4, i.e. 4 simultaneous tasks
```

### related links
* [C++ Concurrency in Action", chapter 2.4](https://www.amazon.com/C-Concurrency-Action-Practical-Multithreading/dp/1933988770) by Anthony Williams
* [Oversubscription](http://blogs.msdn.com/b/visualizeparallel/archive/2009/12/01/oversubscription-a-classic-parallel-performance-problem.aspx)

## te_thread_unjoinable.h
RAII object which makes std::thread unjoinable all paths

```cpp
	// ...
	{
		 joined_thread t = std::thread({...});
	// ...
    }	// <--- waiting here unless the thread function (t, {...}) terminates
```

### related link
* [Effective Modern C++, item 37](http://shop.oreilly.com/product/0636920033707.do) by Scott Meyers


## te_container.h
contains generic container interfaces with no race conditions
* threadsafe_stack is an analog of std::stack<>
* threadsafe_queue is an analog of std::queue<>
### related links
* [C++ Concurrency in Action", chapter 3.2.3, stack](https://www.amazon.com/C-Concurrency-Action-Practical-Multithreading/dp/1933988770) by Anthony Williams
* [C++ Concurrency in Action", chapter 4.1.2, queue](https://www.amazon.com/C-Concurrency-Action-Practical-Multithreading/dp/1933988770)

## te_sequence.h
a generic sequential container interface with no race conditions
* threadsafe_vector is an analog of std::vector<>
* threadsafe_list is an analog of std::list<>


