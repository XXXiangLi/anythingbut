#include "snake.h"

void Snake::setup(ofColor _c){
    targetC = _c;
    _c.r=255;
    _c.g=0;
    _c.b=0;
}
//---------------------
void Snake::update(){
    
}
//---------------------
void Snake::draw(){
    ofDisableArbTex();
    ofLoadImage(stairsTex,"stairs.jpg");
    ofPushStyle();
    for (int i=1; i<linePoints.size(); i++){
        radius = ofMap(i,1,linePoints.size()/2,25,0);
        heart_radius = ofMap(i,1,linePoints.size(),1.0,0.5);
        alpha =ofMap(i,1,linePoints.size()/2,255,0);
        ofColor black;
        black.r=255;//217, 9, 9
        black.g=0;
        black.b=0;
        ofColor p = targetC.getLerped(black, float(i));
        //ofSetColor(p,alpha);
        stairsTex.bind();
        ofDrawSphere(linePoints[i].x, linePoints[i].y,0, radius);
        stairsTex.unbind();
        
        ofPushMatrix();
        ofPushStyle();
        ofTranslate(linePoints[i].x, linePoints[i].y);
        ofScale(heart_radius);
       // ofRotateDeg(180);
        ofSetColor(217,9,9,alpha);
        ofDrawCircle(50,40,30);
        ofDrawCircle(75,40,30);
        ofDrawTriangle(31, 63, 95, 63, 63, 95);
        ofPopStyle();
        ofPopMatrix();
    }
    ofPopStyle();
}
//---------------------
void Snake::addLocation(int x, int y){
    
    linePoints.push_front(ofPoint(x,y));
    //if (linePoints.size()>80) linePoints.pop_back();
}
//---------------------
void Snake::length(int t){

    if (linePoints.size()>t) linePoints.pop_back();
}
