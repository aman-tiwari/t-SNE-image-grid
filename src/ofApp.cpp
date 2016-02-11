#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){

    gui.setup();
    gui.add( scale.setup("scale: ", 1, 0.0, 8) );
    //setup ofxCCV
    ofLog() << "loading ccv" << endl;
    ccv.setup("image-net-2012.sqlite3");
    ofLog() << "loaded ccv" << endl;
    
    //load images
    int ii = 0;
    while(images.size() < 841 && ii < 2000) {
        if(ii%100 == 0) {
            ofLog() << "loading image: " + to_string(ii) << endl;
        }
        if(ofFile::doesFileExist("00000/" + ofToString(ii, 7,'0') + ".jpg", true)) {
            ofImage temp;
            temp.load("00000/" + ofToString(ii, 7,'0') + ".jpg");
            
            images.push_back(temp);
        }
        ii++;
    }
    
    int n_imgs = images.size();
    
    for(int n = 1; n < ceil(sqrt(n_imgs) + 1); n++) {
        if(n_imgs%n == 0) {
            grid_y = n;
            grid_x = n_imgs/n;
            
        }
    }
    
    ofLog() << "grid x: " + to_string(grid_x) + " grid y: " + to_string(grid_y) << endl;

    ofLog() << to_string(images.size()) << endl;
    
    ofLog() << "calculating features" << endl;
    
    for(int i = 0; i < images.size(); i++) {
        if(i % 20 == 0) {
            ofLog() << "Calculated features for: " + to_string(i) + "th image" << endl;
        }
        features.push_back(ccv.encode(images[i], ccv.numLayers() - 1));
    }
    
    ofLog() << "calcuated features, n: " + to_string(features.size())  << endl;
    
    int dims = 2;
    float perplexity = 30;
    float theta = 0.5;
    bool normalize = true;
    
    ofLog() << "starting tsne" << endl;
    tsne_points = tsne.run(features, dims, perplexity, theta, normalize, true);
}

//--------------------------------------------------------------
void ofApp::update() {
    iter++;
    tsne_points = tsne.iterate();
    tsne_vecs.clear();
    for(auto point : tsne_points) {
        tsne_vecs.push_back(ofVec2f(point[0], point[1]));
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    gui.draw();
 //   cam.begin();
    if(iter == 1002) {
        grid = makeGrid(grid_x, grid_y);
        ofLog() << "grid size: " + to_string(grid_x * grid_y) << endl;
        ofLog() << "solving grid" << endl;
        solved_grid = solver.match(tsne_vecs, grid);
        ofLog() << "solved grid" << endl;
        
    } else if (iter > 1002) {
        for(int i = 0; i < solved_grid.size(); i++) {
            images[i].draw(solved_grid[i].x * ofGetWidth() * scale,
                           solved_grid[i].y * ofGetHeight() * scale,
                           ofGetWidth()/grid_x * scale, ofGetHeight()/grid_y * scale);
        }
    } else {
        for(int i = 0; i < tsne_vecs.size(); i++) {
            images[i].draw(tsne_vecs[i][0] * ofGetWidth() * scale,
                           tsne_vecs[i][1] * ofGetHeight() * scale,
                           50 * scale, 50 * scale);
        }
    }
  //  cam.end();
    //images[1].draw(0,0);
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

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
