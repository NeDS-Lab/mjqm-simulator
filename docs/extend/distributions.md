# Distributions

Distribution implementations ara located in the `math` library, and they are composed of two parts: the sampler and the loader.

The sampler is a header-only class that generates random numbers following the distribution.

The loader is a function to read the distribution parameters from the TOML configuration file and creates the sampler object.

# Adding a new distribution

If you want to add a new distribution to the simulator, you need to follow these 4 (high-level) steps:

1. Create a new class in the `mjqm-samplers` lib, that extends the `DistributionSampler` class and implements all required methods.
2. Add the new distribution to the `mjqm-math/samplers.h` imports.
3. Add the new loader declaration to `mjqm-settings/toml_distributions_loader.h`, including it in the `distribution_loaders` map at the end.
4. Add the new loader implementation to `mjqm-settings/toml_distributions_loader.cpp`, taking care of validating the parameters and creating the new distribution object.

Let's see an example of how to add a new distribution to the simulator. We'll take the exponential distribution as an example, even though it's already implemented in the simulator.

## `DistributionSampler` interface

[sampler.h](https://raw.githubusercontent.com/NeDS-Lab/mjqm-simulator/refs/heads/main/libs/math/include/mjqm-math/sampler.h ":include :type=code cpp :fragment=interface")

The interface expects the following methods to be implemented:

- a constructor that forwards the name of the distribution instance to the interface
  (the name is generated based on the job class during the loading from the TOML file)
- `double sample()` that generates a random number following the distribution
- `double getMean() const` that returns the theoretical mean of the distribution
- `double getVariance() const` that returns the theoretical variance of the distribution
- `explicit operator std::string() const` that returns the name of the distribution, along with its parameters and its theoretical mean and variance.
- `std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const` that builds a new instance of the distribution with the same parameters.

To do so, it offers the following protected methods:

- `double randU01()` that generates a random number following the uniform distribution between 0 and 1.
- two constructors, accepting either a `std::string` or a `std::string_view`. For generic purposes we commonly use `const std::string_view& name`, as the RngStreams library will copy its content into a new `std::string`.

### Good practices

To achieve a more cohesive library, there are some good practices to follow when implementing a new distribution:

- The first parameter of the constructor should be the name of the instance, followed by the distribution-specific parameters.
  - The constructor should expect the actual distribution parameters as arguments, instead of some value(s) to compute them.
- If some parameter can be computed from the `mean`, `rate`, or other pseudo-parameters, it is recommended to implement a static method `with_{param}` accepting the pseudo-parameters, along with required parameters and the instance name, and returning a new instance of the distribution as `std::unique_ptr<DistributionSampler>`.
- The theoretical mean and variance should be defined as constants in the class, and computed once.
- The `operator std::string` should return a format like `distribution_name (param1=val.ue ; param2=val.ue => mean=getMean() ; variance=getVariance())`

## Create a new class

As we are in a header-only library (`.hpp` extension), we need to define the class implementation in the header file.
This also allows the compiler to inline the methods and optimize the code.
The class should be surrounded by the usual include guards.

#### Class skeleton

We can start by defining the class skeleton extending the `DistributionSampler` class.
To avoid name clashes, it's a good practice to use the prefix `MJQM_SAMPLERS_` for the include guards.

```cpp
// libs/math/include/mjqm-samplers/exponential.hpp
#ifndef MJQM_SAMPLERS_EXPONENTIAL_H
#define MJQM_SAMPLERS_EXPONENTIAL_H

#include <mjqm-math/sampler.h>

class Exponential : public DistributionSampler {
    // ...
};

#endif // MJQM_SAMPLERS_EXPONENTIAL_H
```

#### Fields

We first define the (usually constant) fields we'll hold in the class, adding the theoretical mean and variance.
In the case of the exponential distribution, we only need to store the $\lambda$ parameter, `lambda`.

The mean and variance formulas should be directly computed in their declarations. For readeability, we can use the `pow` function from the `cmath` library to compute the variance.

$$
\mu = \frac{1}{\lambda} \quad \text{and} \quad \sigma^2 = \frac{1}{\lambda^2}
$$

```cpp
// libs/math/include/mjqm-samplers/exponential.hpp
// ...
#include <cmath>
// ...
class Exponential : public DistributionSampler {
public: // descriptive parameters and statistics
    const double lambda;
    const double mean = 1. / lambda;
    const double variance = 1. / pow(lambda, 2);
    // ...
};
```

#### Operative methods

Out of the methods defined in the `DistributionSampler` interface, the `sample` method is the main one that should be implemented by our class, while the mean and variance getters are only required to _force_ us to compute them.

All of them _can_ and _should_ be inlined, while only the `sample` method cannot be labeled `const`, as the RNG held by the super-class will change its internal state.

For sampling, we want to use the `randU01()` method from the super-class, so we will use the formula

$$
X \sim \text{Exp}(\lambda) \quad \text{if} \quad X = -\log(U) / \lambda \quad \text{where} \quad U \sim \text{U}(0, 1)
$$

```cpp
// libs/math/include/mjqm-samplers/exponential.hpp
// ...
#include <cmath>
// ...
class Exponential : public DistributionSampler {
public: // operative methods
    inline double getMean() const override { return mean; }
    inline double getVariance() const override { return variance; }
    inline double sample() override { return -log(randU01()) / lambda; }
    // ...
};
```

#### Constructors

Now, we can define the constructor. As the exponential distribution is defined by the single parameter $\lambda$, we should define the constructor to only receive this parameter (along with the name).

As different costructor variants, we can define two idiomatic static methods: `with_rate` and `with_mean`, where the second one computes $\lambda = 1 / \mu$.

Also, here we define the `clone` method required by the interface, which should return a new instance of the distribution with the same parameters.

```cpp
// libs/math/include/mjqm-samplers/exponential.hpp
// ...
#include <memory>
#include <string_view>
// ...
class Exponential : public DistributionSampler {
public: // direct and indirect constructors
    explicit Exponential(const std::string_view& name, double lambda) :
    DistributionSampler(name), lambda(lambda) {}

    static std::unique_ptr<DistributionSampler> with_rate(const std::string_view& name, const double rate) {
        return std::make_unique<Exponential>(name, rate);
    }
    static std::unique_ptr<DistributionSampler> with_mean(const std::string_view& name, const double mean) {
        return std::make_unique<Exponential>(name, 1. / mean);
    }

    std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const override {
        return std::make_unique<Exponential>(name, lambda);
    }
    // ...
};
```

#### `operator std::string`

Finally, we define the `operator std::string` method, which should return a string with the distribution name, its parameters, and the theoretical mean and variance.

```cpp
// libs/math/include/mjqm-samplers/exponential.hpp
// ...
#include <sstream>
#include <string>
// ...
class Exponential : public DistributionSampler {
public: // string conversion
    explicit operator std::string() const override {
        std::ostringstream oss;
        oss << "Exponential (lambda=" << lambda << " => mean=" << mean << " ; variance=" << variance << ")";
        return oss.str();
    }
};
```
