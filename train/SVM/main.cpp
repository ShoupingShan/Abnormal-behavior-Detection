#include <iostream>
#include"opencv2/opencv.hpp"
#include"opencv2/core/core.hpp"
#include"opencv2/imgproc/imgproc.hpp"
#include"opencv2/highgui/highgui.hpp"
#include"opencv2/ml/ml.hpp"
#include<cstdlib>
#include<stdio.h>
#include<string.h>
#include <sys/time.h>
#define TESTNUM 0      ///测试样本数
#define TRAINNUM 245   ///训练样本数
#define TYPENUM 2      ///类别
#define vec_len 16900


//float alpha[num_support];
//float support[num_support][all_len];
//int abnormal_flag=1;


using namespace std;
using namespace cv;
struct fromfile{
 float img[vec_len];
 float type;
};
//float sample[2][3];
float trainLabels[TRAINNUM];
float testLabels[TESTNUM];
fromfile f[TRAINNUM];
fromfile m[TESTNUM];
Mat train[TRAINNUM];  ///训练数据
Mat test[TESTNUM];   ///测试数据
Mat trainmat;         ///训练矩阵
Mat trainLabel;       ///训练标签
Mat testLabel;        ///测试标签


Mat mergeRows(Mat A, Mat B)
{
	int totalRows;
	if (A.cols == B.cols&&A.type() == B.type())
	{
		totalRows = A.rows + B.rows;
	}
	else
	{
		std::cout << "Error 维数不匹配!" << std::endl;
	}
	Mat mergedDescriptors(totalRows, A.cols, A.type());
	Mat submat = mergedDescriptors.rowRange(0, A.rows);
	A.copyTo(submat);
	submat = mergedDescriptors.rowRange(A.rows, totalRows);
	B.copyTo(submat);
	return mergedDescriptors;
}


int main(int argc,char**argv)
{

    char s[200];
    cout << CV_VERSION<<endl;
    ///读取训练样本
    FILE *fp=fopen("dir.txt","r");
    int i=0,j=0;
    while (fp!=NULL&&j<TRAINNUM)
	{
        fscanf(fp,"%s",s);
	    if(s[0]=='f'){trainLabels[j]=(float)1.0;f[j].type=1;}
        else{trainLabels[j]=(float)0.0;f[j].type=0;}
        freopen(s,"r",stdin);
        i=0;
        while(scanf("%f",&f[j].img[i])!=EOF&&i<vec_len)///如果数据超出不读
        {
            i++;
        }
        for(;i<vec_len;i++)
            f[j].img[i]=0.00001;///如果数据不足补全
        j++;
	}
    for(i=0;i<TRAINNUM;i++)
    {
     train[i]=Mat(1,vec_len,CV_32FC1,f[i].img);
    }
    trainLabel=Mat(TRAINNUM,1,CV_32FC1,trainLabels);
    ///读取测试样本
    FILE *fp2=fopen("test.txt","r");
    i=0,j=0;
    while (fp2!=NULL&&j<TESTNUM)
	{
        fscanf(fp2,"%s",s);
	    if(s[0]=='f'){testLabels[j]=(float)1.0;m[j].type=1;}
        else{testLabels[j]=(float)0.0;m[j].type=0;}
        freopen(s,"r",stdin);
        i=0;
        while(scanf("%f",&m[j].img[i])!=EOF&&i<vec_len)///如果数据超出不读
        {
            i++;
        }
        for(;i<vec_len;i++)
            m[j].img[i]=0.00001;///如果数据不足补全
        j++;
	}
    for(i=0;i<TESTNUM;i++)
    {
     test[i]=Mat(1,vec_len,CV_32FC1,m[i].img);
    }
    /*************************合成训练矩阵********************************/
   trainmat=train[0].clone();
    for (i = 1; i < TRAINNUM; i++)
			{
			trainmat = mergeRows(trainmat, train[i]);
			//printf("%d\n",i);
			}
    printf("trainmat: %d %d\n",trainmat.rows,trainmat.cols);
    printf("train:    %d %d\n",trainLabel.rows,trainLabel.cols);

    /* testmat=train[N-100].clone();
    for (i = N-100; i < N; i++)
			{
			testmat = mergeRows(testmat, train[i]);
			printf("%d\n",i);
			}
    */

    //freopen("out.txt","w",stdout);
    //cout<<trainmat<<endl;

    /*int i,j;
    for(i=0;i<2;i++)
    for(j=0;j<3;j++)
    cin>>sample[i][j];
    for(i=0;i<2;i++)
    cin>>labels[i];
    Mat tr=Mat(2,3,CV_32FC1,sample);
    Mat res=Mat(2,1,CV_32FC1,labels);

        PCA pca=PCA(tr,Mat(),CV_PCA_DATA_AS_ROW,(double)0.8);
        Mat cl=pca.project(tr);
        cout<<cl<<endl;
    */
    CvSVMParams params=CvSVMParams();
    params.svm_type=SVM::C_SVC;
    params.kernel_type=SVM::RBF;
    params.gamma=2.2500000000000003e-03;
    params.C=1;
    params.term_crit=cvTermCriteria(CV_TERMCRIT_EPS,100000,0.000001);

    CvSVM SVM;

    SVM.train_auto(trainmat, trainLabel,Mat(),Mat(),params,10);
    SVM.save("trainfire.xml");

    ///SVM.train(tr,res,Mat(),Mat(),params);
    /*int c = SVM.get_support_vector_count();
    cout<<endl;
    for (int i=0; i<c; i++)
    {
      const float* v = SVM.get_support_vector(i);
      cout<<*v<<" ";
    }
    cout<<endl;
    cout << "Over" << endl;*/
    /*int cnt=0,n;
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

    for(k=0;k<TESTNUM;k++){
    float res=-bvalue;
        for(i=0;i<num_support;i++){
             float sum=0;
            for(j=0;j<all_len;j++)
                sum+=(m[k].img[j]-support[i][j])*(m[k].img[j]-support[i][j]);
            //printf("%f\n",sum);
            sum=exp(-sum*sigma);
        res+=alpha[i]*sum;
        }
        if(res>0)
        abnormal_flag=0;
        else
        abnormal_flag=1;

        printf("%f  %d   %f\n",res,abnormal_flag,testLabels[k]);
        if(abs(abnormal_flag-testLabels[k])<0.000001)
        cnt++;

	}
	float ans=(float)cnt/(float)TESTNUM;
    printf("right rate: %f \n",ans);
*/

    int cnt=0;
    for(i=0;i<TESTNUM;i++)
    {
        float res=-1.0;
        res=SVM.predict(test[i]);
        printf("%f %f\n",res,testLabels[i]);
        if(abs(res-testLabels[i])<0.000001)
            cnt++;
    }
   //float ans=(float)cnt/(float)TESTNUM;
   //printf("right rate: %f \n",ans);
    freopen("out.txt","w",stdout);
    int c = SVM.get_support_vector_count();
    printf("support vector is %d\n",c);
    for(i=0;i<c;i++)
    {
      const float* v = SVM.get_support_vector(i);
      for(j=0;j<vec_len;j++)
      printf("%f ",v[j]);
      printf("\n");
    }
    return 0;
}
