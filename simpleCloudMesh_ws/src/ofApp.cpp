#include "ofApp.h"
using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
    camMove = false;
    camWidth = 640;
    camHeight = 360;
    cam.setDistance(700);
    
    //Upload main video
    myVidPlayer.load("images/lilUzi.mp4");
    //    myVidPlayer.setVolume(0.0);
    myVidPlayer.play();
    myVidPlayer.setLoopState(OF_LOOP_NORMAL);
    
    //Mesh Set up
    mesh.setMode(OF_PRIMITIVE_POINTS);
    mesh.enableColors();
    mesh.enableIndices();
    
    //3D draw settings
    ofBackground(0);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofEnableDepthTest();
    
    //light
    light.enable();
    light.setPosition(300, 300, 500);
    
    // GUI
    gui.setup();
    gui.add(pyrScale.setup("pyrScale", .5, 0, 1));
    gui.add(levels.setup("levels", 4, 1, 8));
    gui.add(winsize.setup("winsize", 8, 4, 64));
    gui.add(iterations.setup("iterations", 2, 1, 8));
    gui.add(polyN.setup("polyN", 7, 5, 10));
    gui.add(polySigma.setup("polySigma", 1.5, 1.1, 2));
    gui.add(flowScale.setup("flowScale", 0.05, 0.0, 0.2));
    gui.add(drawFlow.setup("draw opticalflow", false));
    gui.add(fullscreen.setup("fullscreen", false));
    gui.add(intensityThresholdSlider.setup("intensityThreshold", 100, 1, 255));
    gui.add(contourThresholdSlider.setup("contourThreshold", 40, 1, 255));
    
        contourFinder.setMinAreaRadius(10);
        contourFinder.setMaxAreaRadius(200);
    
//        scaleingToOfWidth = ofGetWidth() / camWidth;
//        scaleingToOfHeight = ofGetHeight() / camHeight;
}

//--------------------------------------------------------------
void ofApp::update(){
    bool bNewFrame = false;
    
    myVidPlayer.update();
    tempImage.setFromPixels(myVidPlayer.getPixelsRef());
    tempImage.resize(camWidth, camHeight);
    ofPixels pixelData = tempImage.getPixels();

    bNewFrame = myVidPlayer.isFrameNew();
    
    if(bNewFrame){
        mesh.clear();
        
        //Camera Interaction Calculation
        flow.setPyramidScale(pyrScale);
        flow.setNumLevels(levels);
        flow.setWindowSize(winsize);
        flow.setNumIterations(iterations);
        flow.setPolyN(polyN);
        flow.setPolySigma(polySigma);
        
        //Calculate Optical Flow
        flow.calcOpticalFlow(tempImage);
        contourFinder.setThreshold(contourThresholdSlider);
        contourFinder.findContours(tempImage);
        
        //For Camera Interactive
        ofVec3f sumOfForceOfFlow;
        int counter = 0;
        float brightness;
        //start looping through every pixele
        for(int y = 0; y<camHeight; y++){
            for(int x = 0; x<camWidth; x++){
                //int tempInd = y * w + x; //Get Index in Pixel Loop
                int id = y * int(camWidth) * 3 + x * 3;
                float r = pixelData[id] / 255.0;
                float g = pixelData[id+1] / 255.0;
                float b = pixelData[id+ 2] / 255.0;
                ofFloatColor c = ofFloatColor(r, g, b, 0.9);
                brightness = (r + g + b) / 3.0f;
                if(brightness >= intensityThresholdSlider/255.0){
                    //3d mapping using saturation as Z coordinate
                    float saturation = c.getSaturation();
                    float z = ofMap(saturation, 0, 1, -100, 100);
                    
                    //need to scale up to Screen Width -> easier on calculation
                    float scaledX = ofMap(x-camWidth/2, -camWidth/2, camWidth/2, -ofGetWidth()/2, ofGetWidth()/2);
                    float scaledY = ofMap(y-camHeight/2, -camHeight/2, camHeight/2, -ofGetHeight()/2, ofGetHeight()/2);
                    ofVec3f vert = ofVec3f(scaledX, scaledY, z);
                    mesh.addVertex(vert);
                    mesh.addColor(c);
                    
                    //For Camera Interactive STORE
                    ofVec2f force;
                    ofVec2f pos;
                    //scaling (x,y) of camWidth/Height up to Screen width and height
                    pos.x = x * float(camWidth)/ float(ofGetWidth());
                    pos.y = y * float(camHeight) / float(ofGetHeight());
                    if (pos.x > 0 && pos.y > 0) {
                        //get direction of optical flow ***need better calculation
                        force = flow.getFlowOffset(pos.x, pos.y) * flowScale;
                        counter++;
                        sumOfForceOfFlow+=force;
                    }
                }
            }
        }
        
        if(camMove){
            //Camera Interaction Calculation
            //Avaraging Summed vec
            //calculate for Camera position from Optical Flow's directional vec's average
            ofVec3f avarage;
            if(counter){
                avarage = sumOfForceOfFlow/counter;
            }
            ofVec3f centerVec = ofVec3f(0, 0 + arrowUpDownY, 800 + arrowUpDownZ);
            ofVec3f position = centerVec+1500*avarage;
            realCam.setPosition(position);
            
            //Get Centeroids from ContourFinder
            //avarage position of every contour mapped to camera Look at
            ofVec2f centroids;
            int numberOfContour = contourFinder.size();
            for(int i = 0; i < numberOfContour; i++) {
                centroids += toOf(contourFinder.getCentroid(i));
            }
            ofVec2f center = centroids/numberOfContour;
            float adjustedX = ofMap(center.x, 0, ofGetWidth(), 0.0, 50.0);
            float adjustedY = ofMap(center.y, 0, ofGetHeight(), 0.0, 50.0);
            ofVec3f lookAt = ofVec3f(0 + adjustedX, 0 + adjustedY , 0);
            realCam.lookAt(lookAt);
        }

    }
//            int numOfVerts = mesh.getNumVertices();
//            cout << mesh.getNumVertices() << endl;
//    
//            //Connect the simillar vertex with index
//            float connectionDistance = 45;
//            for(int a=0; a<numOfVerts; a++){
//                ofVec3f verta = mesh.getVertex(a);
//                for (int b=a+1; b<numOfVerts; ++b) {
//                    ofVec3f vertb = mesh.getVertex(b);
//                    float dist = verta.distance(vertb);
//                    if(dist <= connectionDistance){
//                        mesh.addIndex(a);
//                        mesh.addIndex(b);
//                    }
//                }
//            }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofEnableDepthTest();
    
    ofColor centerColor = ofColor(85, 78, 68);
    ofColor edgeColor(0, 0, 0);
    ofBackgroundGradient(centerColor, edgeColor, OF_GRADIENT_CIRCULAR);
    
    if(!camMove){
        cam.begin();
    } else {
        realCam.begin();
    }
    ofScale(1, -1, 1);
    glPointSize(5);
    mesh.draw();

    ofPushMatrix();
    ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2);
    if (drawFlow) {
        float xScaleRate = float(ofGetWidth()) / float(camWidth);
        float yScaleRate = float(ofGetHeight()) / float(camHeight);
        ofSetColor(255, 12, 255);
        flow.draw(0,0,ofGetWidth(),ofGetHeight());
        ofSetColor(15, 255, 25);
        ofScale(xScaleRate, yScaleRate, 1.0);
        cout << xScaleRate << endl;
        contourFinder.draw();
    }
    ofPopMatrix();
    if(!camMove){
        cam.end();
    } else {
        realCam.end();
    }
    
//    myVidPlayer.draw(0, 0, ofGetWidth(),ofGetHeight());
    ofDisableDepthTest();
    gui.draw();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == ' ') camMove=!camMove; //space
    if(key == 357) arrowUpDownZ+=80; //arrowUp
    if(key == 359) arrowUpDownZ-=80; //arrowUp
    if(key == 356) arrowUpDownY+=120; //arrowUp
    if(key == 358) arrowUpDownY-=120; //arrowUp
    cout << key << endl;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
