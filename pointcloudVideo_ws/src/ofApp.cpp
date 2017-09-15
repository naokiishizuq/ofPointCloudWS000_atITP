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
    myVidPlayer2.load("images/lilUzi.mp4");
//    myVidPlayer2.setVolume(0.0);
    myVidPlayer2.play();
    myVidPlayer2.setLoopState(OF_LOOP_NORMAL);

    //SynthedVid
    myVidGrabber.setVerbose(true);
    myVidGrabber.setup(camWidth,camHeight);

    myVidPlayer.load("images/offTheWall.mp4");
    myVidPlayer.setVolume(0.0);
    myVidPlayer.play();
    myVidPlayer.setLoopState(OF_LOOP_NORMAL);
    
    //Mesh Set up
    mesh.setMode(OF_PRIMITIVE_POINTS);
    mesh.enableColors();
    mesh.enableIndices();
    
    camMesh.setMode(OF_PRIMITIVE_POINTS);
    camMesh.enableColors();
    camMesh.enableIndices();
    
    //3D draw settings
    ofBackground(0);
    ofEnableBlendMode(OF_BLENDMODE_ADD);
//    ofEnableNormalizedTexCoords();
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
    gui.add(intensityThresholdSlider.setup("intensityThreshold", 70, 1, 99));
    gui.add(moreOnLessOffToggle.setup("moreOnLessOff", true));
    gui.add(contourThresholdSlider.setup("contourThreshold", 40, 1, 255));
    
    contourFinder.setMinAreaRadius(10);
    contourFinder.setMaxAreaRadius(200);
}

//--------------------------------------------------------------
void ofApp::update(){
    bool bNewFrame = false;
    myVidPlayer.update();
    tempImage.setFromPixels(myVidPlayer.getPixelsRef());
    tempImage.resize(camWidth, camHeight);
    
    myVidPlayer2.update();
    tempImage2.setFromPixels(myVidPlayer2.getPixelsRef());
    tempImage2.resize(camWidth, camHeight);
    
    bNewFrame = myVidPlayer.isFrameNew();
    if(bNewFrame){
        mesh.clear();
        camMesh.clear();
        
        ofPixels pixelData = tempImage.getPixels();
        ofPixels synthData = tempImage2.getPixels();

        flow.setPyramidScale(pyrScale);
        flow.setNumLevels(levels);
        flow.setWindowSize(winsize);
        flow.setNumIterations(iterations);
        flow.setPolyN(polyN);
        flow.setPolySigma(polySigma);
        
        //Calculate Optical Flow
        flow.calcOpticalFlow(tempImage2);
        contourFinder.setThreshold(contourThresholdSlider);
        contourFinder.findContours(tempImage);
        
        
        ofVec3f result;
        float intensityThreshold = intensityThresholdSlider;//99.0;
        int counter = 0;
        for(int y = 0; y<camHeight; y++){
            for(int x = 0; x<camWidth; x++){
//                int tempInd = y * w + x;
                int id = y * int(camWidth) * 3 + x * 3;
                float r = pixelData[id] / 255.0;
                float g = pixelData[id+1] / 255.0;
                float b = pixelData[id+ 2] / 255.0;
                float intensity, tempThresh;
                float brightness = (r + g + b) / 3.0f;
                float percentage = brightness * 100;
                ofFloatColor c = ofFloatColor(r, g, b, 0.9);
                if(moreOnLessOffToggle){
                    intensity = percentage;
                    tempThresh = intensityThreshold;
                } else {
                    intensity = intensityThreshold;
                    tempThresh = percentage;
                }
                if(intensity >= tempThresh){
                    float saturation = c.getSaturation();
                    float z = ofMap(saturation, 0, 1, -100, 100);
                    float scaledX = ofMap(x-camWidth/2, -camWidth/2, camWidth/2, -ofGetWidth()/2, ofGetWidth()/2);
                    float scaledY = ofMap(y-camHeight/2, -camHeight/2, camHeight/2, -ofGetHeight()/2, ofGetHeight()/2);
                    
                    ofVec3f vert = ofVec3f(scaledX, scaledY, z);
                    mesh.addVertex(vert);
                    mesh.addColor(c);
//                    camMesh.addVertex(vert);
//                    ofFloatColor csEmp = ofFloatColor(0.0,0.0,0.0, 0.0);
//                    camMesh.addColor(csEmp);
                    ofVec2f force;
                    ofVec2f pos;
                    pos.x = x * float(tempImage.getWidth()) / float(ofGetWidth());
                    pos.y = y * float(tempImage.getHeight()) / float(ofGetHeight());
                    if (pos.x > 0 && pos.y > 0) {
                        force = flow.getFlowOffset(pos.x, pos.y) * flowScale;
                        counter++;
                        result+=force;
                    }

                } else {
                    float rs = synthData[id] / 255.0;
                    float gs = synthData[id+1] / 255.0;
                    float bs = synthData[id+ 2] / 255.0;
                    ofFloatColor cs = ofFloatColor(rs, gs, bs, 1);
                    float saturation = cs.getSaturation();
                    float z = ofMap(saturation, 0, 1, -100, 100);
                    float scaledXs = ofMap(x-camWidth/2, -camWidth/2, camWidth/2, -ofGetWidth()/2, ofGetWidth()/2);
                    float scaledYs = ofMap(y-camHeight/2, -camHeight/2, camHeight/2, -ofGetHeight()/2, ofGetHeight()/2);
//                    ofVec3f verts = ofVec3f(x-camWidth/2, y-camHeight/2, z);
                    ofVec3f verts = ofVec3f(scaledXs, scaledYs, z);
                    camMesh.addVertex(verts);
                    camMesh.addColor(cs);
                }
            }
        }
        
        //Camera Interaction Calculation
        //Calculate Camera vector from Optical Flow
        //ofVec3f added;
        ofVec3f avarage;
        if(counter){
            avarage = result/counter;
        }
          ofVec3f centerVec = ofVec3f(0, 0 + arrowUpDownY, 800 + arrowUpDownZ);
        ofVec3f position = centerVec+1500*avarage;
        realCam.setPosition(position);
        
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
        
        
        int numOfVerts = mesh.getNumVertices();
        //        cout << mesh.getNumVertices() << endl;
        //Connect the simillar vertex with index
//        float connectionDistance = 45;
//        for(int a=0; a<numOfVerts; a++){
//            ofVec3f verta = mesh.getVertex(a);
//            for (int b=a+1; b<numOfVerts; ++b) {
//                ofVec3f vertb = mesh.getVertex(b);
//                float dist = verta.distance(vertb);
//                if(dist <= connectionDistance){
//                    mesh.addIndex(a);
//                    mesh.addIndex(b);
//                }
//            }
//        }
    }// end of if(bNewFrame)
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
    camMesh.draw();
    mesh.draw();
    
    ofPushMatrix();
    ofTranslate(-ofGetWidth()/2, -ofGetHeight()/2);
    if (drawFlow) {
        ofSetColor(255, 12, 255);
        flow.draw(0,0,ofGetWidth(),ofGetHeight());
        ofSetColor(15, 255, 25);
        ofScale(ofGetWidth()/camWidth, ofGetHeight()/camHeight, 1);
        contourFinder.draw();
    }
    ofPopMatrix();
    if(!camMove){
        cam.end();
    } else {
        realCam.end();
    }
    
//    myVidGrabber.draw(0, 0, ofGetWidth(),ofGetHeight());
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

void distSq(float x1, float y1, float z1, float x2, float y2, float z2) {
    float d = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) +(z2-z1)*(z2-z1);
    return d;
}
