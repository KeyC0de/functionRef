<h1 align="center">
	<a href="https://github.com/KeyC0de/functionRef">functionRef</a>
</h1>
<hr>


My own implementation of another functionRef.
I was desperate of finding out just how std::function<> works. So I scoured the internet, books what have you. To find more information keywords:

1. Type erasure.
2. callable objects
3. std::invoke
4. higher order functions
5. lambda calculus

I finally made it through and I share it with you!

Based on std::function, functionRef is a non-owning wrapper for an arbitrary callable object.

Implementation notes:

- A primary template is used to match the complete type of a callable, say `void( int, int )` or int. The primary template remains an empty struct.
- We want to differentiate the type `Callable` to its return type and argument type(s). Therefore partial template specialization is used.
- The callable's type is deduced from the templated `functionRef`'s constructor. This is the only place where we can acquire information about the pointee's (ie the callable object) type.
- The `enable_if`s in the constructor constrain the class to accept Callable types only.
- The third `enable_if` is to prevent the constructor hijacking the copy_constructor (comment it out and see test case 13. fail; you'll realize why it's needed then)
- `void*` type erasure drives `functionRef` as does `std::function` (although std function plays with template-OO-virtual type erasure). The common interface required by all wrapped objects is they have to be [`Callable`s](https://en.cppreference.com/w/cpp/named_req/Callable).
- There are 2 members:
	1. a `void*` data member `m_pAddr` that points to the memory location of the callable object. In the constructor the callable object is converted from function/method pointer to a `void*` and stored in `m_pAddr`. Be warned that this is somewhat hacky but it works well no strings attached.
	2. a generic function pointer for a callable object `TReturn(*)(void*, TArgs...) m_pFn` whereby given the aforementioned `m_pAddr` it calls the callable object. This function pointer sets-up the `INVOKE` operation at the time of invocation of its `operator()`. `m_pFn` gets initialized by a stateless lambda which converts the `void* m_addr` just supplied, to function pointer and gets its arguments perfectly forwarded to it. In the case of a member function `m_pFn` acquires its type later through the `setTarget` member call, which converts the target member function (pointer) to a function pointer and stores it in `m_pFn` (yes this is hacky territory but works all the time).
	The two members both point to the exact same address in memory (in most architectures) but they are different representations for the specified operation on the address, one reads data from the address, the other executes instructions starting at that address.
- In contrast to `std::function`, `functionRef` stores a pointer to its target, it doesn't copy its target. If the target is not a glvalue it will vaporize after the call to the constructor returns (it may still be valid but it's UB territory). However we still want to be able to also capture such temporary callables, such as lambdas created on the fly, as this is often needed as one-time thing call.
- If the constructor is passed an lvalue, `T` is an lvalue reference type, and thus `T*` would be ill-formed. That is why I use `add_pointer_t<T>` st `T*` would point to the actual object.
- If the callable object is of const-qualified type, then `reinterpret_cast<void*>( addressof( callable ) )` could not cast away the constness of the object (nor could implicitly convert to `void*` because that would drop const and C++ is strict on enforcing const-correctness rules). Thus we need `const_cast` first for that. This is why I use (`void*`) to make sure that explicit conversion to `void*` happens for both const and non-const callables.
- There is no need for templated `operator()` in use with `std::forward`, as the value categories of the arguments are already "locked in" as they were passed to the constructor
- The destructor doesn't delete or invalidate the underlying object in any way (although it may not be possible to access it afterwards. Note that `functionRef` is only a view/ref class, not a value class, like `std::function`). The destructor destructs the wrapper ie `functionRef` itself. We must make sure the pointee callable object lives at least as long as the wrapper `functionRef` object.
- `functionRef` is able to call member functions too, including virtual member functions. The process requires an intermediate setup step though. The Class of the target object and the target member function are not supplied as part of the struct, because then we would have to store them and increase `functionRef`'s size requirements permanently even if the target member was not a member function. Instead I've set it up such that an intermediate step is required to setup the target member function and the target object to call with it and only then call the function as a regular non-member function, through `functionRef`, simply by supplying its arguments (as you would do with any other function `functionRef` suports).

For example:
you first have to:</br>
`myFRef.setTarget( targetObj, &TargetClass::targetMemberFunction)`

and then

`myFRef(args...)`

- Although member functions are handled with no issues the best way to call member functions is to use a lambda that will capture the target object, or pass it as a parameter at the call site, and then call its member functions like so: `function_ref<void(T&)> ref( []( T& obj ){ obj.foo(); });`
- `functionRef` doesn't "call" member data although they are in theory "Callables" - and that is intentional

I provide numerous test cases in main.cpp which you can test for yourself.

I used Windows 8.1 x86_64, Visual Studio 2017, C++17 to build the project. I've also tested it working on Linux.


# Contribute

Please submit any bugs you find through GitHub repository 'Issues' page with details describing how to replicate the problem. If you liked it or you learned something new give it a star, clone it, laugh at it, contribute to it whatever. I appreciate all of it. Enjoy.


# License

Distributed under the GNU GPL V3 License. See "GNU GPL license.txt" for more information.


# Contact

email: *nik.lazkey@gmail.com*</br>
website: *www.keyc0de.net*


# Acknowledgements

[open-std function_ref proposal](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0792r2.html)</br>
[Implementing function_view is harder than you might think - Foonathan blog](https://foonathan.net/2017/01/function-ref-implementation/)
