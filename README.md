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
