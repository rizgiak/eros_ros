//============================================================================
// Name        : gy_stream.cpp
// Author      : khilmi@eros
// Version     : 8.0
// Copyright   : EROS Re-BORN
// Description : We Must Win This Competition, No Matter What !!
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <iostream>
#include <fstream>
#include <math.h>

#include "ros/ros.h"
#include <robotcontrol/LowLevel.h>
#include <std_msgs/Int8.h>
//#include <sys/time.h> //Timelapsed stamp header, uncomment if you want to use this

#define round(a)(int)(a+0.5)
#define MAX_DATA 7
using namespace std;

int fdGrk = 0;
uint8_t serialIn[MAX_DATA];											//RAW data from SubController
char USBTX[100],system_check[100];
robotcontrol::LowLevel llData;
//bool flaggettime;													//Uncomment this for Time Lapsed Stamp 

int configure_port(int fd){
	struct termios port_settings;

	cfsetispeed(&port_settings, B57600);    // set baud rates
	cfsetospeed(&port_settings, B57600);

	memset(&port_settings,0,sizeof(port_settings));
	port_settings.c_cflag &= ~PARENB;    // set no parity, stop bits, data bits
	port_settings.c_cflag &= ~CSTOPB;
	port_settings.c_cflag &= ~CSIZE;
	port_settings.c_cflag |= CS8;

	port_settings.c_cc[4]=0;
	tcsetattr(fd, TCSANOW, &port_settings);    // apply the settings to the port

	return(fd);
}

bool impUSBTX(){
	char file[]={"/home/eros/param/USBTX.eros"};
	ifstream buka(file);
	string line;
	if (buka.is_open()){
		if(buka.good()){
			char buffer[50]={};
			getline(buka,line);
			for(int i=0; i<line.length(); i++){
				buffer[i]=line[i];
			}
			memcpy(USBTX,buffer,sizeof(USBTX));
			printf("Device: %s--\n\n",USBTX);
		}
		buka.close();
		return 1;
	} else return 0;
}

void parsing(){
	int16_t data[MAX_DATA];
    for(int i=0; i<MAX_DATA; i++)data[i]=serialIn[i];
	// if(data[0]>=128)data[0]-=256;
	// if(data[1]>=128)data[1]-=256;
	// if(data[2]>=128)data[2]-=256;
	// if(data[3]>=128)data[3]-=256;
	fprintf(stderr,"acc-x[%4d]",data[0]);
    fprintf(stderr,"acc-y[%4d]",data[1]);
	fprintf(stderr,"acc-z[%4d]",data[2]);
	fprintf(stderr,"gy-x[%4d]",data[3]);
	fprintf(stderr,"gy-y[%4d]",data[4]);
    fprintf(stderr,"gy-y[%4d]\n",data[5]);

	//Treshold
	// razorData.acc1 = data[0];
	// razorData.acc2 = data[1];
	// razorData.gyro1 = data[2];
	// razorData.gyro2 = data[3];
}

void *serialPublisher(void *obj){

	sprintf(system_check,"stty -F %s 115200",USBTX);
	printf("%s",system_check);
	close(fdGrk);
	system(system_check);
	system(system_check);
	system(system_check);	
	
	close(fdGrk);	//Close serial communication, to ensure that serial is closed before we open
	fdGrk = open(USBTX, O_RDWR | O_NOCTTY | O_NONBLOCK);
	configure_port(fdGrk);
	unsigned char it=0,ready,cM,ceksum,ceksum2;
	unsigned char buff[24],count;
	while(1){
		unsigned char kdata[1];
		int k = 0;
		try{
			k= read(fdGrk, kdata, 1);
		}
		catch(exception& e){
			close(fdGrk);
			fdGrk = open(USBTX, O_RDWR | O_NOCTTY | O_NONBLOCK);
			configure_port(fdGrk);
			k= read(fdGrk, kdata, 1);
			printf("There is something error \n");
		}

		/** Read Serial Data from Razor ****************************/
		//fprintf(stderr,"loop\n",k);
		if (k > 0){
			fprintf(stderr,"in..%d\n",k);
			// if(kdata[0]=='Z' && it!=10){
			// 	fprintf(stderr,"in..1\n");
			// 	ready=1; it=0;
			// }else if(ready==1){
			// 	if(kdata[0]=='Y'){
			// 		fprintf(stderr,"in..2\n");
			// 		ready=2;it=0;
			// 	}else ready=0;
			// }else if(ready==2){
			// 	buff[it]=kdata[0];
			// 	if(it>=MAX_DATA){
			// 		for(cM=0;cM<=it;cM++)
			// 			serialIn[cM]=buff[cM];
			// 		parsing();
			// 		it=ready=0;
			// 	}else it++;
			// }else it=ready=0;
		}
	}	
}

int main(int argc, char **argv){
	//struct timeval t1, t2;  	//Time Lapsed Stamp Variable, uncomment if you want to use this
	//double elapsedTime;		//Time Lapsed Stamp Variable, uncomment if you want to use this

	cout << "Bismillah" << endl;
	cout << "SerialComm OK" << endl;
	
	if(impUSBTX() == 1){
		fprintf(stderr,".. USBTX.eros Loaded..\n");
	}

	ros::init(argc, argv, "gy_stream");

  	ros::NodeHandle n;
	//ros::Publisher razorPub = n.advertise<robotcontrol::Razor>("/robotcontrol/razor", 100); //100 queue data
	pthread_t thread1;
	ros::Rate rate(100); //100Hz sensor data
	pthread_create(&thread1,NULL,serialPublisher,NULL);
	while(ros::ok()){
		//tt++;
		//IntelligentPublish();
		//razorPub.publish(razorData);
		//x.push_back(tt);
		//y.push_back(razorData.gyro1);
    		//ROS_INFO("TPublishing to Intel.. [%d][%d][%d][%d]",razorData.acc1,razorData.acc2,razorData.gyro1,razorData.gyro2);
		//p.plot_data(x,y);
		rate.sleep();
		ros::spinOnce();
	}
	return 0;
}
