 # Abnormal behavior detection
 ## Introduction
      Detect whether there is any abnormal behavior occurs
    in the video surveillance. It's going to alert when it
    happens.
 ## Author
* [@ShoupingShan](https://github.com/ShoupingShan)

* [@finaldong](https://github.com/finaldong)

## Platform
* [Ubuntu 14.04](https://www.ubuntu.com/download/desktop)
* [OpenCV 2.4.10](http://opencv.org/)
* [CUDA 8.0](https://developer.nvidia.com/cuda-downloads)
* [GTX 850M](https://www.geforce.com/hardware/notebook-gpus/geforce-gtx-850m)
* [YOLO V2](https://pjreddie.com/darknet/yolo/)
## Effect
![one](http://img.027cgb.cn/20170721/20177211871775731906.png)

![two](http://img.027cgb.cn/20170721/2017721151775731906.png)
## How to run

1. git clone git@github.com:ShoupingShan/Abnormal-behavior-Detection.git
2. cd Abnormal-behavior-Detection
3. make
4. wget https://pjreddie.com/media/files/yolo.weights
5. ./darknet detector demo cfg/coco.data cfg/yolo.cfg yolo.weights -c 0

## Attention
  > Makefile

  > This is what I use, uncomment if you know your arch and want to specify

  > ARCH=  -gencode arch=compute_50,code=compute_50

## Contact Us

  *shp395210@outlook.com*
