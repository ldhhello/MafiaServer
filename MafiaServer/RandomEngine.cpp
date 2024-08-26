#include "RandomEngine.h"

RandomEngine::RandomEngine()
{
    random_device rd;
    mt.seed(rd());
}

RandomEngine::result_type RandomEngine::operator() ()
{
    lock_guard<mutex> guard(m);
    
    return mt();
}

RandomEngine::result_type RandomEngine::operator() (int min_, int max_)
{
    lock_guard<mutex> guard(m);
    
    uniform_int_distribution<int> dis(min_, max_);
    
    return dis(mt);
}

RandomEngine random_engine;
