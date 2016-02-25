#pragma once

#include "ofMain.h"
#include "ofxCcv.h"
#include "ofxTSNE.h"
#include "ofxAssignment.h"
#include "ofxJSON.h"
#include "ofxCv.h"
using namespace cv;
using namespace ofxCv;

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
    
        void exit();
    
    void scan_dir_imgs(ofDirectory dir, vector<ofFile>& image_files);
    void save_features(vector<vector<float> > features, ofFile features_file);
    
    bool saved = false;
    bool DRAW_TSNE = false;
    ofxAssignment solver;
    ofxTSNE tsne;
    ofxCcv ccv;
    vector<ofFile> image_files;
    vector<ofImage> images;
    vector<vector<float> > features;
    vector<vector<double>> tsne_points;
    vector<ofVec2f> tsne_vecs;
    
    vector<ofVec2f> grid;
    vector<ofVec2f> solved_grid;

    
    int dims = 0;
    float perplexity = 0;
    float theta = 0;
    bool normalize = true;

    int iter = 0;
    int grid_x = 0;
    int grid_y = 0;
    
    ofVec3f cam_target;
    
    ofImage result;
    ofFbo fbo;

    ofxJSONElement features_json;
    bool resave_features = false;
    bool features_saved = false;

};
