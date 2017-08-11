# gzdec

GStreamer gzip decoder plugin v1.2
---------------------------------------------

- using zlib library for decompression
- filter-like element with a sink pad and a source pad
- works on Linux Ubuntu 16.10, 16.04 LTS, Fedora 24(x86_64)
- supports both GStreamer version 1.0 and 0.10

## Build
- Please use autotool (autogen.sh)

## Note for Linux system
- This will generate 2 plugins for GStreamer 1.0 and 0.10 so it requires both devel package to build.
- This also requires zlib library

[Ubuntu]
```sh
sudo apt-get install gstreamer1.0-* gstreamer0.10-*
sudo apt-get install zlib1g zlib1g-dev
```

[Fedora]
```sh
sudo dnf install gstreamer1-1.x.x gstreamer1-devel-1.x.x
sudo dnf install zlib-1.2.x zlib-static-1.2.x
```

## Usage
```sh
gst-launch filesrc location=<input_filename.gz> ! gzdec ! filesink location=<output_filename>
```
## Todo
- bzip support
