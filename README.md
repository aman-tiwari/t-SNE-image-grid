# t-SNE-image-grid

An openframeworks project that creates a grid of images from a t-SNE projection of ImageNet descriptors of those images.

Run ```./setup.sh``` to download the pretrained ImageNet classifier used to generate the descriptiors.

Reads settings from the ```data/settings.json``` file:
```
{"image_sets" :
    [
        {"directory":"cool_pics/"},
        {"directory": "nice_pics/"}
    ],
 "n_images" : 20, 
 "draw_tsne": false,
 "dims": 2,
 "perplexity": 35,
 "theta": 0.5,
 "normalize": true}

```
```image_sets``` is a list of directories relative to the settings.json file that contain the images you want to cluster (sub-directories within these are scanned too).

```n_images``` specifices how many images to pick from each image set.

```draw_tsne``` specifies whether to draw the in-progress t-sne to the screen (set to ```true``` if you love hypercubes)

Saves a JSON file containing the descriptors in ```<image_set_directory>/features_4096.json```.

I highly reccomend that you run from the console to see logging output (```cd tSNE_images_gridDebug.app/Contents/MacOS/```, ```./tSNE_images_gridDebug```)

Uses:
  * [ofxCcv](https://github.com/kylemcdonald/ofxCcv)
  * [ofxTSNE](https://github.com/genekogan/ofxTSNE)
  * [ofxAssignment](https://github.com/kylemcdonald/ofxAssignment)
  * [ofxJSON](https://github.com/jefftimesten/ofxJSON)
