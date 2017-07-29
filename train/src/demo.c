#include "network.h"
#include "detection_layer.h"
#include "region_layer.h"
#include "cost_layer.h"
#include "utils.h"
#include "parser.h"
#include "box.h"
#include "image.h"
#include "demo.h"
#include <sys/time.h>

#define FRAMES 3

#ifdef OPENCV
float *data_svm;
float *data_diff;
int cnt=0;

static char **demo_names;
static image **demo_alphabet;
static int demo_classes;

static float **probs;
static box *boxes;
static network net;
static image in   ;
static image in_s ;
static image det  ;
static image det_s;
static image disp = {0};
static CvCapture * cap;
static float fps = 0;
static float demo_thresh = 0;
static float demo_hier_thresh = .5;

static float *predictions[FRAMES];
static int demo_index = 0;
static image images[FRAMES];
static float *avg;

void *fetch_in_thread(void *ptr)
{
    in = get_image_from_stream(cap);
    if(!in.data){
        error("Stream closed.");
    }
    in_s = resize_image(in, net.w, net.h);
    return 0;
}

void *detect_in_thread(void *ptr)
{
    float nms = .4;

    layer l = net.layers[net.n-1];
    float *X = det_s.data;
    float *prediction = network_predict(net, X);
    ///////////////////////////////////////////修改/////14:36
   /* int count=0;
    while(net.layers[net.n-2].output!=NULL)
    {//printf("%f\n",&net.layers[net.n-2].output);
      net.layers[net.n-2].output++;
      count++;
    }
     printf("count=%d\n",count);
     while(1)
      { printf("层数=%d\n",net.n);
       }
         layer con_l=net.layers[net.n-3];  //最后一层卷积层
int i;
    while(1){printf("ddddd=%d\n",sizeof(l.output));
     for( i=0;i<sizeof(l.output);i++)
     {printf("val=%f\n",&l.output); }
     }
*/

    memcpy(predictions[demo_index], prediction, l.outputs*sizeof(float));
    mean_arrays(predictions, FRAMES, l.outputs, avg);
    l.output = avg;
    free_image(det_s);
    if(l.type == DETECTION){
        get_detection_boxes(l, 1, 1, demo_thresh, probs, boxes, 0);
    } else if (l.type == REGION){
        get_region_boxes(l, 1, 1, demo_thresh, probs, boxes, 0, 0, demo_hier_thresh);
    } else {
        error("Last layer must produce detections\n");
    }

    if (nms > 0) do_nms(boxes, probs, l.w*l.h*l.n, l.classes, nms);
    printf("\033[2J");
    printf("\033[1;1H");
    printf("\nFPS:%.1f\n",fps);
    printf("Objects:");
    printf("检测到物体\n\n");

    images[demo_index] = det;
    det = images[(demo_index + FRAMES/2 + 1)%FRAMES];
    demo_index = (demo_index + 1)%FRAMES;

    draw_detections(det, l.w*l.h*l.n, demo_thresh, boxes, probs, demo_names, demo_alphabet, demo_classes);

    return 0;
}

double get_wall_time()
{
    struct timeval time;
    if (gettimeofday(&time,NULL)){
        return 0;

    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}
void mydemo(char *cfgfile, char *weightfile)
{
    printf("OurDemo is Running\n");
    // shp 19:51
    int i;
       data_svm=(float*)calloc(1690,sizeof(float));
       data_diff=(float*)calloc(1690,sizeof(float));
       
      FILE *fp=fopen("/home/shp/Documents/trainfire.txt","r");
      net = parse_network_cfg(cfgfile);
      if(weightfile)
   	 load_weights(&net, weightfile);
    set_batch_network(&net, 1);
     srand(2222222);
    char s[200];

    int j=0;
    cvNamedWindow("Demo", CV_WINDOW_NORMAL);
    cvMoveWindow("Demo", 0, 0);
    cvResizeWindow("Demo", net.w, net.h);

    if (fp!=NULL)
	{
	  while ( j <= 245)
	  {   j++;
	     fscanf(fp,"%s",s);
	     cnt=0;
                                    /*需要修改testnum=330(47) trainnum=869(43)*/
       		if(j<220)continue;
	       if(j>=246)break;
	     image in,in_s;
	  // stpcpy(s_copy,s);
	      char *sout=s+28;

    	   float *X;
   	   int cnt_for_pick,step_length;
  	   int numframes=0;
  	   //scanf("%d",&numfiles);
            printf("现在处理%s\n",s);
            cap = cvCaptureFromFile(s);
              int v=strlen(sout);
              sout[v-3]='d';
              sout[v-2]='a';
              sout[v-1]='t';
            numframes=(int) cvGetCaptureProperty(cap, CV_CAP_PROP_FRAME_COUNT);
            step_length=numframes/10;
            if(!cap)
                printf("Can't find file %s",s);
            in = get_image_from_stream(cap);
             if(!in.data){
                    error("Stream closed.");
                }
            in_s = letterbox_image(in, net.w, net.h);
           // show_image(in_s, "Demo");
           // cvWaitKey(10);
            cnt_for_pick=0;
            int cnt=1;
            while(in.data)
            {
                if(cnt_for_pick==0)
                {
                    cnt_for_pick=step_length;
                }
                else
                {
                    in = get_image_from_stream(cap);
                    if(!in.data)
                    	break;
                    in_s = letterbox_image(in, net.w, net.h);
                  //  show_image(in_s, "Demo");
                  //  cvWaitKey(10);
                    cnt_for_pick--;
                    continue;
                }
               	if(cnt<=10)
                {
                //	printf("%d\n",cnt);
                	mynetwork_predict(net, in_s.data,sout);
                	cnt++;
                }
                else
                break;
            }
            printf("the video is done in %s\n",sout);
            printf("the %d file is done in %s\n",j,s);

        }
	  printf("处理完成!\n");
	}


    return ;
}

void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int frame_skip, char *prefix, float hier_thresh)
{
mydemo(cfgfile,weightfile);
return ;
/*
    //skip = frame_skip;
    image **alphabet = load_alphabet();
    int delay = frame_skip;
    demo_names = names;
    demo_alphabet = alphabet;
    demo_classes = classes;
    demo_thresh = thresh;
    demo_hier_thresh = hier_thresh;
   // printf("Demo\n");
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);

    srand(2222222);

    if(filename){
        printf("video file: %s\n", filename);
        cap = cvCaptureFromFile(filename);
    }else{
        cap = cvCaptureFromCAM(cam_index);
    }

    if(!cap) error("Couldn't connect to webcam.\n");

    layer l = net.layers[net.n-1];
    int j;

    avg = (float *) calloc(l.outputs, sizeof(float));
    for(j = 0; j < FRAMES; ++j) predictions[j] = (float *) calloc(l.outputs, sizeof(float));
    for(j = 0; j < FRAMES; ++j) images[j] = make_image(1,1,3);

    boxes = (box *)calloc(l.w*l.h*l.n, sizeof(box));
    probs = (float **)calloc(l.w*l.h*l.n, sizeof(float *));
    for(j = 0; j < l.w*l.h*l.n; ++j) probs[j] = (float *)calloc(l.classes, sizeof(float));

    pthread_t fetch_thread;
    pthread_t detect_thread;

    fetch_in_thread(0);
    det = in;
    det_s = in_s;

    fetch_in_thread(0);
    detect_in_thread(0);
    disp = det;
    det = in;
    det_s = in_s;

    for(j = 0; j < FRAMES/2; ++j){
        fetch_in_thread(0);
        detect_in_thread(0);
        disp = det;
        det = in;
        det_s = in_s;
    }

    int count = 0;
    if(!prefix){
      //  cvNamedWindow("Demosss", CV_WINDOW_NORMAL);
      //  cvMoveWindow("Demosss", 0, 0);
       // cvResizeWindow("Demosss", 1352, 1013);
    }

    double before = get_wall_time();

//video detect part
    while(1){
        ++count;
        if(1){
            if(pthread_create(&fetch_thread, 0, fetch_in_thread, 0)) error("Thread creation failed");
            if(pthread_create(&detect_thread, 0, detect_in_thread, 0)) error("Thread creation failed");

            if(!prefix){
                show_image(disp, "Detect Window");
//		printf("debug/n");
                int c = cvWaitKey(1);
                if (c == 10){
                    if(frame_skip == 0) frame_skip = 60;
                    else if(frame_skip == 4) frame_skip = 0;
                    else if(frame_skip == 60) frame_skip = 4;
                    else frame_skip = 0;
                  }
                }
	 else{
                char buff[256];
                sprintf(buff, "%s_%08d", prefix, count);
                 save_image(disp, buff);
            }

            pthread_join(fetch_thread, 0);
            pthread_join(detect_thread, 0);

            if(delay == 0){
                free_image(disp);
                disp  = det;
            }
            det   = in;
            det_s = in_s;
        }else {
            fetch_in_thread(0);
            det   = in;
            det_s = in_s;
            detect_in_thread(0);
            if(delay == 0) {
                free_image(disp);
                disp = det;
            }
            show_image(disp, "Detect Window ");
            cvWaitKey(1);
        }
        --delay;
        if(delay < 0){
            delay = frame_skip;

            double after = get_wall_time();
            float curr = 1./(after - before);
            fps = curr;
            before = after;
        }
    }*/
}
#else
void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int frame_skip, char *prefix, float hier_thresh)
{
    fprintf(stderr, "Demo needs OpenCV for webcam images.\n");
}
#endif
