#pragma once

#include "ofMain.h"
#include "ofxAssimpModelLoader.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include "ofxOsc.h"
#include "snake.h"
#include "particleSystem.hpp"

#define HOST "localhost"
#define PORT 9494

class ofApp : public ofBaseApp {
public:
	
	void setup();
	void update();
	void draw();
	void exit();
	
	void drawPointCloud();
    void drawPointCloudsecond();
    
 //flareBar
    void flareBar();
    ofShader flare;
    vector<ParticleSystem> systems;
    ofVec2f gravity;
    ofVec2f systemLocation;
    float timestamp;
    float timer;
    ofParameter<float> flareBarRange;
    ofParameter<float> flareBarThickness;
    
    ofMesh pointCloud;
    vector<int> pointIndex;

    //audio input
    void audioIn(ofSoundBuffer & input);
            vector <float> left;
            vector <float> right;
            vector <float> volHistory;
            int     bufferCounter;
            int     drawCounter;
            float smoothedVol;
            float scaledVol;
            ofSoundStream soundStream;
    
    //noise
    int width;
    int height;
    int spacing;
    ofMesh mesh;
    float noiseScale;
    float noise;
    
    
    //models
    ofxAssimpModelLoader head;
    ofxAssimpModelLoader stairs;
    ofxAssimpModelLoader lamb;
    ofxAssimpModelLoader pig;
    ofxAssimpModelLoader pighead;
    ofxAssimpModelLoader sheephead;

    ofTexture cloudsTex;
    ofTexture stairsTex;
    ofTexture lambTex;
    ofTexture pigTex;
    ofTexture pigheadTex;
    ofTexture sheepheadTex;

    
    //sun
    ofMesh base_mesh, draw_mesh;
    
	ofxKinect kinect;
	
	ofxCvColorImage colorImg;
	
	ofxCvGrayscaleImage grayImage; // grayscale depth image
	ofxCvGrayscaleImage grayThreshNear; // the near thresholded image
	ofxCvGrayscaleImage grayThreshFar; // the far thresholded image
	
	ofxCvContourFinder contourFinder;
	
	bool bThreshWithOpenCV;
	bool bDrawPointCloud;
	
	int nearThreshold;
	int farThreshold;
    int nearThreshold_gray;
    int farThreshold_gray;
	
	int angle;
	
//camera
	ofEasyCam easyCam;
    
    int count;
    Snake snake;
    int closestColorX, closestColorY;
    float rTarget;
    float gTarget;
    float bTarget;
    float threshold;
    bool RGBIsOn;
    
    // word
    ofTrueTypeFont font;
    vector <string> words;

    //one circle
    void animate();
    float circle_radius;
    int total;
    ofMesh world;
    float rot;
    float animateZPos;
    bool animation;
    float xoff,yoff;
    
//back
    ofImage backimage;
    ofMesh backmesh;
    vector<ofVec3f> backoffsets;
    
//sender
    ofxOscSender sender;
};
