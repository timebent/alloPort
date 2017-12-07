//
//  audioTools.cpp
//  
//
//  Created by John Thompson on 11/15/17.
//
//

#include "audioTools.hpp"


double ofRandom(double x, double y) {
    float high = std::max(x, y);
    float low = std::min(x, y);
    return std::max(low, (low + ((high - low) * rand() / float(RAND_MAX))) * (1.0f - std::numeric_limits<float>::epsilon()));
}

//


