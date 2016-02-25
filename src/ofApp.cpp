#include "ofApp.h"
#define IMG_SIZE 128
#define FEATURE_VEC_LEN 4096

const string allowed_ext[] = {"jpg", "png", "gif", "jpeg"};

void ofApp::scan_dir_imgs(ofDirectory dir, vector<ofFile>& image_files)

{
    int i, size;
    ofDirectory new_dir;
    
    size = dir.listDir();

    for (i = 0; i < size; i++) {
        
        if (dir.getFile(i).isDirectory()) {
            
            new_dir = ofDirectory(dir.getFile(i).getAbsolutePath());
            new_dir.listDir();
            new_dir.sort();
            scan_dir_imgs(new_dir, image_files);
            
        } else if (std::find(std::begin(allowed_ext),
                             std::end(allowed_ext),
                             dir.getFile(i).getExtension()) != std::end(allowed_ext)) {
            
            image_files.push_back(dir.getFile(i));
            
        }
    }
}
//--------------------------------------------------------------
void ofApp::setup() {

    //setup ofxCCV
    ofLog() << "loading ccv" << endl;
    ccv.setup("image-net-2012.sqlite3");
    ofLog() << "loaded ccv" << endl;
    
    
    std:vector<ofDirectory> image_dirs;
    
    ofFile settings_file("settings.json");
    
    ofxJSONElement settings_json;
    int n_images = 0;
    
    if(settings_file.exists()
       && settings_json.open(settings_file.getAbsolutePath())) {
        
        ofLog() << "successfully opened settings.json" << endl;
        
        Json::Value image_sets = settings_json["image_sets"];
        
        ofLog() << "image directories: " << endl;
        
        //Calcuates total number of images
        for(int i = 0; i < image_sets.size(); i++) {
            ofLog() << image_sets[i]["directory"].asString() << endl;
            image_dirs.push_back(ofDirectory(image_sets[i]["directory"].asString()));
            // only doing this to work out number of images we have
            vector<ofFile> temp_vec;
            scan_dir_imgs(image_dirs[i], temp_vec);
            n_images += temp_vec.size();
        }
        
        if(isdigit(settings_json["n_images"].asString()[0])) {
            n_images = settings_json["n_images"].asInt();
        }
        
        
        DRAW_TSNE = settings_json["draw_tsne"].asBool();
        
        dims = settings_json["dims"].asInt();
        perplexity = settings_json["perplexity"].asFloat();
        theta = settings_json["theta"].asFloat();
        normalize = settings_json["normalize"].asBool();
        
    } else {
        
        ofLog() << "err: settings.json not found!" << endl;
        ofBaseApp::exit();
        
    }
    
    // Loads n images, where n is the nearest square to the number
    // of available images (so we can get a nice square grid
    ofLog() << "n imgs: " + to_string(n_images);
    ofLog() << "nearest square: " + to_string(n_images);
    
    
    //load features & images
    
    for(auto image_dir : image_dirs) {
        
        ofLog() << "loadings features & images from: " + image_dir.path() << endl;
        
        vector< vector<float> > temp_features;

        
        // Loads features
        
        //Finds features file
        ofFile features_file(image_dir.path() + "features_" + to_string(FEATURE_VEC_LEN) + ".json", ofFile::ReadWrite);

        if(features_file.exists() && features_json.open(features_file.getAbsolutePath())) {
            ofLog() << "features file exists" << endl;
            ofLog() << "successfully opened " + features_file.path() << endl;
            
            int n_features = features_json["n_features"].asInt();
            int feature_size = features_json["feature_size"].asInt();
            
            if(n_images < n_features) {
                n_features = n_images;
            }
            
            Json::Value features_data = features_json["features"];
            
            //Loads features from features JSON
            for(int i = 0; i < n_features; i++) {
                vector<float> feature;
                Json::Value feature_data = features_data[i];
                for(int j = 0; j < feature_size; j++) {
                    feature.push_back(stof(feature_data[j].asString()));
                }
                temp_features.push_back(feature);
            }
            
            features_file.close();
        } else {
            resave_features = true;
        }
        ofLog() << "features read successfully: " + to_string(temp_features.size()) << endl;
        ofLog() << "loading image paths from dir: "  + image_dir.path() << endl;
        
        vector<ofFile> image_files;
        
        //Loads directory of images into image_files vector
        scan_dir_imgs(image_dir, image_files);
        ofLog() << image_files.size();
        ofLog() << n_images;
        if(image_files.size() < n_images) {
            ofLog() << "error: not enough images for n_images setting" << endl;
        }
        //Loads images into global images vector
        ofLog() << "loadings images" << endl;
        for(int i = 0; i < n_images; i++) {
            ofImage temp;
            temp.load(image_files[i]);
            if(temp.getHeight() > temp.getWidth()) {
                temp.crop(0,
                          (temp.getHeight() - temp.getWidth())/2,
                          temp.getWidth(),
                          temp.getWidth() + (temp.getHeight() - temp.getWidth())/2);
            } else {
                temp.crop((temp.getWidth() - temp.getHeight())/2,
                          0,
                          temp.getHeight() + (temp.getWidth() - temp.getHeight())/2,
                          temp.getHeight());
            }
            
            temp.resize(IMG_SIZE, IMG_SIZE);
            images.push_back(temp);
            if(i >= temp_features.size()) {
                if(i%10 == 0) {
                    ofLog() << "calculating features for image: " + to_string(i) << endl;
                }
                temp_features.push_back(ccv.encode(temp, ccv.numLayers() - 1));
                resave_features = true;
                
            }
        }
        
        //Rewrites features.json if some features have been recalculated
        if(resave_features) {
            save_features(temp_features, features_file);
        }
        
        features.insert(features.end(), temp_features.begin(), temp_features.end());
    }
    
    // the below computes the grid x & y sizes such that the
    // grid is as close to a square as possible
    
    for(int n = 1; n < ceil(sqrt(images.size()) + 1); n++) {
        if(images.size()%n == 0) {
            grid_y = n;
            grid_x = images.size()/n;
            
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

void ofApp::save_features(vector<vector<float> > temp_features, ofFile features_file) {
    ofLog() << "resaving " + features_file.path() << endl;
    ofLog() << "saving features" << endl;
    ofFile new_features_file(features_file, ofFile::WriteOnly);
    if(!new_features_file.exists()) {
        new_features_file.create();
    }
    Json::Value n_features(Json::uintValue);
    n_features = (uint)temp_features.size();
    
    Json::Value feature_size(Json::uintValue);
    feature_size = (uint)temp_features[0].size();
    Json::Value features_vec(Json::arrayValue);
    
    for(int ii = 0; ii < temp_features.size(); ii++) {
        
        Json::Value feature_vec;
        for(int jj = 0; jj < temp_features[ii].size(); jj++) {
            Json::Value temp(Json::realValue);
            temp = temp_features[ii][jj];
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
//--------------------------------------------------------------
void ofApp::update() {
    iter++;
    if(DRAW_TSNE) {
        if(iter < 1000) {
            tsne_points = tsne.iterate();
        }
    }
    
}

bool compare_x(ofVec2f p_1, ofVec2f p_2) {
    return (p_1.x < p_2.x);
}
bool compare_y(ofVec2f p_1, ofVec2f p_2) {
    return (p_1.y < p_2.y);
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
            ofLog() << "allocating mat" << endl;
            
            Mat res_mat(IMG_SIZE * grid_y, IMG_SIZE * grid_x, CV_8UC3);

            //ofFbo fbo;
            //fbo.allocate(IMG_SIZE * grid_x, IMG_SIZE * grid_y, OF_IMAGE_COLOR);

            //fbo.clear();
            ofLog() << "allocation successful" << endl;
        /*
            cv::Mat res_mat = Mat(IMG_SIZE * grid_y, IMG_SIZE * grid_x, CV_8UC3);
            unsigned char* char_pix = (unsigned char*)std::malloc(sizeof(unsigned char) * 128 * 128 * grid_x * grid_y * 3);
            
            std::stable_sort(solved_grid.begin(), solved_grid.end(), compare_x);
            std::stable_sort(solved_grid.begin(), solved_grid.end(), compare_y); */
            fbo.begin();
            for(int i = 0; i < solved_grid.size(); i++) {
                
                /*ofPixels img_pixels = images[i].getPixels();
                ofLog() << img_pixels.getNumChannels();
                unsigned char* raw_data = img_pixels.getData();
                
                //top left corner of img
                int width = (IMG_SIZE * grid_x) * 3;
                int heigth = (IMG_SIZE * grid_y);
                int cor_x = solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x) * 3;
                int cor_y = solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_y-1)/(float)(grid_y);
                
                for(int yy = 0; yy < 128; yy++) {
                    int start_i = (cor_y + yy) * width + cor_x;
                    for(int xx = 0; xx < 128 * 3; xx++) {
                        char_pix[start_i + xx] = raw_data[yy * 128 + xx];
                    }
                }*/
                /*images[i].getPixels().pasteInto(pix,
                                                solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x),
                                                solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_y-1)/(float)(grid_y));*/
                
                /*images[i].draw(solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x),
                               solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_y-1)/(float)(grid_y));*/
                Mat temp = toCv(images[i]);
                temp.copyTo(res_mat(cv::Rect(solved_grid[i].x * (IMG_SIZE * grid_x) * (float)(grid_x-1)/(float)(grid_x),
                                          solved_grid[i].y * (IMG_SIZE * grid_y) * (float)(grid_y-1)/(float)(grid_y),
                                          temp.cols,
                                          temp.rows)));
                
            }
            /*ofFile rgba_file("result_b_" + to_string(ofGetUnixTime()) + ".rgb", ofFile::WriteOnly);
            rgba_file.write((char*)char_pix, sizeof(unsigned char) * 128 * 128 * grid_x * grid_y * 3);
            rgba_file.close();
            std::free(char_pix);
            //ofSleepMillis(1000);*/
            //ofImage res;
            //res.grabScreen(0, 0, (IMG_SIZE * grid_x), (IMG_SIZE * grid_y));
            //fbo.end();
            //result.setFromPixels(pix);
            ofLog() << "paste successful" << endl;
     //       saveMat(res_mat, "result_L_" + to_string(ofGetUnixTime()) + ".rgb");
     //       ofLog() << "saved mat" << endl;
            
            //ofPixels bug causes it to crash if you allocate a pixel array larger than 8192*8192, so we have to
            //save as a raw binary image and use imagemagick to convert it to something viewable
            if(grid_x * 128 > 8192 || grid_y * 128 > 8192) {
                
                ofLog() << "saving as a raw .rgb image, use imagemagick to convert" << endl;
                ofFile res_img("result_L_" + to_string(ofGetUnixTime())
                               + "_" + to_string(grid_x * 128)
                               + "_" + to_string(grid_y * 128)
                               + ".rgb", ofFile::WriteOnly);
                
                uint8_t* pixelPtr = (uint8_t*)res_mat.data;
                int cn = res_mat.channels();

                for(int i = 0; i < res_mat.rows; i++)
                {
                    for(int j = 0; j < res_mat.cols; j++)
                    {
                        res_img << (char)pixelPtr[i*res_mat.cols*cn + j*cn + 0];
                        res_img << (char)pixelPtr[i*res_mat.cols*cn + j*cn + 1];
                        res_img << (char)pixelPtr[i*res_mat.cols*cn + j*cn + 2];

                        // do something with BGR values...
                    }
                }
                
                res_img.close();
            } else {
                ofLog() << "saving as a png" << endl;
                saveImage(res_mat, "result_L_" + to_string(ofGetUnixTime()) + ".png");
            }
            //ofSaveImage(res, "result_L_" + to_string(ofGetUnixTime()) + ".jpeg", OF_IMAGE_QUALITY_BEST);
            ofLog() << "image save successful" << endl;

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
