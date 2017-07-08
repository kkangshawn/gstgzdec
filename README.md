#gzdec

GStreamer gzip decoder plugin
---------------------------------------------

- using zlib library for decompression
- filter-like element with a sink pad and a source pad
- works on Linux Ubuntu 16.10/Fedora 24(x86_64)

##Build
- Please use autotool (autogen.sh)

##Note for Linux system
- This will generate 2 plugins for GStreamer 1.0 and 0.10 so it requires both devel package to build.
- This also requires zlib static library
```sh
sudo dnf install gstreamer1-1.x.x gstreamer1-devel-1.x.x
sudo dnf install zlib-1.2.x zlib-static-1.2.x
```


##Usage
```sh
gst-launch-1.0 filesrc location=<input_filename.gz> ! gzdec ! filesink location=<output_filename>
```
##Todo
disabled GStreamer 0.10 support temporarily because too many declarations have altered from 0.10
bzip support
