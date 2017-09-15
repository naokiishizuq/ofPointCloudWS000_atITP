#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxGui.h"


//#define _USE_LIVE_VIDEO		// uncomment this to use a live camera
// otherwise, we'll use a movie file

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void distSq(float x1, float y1, float z1, float x2, float y2, float z2);
    
    
    ofVideoPlayer 		myVidPlayer;
        
    // Optical Flow -> used for camera interaction
    ofxCv::FlowFarneback flow;
    // contour -> used for camera interaction
    ofxCv::ContourFinder contourFinder;
    
    //Temporary image and output mesh
    ofImage tempImage;
    ofVboMesh mesh;
    
    //Variables for controle and stuff
    int camWidth;
    int camHeight;
    Boolean camMove;
    int arrowUpDownZ;
    int arrowUpDownX;
    int arrowUpDownY;
    
    
    
    //3D setting and Basic Video n Image
    ofCamera realCam;
    ofEasyCam cam;
    ofLight light;
    
    // GUI
    ofxPanel gui;
    ofxFloatSlider pyrScale;
    ofxIntSlider levels;
    ofxIntSlider winsize;
    ofxIntSlider iterations;
    ofxIntSlider polyN;
    ofxFloatSlider polySigma;
    ofxFloatSlider flowScale;
    ofxToggle drawFlow;
    ofxToggle fullscreen;
    ofxButton resetParticleButton;
    ofxIntSlider intensityThresholdSlider;
    ofxIntSlider contourThresholdSlider;
};
