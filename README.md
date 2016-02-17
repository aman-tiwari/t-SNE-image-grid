# t-SNE-image-grid

An openframeworks project that creates a grid of images from a t-SNE projection of ImageNet descriptors.

Run ```./setup.sh``` to download the pretrained ImageNet classifier used to generate the descriptiors.

Expects images to be in/in subdirectories within ```data/images``` (recurses into subdirectories).
Saves a JSON file containing the descriptors in ```images/features_4096.json```.

Optionally reads settings from the ```settings.json``` file:
```
{ "n_images": 4096, 
  "draw_tsne": false, 
  "dims": 2,
  "perplexity": 30,
  "theta": 0.5,
  "normalize": true }
```

Any non-integer value for ```n_images``` loads all the available images.

Run from the console to see logging output (```cd tSNE_images_gridDebug.app/Contents/MacOS/```, ```./tStSNE_images_gridDebug```)

Uses:
  * [ofxCcv](https://github.com/kylemcdonald/ofxCcv)
  * [ofxTSNE](https://github.com/genekogan/ofxTSNE)
  * [ofxAssignment](https://github.com/kylemcdonald/ofxAssignment)
  * [ofxJSON](https://github.com/jefftimesten/ofxJSON)
