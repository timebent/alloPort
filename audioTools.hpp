//
//  audioTools.hpp
//  
//
//  Created by John Thompson on 11/15/17.
//
//

#ifndef audioTools_h
#define audioTools_h

#include "Gamma/Oscillator.h"
#include "Gamma/Filter.h"
#include "Gamma/Envelope.h"
#include "Gamma/Delay.h"
#include "Gamma/Effects.h"
#include <math.h>
#include <array>
#include <stdio.h>

using namespace gam;

double ofRandom(double x, double y);

struct hannWinFunctor {
    inline double operator()(long windowLength, long windowPos) {
        return 0.5 * (1.0 - cos((6.28318530717958647692 * windowPos) / (windowLength - 1)));
    }
};

class Grain {
    
public:
    double freq;
    double amp;
    double dur;
    double pan;
    double currentSample;
    unsigned int windowPos;
    int nextOnset;
    double channels[2];
    double start;
    int remain; // in samples
    // double cutOff;
    double resonance;
    double currentVolume;
    
    Saw<> osc;
    Biquad<> lpf;
    LFO<> lfo;
    AD<> env;
    // OnePole<> cutOffSmoother;
    hannWinFunctor window;
    Pan<> panner;
    
    Grain() {
        freq = 440;
        amp = 0.1;
        pan = 0;
        dur = 0;
        windowPos = 0;
        remain = 0;
        nextOnset = 48000;
        start = nextOnset;
        lpf.type(LOW_PASS);		// Set filter to low-pass response
        lpf.res(4);				// Set resonance amount to emphasize filter
        lpf.freq(1000);
        osc.freq(440);
        env.attack(dur * 0.5);		// Set short (10 ms) attack
        env.decay(dur * 0.5);			// Set longer (400 ms) decay
    }
    

    Grain(double _freq, double _amp, double _pan, double _dur, int _nextOnset, int sampleRate) {
        freq = _freq;
        amp = _amp;
        pan = _pan;
        dur = _dur; // in samples
        windowPos = 0;
        // busy = false;
        remain = (_dur);
        nextOnset = _nextOnset;
        start = _nextOnset;
        lpf.type(LOW_PASS);		// Set filter to low-pass response
        lpf.res(4);				// Set resonance amount to emphasize filter
        lpf.freq(1000);
        osc.freq(_freq);
        env.attack((dur/(double) sampleRate) * 0.5);
        env.decay(((dur/(double) sampleRate) * 0.5));
    }
    
    void setAll(double _freq, double _amp, double _pan, double _dur, int _nextOnset, int sampleRate) {

        freq = _freq;
        amp = _amp;
        pan = _pan;
        dur = _dur; // in samples
        windowPos = 0;
        remain = (_dur); // in samples
        nextOnset = _nextOnset; // in samples
        start = _nextOnset;
        lpf.type(LOW_PASS);		// Set filter to low-pass response
        lpf.res(16);				// Set resonance amount to emphasize filter
        lpf.freq(1000);
        osc.freq(_freq);
        env.reset();
        env.attack((dur/(double) sampleRate) * 0.5);		// Set short (10 ms) attack
        env.decay((dur/(double) sampleRate) * 0.5);			// Set longer (400 ms) decay

        panner.pos(pan);
    }
    
    void setFreq(double _freq) {
        freq = _freq;
        osc.freq(_freq);
    }
    
};

#endif /* audioTools_h */
