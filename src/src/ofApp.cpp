#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
    ofSetBackgroundAuto(true);
    
	ofSetLogLevel(OF_LOG_VERBOSE);
	kinect.setRegistration(true);
    
	kinect.init();
	
	kinect.open();
	if(kinect.isConnected()) {
		ofLogNotice() << "sensor-emitter dist: " << kinect.getSensorEmitterDistance() << "cm";
		ofLogNotice() << "sensor-camera dist:  " << kinect.getSensorCameraDistance() << "cm";
		ofLogNotice() << "zero plane pixel size: " << kinect.getZeroPlanePixelSize() << "mm";
		ofLogNotice() << "zero plane dist: " << kinect.getZeroPlaneDistance() << "mm";
	}
	
	
	colorImg.allocate(kinect.width, kinect.height);
	grayImage.allocate(kinect.width, kinect.height);
	grayThreshNear.allocate(kinect.width, kinect.height);
	grayThreshFar.allocate(kinect.width, kinect.height);
	
	nearThreshold = 500;//for kinect - check 3d people
	farThreshold = 1000;
    nearThreshold_gray = 255;// for following
    farThreshold_gray = 250;
	bThreshWithOpenCV = true;
	
	ofSetFrameRate(60);
	
	// zero the tilt on startup
	angle = 0;
	kinect.setCameraTiltAngle(angle);
    rTarget = 255;
    gTarget = 182;
    bTarget = 193;
    threshold = 2.5;
    RGBIsOn = false;
        ofSetCircleResolution(80);
    
    //sound
        soundStream.printDeviceList();
        int bufferSize = 256;//length
        left.assign(bufferSize, 0.0);
        right.assign(bufferSize, 0.0);
    
        volHistory.assign(400, 0.0);
        bufferCounter    = 0;
        drawCounter        = 0;
        smoothedVol     = 0.0;
        scaledVol        = 0.0;
        ofSoundStreamSettings settings;
        auto devices = soundStream.getMatchingDevices("default");
        if(!devices.empty()){
            settings.setInDevice(devices[0]);
        }
        settings.setInListener(this);
        settings.sampleRate = 44100;
        settings.numOutputChannels = 0;
        settings.numInputChannels = 2;
        settings.bufferSize = bufferSize;
        soundStream.setup(settings);
   
    //noise
    width=900;
    height=900;
    spacing=50;
    ofPushStyle();
    for (int x = 0; x < width; x++){
        for (int y = 0; y<height; y++){
            int xPos=spacing*x;
            int yPos=spacing*y;
            ofVec3f position = ofVec3f(xPos,yPos,0);
            mesh.addVertex(ofPoint(xPos,yPos,0));
            mesh.addColor(ofFloatColor(255));
        }
    }
    for (int x = 0; x < width-5; x++){
        for (int y=0; y < height-5; y++){
            mesh.addIndex(x+y*width);
            mesh.addIndex((x+5)+y*width);
            mesh.addIndex(x+(y+5)*width);
            mesh.addIndex((x+5)+y*width);
            mesh.addIndex((x+5)+(y+5)*width);
            mesh.addIndex(x+(y+5)*width);
        }
    }
    ofPopStyle();
    
    //model
    head.loadModel("head.obj", 20);
    stairs.loadModel("stairs.obj", 20);
    lamb.loadModel("lamb.obj", 20);
    pig.loadModel("pig.obj", 20);
    pighead.loadModel("pighead.obj", 20);
    sheephead.loadModel("sheephead.obj", 20);
    
    
    //texture
    ofDisableArbTex();
    ofLoadImage(stairsTex,"stairs.jpg");
    ofLoadImage(cloudsTex,"clouds.png");
    ofLoadImage(lambTex,"lamb.jpg");
    ofLoadImage(pigTex,"pig.png");
    ofLoadImage(pigheadTex,"pighead.png");
    ofLoadImage(sheepheadTex,"sheephead.png");
    
    //sun
    //reference:https://neort.io/art/c5rrngc3p9fe3sqppf8g?index=188&origin=user_profile
    auto ico_sphere = ofIcoSpherePrimitive(250, 2);
        this->base_mesh = ico_sphere.getMesh();

    //
    circle_radius = 200;
    total = 30;
    rot = 0;
    animateZPos = 1;
    world.setMode(OF_PRIMITIVE_TRIANGLES);
    animation = true;
    for (int i= 0; i < total; i++) {
        float lat = ofMap(i,0,total-1,0.0,PI);
        for (int j= 0; j<total; j++) {
            float lon = ofMap(j,0,total-1,0.0,TWO_PI);
            float x = circle_radius * sin(lat) * cos(lon);
            float y = circle_radius * sin(lat) * sin(lon);
            float z = circle_radius * cos(lat);
            world.addVertex(ofVec3f(x,y,z));
        }
    }
    
    for (int j = 0; j < total - 1; j++) {
        for (int i = 0 ; i < total - 1 ; i++) {
            world.addIndex(i+j*total);         // 0
            world.addIndex((i+1)+j*total);     // 1
            world.addIndex(i+(j+1)*total);     // 6
            world.addIndex((i+1)+j*total);     // 1
            world.addIndex((i+1)+(j+1)*total); // 7
            world.addIndex(i+(j+1)*total);     // 6
        }
    }
    
    //words
    font.load("terminal-grotesque_open.otf", 15);
    words = { "SCREAMING", "OVERINDULGENT", "ENVIOUS","UNBRIDLED" };
    
    //back
    backimage.load("back.png");
    backmesh.setMode(OF_PRIMITIVE_LINES);
    backmesh.enableIndices();

        float intensityThreshold = 80.0;
        int w = backimage.getWidth();
        int h = backimage.getHeight();
    for (int x=0; x<w; x+=2) {
        for (int y=0; y<h; y+=2) {
            ofColor c = backimage.getColor(x, y);
            float intensity = c.getLightness();
            if (intensity >= intensityThreshold) {
                float saturation = c.getSaturation();
                float z = ofMap(saturation, 0, 255, -100, 100);
                ofVec3f pos(x*4, y*4, z);
                backmesh.addVertex(pos);
                backmesh.addColor(c);
                backoffsets.push_back(ofVec3f(ofRandom(0,100000), ofRandom(0,100000), ofRandom(0,100000)));
            }
        }
    }
    float connectionDistance = 30;
    int numVerts = backmesh.getNumVertices();
    for (int a=0; a<numVerts; a+=3) {
        ofVec3f verta = backmesh.getVertex(a);
        for (int b=a+1; b<numVerts; b+=3) {
            ofVec3f vertb = backmesh.getVertex(b);
            float distance = verta.distance(vertb);
            if (distance <= connectionDistance) {
                backmesh.addIndex(a);
                backmesh.addIndex(b);
            }
        }
    }
    
    //flareBar
    //reference:https://github.com/peterobbin/wangl073_ss2015
    flare.load( "flare.vert", "flare.frag" );
    timestamp = ofGetElapsedTimef();
    gravity.set( 0.0, 0.02 );
    
    //sender.....MAXMSP......control frequency and volume
    sender.setup(HOST, PORT);
}

//--------------------------------------------------------------
void ofApp::update() {
    ofBackground(0);
    scaledVol = ofMap(smoothedVol, 0.00, 0.008, 0.00, 2.00, true);//0.0, 0.17, 0.0, 1.0
    
    volHistory.push_back( scaledVol );
    if( volHistory.size() >= 400 ){
        volHistory.erase(volHistory.begin(), volHistory.begin()+1);
    }
//noise
noiseScale = scaledVol * .01;
for (int i=0;i< mesh.getVertices().size();i++){
    ofVec3f vert = mesh.getVertex(i);
    noise=ofNoise(vert.x*noiseScale,vert.y*noiseScale,float(noiseScale*ofGetFrameNum()));
   vert.z = noise * 900+1000;
mesh.setVertex(i, vert);
   int color;
    color=ofMap(scaledVol,0.,1.,0,255);
    int alph;
    alph =ofMap(scaledVol,0.,1.,60,250);
    ofColor color2;
    color2=ofColor(ofRandom(-noise*255,noise*255),alph);
    mesh.setColor(i, color2);
}

//sun
this->draw_mesh.clear();

int x = -2000;
float scaledVolnum;
scaledVolnum=ofMap(scaledVol,0.0,1.0,1.0,10.0);
    for (int y = 0; y <= 1400; y += 1400) {
        for (auto v : this->base_mesh.getVertices()) {
            for (int i = 0; i < 2; i++) {
                auto rotation_x = glm::rotate(glm::mat4(),ofMap(ofNoise(glm::vec4((v*scaledVolnum + glm::vec3(x, y, 0)) * 0.15,ofGetFrameNum() * 0.01)), 0, 1, -90, 90)* (float)DEG_TO_RAD,glm::vec3(1, 0, 0));
                auto rotation_y = glm::rotate(glm::mat4(), ofMap(ofNoise(glm::vec4((v *scaledVolnum+ glm::vec3(x, y, 0)) * 0.15, ofGetFrameNum() * 0.01)), 0, 1, -90, 90) * (float)DEG_TO_RAD, glm::vec3(0, 1, 0));
                auto rotation_z = glm::rotate(glm::mat4(), ofMap(ofNoise(glm::vec4((v *scaledVolnum+ glm::vec3(x, y, 0)) * 0.15,ofGetFrameNum() * 0.01)), 0, 1, -90, 90) * (float)DEG_TO_RAD, glm::vec3(0, 0, 1));
 
                v = glm::vec4(v*scaledVolnum, 0) * rotation_z * rotation_y * rotation_x;
            }
            this->draw_mesh.addVertex(v*scaledVolnum + glm::vec3(x*scaledVolnum, y*scaledVolnum, -5000));
        }
 
    }
    
	kinect.update();
	if(kinect.isFrameNew()) {
		grayImage.setFromPixels(kinect.getDepthPixels());
		if(bThreshWithOpenCV) {
			grayThreshNear = grayImage;
			grayThreshFar = grayImage;
			grayThreshNear.threshold(nearThreshold_gray, true);
			grayThreshFar.threshold(farThreshold_gray);
			cvAnd(grayThreshNear.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		} else {
			ofPixels & pix = grayImage.getPixels();
			int numPixels = pix.size();
			for(int i = 0; i < numPixels; i++) {
				if(pix[i] < nearThreshold_gray && pix[i] > farThreshold_gray) {
					pix[i] = 255;
				} else {
					pix[i] = 0;
				}
            }
        }
		grayImage.flagImageChanged();
		contourFinder.findContours(grayImage, 10, (kinect.width*kinect.height)/2, 20, false);
        
	}
    closestColorX = 0;
    closestColorY = 0;
    int w = grayImage.getWidth();
    int h = grayImage.getHeight();
    count = 0;
    for (int y=0; y<h; y++) {
        for (int x= 0; x<w; x++) {
            ofColor colorAtXY = grayImage.getPixels().getColor(x, y);
            int brightness = colorAtXY.getBrightness();
            if(brightness >= 2.4){
                closestColorX+= x;
                closestColorY+= y;
                count++;
            }
        }
    }

    if (count>0) {
        closestColorX = closestColorX / count;
        closestColorY = closestColorY / count;
       snake.addLocation(closestColorX*(float(ofGetWidth())/ (float(grayImage.width))),closestColorY*(float(ofGetHeight())/ (float (grayImage.height)))+sin(ofGetFrameNum()*20)*20);
    }
    //
    animate();
    
    //back
    int numVerts = backmesh.getNumVertices();
    for (int i=0; i<numVerts; i++) {
        ofVec3f vert = backmesh.getVertex(i);
        float time = ofGetElapsedTimef();
        float timeScale = 5.0;
        float displacementScale = 0.50+sin(scaledVol)*100;
        ofVec3f timeOffsets = backoffsets[i];
        vert.x += (ofSignedNoise(time*timeScale+timeOffsets.x)) * displacementScale;
        vert.y += (ofSignedNoise(time*timeScale+timeOffsets.y)) * displacementScale;
        vert.z += (ofSignedNoise(time*timeScale+timeOffsets.z)) * displacementScale;
        backmesh.setVertex(i, vert);
    }
    //
    for( int i = 0; i < systems.size(); i++ ) {
        systems[i].update( gravity );
    }
    timer = ofGetElapsedTimef() - timestamp;
    timer = 1.0 - ofClamp( timer, 0.0, 3.0 ) / 3.0;
}

//--------------------------------------------------------------
void ofApp::draw() {
    cout << scaledVol << endl;
    ofEnableDepthTest();
    easyCam.begin();
    //
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(0,-1000,-6000);
    ofScale(2+scaledVol/2,2+scaledVol/2,2+scaledVol/2);
    backimage.resize(200, 200);
    if(scaledVol > 0.5){  //.....................................................................2️⃣
    backmesh.draw();
    }
    ofPopStyle();
    ofPopMatrix();
    
    //.....................................lamb
    ofPushStyle();
        ofPushMatrix();
   ofTranslate(-500,-500,0);
int nw = grayImage.getWidth();
    int nh = grayImage.getHeight();
    float down;
    float up;
    float lambscale = ofRandom(0.1,0.5);
    for (int y=0; y<nh; y+=55) {
        for (int x= 0; x<nw; x+=65) {
            ofColor colorAtXY = grayImage.getPixels().getColor(x, y);
                    if(colorAtXY == ofColor(255)){
                        down = ofMap(x, 0, nw, 0, 100);
                        up =ofMap(y, 0, nh, 0, 100);
                        lamb.setRotation(1, 90, 1,0, 0);
                        lamb.setRotation(3, ofGetFrameNum()*PI*60, 0,0,1);
                        lamb.setRotation(2, ofRandom(ofGetFrameNum()*TWO_PI), 0,1,0);
                        lamb.setPosition(x*(float(ofGetWidth())/float(grayImage.width)), y*(float(ofGetHeight())/float (grayImage.height))-down+up,-500);
                        lamb.setScale(lambscale,lambscale,lambscale);
                        lambTex.bind();
                        lamb.drawFaces();
                        lambTex.unbind();
                    }
        }
    }
    ofPopMatrix();
    ofPopStyle();
    
    //...................................mesh
    if(scaledVol <= 0.5){//.....................................................................1️⃣
    ofPushMatrix();
    ofPushStyle();
    ofRotateY(30);
    ofRotateX(90);
    ofRotateZ(-130);
    ofTranslate(-5000,-3500,0);
    mesh.drawVertices();
    ofPopMatrix();
    ofPopStyle();
    }
    
    // draw the average volume:
    ofPushStyle();
        ofPushMatrix();
    ofRotateYDeg( sin(ofGetFrameNum()*.5)*TWO_PI );
        ofTranslate(200,500,300);//x large right, y large right ,z
        ofSetColor(225);
    ofDrawBitmapString("BETWEEN HEAVEN AND EARTH", 4, -400);
    ofPushStyle();
        ofPushMatrix();
    ofTranslate(-500, -400,0);
    //....................................second
    string word = words[ ofRandom( words.size() ) ];
    for (int i = 0; i <360; i+=10){
        ofPushMatrix();
        ofRotateZDeg( (ofGetSeconds() * 60) -90 );
        ofRotateZDeg(i);
        if(scaledVol <= 0.5){//.....................................................................1️⃣
        font.drawString( word , 100,0 );
        }
        ofSetColor(255, 255, 255, i - 100);
        ofPopMatrix();
    }
    //....................................min
    for (int i = 0; i <360; i+=10){
        ofPushMatrix();
        ofRotateYDeg( (ofGetMinutes() * 36) - 90 );
        ofRotateYDeg(i);
        if (ofGetSeconds()%2  == 0){
            if(scaledVol <= 0.5){//.....................................................................1️⃣
            font.drawString("PIG", 130,0 );
            }
        } else {
            if(scaledVol <= 0.5){//.....................................................................1️⃣
            font.drawString("SLEEP", 130,0 );
            }
        }
        ofSetColor(255, 255, 255, i -100);
        ofPopMatrix();
    }
    ofPopMatrix();
    ofPopStyle();
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(-500+ sin(ofGetFrameNum())*400,-350, -800);
    ofRotateX(rot);
    ofRotateY(rot);
    ofSetColor(noise*255);
    if(scaledVol > 0.5){  //.....................................................................2️⃣
    world.drawVertices();
    }
    for (int i=0; i < world.getVertices().size(); i+=500) {
        float x = world.getVertex(i).x;
        float y = world.getVertex(i).y;
        float z = world.getVertex(i).z;
        ofSetColor(217, 9, 9,190);
        float length = sqrt(x*x+y*y+z*z);
        x = x / length;
        y = y / length;
        z = z / length + sin(ofGetElapsedTimef()) * 30;
        float sanim = 1.1;
        //....................................sphere red
        if(scaledVol > 0.5){  //.....................................................................2️⃣
        cloudsTex.bind();
        ofDrawSphere(x*sanim, y*sanim, z*sanim, 250+scaledVol * 360.0f);
        cloudsTex.unbind();
        }
    }
    ofPopMatrix();
    ofPopStyle();
    ofTranslate(0,-2000,0);
        ofBeginShape();
        for (unsigned int i = 0; i < volHistory.size(); i++){
            if( i == 0 ) ofVertex(i, 400);
            ofVertex(i, 400 - volHistory[i] * 70);
            if( i == volHistory.size() -1 ) ofVertex(i, 400);
        }
    ofEndShape(false);
    ofPopMatrix();
    ofPopStyle();
    drawCounter++;
    
    //...................................human
    ofPushMatrix();
    ofPushStyle();
    ofTranslate(0,100,-100+(100*scaledVol));
    if(scaledVol <= 0.5){    //.....................................................................1️⃣
        drawPointCloudsecond();
    }
    else if(scaledVol > 0.15){//.....................................................................2️⃣
        drawPointCloud();
    }
    //.................................snake_heart
    ofTranslate(-600,500,0);
    snake.length(50+scaledVol*100);
    if(scaledVol > 0.2){
        ofTranslate(0,ofGetFrameNum()*10,0);
    }
    ofRotateXDeg(180);
    snake.draw();
    ofPopStyle();
    ofPopMatrix();
    
    //.................................sphere
    ofPushMatrix();
    ofPushStyle();
    ofRotateZ(45);
    ofTranslate(3000,-1000,1000);
    ofScale((scaledVol*2+.5));
        for (auto& vertex : this->draw_mesh.getVertices()) {
            ofDrawSphere(vertex, 2*(scaledVol+5));
        }
    ofPopMatrix();
    ofPopStyle();
    
    //.................................head
    if(scaledVol <= 0.5){//.....................................................................1️⃣
    ofPushMatrix();
    ofPushStyle();
        float scalechange =scaledVol * 1.1;
        ofSetColor(ofRandom(150+scaledVol*105), 170);
        head.setRotation(0, 180, 1, 0, 0);//x
        head.setRotation(1, 270, 0, 1,0);
        head.setRotation(1, sin(ofGetFrameNum()*.5)*10* TWO_PI, 0, 1,0);//y
        head.setScale(scalechange,scalechange,scalechange);
    ofTranslate(-1000, -200,-900);
    for(int z=0;z<2000;z+=400){
    for(int x=0;x<2000;x+=400){
        head.setPosition(x,0,z);
       head.draw(OF_MESH_POINTS);
    }
    }
        ofPopStyle();
        ofPopMatrix();
    }
    
    //.................................pig
    if(scaledVol > 0.5){//.....................................................................2️⃣
        ofPushMatrix();
        ofPushStyle();
    float scalechange =.1+scaledVol * .41;
    pig.setRotation(0,180,1,0,0);
    pig.setRotation(1, 180+sin(ofGetFrameNum()*.5)*10* TWO_PI, 0, 1,0);//y
    pig.setScale(scalechange,scalechange,scalechange);
        ofTranslate(-1000, -200,-900);
        for(int z=0;z<2000;z+=300){
        for(int x=0;x<2000;x+=200){
            pig.setPosition(x,0,z);
            pigTex.bind();
            pig.drawFaces();
            pigTex.unbind();
        }
        }
            ofPopStyle();
            ofPopMatrix();
    }
    
    //.................................stairs
    if(scaledVol > 0.5){//.....................................................................2️⃣
    ofPushMatrix();
    ofPushStyle();
    stairs.setRotation(1, sin(ofGetFrameNum()*.1)*30* TWO_PI, 0, 1,0);//y
    stairs.setPosition(-2500, 1000,-1000-sin(ofGetFrameNum()*5)*100);
    stairsTex.bind();
    stairs.setScale(20,20,20);
    stairs.drawFaces();
    stairsTex.unbind();
    ofPopStyle();
    ofPopMatrix();
     }
    
    //.................................pighead
   if(scaledVol > 0.5){//..................................................................2️⃣
    ofPushMatrix();
    ofPushStyle();
    pighead.setPosition((closestColorX-200)*(float(ofGetWidth())/ float(grayImage.width)),(closestColorY-300)*(float(ofGetHeight())/ float (grayImage.height)),-300);
    pigheadTex.bind();
    pighead.drawFaces();
    pigheadTex.unbind();
    ofPopStyle();
    ofPopMatrix();
    }
    
    //.................................sheephead
    if(scaledVol > 0.5){//..................................................................2️⃣
    ofPushMatrix();
    ofPushStyle();
    ofRotateZDeg(90);
    ofRotateXDeg(55 +sin(ofGetFrameNum())*10);
    ofRotateYDeg(90);
    sheephead.setRotation(2,-45,0,0,1);
    sheephead.setScale(3,3,3);
    sheephead.setPosition(200,-7000,0);
    sheepheadTex.bind();
    sheephead.drawFaces();
    sheepheadTex.unbind();
    ofPopStyle();
    ofPopMatrix();
    ofPushMatrix();
    ofPushStyle();
    ofRotateZDeg(-90);
    ofRotateXDeg(55 +sin(ofGetFrameNum())*10);
    ofRotateYDeg(-90);
     sheephead.setRotation(2,45,0,0,1);
     sheephead.setScale(3,3,3);
    sheepheadTex.bind();
    sheephead.drawFaces();
    sheepheadTex.unbind();
    ofPopStyle();
    ofPopMatrix();
     }
    
    //.................................flareBar
    if(scaledVol <= 0.5){//.....................................................................1️⃣
    ofPushMatrix();
    ofPushStyle();
       ofEnableBlendMode( OF_BLENDMODE_ADD );
        ofTranslate(-500,-500,0);
     int fnw = grayImage.getWidth();
         int fnh = grayImage.getHeight();
         for (int y=0; y<fnh; y+=55) {
             for (int x= 0; x<fnw; x+=65) {
                 ofColor colorAtXY = grayImage.getPixels().getColor(x, y);
                         if(colorAtXY == ofColor(255)){
                             ParticleSystem system( ofVec2f((closestColorX-200)*(float(ofGetWidth())/ float(grayImage.width)),closestColorY*(float(ofGetHeight())/ float (grayImage.height))));
                                 systemLocation = ofVec2f((closestColorX-200)*(float(ofGetWidth())/ float(grayImage.width)),closestColorY*(float(ofGetHeight())/ float (grayImage.height)));
                                 systems.push_back( system );
                                 timestamp = ofGetElapsedTimef();
                         }
             }
         }
        ofSetColor(80,100);
        flareBar();
        ofPopStyle();
        ofPopMatrix();
    }

    easyCam.end();
    ofDisableDepthTest();
    
    //sending
    string buf;
    buf = "sending osc messages to" + string(HOST) + ofToString(PORT);
    ofxOscMessage m;
    m.setAddress("/scaledVol");
    if(scaledVol > 0.5){//..................................................................2️⃣
        m.addIntArg(1);
    }else if(scaledVol <= 0.5){//.....................................................................1️⃣
        m.addIntArg(0);
    }
    m.addIntArg(closestColorX);
    m.addIntArg(closestColorY);
    sender.sendMessage(m, false);
}

//--------------------------------------------------------------
void ofApp::drawPointCloudsecond(){
     ofMesh pointCloud;
     pointCloud.setMode(OF_PRIMITIVE_POINTS);
    // generate points in a mesh object for our point cloud
    for (int y = 0; y < kinect.height; y +=(3+scaledVol*20)){
        for (int x = 0; x < kinect.width; x+=(3+scaledVol*20)) {
            ofVec3f point;
            point = kinect.getWorldCoordinateAt(x, y);
            if (point.z > nearThreshold && point.z < farThreshold){
                pointCloud.addVertex(point);
                 ofColor col;
                if(ofGetSeconds() <= 10 ){
                 col.set( ofMap(point.z, 500, 1000, 255,0 ));
                }
                else if(ofGetSeconds() > 10 && ofGetSeconds() <= 20 ){
                    col.setHsb( ofMap(point.z, 500, 1000, 255,100 ),ofMap(point.z, 500, 1000, 255,0 ),ofMap(point.z, 500, 1000, 255,0 ));
                }
                else if(ofGetSeconds() > 20 && ofGetSeconds() <= 30 ){
                    col.set( ofMap(point.z, 500, 1000, 255,0 ));
                }
                else if (ofGetSeconds() > 30  && ofGetSeconds() <= 40 ){
                col.setHsb( ofMap(point.z, 500, 1000, 255,100 ),ofMap(point.z, 500, 1000, 255,0 ),ofMap(point.z, 500, 1000, 255,0 ));
                }
                else if(ofGetSeconds() > 40 && ofGetSeconds() <= 50 ){
                    col.set( ofMap(point.z, 500, 1000, 255,0 ));
                }
                else if (ofGetSeconds() > 50 ){
                col.setHsb( ofMap(point.z, 500, 1000, 255,100 ),ofMap(point.z, 500, 1000, 255,0 ),ofMap(point.z, 500, 1000, 255,0 ));
                }
                 pointCloud.addColor(col);
            }
        }
    }
    glPointSize(2);
    ofEnableDepthTest();
    ofPushMatrix();
    ofScale(1,-1,-1);
    ofTranslate(0,0, -1000);
    pointCloud.drawVertices();
    ofPopMatrix();
    ofDisableDepthTest();
}
//--------------------------------------------------------------
void ofApp::drawPointCloud() {
     ofMesh pointCloud;
     pointCloud.setMode(OF_PRIMITIVE_POINTS);
    for (int y = 0; y < kinect.height; y +=2){
        for (int x = 0; x < kinect.width; x+=2) {
            ofVec3f point;
            point = kinect.getWorldCoordinateAt(x, y);
            if (point.z > nearThreshold && point.z < farThreshold){
                pointCloud.addVertex(point);
                 pointCloud.addColor(kinect.getColorAt(x, y));
            }
        }
    }
    glPointSize(2);
    
    ofEnableDepthTest();
    ofPushMatrix();
    ofScale(1,-1,-1);
    ofTranslate(0,0, -1000);
     pointCloud.drawWireframe();
    ofPopMatrix();
    ofDisableDepthTest();
}

//--------------------------------------------------------------
void ofApp::exit() {
	kinect.setCameraTiltAngle(0); // zero the tilt on exit
	kinect.close();
}
//--------------------------------------------------------------
void ofApp::audioIn(ofSoundBuffer & input){
    float curVol = 0.0;
    int numCounted = 0;
    for (size_t i = 0; i < input.getNumFrames(); i++){
        left[i]     = input[i*2]*0.5;
        right[i]    = input[i*2+1]*0.5;
        curVol += left[i] * left[i];
        curVol += right[i] * right[i];
        numCounted+=2;
    }
    curVol /= (float)numCounted;
    curVol = sqrt( curVol );
    smoothedVol *= 0.93;
    smoothedVol += 0.07 * curVol;
    bufferCounter++;
}
//--------------------------------------------------------------
void ofApp::animate() {
    world.clear();
    for (int i= 0; i < total; i++) {
        float lat = ofMap(i,0,total-1,0.0,PI);//pi
        for (int j= 0; j<total; j++) {
            float lon = ofMap(j,0,total-1,0.0,TWO_PI);
            if(animation)
                animateZPos = ofMap(ofNoise(i*ofGetElapsedTimef()*0.75, j*ofGetElapsedTimef()*0.75),0,1,-10,10);
            float x = (animateZPos+circle_radius) * sin(lat) * cos(lon);
            float y = (animateZPos+circle_radius) * sin(lat) * sin(lon);
            float z = (animateZPos+circle_radius) * cos(lat);
            world.addVertex(ofVec3f(x,y,z));
        }
    }
    
    for (int j = 0; j < total - 1; j++) {
        for (int i = 0 ; i < total - 1 ; i++) {
            world.addIndex(i+j*total);
            world.addIndex((i+1)+j*total);
            world.addIndex(i+(j+1)*total);
            world.addIndex((i+1)+j*total);
            world.addIndex((i+1)+(j+1)*total);
            world.addIndex(i+(j+1)*total);
        }
    }
    
    if(animation) {
        rot = rot + 6.9;
    }
}
//--------------------------------------------
void ofApp::flareBar()
{
    for( int i = 0; i < systems.size(); i++ ) {
        systems[i].draw();
        for( int j = 0; j < systems[i].particleList.size(); j += 230 ) {

            float lifespan = ofClamp( systems[i].particleList[j].lifespan / 255, 0.0, 1.0 );
            flare.begin();
            flare.setUniform2f( "u_resolution", ofGetWidth(), ofGetHeight() );
            flare.setUniform1f( "u_time", ofGetElapsedTimef() );
            flare.setUniform2f( "u_mouse", systems[i].particleList[j].pos.x, systems[i].particleList[j].pos.y );
            flare.setUniform1f( "u_particle_lifespan", lifespan );
            flare.setUniform1f( "u_range", flareBarRange );
            flare.setUniform1f( "u_thickness", flareBarThickness );
            flare.end();

        }
    }
}
