# Distributions

Distribution implementations ara located in the `math` library, and they are composed of two parts: the sampler and the loader.

The sampler is a header-only class that generates random numbers following the distribution

The loader is function to read the distribution parameters from the TOML configuration file and creates the sampler object.

# Adding a new distribution
If you want to add a new distribution to the simulator, you need to follow these 4 (high-level) steps:

1. Create a new class in the `mjqm-samplers` lib, that extends the `DistributionSampler` class and implements all required methods.
2. Add the new distribution to the `mjqm-math/samplers.h` imports.
3. Add the new loader declaration to `mjqm-settings/toml_distributions_loader.h`, including it in the `distribution_loaders` map at the end.
4. Add the new loader implementation to `mjqm-settings/toml_distributions_loader.cpp`, taking care of validating the parameters and creating the new distribution object.

Let's see an example of how to add a new distribution to the simulator. We'll take the exponential distribution as an example, even though it's already implemented in the simulator.

## `DistributionSampler` interface
[sampler.h](https://raw.githubusercontent.com/NeDS-Lab/mjqm-simulator/refs/heads/main/libs/math/include/mjqm-math/sampler.h ':include :type=code cpp :fragment=interface')

The interface expects the following methods to be implemented:
- a constructor that forwards the name of the distribution instance to the interface
- [`double sample()` cpp] that generates a random number following the distribution
- [`double getMean() const` cpp] that returns the theoretical mean of the distribution
- [`double getVariance() const` cpp] that returns the theoretical variance of the distribution
- [`explicit operator std::string() const` cpp] that returns the name of the distribution, along with its parameters and its theoretical mean and variance.
- [`std::unique_ptr<DistributionSampler> clone(const std::string_view& name) const` cpp] that builds a new instance of the distribution with the same parameters.

There are some good practices to follow when implementing a new distribution:
- The theoretical mean and variance should be defined as constants in the class, and computed once.
- The `operator std::string` should return a format like `distribution_name (param1=val.ue ; param2=val.ue => mean=getMean() ; variance=getVariance())`
- The constructor should expect the actual distribution parameters as arguments, instead of some value to compute them.
- If some parameter can be computed from the `mean`, `rate`, or other pseudo-parameters, it is recommended to implement a static method `with_{param}` accepting the pseudo-parameters, along with required parameters and the instance name, and returning a new instance of the distribution as `std::unique_ptr<DistributionSampler>`.

## Create a new class

As we are in a header-only library, we need to define the class implementation in the header file.
We can start by defining the class skeleton, which should extend the `DistributionSampler` class, and it should be surrounded by the classic include guards.
To avoid name clashes, it's a good practice to use the prefix `MJQM_SAMPLERS_` for the include guards.

```cpp
// libs/math/include/mjqm-samplers/exponential.hpp
#ifndef MJQM_SAMPLERS_EXPONENTIAL_H
#define MJQM_SAMPLERS_EXPONENTIAL_H

#include "RngStream.h"
#include "mjqm-math/sampler.h"

class Exponential : public DistributionSampler {
    // ...
};

#endif // MJQM_SAMPLERS_EXPONENTIAL_H
```
