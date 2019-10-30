/*
 * test.cpp
 * Returns position of a certain element in a randomly generated vector, if found.
 *
 *  Created on: 25 oct. 2019
 *      Author: Cristina Vilchez Moya
 */

#include <iostream>
#include <vector>
#include <numeric>
#include <utility>
#include <algorithm>
#include <random>
#include <string>
#include "grppi/grppi.h"
#include "grppi/reduce.h"
#include <chrono>

/*N = size of the vectors
 *EL = number to look for
 *NUM_TESTS = number of tests to run for that vector size
 *NUM_BACK = number of backends to try (in order)
 *BACKEND = seq, thr, omp, tbb, ff
 * */

int N = 1000, EL = 5, NUM_TESTS = 5, NUM_BACK = 3;
enum backend { seq, thr, omp, tbb, ff};

std::string backString(backend opt){
	switch (opt){
	case seq: return "seq";
	case thr: return "thr";
	case omp: return "omp";
	case tbb: return "tbb";
	case ff: return "ff";
	default: ;
	}
	return NULL;
}

grppi::dynamic_execution execution_mode(backend opt) {
  using namespace grppi;
  switch(opt){
  case 0: return sequential_execution{};
  case 1: return parallel_execution_native{};
  case 2: return parallel_execution_omp{};
  case 3: return parallel_execution_tbb{};
  case 4: return parallel_execution_ff{};
  default: return {};}
  return {};
}


struct range{
	int first, last, el;

	range(): first{}, last{}, el{} {}
	range(int f, int l, int e): first{f}, last{l}, el{e} {}
};

std::vector<range> divide(range r) {
  int mid = (r.first+r.last)/2;
  return { {r.first,mid, r.el} , {mid + 1, r.last, r.el} };
}

double search(const int n, const int el, std::vector<int> &v,  backend opt){
	grppi::dynamic_execution ex = execution_mode(opt);

	range r{0, (int)v.size()-1, el};

	auto start = std::chrono::high_resolution_clock::now();

	//if (opt == thr) {std::cout << (grppi::parallel_execution_native(ex)).concurrency_degree() <<std::endl; }

	grppi::divide_conquer(ex,r,
			[](auto r) -> std::vector<range> { return divide(r); },
			[](auto r) { return r.first == r.last;},
			[v](auto r) { if(v[r.first] == r.el) return r.first;
						else return -1;},
			[](auto r1, auto r2) { if(r1 != -1 && (r1<=r2 || r2 == -1)) return r1;
			else return r2;});

	auto finish = std::chrono::high_resolution_clock::now();

	return std::chrono::duration<double>(finish - start).count();
}

//double search(const int n, const int el, std::vector<int> &v, backend opt){
//	grppi::dynamic_execution ex = execution_mode(opt);
//
//
//	auto start = std::chrono::high_resolution_clock::now();
//
//	 grppi::map_reduce(ex, v.begin(), v.end(),
//			bool{},
//			[el](int r) { if( r == el) return true;
//						else return false;},
//			[](auto r1, auto r2) { return r1||r2;});
//
//	auto finish = std::chrono::high_resolution_clock::now();
//
//	return std::chrono::duration<double>(finish - start).count();
//}



int main(int argc, char **argv) {

  using namespace std;

  vector <vector<double>>times(NUM_BACK, vector<double>(NUM_TESTS));
  vector<int> v(N);
  vector<double> total(NUM_BACK,0.0);


  random_device rdev;
  uniform_int_distribution<> gen{0,9};


  //For each repetition of the test
  for(int i = 0; i < NUM_TESTS; ++i){

	  //create a vector of size N with random values
	  for (int i=0; i<N; ++i) {
		 v[i] = gen(rdev);
	  }

	  //test that same vector for each backend
	  for(int opt = 0; opt<NUM_BACK; ++opt){
		  double s = search(N,EL,v,(backend) opt);

		  times[opt][i] = s;
		  total[opt] += s;
	  }


  }

  for(int i = 0; i<NUM_BACK;++i){
  cout<<"The elapsed times for vectors of size " << N << " and backend " << backString((backend)i) <<" are:"<<endl;

  for (auto j: times[i]){
   	std::cout<< j<<' ';
    cout<<endl;}
  cout<<"The mean is " << total[i]/NUM_TESTS << endl;
  }
  return 0;
}




