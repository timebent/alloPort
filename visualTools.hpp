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
#include "allocore/io/al_App.hpp"
#include "allocore/system/al_Time.hpp"

using namespace al;

class Sphere : public Mesh {
public:
    
    Sphere() {
        addSphere(m);
        m.primitive(Graphics::TRIANGLES);
    }
    
//   void translate(float x, float y, float z) {
//        m.translate(x, y, z);
//    }
    
    void draw(Graphics& g) {
        g.draw(m);
    }
    
//    void ribbonize(float width=0.04, bool faceBinormal=false) {
//        m.ribbonize(0.04, false);
//    }
//    
//    
//    void scale(float x, float y, float z) {
//        m.scale(x, y, z);
//    }
    Mesh m;
};


class Cylinder {
public:
    
    Cylinder() {
        addCylinder(m);
        m.primitive(Graphics::QUAD_STRIP);
       // m.rotate(90, 1, 0, 0);
       // m.color(RGB(0.1, 0.5, 0.1));
        currentX = 0;
    }
    
    void translate(float x, float y, float z) {
        m.translate(x, y, z);
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
    
    void color(RGB rgb) {
        // m.color(rgb);
    }
    
    void scale(float x, float y, float z) {
        m.scale(x, y, z);
    }
    
    void draw(Graphics& g) {
        g.draw(m);
    }
    
    double rotate() {
        mRotate = fmod(mRotate + 1, 360);
        return mRotate;
    }
    
    void reset() {
     //   m.reset();
    }
    
//    float angle() {
//        return ++myAngle%360;
//    }


    Mesh getMesh() {return m;};
   
    float currentX=0, currentY=0, currentZ=0;
    
private:
     Mesh m;
    double mStartTime=0, mOnset=0, mDuration;
    double mRotate=0;
    
};

#endif /* visualTools_hpp */
