#pragma once

#include "ofMain.h"
#include "ofxCcv.h"
#include "ofxTSNE.h"
#include "ofxGui.h"
#include "ofxAssignment.h"

class ofApp : public ofBaseApp {

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
    
    ofxAssignment solver;
    ofxPanel gui;
    ofxFloatSlider scale;
    ofxTSNE tsne;
    ofxCcv ccv;
    vector<ofImage> images;
    vector<vector<float>> features;
    vector<vector<double>> tsne_points;
    vector<ofVec2f> tsne_vecs;
    
    vector<ofVec2f> grid;
    vector<ofVec2f> solved_grid;
    ofEasyCam cam;
    int iter = 0;
    int grid_x = 0;
    int grid_y = 0;
    
};
