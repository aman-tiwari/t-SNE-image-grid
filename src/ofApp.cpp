#include "ofApp.h"
#define IMG_SIZE 128
#define FEATURE_VEC_LEN 4096
#define DRAW_TSNE false

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
    ofFile features_file("images/features_" + to_string(FEATURE_VEC_LEN) + ".json", ofFile::ReadWrite);
    int start_n = 0;
    if(features_file.exists() && features_json.open(features_file.getAbsolutePath())) {
        ofLog() << "features file exists" << endl;
        ofLog() << "successfully opened file" << endl;
        int n_features = features_json["n_features"].asInt();
        int feature_size = features_json["feature_size"].asInt();
        
        Json::Value features_data = features_json["features"];
        
        for(int i = 0; i < n_features; i++) {
            vector<float> feature;
            Json::Value feature_data = features_data[i];
            for(int j = 0; j < feature_size; j++) {
                feature.push_back(stof(feature_data[j].asString()));
            }
            features.push_back(feature);
        }
        features_file.close();
        ofLog() << "features read successfully: " + to_string(features.size()) << endl;
        
        /*if(n_images > features.size()) {
            int diff = n_images - features.size();
            ofLog() << "more images than features, computing remaining " + to_string(diff) << endl;
            start_n = features.size();
        } else if(features.size() > n_images) {
            ofLog() << "more features than images, using only first " + to_string(features.size()) + " images" << endl;
            n_images = features.size();
        }*/
        
    }

    features_file.close();
    for(int i = 0; i < n_images; i++) {
        ofImage temp;
        temp.load(dir.getPath(i));
        if(temp.getHeight() > temp.getWidth()) {
            temp.crop(0, 0, temp.getWidth(), temp.getWidth());
        } else {
            temp.crop(0, 0, temp.getHeight(), temp.getHeight());
        }
        
        temp.resize(IMG_SIZE, IMG_SIZE);
        images.push_back(temp);
        if(i >= features.size()) {
            if(i%10 == 0) {
                ofLog() << "calculating features for image: " + to_string(i) << endl;
            }
            features.push_back(ccv.encode(temp, ccv.numLayers() - 1));
            resave_features = true;
            
        }
    }
    
    if(resave_features) {
        string file_to_del = features_file.getAbsolutePath() + "images/features_" + to_string(FEATURE_VEC_LEN) + ".json";
        ofLog() << remove(file_to_del.c_str());
        ofLog() << "saving features" << endl;
        ofFile new_features_file("images/features_" + to_string(FEATURE_VEC_LEN) + ".json", ofFile::ReadWrite);
        
        features_saved = true;
    }
    // the below computes the grid x & y sizes such that the
    // grid is as close to a square as possible
    
    for(int n = 1; n < ceil(sqrt(n_images) + 1); n++) {
        if(n_images%n == 0) {
            grid_y = n;
            grid_x = n_images/n;
            
        }
    }
    
    ofLog() << "grid x: " + to_string(grid_x) + " grid y: " + to_string(grid_y) << endl;

    ofLog() << "n imgs: " + to_string(images.size()) << endl;
    
    
    int dims = 2;
    float perplexity = 30;
    float theta = 0.5;
    bool normalize = true;
    
    ofLog() << "starting tsne" << endl;
    tsne_points = tsne.run(features, dims, perplexity, theta, normalize, DRAW_TSNE);

    //result.allocate(ofNextPow2(IMG_SIZE * grid_x), ofNextPow2(IMG_SIZE * grid_y), OF_IMAGE_COLOR);
    
    if(!DRAW_TSNE) {
        iter = 999;
    }
}


void ofApp::exit() {
}


//--------------------------------------------------------------
void ofApp::update() {
    iter++;
    if(DRAW_TSNE) {
        if(iter < 1000) {
            tsne_points = tsne.iterate();
        }
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
            ofLog() << "attempting to save image" << endl;
            ofClear(255,255,255, 0);
            ofLog() << "allocating pixels" << endl;
            ofPixels pix;
            pix.allocate(IMG_SIZE * grid_x, IMG_SIZE * grid_y, OF_IMAGE_COLOR);
            ofLog() << "allocation successful" << endl;

            for(int i = 0; i < solved_grid.size(); i++) {
                images[i].getPixels().pasteInto(pix,
                                                solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x),
                                                solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_y-1)/(float)(grid_y));
            }
            
            //ofSleepMillis(1000);
            
            //result.setFromPixels(pix);
            ofLog() << "paste successful" << endl;
            ofSaveImage(pix, "result_" + to_string(ofGetUnixTime()) + ".png", OF_IMAGE_QUALITY_HIGH);
            ofLog() << "save successful" << endl;

            //result.save("result_" + to_string(ofGetUnixTime()) + ".png");
            saved = true;

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
    } else if(DRAW_TSNE) {
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
