#include "ofApp.h"
#define IMG_SIZE 128

//--------------------------------------------------------------
void ofApp::setup(){

    gui.setup();
    gui.add( scale.setup("scale: ", 1, 0.0, 8) );
    //setup ofxCCV
    ofLog() << "loading ccv" << endl;
    ccv.setup("image-net-2012.sqlite3");
    ofLog() << "loaded ccv" << endl;
    
    ofDirectory dir = ofDirectory("images");
    dir.allowExt("jpg");
    
    // Loads n images, where n is the nearest square to the number
    // of available images (so we can get a nice square grid
    ofLog() << "n imgs: " + to_string(dir.listDir());
    int n_images = pow(floor(sqrt(dir.listDir())), 2);
    ofLog() << "nearest square: " + to_string(n_images);
    //load images
    int ii = 0;
    for(int i = 0; i < n_images; i++) {
        if(i%10 == 0) {
            ofLog() << "calculating features for image: " + to_string(i) << endl;
        }
        ofImage temp;
        temp.load(dir.getPath(i));
        if(temp.getHeight() > temp.getWidth()) {
            temp.crop(0, 0, temp.getWidth(), temp.getWidth());
        } else {
            temp.crop(0, 0, temp.getHeight(), temp.getHeight());
        }

        temp.resize(IMG_SIZE, IMG_SIZE);
        images.push_back(temp);
        features.push_back(ccv.encode(temp, ccv.numLayers() - 1));
    }
        
    // the below computes the grid x & y sizes such that the
    // grid is as close to a square as possible
    int n_imgs = images.size();
    
    for(int n = 1; n < ceil(sqrt(n_imgs) + 1); n++) {
        if(n_imgs%n == 0) {
            grid_y = n;
            grid_x = n_imgs/n;
            
        }
    }
    
    ofLog() << "grid x: " + to_string(grid_x) + " grid y: " + to_string(grid_y) << endl;

    ofLog() << "n imgs: " + to_string(images.size()) << endl;
    
    
    int dims = 2;
    float perplexity = 30;
    float theta = 0.5;
    bool normalize = true;
    
    ofLog() << "starting tsne" << endl;
    tsne_points = tsne.run(features, dims, perplexity, theta, normalize, true);

    result.allocate(ofNextPow2(IMG_SIZE * grid_x), ofNextPow2(IMG_SIZE * grid_y), OF_IMAGE_COLOR);

}

//--------------------------------------------------------------
void ofApp::update() {
    iter++;
    if(iter < 1000) {
        tsne_points = tsne.iterate();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if(iter == 1002) {
        for(int i = 0; i < tsne_points.size(); i++) {
            ofVec2f point = ofVec2f(tsne_points[i][0], tsne_points[i][1]);
            tsne_vecs.push_back(ofVec2f(point));
        }
        grid = makeGrid(grid_x, grid_y);
        ofLog() << "grid size: " + to_string(grid_x * grid_y) << endl;
        ofLog() << "solving grid" << endl;
        solved_grid = solver.match(tsne_vecs, grid, false);
        ofLog() << "solved grid" << endl;
    
    } else if (iter > 1002) {
        //saver.begin();
        if(!saved) {
            ofClear(255,255,255, 0);
            ofPixels pix;
            pix.allocate(ofNextPow2(IMG_SIZE * grid_x), ofNextPow2(IMG_SIZE * grid_y), OF_IMAGE_COLOR);
            for(int i = 0; i < solved_grid.size(); i++) {
                images[i].draw(solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x),
                               solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_x-1)/(float)(grid_x),
                               IMG_SIZE,
                               IMG_SIZE);
                images[i].getPixels().pasteInto(pix, solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x), solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_x-1)/(float)(grid_x));
            }
            
            //ofSleepMillis(1000);
            
            result.setFromPixels(pix);
            if(!saved) {
                result.save("result_" + to_string(ofGetUnixTime()) + ".png");
                saved = true;
            }

        }
        if(saved) {
            for(int i = 0; i < solved_grid.size(); i++) {
                images[i].draw(solved_grid[i].x * ofGetWidth() * (float)(grid_x-1)/(float)(grid_x),
                               solved_grid[i].y * ofGetHeight() * (float)(grid_x-1)/(float)(grid_x),
                               ofGetWidth()/(float)grid_x,
                               ofGetHeight()/(float)grid_y);
            }
            
        }
        //saver.end();
    } else {
        for(int i = 0; i < tsne_points.size(); i++) {
            images[i].draw(tsne_points[i][0] * ofGetWidth() * scale,
                           tsne_points[i][1] * ofGetHeight() * scale,
                           50 * scale, 50 * scale);
        }
    }
    //images[1].draw(0,0);

    

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
   /* if(key =='s') {
        saver.finish("frame_" + ofToString(ofGetFrameNum()) + "_high.png", true);
    } */
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
