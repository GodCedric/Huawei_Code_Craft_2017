#ifndef __GETTIME__H__
#define __GETTIME__H__

#include <sys/timeb.h>

double gettime(){
    //程序计时
    struct timeb rawtime;
	ftime(&rawtime);
	const static unsigned long start_s = rawtime.time;     //初始s
	const static int start_ms = rawtime.millitm;           //初始ms
	//计算时间差
	unsigned long out_s = rawtime.time - start_s;         //s时间差
    int out_ms = rawtime.millitm - start_ms;              //ms时间差
    if (out_ms < 0)
    {
        out_ms += 1000;
        out_s -= 1;
    }
    return out_s + out_ms/1000.0;
}


#endif
