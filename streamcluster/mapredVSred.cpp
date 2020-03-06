//g++ -I/home/crivilch/Downloads/grppi-master/include -g -Wall -pthread -fopenmp -o t prueab2.cpp -std=c++14 -DGRPPI_OMP 


using namespace std;

#include <cassert>
#include <vector>
#include <tuple>
#include <iostream>
#include "grppi/grppi.h"
#include <string>
#include <stdio.h>
#include <sys/time.h>
#include <omp.h>
#include <pthread.h>
#include <sstream>

enum backend {thr, omp, seq, tbb, ff};

static backend opt;

backend stringToBackend(std::string opt){
	if(opt =="seq")return seq;
	if(opt == "thr") return thr;
	if(opt == "omp") return omp;
	if(opt == "tbb")  return tbb;
	else return ff;
}

grppi::dynamic_execution execution_mode(backend opt, int threads) {
  using namespace grppi;
  switch(opt){
  case 2: return sequential_execution{};
  case 0: return parallel_execution_native(threads);
  case 1: return parallel_execution_omp(threads);
  case 3: return parallel_execution_tbb{};
  case 4: return parallel_execution_ff{};
  default: return {};}
  return {};
}


typedef struct {
  float weight;
  float *coord;
  long assign;  /* number of point where this one is assigned */
  float cost = 1.27;  /* cost of that assignment, weight*distance */

} Point;

static double time_end;
static double time_begin;

void writt(double time_begin, double time_end, char* outputFile){
	
	FILE *file = fopen(outputFile, "a");
	if(file == NULL) {
	    printf("ERROR: Unable to open file `%s'.\n", outputFile);
	    exit(1);
	 }
	 int rv = fprintf(file, "%f\n", time_end-time_begin);
	 if(rv < 0) {
	    printf("ERROR: Unable to write to file `%s'.\n", outputFile);
	    fclose(file);
	 }
	fclose(file);
}

int main(int argc, char ** argv){

	
	int size = atoi(argv[1]);
	opt = stringToBackend(argv[2]);
	int nproc = atoi(argv[3]);

	Point * v = (Point*)malloc(sizeof(Point)*size);

	grppi::dynamic_execution ex = execution_mode(opt, nproc);
        
	Point p; p.cost = 0;
	
	grppi::map(ex, v, size, v, [](Point&x){ x.cost = 1.27; return x;});
	
	//for (Point i:vv) cout<<i.cost<<' ';
	struct timeval t;
	gettimeofday(&t,NULL);
	time_begin = (double)t.tv_sec+(double)t.tv_usec*1e-6;

	
	Point c = grppi::reduce(ex, v, size, p, [](const Point & x, const Point & y){
		Point w;
		w.cost= x.cost + y.cost;
		
		return w;

	});

	gettimeofday(&t,NULL);
  	time_end = (double)t.tv_sec+(double)t.tv_usec*1e-6;
	writt(time_begin, time_end, "red.csv");
	
	cout<<c.cost<<'\n';
	
	
	gettimeofday(&t,NULL);
	time_begin = (double)t.tv_sec+(double)t.tv_usec*1e-6;

	float d = grppi::map_reduce(ex, v, size, 0.0, [](Point x){return x.cost;},[](float x, float y){return x+y;});

	gettimeofday(&t,NULL);
  	time_end = (double)t.tv_sec+(double)t.tv_usec*1e-6;
	writt(time_begin, time_end, "mapred.csv");

	cout<<d<<'\n';
	
	assert((c.cost - d)<0.1);
return 0;
}
