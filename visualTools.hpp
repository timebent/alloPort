//
//  visualTools.hpp
//  
//
//  Created by John Thompson on 11/29/17.
//
//

#ifndef visualTools_hpp
#define visualTools_hpp

#include <stdio.h>
#include <math.h>
// #include "allocore/io/al_App.hpp"
// #include "allocore/system/al_Time.hpp"

// using namespace al;


class Shape {
public:
    
    Shape() {
        currentX = 0;
    }
    
    double startTime() {
        return mStartTime;
    }
    
    void startTime(double startTime) {
         mStartTime = startTime;
    }

    double onset() {
        return mOnset;
    }
    
    void onset(double onset) {
         mOnset = onset;
    }
    
    double duration() {
        return mDuration;
    }
    
    void duration(double duration) {
         mDuration = duration;
    }
    
    double rotateY() {
        mRotate = fmod(mRotate + 1, 360);
        return mRotate;
    }
    
    double rotateX() {
        mRotate = fmod(mRotate + (3.1428/3), 360);
        return mRotate;
    }
    
    float currentX=0, currentY=0, currentZ=0;
    
private:
     // Mesh m;
    double mStartTime=0, mOnset=0, mDuration;
    double mRotate=0;
    
};

#endif /* visualTools_hpp */
