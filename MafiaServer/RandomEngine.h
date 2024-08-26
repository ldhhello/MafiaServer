#pragma once
#include "Include.h"

// RandomEngine: 전체 시스템에서 사용할수 있는 랜덤 엔진
// Thread-safe 하게 작동한다!
class RandomEngine
{
private:
    mt19937 mt;
    mutex m;
    
public:
    RandomEngine();
    
    using result_type = unsigned int;
    
    constexpr result_type static min() { return mt19937::min(); }
    constexpr result_type static max() { return mt19937::max(); }
    
    result_type operator() ();
    
    result_type operator() (int min_, int max_);
};

extern RandomEngine random_engine;
