
#include "audioTools.hpp"
#include "visualTools.hpp"
#include <array>
#include "allocore/io/al_App.hpp"
#include "GLV/glv.h"
#include "alloGLV/al_ControlGLV.hpp"
#include "allocore/ui/al_Parameter.hpp"


// using namespace gam;
using namespace al;

struct SequenceStrategy {
    
    double nextOnset;
    double nextDur;
    int sampleRate;
    int currentOnset;
    bool go;
    rnd::Random<> randomNum;
    
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
        nextOnset = randomNum.uniform(lo, hi + 1);
    }
    
    void randomDur(double lo, double hi) {
        nextDur =  randomNum.uniform(lo, hi + 1);
    }
    
    void onsetDecrement() {
        currentOnset--;
    }
};

// global clock ... not sure why it has to be global
Clock myClock(true);
Parameter durSliderParam("dur", "", 1);
Parameter ampSliderParam("amp", "", 0.01);
Parameter delayTimeSliderParam("delaytime", "", 0.1);
Parameter feedbackSliderParam("feedback", "", 0.6);

// function prototypes
int scheduleGrain(double onset, double freq, double dur);
int mouseDrawAndSchedule(int x, int y);
void setWindowTime(int &time);







