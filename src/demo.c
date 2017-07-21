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

#define per_len 1690
#define all_len 16900
#define num_support 234
#define sigma 1.0000000000000001e-05
#define bvalue -0.7557037844985270e+00
#ifdef OPENCV

float* data_svm[10];
int cnt2=0;
int flag_ok=0;
static float alpha[num_support];
static float support[num_support][all_len];
static int abnormal_flag=0;
static float res=0;

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
	if(flag_ok)
	{
		int i,j,k;
	  	res=-bvalue;
		for(i=0;i<num_support;i++){
		     float sum=0;
		    for(j=0;j<10;j++)
			    for(k=0;k<per_len;k++)
			    sum+=(data_svm[(cnt2+j+9)%10][k]-support[i][j*per_len+k])*(data_svm[(cnt2+j+9)%10][k]-support[i][j*per_len+k]);
		    sum=exp(sum*(-sigma));
		    res+=alpha[i]*sum;
		}
	    if (res<0)
	    	abnormal_flag=1;
	    else
		abnormal_flag=0;
 	}
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
    printf("%d\n",cnt2);
    if(abnormal_flag==1)
         printf("检测到异常!!!!!\n");
    else
         printf("正常\n");
    printf("%f\n",res);
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

void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int frame_skip, char *prefix, float hier_thresh)
{
    int i,j,k,n;
    image **alphabet = load_alphabet();
    int delay = frame_skip;
    demo_names = names;
    demo_alphabet = alphabet;
    demo_classes = classes;
    demo_thresh = thresh;
    demo_hier_thresh = hier_thresh;
    printf("Demo\n");
    net = parse_network_cfg(cfgfile);
    if(weightfile){
        load_weights(&net, weightfile);
    }
    set_batch_network(&net, 1);
    //pre space
 
    for(i=0;i<10;i++)
        data_svm[i]=(float*)calloc(per_len,sizeof(float));
        
    freopen("/home/shp/darknet/out.txt","r",stdin);
    scanf("%d",&n);
    for(i=0;i<n;i++)
        for(j=0;j<all_len;j++)
        scanf("%f",&support[i][j]);
   
    printf("load is done\n");
    freopen("/home/shp/darknet/alpha.txt","r",stdin);
    for(i=0;i<n;i++)
        scanf("%f",&alpha[i]);

    printf("alpha is done\n");

    srand(2222222);
    if(filename){
        printf("video file: %s\n", filename);
        cap = cvCaptureFromFile(filename);
    }else{
        cap = cvCaptureFromCAM(cam_index);
    }

    if(!cap) error("Couldn't connect to webcam.\n");

    layer l = net.layers[net.n-1];

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
        //cvNamedWindow("Demosss", CV_WINDOW_NORMAL);
        //cvMoveWindow("Demosss", 0, 0);
        //cvResizeWindow("Demosss", 1352, 1013);
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
            if(abnormal_flag)
            ///turn color to red
            {
                for(i=0;i<disp.h;i++)
                    for(j=0;j<disp.w;j++)
                            disp.data[i*disp.w+j*3]=(255.0);
            }
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
    }
}
#else
void demo(char *cfgfile, char *weightfile, float thresh, int cam_index, const char *filename, char **names, int classes, int frame_skip, char *prefix, float hier_thresh)
{
    fprintf(stderr, "Demo needs OpenCV for webcam images.\n");
}
#endif
