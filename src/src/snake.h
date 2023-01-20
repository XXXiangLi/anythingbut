#pragma once
#include "ofMain.h"

class Snake : public ofBaseApp{
public:
    void setup(ofColor _c);
    void update();
    void draw();
    void addLocation(int x, int y);
    void length(int t);
    ofColor targetC;
    deque <ofPoint> linePoints;
    
    float radius;
    float heart_radius;
    float alpha;
    ofTexture stairsTex;

    
};
