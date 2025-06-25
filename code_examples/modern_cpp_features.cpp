// modern_cpp_features.cpp
// Code examples for advanced modern C++ features

// --- Advanced Lambda Expressions and Captures ---
#include <iostream>
#include <vector>
#include <algorithm>

void lambda_examples() {
    int x = 10;
    // 'mutable' allows the lambda to modify its captured variables (by value) inside its body.
    // However, since x is captured by value, any modification to x inside the lambda does NOT affect the original x outside the lambda.
    // The changes to x are local to the lambda object's internal copy.
    // No, it is not possible to modify a captured variable by value inside a lambda unless you explicitly mark the lambda as 'mutable'.
    // By default, the operator() of a lambda is 'const', so all captured variables are treated as const within the lambda body.
    // This means that if you try to modify 'x' (captured by value) inside the lambda without 'mutable', you will get a compiler error.
    // Example:
    // auto f = [x](int y) { x += y; }; // Error: assignment of read-only variable 'x'
    // To allow modification of the captured copy of 'x', you must declare the lambda as 'mutable':
    // auto f = [x](int y) mutable { x += y; return x; }
    // Note: This only modifies the lambda's internal copy of 'x', not the original 'x' outside the lambda.
    auto add = [x](int y) mutable { return x += y; };
    std::cout << "Lambda add: " << add(5) << std::endl;
    auto generic = [](auto a, auto b) { return a + b; };
    std::cout << "Generic lambda: " << generic(2, 3.5) << std::endl;
}

// --- Move Semantics and Perfect Forwarding ---
#include <utility>
#include <string>

void move_semantics() {
    std::string a = "Hello";
    std::string b = std::move(a); // a is now in a valid but unspecified state
    std::cout << "Moved string: " << b << std::endl;
}

// --- Perfect Forwarding: What and Why? ---
//
// Perfect forwarding is a technique in C++ (introduced with C++11) that allows a function template
// to forward its arguments to another function (such as a constructor or another function template)
// while preserving the value category (lvalue/rvalue) and const/volatile qualifiers of the arguments.
//
// This is crucial when writing generic code, such as factory functions or wrapper functions, where you
// want to pass arguments to another function exactly as they were received, without unnecessary copies
// or incorrect overload resolution.
//
// The canonical example is a factory function:
//
//   template<typename T, typename... Args>
//   T create(Args&&... args) {
//       return T(std::forward<Args>(args)...);
//   }
//
// Here, Args&&... is a "forwarding reference" (also known as a "universal reference").
// std::forward<Args>(args)... ensures that if the caller passes an lvalue, it is forwarded as an lvalue;
// if the caller passes an rvalue, it is forwarded as an rvalue.
//
// Why does this matter?
// - It enables efficient code: rvalues can be moved, lvalues can be copied or bound as references.
// - It preserves the intent of the caller, allowing the callee to select the correct overloads and constructors.
// - It is essential for writing generic wrappers, factories, and higher-order functions in modern C++.
//
// Example: Demonstrating perfect forwarding
#include <vector>

template<typename T, typename... Args>
T make_with_forwarding(Args&&... args) {
    // Forwards arguments perfectly to T's constructor
    return T(std::forward<Args>(args)...);
}

void perfect_forwarding_example() {
    std::vector<int> v1 = make_with_forwarding<std::vector<int>>(5, 42); // Calls vector(size_type, value)
    std::cout << "v1: ";
    for (auto i : v1) std::cout << i << " ";
    std::cout << std::endl;

    std::vector<int> init = {1, 2, 3};
    std::vector<int> v2 = make_with_forwarding<std::vector<int>>(init.begin(), init.end()); // Calls vector(iter, iter)
    std::cout << "v2: ";
    for (auto i : v2) std::cout << i << " ";
    std::cout << std::endl;
}
//
// If you did not use perfect forwarding (e.g., if you used Args... args instead of Args&&... args, or omitted std::forward),
// you could end up with unnecessary copies or even compilation errors due to incorrect overload resolution.
//
// In summary: Perfect forwarding is a key tool for writing efficient, generic, and correct C++ code that works seamlessly
// with both lvalues and rvalues.
//
// (If you want to see more advanced or tricky cases, or want to discuss pitfalls, let me know!)


template<typename T, typename... Args>
T create(Args&&... args) {
    return T(std::forward<Args>(args)...);
}

// --- Rvalue References and Resource Management ---
class Resource {
public:
    Resource() { std::cout << "Resource acquired\n"; }
    ~Resource() { std::cout << "Resource released\n"; }
    Resource(Resource&&) noexcept { std::cout << "Resource moved\n"; }
    Resource& operator=(Resource&&) noexcept { std::cout << "Resource move-assigned\n"; return *this; }
};

// --- C++17 Features: Structured Bindings, if constexpr, fold expressions ---
#include <tuple>

auto get_pair() { return std::make_pair(1, 2.5); }

void structured_bindings() {
    auto [i, d] = get_pair();
    std::cout << "Structured bindings: " << i << ", " << d << std::endl;
}

template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

// --- Threading and Concurrency ---
#include <thread>
#include <future>

// --- Promise/Future: Manually setting a value/exception for a future ---
// Use case: When one thread needs to signal a result or an event to another thread that is waiting for it.
// The 'promise' is the "writer" end that sets the value, and the 'future' is the "reader" end that retrieves it.
// This is ideal for one-off, asynchronous communication channels.
void promise_future_example() {
    std::promise<int> p;
    std::future<int> f = p.get_future();

    // Start a worker thread that will eventually fulfill the promise.
    // The promise is moved into the lambda to transfer ownership.
    std::thread worker([p = std::move(p)]() mutable {
        std::cout << "Worker thread is performing some work...\n";
        try {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::seconds(2));
            p.set_value(100); // Fulfill the promise with a value
            std::cout << "Worker thread has set the value.\n";
        } catch (...) {
            // In case of an error, set an exception on the promise
            p.set_exception(std::current_exception());
        }
    });

    std::cout << "Main thread is waiting for the result from the promise...\n";
    int result = f.get(); // Blocks until the promise is fulfilled (either with a value or an exception)
    std::cout << "Main thread received result via promise/future: " << result << std::endl;

    worker.join();
}

// --- Packaged Task: Bundling a callable with its future ---
// Use case: A higher-level abstraction than promise/future. It wraps a callable (function, lambda, etc.)
// so that it can be invoked asynchronously. The result of the callable is automatically stored in an
// associated future. This is perfect for task queues or thread pools where you want to submit a task
// and get a future back to retrieve its result later, without manually managing a promise.
int long_computation(int input) {
    std::cout << "Packaged task running long computation...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return input * 10;
}

void packaged_task_example() {
    // Create a packaged_task that wraps the long_computation function.
    // The template argument is the function signature: `int(int)` means a function returning int and taking an int.
    std::packaged_task<int(int)> task(long_computation);
    
    // Get the future associated with the task *before* moving the task.
    std::future<int> f = task.get_future();

    // Move the task to a new thread to be executed.
    // Note: packaged_task is not copyable, only movable. We also pass the argument for the function.
    std::thread task_thread(std::move(task), 5);

    std::cout << "Main thread has dispatched the packaged_task and is waiting...\n";
    int result = f.get(); // Blocks until the task completes and the result is available.
    std::cout << "Main thread received result from packaged_task: " << result << std::endl;

    task_thread.join();
}

void thread_example() {
    std::thread t([]{ std::cout << "Thread running\n"; });
    t.join();
    auto fut = std::async([]{ return 42; });
    std::cout << "Async result: " << fut.get() << std::endl;
}

// --- Type Traits and SFINAE ---
#include <type_traits>
// Type Traits are a set of compile-time templates that provide information about types.
// They allow you to inspect, compare, and transform types at compile time.
// This is incredibly useful for template metaprogramming and writing generic, yet type-safe, code.
// For example, `std::is_integral<T>::value` is true if T is an integral type (int, char, etc.).
// C++17 introduced `_v` variable templates (e.g., `std::is_integral_v<T>`) for convenience.

// SFINAE stands for "Substitution Failure Is Not An Error".
// It's a core C++ principle that applies during template overload resolution.
// If substituting a template argument into a function signature leads to an invalid type,
// the compiler doesn't stop with an error. Instead, it simply removes that function
// from the list of possible overloads.
// We can leverage this with type traits to enable or disable function templates based on type properties.
// `std::enable_if` is a common tool for this.

// Example: Let's create two function templates. One for integral types, one for floating-point types.
// SFINAE will ensure the correct one is called.

// This version is enabled only if T is a floating-point type.
// The second template parameter to `enable_if_t` is the resulting type if the condition is true.
template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> double_if_float(T value) {
    return value * 2.0;
}

// The function below (`double_if_int`) is another example. It's enabled only if T is an integral type.
// If you try to call `double_if_int(3.14)`, the substitution of T=double into `std::is_integral_v<T>`
// results in `false`. `std::enable_if<false, T>` has no `::type` member, causing a substitution failure.
// Due to SFINAE, this isn't an error; the compiler just looks for other overloads.
// To answer your question: No, this mechanism does not lead to implicit casting from a float to an int
// in the way a non-template function `void func(int);` would when called with `func(3.14)`.
//
// Here's why: Template argument deduction happens before argument conversion.
// 1. When you call `double_if_int(3.14)`, the compiler deduces the template type `T` as `double`.
// 2. It then tries to substitute `T=double` into the function's full signature.
// 3. This requires evaluating `std::enable_if_t<std::is_integral_v<double>, double>`.
// 4. `std::is_integral_v<double>` is `false`, which causes a substitution failure inside `std::enable_if_t`.
//
// Because of SFINAE, this is not a hard error. The compiler simply discards this `double_if_int`
// template from the list of potential candidates for this specific call. It never gets to the point of
// considering an implicit conversion for the argument. If no other overload matches (like `double_if_float`),
// you will get a "no matching function" compile error.

template<typename T>
std::enable_if_t<std::is_integral_v<T>, T> double_if_int(T value) {
    return value * 2;
}
// To answer your question: "Can the following lead to a compiler overflow?"
// Yes, absolutely. While SFINAE itself is a filtering mechanism, template metaprogramming
// in general can cause the compiler to hit its internal limits, which can be thought of
// as a "compiler overflow". The most common issue is exceeding the maximum template
// instantiation depth.

// Compilers use recursion to instantiate templates. If a template recursively instantiates
// itself too many times, it will exceed a built-in limit and the compilation will fail.

// Example: A recursive template to demonstrate hitting the depth limit.
template<int N>
struct RecursiveDepth {
    // Each instantiation of RecursiveDepth<N> requires the instantiation of RecursiveDepth<N-1>.
    static constexpr int value = RecursiveDepth<N - 1>::value + 1;
};

// The base case to stop the recursion.
template<>
struct RecursiveDepth<0> {
    static constexpr int value = 0;
};

void template_depth_example() {
    // This is a safe, shallow instantiation. It will compile without any issues.
    // Note: To run this, you'll need to add `template_depth_example();` to main().
    std::cout << "RecursiveDepth<10>::value = " << RecursiveDepth<10>::value << std::endl;

    // However, if you try to instantiate the template with a very large number,
    // you will trigger a fatal compiler error. The exact limit depends on the compiler
    // and its flags (e.g., g++ and clang use -ftemplate-depth=N). The default is
    // often around 900-1024.

    // UNCOMMENTING THE LINE BELOW WILL LIKELY CAUSE A COMPILATION ERROR like:
    // "fatal error: template instantiation depth exceeds maximum of 900"
    //
    // int deep_value = RecursiveDepth<2000>::value;
}
// --- Constexpr Recursion Depth ---
//
// You've asked a great question about controlling the depth of constexpr function calls,
// similar to how template instantiation depth is controlled. You are right to suspect
// there's a similar mechanism.
//
// When a function like `factorial` is called in a `constexpr` context (like initializing
// a `constexpr` variable), the compiler must evaluate it. If the function is recursive,
// the compiler will execute the recursive calls. This evaluation happens entirely within
// the compiler, before the program is linked or run.
//
// To prevent infinite recursion and excessive compile times, compilers impose a limit
// on the depth of `constexpr` function calls. This is very much like the template
// instantiation depth limit.
//
// For compilers like GCC and Clang, you can control this limit with the flag:
// `-fconstexpr-depth=N`
//
// The default value is typically around 512.

void constexpr_depth_example() {
    // Note: To run this, you'll need to add `constexpr_depth_example();` to main().
    // This call is shallow and will compile just fine.
    constexpr int small_factorial = factorial(10);
    std::cout << "constexpr factorial(10) = " << small_factorial << std::endl;

    // UNCOMMENTING THE LINE BELOW will likely cause a compilation error like:
    // "error: constexpr evaluation exceeded maximum depth of 512"
    //
    // constexpr int large_factorial = factorial(1000);
}

// --- Constexpr and Compile-Time Computation ---
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);
}

void constexpr_example() {
    constexpr int val = factorial(5);
    std::cout << "Constexpr factorial(5): " << val << std::endl;
}

// --- Main to run all examples ---
int main() {
    lambda_examples();
    move_semantics();
    Resource r1;
    Resource r2 = std::move(r1);
    structured_bindings();
    std::cout << "Sum: " << sum(1, 2, 3) << std::endl;
    thread_example();
    std::cout << "Double if int: " << double_if_int(10) << std::endl;
    constexpr_example();
    return 0;
} 