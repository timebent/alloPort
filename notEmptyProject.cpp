#include "allocore/io/al_App.hpp"
#include "notEmptyProject.hpp"

using namespace al;
using namespace gam;

// implement spin cycle button

class MyApp : public App{
public:
    
    int appSampleRate = 48000;
    int appChannels = 2;
    int appBufferSize = 128;
    int width = 1600;
    int height = 1200;
    double durSliderValue;
    double ampSliderValue;
    al_sec currentTime = 0;
    // ViewpointWindow* vw;
    GLVBinding gui;
    
    glv::Slider durSlider;
    glv::Slider windowTimeSlider;
    glv::Slider delayTimeSlider;
    glv::Slider ampSlider;
    glv::Button quantizeButton;
    glv::Button mirrorButton;
    glv::Button spinCycleButton;
    glv::Button threeDButton;
    glv::Table layout;
    
    // double rotate = 0;
    
    MyApp() {
        
        projectionMode = false;
        quantizeFreq = 0;

        windowTime = 10000;
        seqStrategy = new SequenceStrategy(100, 100, appSampleRate);
        
        nav().pos(0,0,4);
        navControl().useMouse(false);
        
        ViewpointWindow* vw;
        vw = initWindow(Window::Dim(width, height));
        vw->title("donkeyKong");
        vw->fullScreen(true);
        
        gui.bindTo(window());
        gui.style().color.set(glv::Color(0.7), 0.5);
        layout.arrangement(">p");
        

        durSlider.interval(10000, 10);
        durSlider.setValue(500);
        layout << durSlider;
        layout << new glv::Label("duration");
        
        windowTimeSlider.interval(50000, 10000);
        windowTimeSlider.setValue(10000);
        layout << windowTimeSlider;
        layout << new glv::Label("windowTime");
        
        delayTimeSlider.interval(0.01, 0.75);
        delayTimeSlider.setValue(0.5);
        layout << delayTimeSlider;
        layout << new glv::Label("delayTime");
        
        ampSlider.interval(0.01, 0.1);
        ampSlider.setValue(0.05);
        layout << ampSlider;
        layout << new glv::Label("ampSlider");
        
        layout << quantizeButton;
        layout << new glv::Label("quantize");
        
        layout << mirrorButton;
        layout << new glv::Label("mirror");
        
        layout << spinCycleButton;
        layout << new glv::Label("spinCycle");
        
        layout << threeDButton;
        layout << new glv::Label("3D");
        

        layout.arrange();
        gui << layout;

        for(int i=0; i<NUMGRAINS; i++){
          Grain& g = grains[i];
          g.remain = 0;
          
          Cylinder& c = myCylinders[i];
          c.scale(1, 1, 1);
          c.translate(0.0, 0.0, 0.0);
          // c.color(RGB(1.0, 0.0, 0.0));
        }
        
        delayTime.lag(0.1);
        delayL.maxDelay(1.0);
        delayL.delay(0.25);
        
        delayR.maxDelay(1.0);
        delayR.delay(0.25);
        
        initAudio((double) appSampleRate, appBufferSize, appChannels, appChannels);
        Sync::master().spu(appSampleRate);
    }
    
    void onKeyDown(const ViewpointWindow& vw, const Keyboard& k) {
        if (k.key() == ' ') {
            std::cout << "Spacebar pressed" << std::endl;
          
            for(int i = 0; i<200; i++) {
                scheduleGrain(ofRandom(1000, 2000), ofRandom(200, 800), durSlider.getValue());
            }
        }
    }
    
    void onMouseDown(const ViewpointWindow& vw, const Mouse& m) {
        int grainIndex = mouseDrawAndSchedule(m.x(), m.y(), vw);
        myCylinders[grainIndex].currentX = map(m.x(), 0, vw.dimensions().w, vw.aspect() * 1, vw.aspect());
        myCylinders[grainIndex].currentY = map(m.y(), 0, vw.dimensions().h, 1, -1);
       // myCylinders[grainIndex].scale(0.1,0.1, 0.1);
      //  myCylinders[grainIndex].scale(ofRandom(0.1, 0.2), ofRandom(0.1, 0.2), ofRandom(0.1, 0.2));
    }
    
    void onMouseUp(const ViewpointWindow& vw, const Mouse& m) {
        // std::cout << "hello" << std::endl;
    }
    
    void onMouseDrag(const ViewpointWindow& vw, const Mouse& m) {
        int grainIndex = mouseDrawAndSchedule(m.x(), m.y(), vw);
        // std::cout << "hello Mouse Down" << std::endl;
        myCylinders[grainIndex].currentX = map(m.x(), 0, vw.dimensions().w, vw.aspect() * 1, vw.aspect());
        myCylinders[grainIndex].currentY = map(m.y(), 0, vw.dimensions().h, 1, -1);
        // myCylinders[grainIndex].scale(0.1,0.1, 0.1);
       // myCylinders[grainIndex].scale(ofRandom(0.1, 0.2), ofRandom(0.1, 0.2), ofRandom(0.1, 0.2));
        // std::cout << map(m.y(), 0, vw->dimensions().h, 1, -1) << std::endl;
    }
    
    void onMouseMove(const Mouse& m) {
       // std::cout << "hello Mouse Move" << std::endl;
    }
    
    // Audio callback
    void onSound(AudioIOData& io){
        myClock.update(myClock.dt());
        durSliderValue = durSlider.getValue();
        ampSliderValue = ampSlider.getValue();
        delayL.delay(delayTime());
        delayR.delay(delayTime());
        currentTime =  myClock() * 1000;
   
   
        for(int i = 0; i < appBufferSize; i++) {
            io.frame(i+1);
            io.out(0) = 0;
        }
        
        for (int i=0; i<NUMGRAINS; i++) {
            
            Grain& g = grains[i];
            
            if (g.remain > 0) { // duration of grain is greater than zero, continue.
                
                //            // is it playing in this block?
                int s_from = g.nextOnset; // nextOnset is how many samples until the grain starts
                // if nextOnset is less than the buffersize then it is playing this block.
                // if nextOnset is less than 0, then we are still playing the grain
                
                int s_to;
                // s_to should be g.remain + nextOnset (clipped at the maximum of buffersize)
                // the amount of samples of the grain played should be subtracted from g.remain
                
                if (s_from < appBufferSize) { // test if nextOnset is within the block.
                    // Things actually play at g.nextOnset
                    // if nextOnset is negative then what? playFrom 0
                    
                    // don't want negative starts:
                    s_from = max(s_from, 0);
                    
                    //                // get the end-point:
                    s_to = s_from + g.remain;
                
                    //                // don't want to go beyond the current block:
                    s_to = min(s_to, appBufferSize);
                   // std::cout << s_to << std::endl;
                    
                    for (int j=s_from; j<s_to; j++) {
                      
                        float envVal = g.env();
                        // g.lpf.freq((g.window(g.dur, g.windowPos) * 10000) + 1000);
                        g.lpf.freq(envVal * 1000 + 500);
                        g.currentSample = g.lpf(g.osc()) * 0.1;
                        // g.currentSample = g.osc() * 0.1;
                        g.currentSample = envVal * g.currentSample;
                        // g.currentSample *= g.window(g.dur, g.windowPos) * g.amp;
                        // g.windowPos++;
                       
                        float2 currentFrame = g.panner(g.currentSample);
                        io.frame(j+1);
                        io.sum(currentFrame[0], 0);
                        io.sum(currentFrame[1], 1);
                        
                        // g.pan(g.currentSample, *io.out(0), *io.out(1));
                    }
                    //                // update the remain counter:
                    //                // this will automatically mark the grain as
                    //                // non-busy if the grain is done.
                    g.remain -= (s_to - s_from);
                    // g.env.trigger=0;
                }
                g.nextOnset -= appBufferSize;
            }
        }
        
        for( int i = 0; i<appBufferSize; i++ ) {
            io.frame(i + 1); // set to the beginning frame
            delayTime(delayTimeSlider.getValue());

            
            float delayedL = delayL(); // read delayed sample from the delay line
            float delayedR = delayR();
            float srcL = io.out(0); // return the current sample
            float srcR = io.out(1);
            delayedL = delayedL * 0.8; // scale the delayed sample
            delayedR = delayedR * 0.8; // scale the delayed sample
            delayL(delayedR + (srcL + srcR) ); // delay the source and feedback a bit of delayed signal
            delayR(delayedL + (srcR + srcL) );
            
            // io.sum(delayed + src, 0, 1);
            io.out(0) = (srcL + delayedL) * ampSliderValue; // output src plus delayed
            io.out(1) = (srcR + delayedR) * ampSliderValue;
            
        }
        
    }
    
    void onAnimate(double dt){
        // how to get width and height of the window?
        
        // rotate += 0.01;
        if(threeDButton.getValue()) {
            projectionMode = 0;
        } else {
            projectionMode = 1;
        }
    }
    
    void onDraw(Graphics& g, const Viewpoint& v){
        
        float aspect = v.viewport().aspect();
        g.nicest();
        
        if(projectionMode) {
            g.pushMatrix(Graphics::PROJECTION);
            g.loadMatrix(Matrix4f::ortho2D(-aspect, aspect, -1, 1)); // using the aspect ratio as the x
        }
        
        for(int i = 0; i<NUMGRAINS; i++) { // can this be optimized?
            
            Cylinder& c = myCylinders[i];
            Grain& grain = grains[i];
            
            // std::cout << c.onset() << " " << c.startTime() << " " << c.duration() << " " << currentTime << std::endl;
            if(grains[i].remain > 0) {
                
                double newPos = map(currentTime, c.startTime(), c.startTime() + c.onset(), aspect * -1,  0);
                c.currentX = newPos;
                
                if(projectionMode) {
                    
                    g.pushMatrix(Graphics::MODELVIEW);
                        g.loadIdentity();
                    
                    if(spinCycleButton.getValue()) {
                        g.rotate(c.rotate() + 100, 0, 1, 0);
                        g.translate(c.currentX, c.currentY, 0);
                    } else {
                        g.translate(c.currentX, c.currentY, 0);
                        g.rotate(c.rotate(), 0, 1, 0);
                    }
                        if(c.onset() + c.startTime() < currentTime) {
                                g.color(1.0, 1.0, 1.0, 1.0);
                                g.scale(0.05, 0.05, 0.05);
                        } else {
                        g.color(0.1, 0.1, 0.9, 1.0);
                        g.scale(0.01, 0.01, 0.01);
                        }

                        c.draw(g);

                    g.popMatrix();
                    
                    if (mirrorButton.getValue()) {
                    g.pushMatrix(Graphics::MODELVIEW);
                        g.loadIdentity();
                        
                        if(spinCycleButton.getValue()) {
                            g.rotate(c.rotate() + 100, 0, 1, 0);
                            g.translate(c.currentX * -1, c.currentY, 0);
                        } else {
                            g.translate(c.currentX * -1, c.currentY, 0);
                            g.rotate(c.rotate(), 0, 1, 0);
                        }
                        
                        if(c.onset() + c.startTime() < currentTime) {
                            g.color(1.0, 1.0, 1.0, 1.0);
                            g.scale(0.05, 0.05, 0.05);
                        } else {
                            g.color(0.1, 0.1, 0.9, 1.0);
                            g.scale(0.01, 0.01, 0.01);
                        }
                        c.draw(g);
                 
                    g.popMatrix();
                    }
                    
                } else {
                
                    g.pushMatrix();
                    
                    if(spinCycleButton.getValue()) {
                        g.rotate(c.rotate() + 100, 0, 1, 0);
                        g.translate(c.currentX, c.currentY, 0);
                    } else {
                        g.translate(c.currentX, c.currentY, 0);
                        g.rotate(c.rotate(), 0, 1, 0);
                    };
                    
                        if(c.onset() + c.startTime() < currentTime) {
                            g.color(1.0, 1.0, 1.0, 1.0);
                            g.scale(0.05, 0.05, 0.05);
                        } else {
                            g.color(0.1, 0.1, 0.9, 1.0);
                            g.scale(0.01, 0.01, 0.01);
                        }
                    
                        c.draw(g);
                
                    g.popMatrix();
                    
                    if (mirrorButton.getValue()) {
                    g.pushMatrix();
                        
                        if(spinCycleButton.getValue()) {
                            g.rotate(c.rotate() + 100, 0, 1, 0);
                            g.translate(c.currentX * -1, c.currentY, 0);
                        } else {
                            g.translate(c.currentX * -1, c.currentY, 0);
                            g.rotate(c.rotate(), 0, 1, 0);
                        }
                        
                        if(c.onset() + c.startTime() < currentTime) {
                            g.color(1.0, 1.0, 1.0, 1.0);
                            g.scale(0.05, 0.05, 0.05);
                        } else {
                            g.color(0.1, 0.1, 0.9, 1.0);
                            g.scale(0.01, 0.01, 0.01);
                        }
                        
                        c.draw(g);
              
                    g.popMatrix();
                    }
                }
            }
        }
        if(projectionMode) {
            g.popMatrix(Graphics::PROJECTION);
        }
    }

    int scheduleGrain(double onset, double freq, double dur){
        
        int i = 0;
        int j = 0;
        while (i < grains.size()) {
            // is this grain busy? If not, break on out and use it.
            if (grains[i].remain <= 0) {
                break; } else {
                    i++;
                }
        }
        
        if (i == grains.size()) {
            // error, all grains busy
            return i;
        } else {
            // here is where we use the grain
            Grain& g = grains[i];
            Cylinder& c = myCylinders[i];
            
            // are these in samples or ms?
            seqStrategy->nextDur = dur;
            seqStrategy->nextOnset = onset;
            
            // al_sec startTime = myClock.now() * 1000;
            al_sec startTime = currentTime;
            c.onset(onset);
            c.duration(dur);
            
            // startTime = startTime / std::chrono::milliseconds(1);
            c.startTime(startTime);
            // c.color(RGB(0.1, 0.5, 0.1));
            
            g.setAll(freq, 0.01, ofRandom(-1, 1), seqStrategy->getDurInSamples(), seqStrategy->getOnsetInSamples(), appSampleRate);
            // sets dur, remain, nextOnset, and start
            
            // gives you the index of the grain ... should you need it.
            return i;
        }
        
        
    }

    void setWindowTime(int &time) {
        windowTime = time;
    }

    int mouseDrawAndSchedule(int x, int y, const ViewpointWindow& vw) {
            
            double onset;
            double freq;
            double cutOff;
            double resonance;
            int indexOfGrain;
            double dur;
            int durMappedToCyl;
            double accumOnset;
            double sphereY;
            windowTime = windowTimeSlider.getValue();
            quantizeFreq = quantizeButton.getValue();
        
            onset = map(x, 0, width, windowTime/1000, windowTime); // onset in ms.
        // onset = 0;
            accumOnset = 0;
        
            freq = map(y, 0, vw.dimensions().h, 0.0, 1.0); // first map the mouse value to between 0 and 1
            freq = map(freq*freq, 0, 1, 1000, 10); // then map the mouse value to between 1000 and 100;
            if(quantizeFreq) { freq = roundToHundredths(freq); }
            freq = min(freq, (double) 1000);
            freq = max(freq, (double) 10);
        
            dur = durSliderValue;
            // dur = map(dur, sliderValue)
            // dur = 500;
            dur = min(dur, windowTime);
      
            // dur = durSlider;
        
            //  for(int i = 0; i < (onsetSlider); i++) {
            indexOfGrain = scheduleGrain(onset, freq, dur); // Schedule a grain
            return indexOfGrain;
        //    }
    }
    
};
  int main(){
      MyApp().start();
  }
