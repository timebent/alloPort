#include "allocore/io/al_App.hpp"
#include "notEmptyProject.hpp"
#include "allocore/system/al_Time.hpp"


// create Parameter elements for all the gui values to redirect and store.
// add number boxes to slider elements

using namespace al;
// using namespace gam;

// implement spin cycle button

class MyApp : public App{
public:
    
    // should be defines for these elements ... I think it is in the stereo panning example
    int appSampleRate = 48000;
    int appChannels = 2;
    int appBufferSize = 128;
    
    int width = 1600;
    int height = 1200;
    
    // audio elements
    static unsigned const NUMGRAINS = 192;
    std::array<Grain, NUMGRAINS> grains;
    gam::Delay<> delayL;
    gam::Delay<> delayR;
    gam::OnePole<> delayTime;
    bool quantizeFreq;
    
    // visual elements
   
    Light light;
    Material material;
    std::array<Shape, NUMGRAINS> myCylinders;
    bool projectionMode;
    Mesh mesh;
    Texture texBlur;
    
    // timing elements
    double windowTime;
    SequenceStrategy* seqStrategy;
    al_sec currentTime = 0;
    
    // GUI elements of the app.
    GLVBinding gui;
    glv::Slider durSlider;
    glv::Slider windowTimeSlider;
    glv::Slider delayTimeSlider;
    glv::Slider ampSlider;
    glv::Slider feedbackSlider;
    glv::Button quantizeButton;
    glv::Button mirrorButton;
    glv::Button spinCycleButton;
    glv::Button threeDButton;
    glv::Table layout;
    
    
    // Other elements
    al::rnd::Random<> randomNum;

    // Constructor
    MyApp() {
        
        // set visual elements
        ViewpointWindow* vw;
        vw = initWindow(Window::Dim(width, height));
        vw->title("donkeyKong");
        vw->fullScreen(true);
        background(RGB(0.0, 0.0, 0.0));
        projectionMode = false;
        nav().pos(0,0,4);
        navControl().useMouse(false);
        int numVertices = addIcosahedron(mesh);
        
        for(int i=0; i<numVertices; i++) {
            float f = (float)i/numVertices;
            mesh.color(HSV(f*0.1+0.2,1,1));
        }
        
        mesh.decompress();
        mesh.generateNormals();
        
        // set GUI elements
        
        gui.bindTo(window());
        gui.style().color.set(glv::Color(0.7), 0.5);
        layout.arrangement(">p");
        // ---------------------
        durSlider.interval(10000, 10);
        durSlider.setValue(500);
        layout << durSlider;
        layout << new glv::Label("duration");
        // ---------------------
        windowTimeSlider.interval(50000, 10000);
        windowTimeSlider.setValue(10000);
        layout << windowTimeSlider;
        layout << new glv::Label("windowTime");
        // ---------------------
        delayTimeSlider.interval(0.01, 0.75);
        delayTimeSlider.setValue(0.5);
        layout << delayTimeSlider;
        layout << new glv::Label("delayTime");
        // ---------------------
        feedbackSlider.interval(0.6, 0.99);
        feedbackSlider.setValue(0.8);
        layout << feedbackSlider;
        layout << new glv::Label("feedbackSlider");
        // ---------------------
        ampSlider.interval(0.01, 0.1);
        ampSlider.setValue(0.05);
        layout << ampSlider;
        layout << new glv::Label("ampSlider");
        // ---------------------
        layout << quantizeButton;
        layout << new glv::Label("quantize");
        // ---------------------
        layout << mirrorButton;
        layout << new glv::Label("mirror");
        // ---------------------
        layout << spinCycleButton;
        layout << new glv::Label("spinCycle");
        // ---------------------
        layout << threeDButton;
        layout << new glv::Label("3D");
        // ---------------------
        layout.arrange();
        gui << layout;
        // ---------------------
    
        // set timing elements
       
        windowTime = 10000;
        seqStrategy = new SequenceStrategy(100, 100, appSampleRate);
     
        // create and set the pool of grains and associated shapes
        for(int i=0; i<NUMGRAINS; i++){
          Grain& g = grains[i];
          g.remain = 0;
          Shape& c = myCylinders[i];
        }
        
        // set audio elements
        quantizeFreq = 0;
        delayTime.lag(0.5);
        delayL.maxDelay(1.0);
        delayL.delay(0.25);
        delayR.maxDelay(1.0);
        delayR.delay(0.25);
        
        initAudio((double) appSampleRate, appBufferSize, appChannels, appChannels);
        gam::Sync::master().spu(appSampleRate);
    }
    
    void onKeyDown(const ViewpointWindow& vw, const Keyboard& k) {
        if (k.key() == ' ') {
            std::cout << "Spacebar pressed" << std::endl;
          
            for(int i = 0; i<128; i++) {
                scheduleGrain(randomNum.uniform(1000, 2000), randomNum.uniform(200, 800), durSlider.getValue());
            }
        }
    }
    
    void onMouseDown(const ViewpointWindow& vw, const Mouse& m) {
        int grainIndex = mouseDrawAndSchedule(m.x(), m.y(), vw);
        myCylinders[grainIndex].currentX = mapRange<double>(m.x(), 0, vw.dimensions().w, vw.aspect() * 1, vw.aspect());
        myCylinders[grainIndex].currentY = mapRange<double>(m.y(), 0, vw.dimensions().h, 1, -1);
    }
    
    void onMouseUp(const ViewpointWindow& vw, const Mouse& m) {
        // std::cout << "hello" << std::endl;
    }
    
    void onMouseDrag(const ViewpointWindow& vw, const Mouse& m) {
        int grainIndex = mouseDrawAndSchedule(m.x(), m.y(), vw);
        myCylinders[grainIndex].currentX = mapRange<double>(m.x(), 0, vw.dimensions().w, vw.aspect() * 1, vw.aspect());
        myCylinders[grainIndex].currentY = mapRange<double>(m.y(), 0, vw.dimensions().h, 1, -1);
    }
    
    void onMouseMove(const Mouse& m) {
       // std::cout << "hello Mouse Move" << std::endl;
    }
    
    // Audio callback
    void onSound(AudioIOData& io){
        myClock.update(myClock.dt());
        durSliderParam = durSlider.getValue();
        ampSliderParam = ampSlider.getValue();
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
                       
                        gam::float2 currentFrame = g.panner(g.currentSample);
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
            delayTimeSliderParam = delayTimeSlider.getValue();
            delayTime(delayTimeSliderParam.get());

            
            float delayedL = delayL(); // read delayed sample from the delay line
            float delayedR = delayR();
            float srcL = io.out(0); // return the current sample
            float srcR = io.out(1);
            delayedL = delayedL * 0.3; // scale the delayed sample
            delayedR = delayedR * 0.3; // scale the delayed sample
            delayL(delayedR + (srcL + srcR) ); // delay the source and feedback a bit of delayed signal
            delayR(delayedL + (srcR + srcL) );
            
            // io.sum(delayed + src, 0, 1);
            io.out(0) = (srcL + delayedL) * ampSliderParam.get(); // output src plus delayed
            io.out(1) = (srcR + delayedR) * ampSliderParam.get();
            
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
        g.shadeModel(al::Graphics::SMOOTH);
        mesh.primitive(Graphics::QUADS);
        texBlur.resize(v.viewport().w, v.viewport().h);
        feedbackSliderParam = feedbackSlider.getValue();
        
        if(!texBlur.shapeUpdated()){
            // Plain (non-transformed) feedback
            texBlur.quadViewport(g, RGB(0.98));
            
            // Outward feedback
            // texBlur.quadViewport(g, RGB(feedbackSliderParam.get()), 2.01, 2.01, -1.005, -1.005);
            
            // Inward feedback
            // texBlur.quadViewport(g, RGB(feedbackSliderParam.get()), 1.99, 1.99, -0.995, -0.995);
            
            // Oblate feedback
             texBlur.quadViewport(g, RGB(feedbackSliderParam.get()), 2.01, 2.0, -1.005, -1.00);
            
            // Squeeze feedback!
           // texBlur.quadViewport(g, RGB(feedbackSliderParam.get()), 2.01, 1.99, -1.005, -0.995);
        }
        
        // Set up material (i.e., specularity)
        material.specular(light.diffuse()*0.9); // Specular highlight, "shine"
        material.shininess(128);			// Concentration of specular component [0,128]
        
        // Activate material
        material();
        
        if(projectionMode) {
            g.pushMatrix(Graphics::PROJECTION);
            g.loadMatrix(Matrix4f::ortho2D(-aspect, aspect, -1, 1)); // using the aspect ratio as the x
            light.globalAmbient(RGB(0.5));	// Ambient reflection for all lights
            light.ambient(RGB(0.5));			// Ambient reflection for this light
        } else {
            light.globalAmbient(RGB(0.1));	// Ambient reflection for all lights
            light.ambient(RGB(0));			// Ambient reflection for this light
        }
        
        // Set up light
        light.diffuse(RGB(1,1,0.5));	// Light scattered directly from light
        light.attenuation(1,1,0);		// Inverse distance attenuation
        //light.attenuation(1,0,1);		// Inverse-squared distance attenuation
        
        // Activate light
        light();
        
        for(int i = 0; i<NUMGRAINS; i++) { // can this be optimized?
            
            Shape& c = myCylinders[i];
            Grain& grain = grains[i];
            
            // std::cout << c.onset() << " " << c.startTime() << " " << c.duration() << " " << currentTime << std::endl;
            if(grains[i].remain > 0) {
                
                double newPos = mapRange<double>(currentTime, c.startTime(), c.startTime() + c.onset(), aspect * -1,  0);
                c.currentX = newPos;
                
                float rotationX = c.rotateX();
                float rotationY = c.rotateY();
                
                if(projectionMode) {
                    
                    g.pushMatrix(Graphics::MODELVIEW);
                        g.loadIdentity();
                    
                    if(spinCycleButton.getValue()) {
                        g.rotate(rotationY + 100, 0, 1, 0);
                        // g.translate(c.currentX, c.currentY, 0);
                        // g.rotate(rotationY, 0, 1, 0);
                        // g.rotate(rotationX, 1,0,0);
                    }
                    
                        g.translate(c.currentX, c.currentY, 0);
                        g.rotate(rotationY, 0, 1, 0);
                        g.rotate(rotationX, 1,0,0);
                    
                        if(c.onset() + c.startTime() < currentTime) {
                                // g.color(1.0, 1.0, 1.0, 1.0);
                                g.scale(0.05, 0.05, 0.05);
                        } else {
                        // g.color(0.1, 0.1, 0.9, 1.0);
                        g.scale(0.01, 0.01, 0.01);
                        }

                    g.draw(mesh);

                    g.popMatrix();
                    
                    if (mirrorButton.getValue()) {
                    g.pushMatrix(Graphics::MODELVIEW);
                        g.loadIdentity();
                        
                        if(spinCycleButton.getValue()) {
                            g.rotate(rotationY + 100, 0, 1, 0);
                            // g.translate(c.currentX * -1, c.currentY, 0);
                           // g.rotate(rotationY, 0, 1, 0);
                            // g.rotate(rotationX, 1,0,0);
                        }
                            g.translate(c.currentX * -1, c.currentY, 0);
                            g.rotate(rotationY, 0, 1, 0);
                            g.rotate(rotationX, 1,0,0);
                        
                        if(c.onset() + c.startTime() < currentTime) {
                            // g.color(1.0, 1.0, 1.0, 1.0);
                            g.scale(0.05, 0.05, 0.05);
                        } else {
                            // g.color(0.1, 0.1, 0.9, 1.0);
                            g.scale(0.01, 0.01, 0.01);
                        }
                        g.draw(mesh);
                 
                    g.popMatrix();
                    }
                    
                } else {
                
                    g.pushMatrix();
                    
                    if(spinCycleButton.getValue()) {
                        g.rotate(rotationY + 100, 0, 1, 0);
                        g.translate(c.currentX, c.currentY, 0);
                        g.rotate(rotationY, 0, 1, 0);
                        g.rotate(rotationX, 1,0,0);
                    } else {
                        g.translate(c.currentX, c.currentY, 0);
                        g.rotate(rotationY, 0, 1, 0);
                        g.rotate(rotationX, 1,0,0);
                    };
                    
                        if(c.onset() + c.startTime() < currentTime) {
                            g.color(1.0, 1.0, 1.0, 1.0);
                            g.scale(0.05, 0.05, 0.05);
                        } else {
                            g.color(0.1, 0.1, 0.9, 1.0);
                            g.scale(0.01, 0.01, 0.01);
                        }
                    
                    g.draw(mesh);
                
                    g.popMatrix();
                    
                    if (mirrorButton.getValue()) {
                    g.pushMatrix();
                        
                        if(spinCycleButton.getValue()) {
                            g.rotate(rotationY + 100, 0, 1, 0);
                            g.translate(c.currentX * -1, c.currentY, 0);
                            g.rotate(rotationY, 0, 1, 0);
                            g.rotate(rotationX, 1,0,0);
                        } else {
                            g.translate(c.currentX * -1, c.currentY, 0);
                            g.rotate(rotationY, 0, 1, 0);
                            g.rotate(rotationX, 1,0,0);
                        }
                        
                        if(c.onset() + c.startTime() < currentTime) {
                            g.color(1.0, 1.0, 1.0, 1.0);
                            g.scale(0.05, 0.05, 0.05);
                        } else {
                            g.color(0.1, 0.1, 0.9, 1.0);
                            g.scale(0.01, 0.01, 0.01);
                        }
                        
                        g.draw(mesh);
              
                    g.popMatrix();
                    }
                }
            }
        }
        if(projectionMode) {
            g.popMatrix(Graphics::PROJECTION);
        }
        
        texBlur.copyFrameBuffer();
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
            Shape& c = myCylinders[i];
            
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
            
            g.setAll(freq, 0.01, randomNum.uniform(-1, 1), seqStrategy->getDurInSamples(), seqStrategy->getOnsetInSamples(), appSampleRate);
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
        
            onset = mapRange<double>(x, 0, width, windowTime/1000, windowTime); // onset in ms.
        // onset = 0;
            accumOnset = 0;
        
            freq = mapRange<double>(y, 0, vw.dimensions().h, 0.0, 1.0); // first map the mouse value to between 0 and 1
            freq = mapRange<double>(freq*freq, 0, 1, 1000, 10); // then map the mouse value to between 1000 and 100;
            if(quantizeFreq) { freq = round<double>(freq, 100); }
            freq = min(freq, (double) 1000);
            freq = max(freq, (double) 10);
        
            dur = durSliderParam.get();
            // dur = map(dur, sliderValue)
            // dur = 500;
            dur = min(dur, windowTime);
      
            // dur = durSlider;
        
            //  for(int i = 0; i < (onsetSlider); i++) {
            indexOfGrain = scheduleGrain(onset, freq, dur); // Schedule a grain
            return indexOfGrain;
        //    }
    }
// private:
     //rnd::Random<> rng;
    
};
  int main(){
      MyApp().start();
  }
