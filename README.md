# t-SNE-image-grid

An openframeworks project that creates a grid of images from a t-SNE projection of ImageNet descriptors.

Run ./setup.sh to download the pretrained ImageNet classifier to generate the imagenet descriptiors.

For now you need to manually edit ofApp.cpp to edit the number of images it uses & update the location of image files relative to the data folder (generated when first built). I'll make this better soon.

Uses:
  * [ofxCcv](https://github.com/kylemcdonald/ofxCcv)
  * [ofxTSNE](https://github.com/genekogan/ofxTSNE)
  * [ofxAssignment](https://github.com/kylemcdonald/ofxAssignment)
