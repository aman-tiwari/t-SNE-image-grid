# t-SNE-image-grid

An openframeworks project that creates a grid of images from a t-SNE projection of ImageNet descriptors.

Run ./setup.sh to download the pretrained ImageNet classifier used to generate the descriptiors.

Expects images in data/images.

Takes a while to start up as it loads the images and then computes the descriptors, un from the console to see all the logging output during this.

Uses:
  * [ofxCcv](https://github.com/kylemcdonald/ofxCcv)
  * [ofxTSNE](https://github.com/genekogan/ofxTSNE)
  * [ofxAssignment](https://github.com/kylemcdonald/ofxAssignment)
