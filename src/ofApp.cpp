#include "ofApp.h"
#define IMG_SIZE 128
#define FEATURE_VEC_LEN 4096

//--------------------------------------------------------------
void ofApp::setup(){

    //setup ofxCCV
    ofLog() << "loading ccv" << endl;
    ccv.setup("image-net-2012.sqlite3");
    ofLog() << "loaded ccv" << endl;
    
    ofDirectory dir = ofDirectory("images/");
    dir.allowExt("jpg");
    
    ofFile settings_file("settings.json");
    ofxJSONElement settings_json;
    int n_images = 0;
    if(settings_file.exists() && settings_json.open(settings_file.getAbsolutePath())) {
        
        ofLog() << "successfully opened settings.json" << endl;
        if(isdigit(settings_json["n_images"].asString()[0])) {
            n_images = settings_json["n_images"].asInt();
            dir.listDir();
        } else {
            n_images = pow(floor(sqrt(dir.listDir())), 2);
        }
        
        DRAW_TSNE = settings_json["draw_tsne"].asBool();
        
        dims = settings_json["dims"].asInt();
        perplexity = settings_json["perplexity"].asFloat();
        theta = settings_json["theta"].asFloat();
        normalize = settings_json["normalize"].asBool();
        
    } else {
        //TSNE settings
        dims = 2;
        perplexity = 30;
        theta = 0.5;
        normalize = true;
        
        n_images = pow(floor(sqrt(dir.listDir())), 2);
        DRAW_TSNE = false;
    }
    // Loads n images, where n is the nearest square to the number
    // of available images (so we can get a nice square grid
    ofLog() << "n imgs: " + to_string(n_images);
    ofLog() << "nearest square: " + to_string(n_images);
    //load images
    ofFile features_file("images/features_" + to_string(FEATURE_VEC_LEN) + ".json", ofFile::ReadWrite);

    if(features_file.exists() && features_json.open(features_file.getAbsolutePath())) {
        ofLog() << "features file exists" << endl;
        ofLog() << "successfully opened features_4096.json" << endl;
        
        int n_features = features_json["n_features"].asInt();
        int feature_size = features_json["feature_size"].asInt();
        
        if(n_images < n_features) {
            n_features = n_images;
        }
        
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
        
    } else {
        resave_features = true;
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
        if(!new_features_file.exists()) {
            new_features_file.create();
        }
        Json::Value n_features(Json::uintValue);
        n_features = (uint)features.size();
        
        Json::Value feature_size(Json::uintValue);
        feature_size = (uint)features[0].size();
        Json::Value features_vec(Json::arrayValue);
        
        for(int ii = 0; ii < features.size(); ii++) {
            
            Json::Value feature_vec;
            for(int jj = 0; jj < features[ii].size(); jj++) {
                Json::Value temp(Json::realValue);
                temp = features[ii][jj];
                feature_vec.append(temp);
            }
            features_vec.append(feature_vec);
        }
        features_json["n_features"] = n_features;
        features_json["feature_size"] = feature_size;
        features_json["features"] = features_vec;
        Json::FastWriter writer;
        new_features_file << writer.write(features_json);
        new_features_file.close();
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
            ofSaveImage(pix, "result_" + to_string(ofGetUnixTime()) + ".jpeg", OF_IMAGE_QUALITY_HIGH);
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
            images[i].draw(tsne_points[i][0] * ofGetWidth(),
                           tsne_points[i][1] * ofGetHeight(),
                           50, 50);
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
