/*
 * search.cpp
 * Returns position of a certain element in a randomly generated vector, if found.
 *
 *  Created on: 17 oct. 2019
 *      Author: Cristina Vilchez Moya
 */

#include <iostream>
#include <vector>
#include <numeric>
#include <utility>
#include <algorithm>
#include <random>
#include "grppi/grppi.h"
#include "grppi/reduce.h"

struct range{
	int first, last, el;

	range(): first{}, last{}, el{} {}
	range(int f, int l, int e): first{f}, last{l}, el{e} {}
};

std::vector<range> divide(range r) {
  int mid = (r.first+r.last)/2;
  return { {r.first,mid, r.el} , {mid + 1, r.last, r.el} };
}

int searchDiv(const int n, const int el, std::vector<int> &v){
	grppi::dynamic_execution ex = grppi::parallel_execution_native{};

	range r{0, (int)v.size()-1, el};
	auto s = grppi::divide_conquer(ex,r,
			[](auto r) -> std::vector<range> { return divide(r); },
			[](auto r) { return r.first == r.last;},
			[v](auto r) { if(v[r.first] == r.el) return r.first;
						else return -1;},
			[](auto r1, auto r2) { if(r1 != -1 && (r1<=r2 || r2 == -1)) return r1;
			else return r2;});


	return s;
}

//int searchMapRed(const int n, const int el, std::vector<int> &v){
//	grppi::dynamic_execution ex = grppi::parallel_execution_native{};
//
//	auto s = grppi::map_reduce(ex, v.begin(), v.end(),
//			bool{},
//			[el](int r) { if( r == el) return true;
//						else return false;},
//			[](auto r1, auto r2) { return r1||r2;});
//
//
//	return s;
//}



int main(int argc, char **argv) {

  using namespace std;

  vector<int> v;

  int n, el;
  cout<< "Insert vector size: ";
  cin>>n;
  cout<<"Insert element to look for: ";
  cin>> el;

  random_device rdev;
  uniform_int_distribution<> gen{0,9};


  for (int i=0; i<n; ++i) {
  	 v.push_back(gen(rdev));
  }
  cout<<"Your vector is ";
  for (auto i: v)
 	      std::cout<< i;
  cout<<endl;

  int s = searchDiv(n,el,v);

  if (s> -1) cout << "The element is at position " << s<<endl;
  else cout << "The element was not found."<<endl;


  return 0;
}
