//
// Created by mccio on 24/01/25.
//

#ifndef SAMPLER_H
#define SAMPLER_H

class sampler {
public:
    typedef double result_type; // for mirroring how std usually does it
    virtual double sample() = 0;
    virtual ~sampler() = default;

    explicit virtual operator std::string() const = 0;
};

#endif // SAMPLER_H
