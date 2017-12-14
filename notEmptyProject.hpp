
#include "audioTools.hpp"
#include "visualTools.hpp"
#include <array>
#include "allocore/math/al_Functions.hpp"
#include "allocore/io/al_App.hpp"
#include "GLV/glv.h"
#include "alloGLV/al_ControlGLV.hpp"



using namespace gam;
using namespace al;

struct SequenceStrategy {
    
    double nextOnset;
    double nextDur;
    int sampleRate;
    int currentOnset;
    bool go;
    
    SequenceStrategy(double _nextOnset, double _nextDur, int _sampleRate) {
        nextOnset = _nextOnset;
        nextDur = _nextDur;
        sampleRate = _sampleRate;
    }
    
    int getOnsetInSamples() {
        return nextOnset * (sampleRate / 1000);
    }
    
    int getDurInSamples() {
        return nextDur * (sampleRate / 1000);
    }
    
    void randomOnset(double lo, double hi) {
        nextOnset = ofRandom(lo, hi + 1);
    }
    
    void randomDur(double lo, double hi) {
        nextDur =  ofRandom(lo, hi + 1);
    }
    
    void onsetDecrement() {
        currentOnset--;
    }
};

// global clock ... not sure why it has to be global
Clock myClock(true);

// function prototypes
int scheduleGrain(double onset, double freq, double dur);
int mouseDrawAndSchedule(int x, int y);
void setWindowTime(int &time);


// Some functions ... should consider replacing this with something already in allocore
double roundToHundredths(double x){
    x /=100;
    return floor(x + 0.5) * 100;
}

float roundToTenths(float x){
    x /=10;
    return floor(x + 0.5) * 10;
}

float map(float s, float a1, float a2, float b1, float b2)
{
    return b1 + (s-a1)*(b2-b1)/(a2-a1);
}




