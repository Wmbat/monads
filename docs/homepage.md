# What is libreglisse?

`libreglisse` is a library of monadic types for C++. It currently contains the `maybe`, `either` and `result` monads.

# Monads

## maybe

A `maybe` monad is a type that may or may not hold a value. By default, it is assumed that the `maybe` is empty unless
specified otherwise.

### Examples

#### Construction

To construct a `maybe` monad, the utility classes `some` and `none` is provided. Allowing for the simple, expressive construction of
a `maybe`

```cpp
maybe<int> my_int = some(1);    // constructs a maybe holding an int with value 1
maybe<int> my_empty_int = none; // constructs an empty maybe with no value
```
You can also check whether a `maybe` contains a value or not using `operator bool()`, `is_some()` and `is_none()`
```cpp
maybe my_ptr = some(std::make_unique<int>(10)); // constructs a maybe holding an int with value 1
if (my_ptr.is_some()) {}    // Checks if the maybe holds a value. true here
if (my_ptr.is_none()) {}    // Checks if the maybe does not hold a value. false here
if (my_ptr) {}              // Same behaviour as `is_some()`. true here
```

#### Access

The `maybe` monad also provides a set of function to access the value stored within,
they are the following: `borrow()`, `take()` and `take_or()`.

To access the value store in an **lvalue** `maybe`, the function `borrow()` is provided. As the name implies, it's main
goal is to borrow the value stored by the monad.
```cpp
maybe my_int = some(1);

int& i = my_int.borrow();  
```
To actually take the value from the monad, it must be an **rvalue** and the function `take()` must be
called instead.
```cpp
maybe my_int = some(1);

int i = std::move(my_int).take();
```
Similarly to `take()`, `take_or()` must only be used on **rvalues**, but offers a fallback parameter in case the monad
holds no value. `borrow()` and `take()` simply use an `assert()` to check for the presence of a value in debug mode.
```cpp
maybe<std::vector<float>> my_vec = none;

std::vector<int> vec = std::move(my_vec).take_or(std::vector({0.0f, 0.0f 0.0f}));
```

#### Transformation

You can also directly modify the value held by the monad if it is present using the `transform()` and `transform_or()`.

```cpp
// maybe holding a string with value "1"
maybe<std::string> maybe_str = maybe<int>(some(1)).transform([](int i){
    return std::to_string(i);
});

// empty maybe
maybe<std::string> empty = maybe<int>(none).transform([](int i){
    return std::to_string(i);
});
```

```cpp
// maybe holding a string of value "1"
std::string maybe_str = maybe<int>(some(1)).transform_or(
    [](int i){ return std::to_string(i); }, 
    std::string("empty"));
        
// maybe holding a string of value "empty"
std::string empty = maybe<int>(none).transform_or(
    [](int i){ return std::to_string(i); }, 
    std::string("empty"));
```

## result

## either
