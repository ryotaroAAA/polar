//
// Created by 鈴木凌太郎 on 2016/10/05.
//


//////////////////////////////////////////////////////////////////////////////////

#ifndef CHANNEL_POLARIZATION_LIB_H
#define CHANNEL_POLARIZATION_LIB_H
#include <iostream>
#include "stdlib.h"
#include "math.h"
#include "Eigen/Core"
#include "Eigen/Geometry"
#include "Eigen/src/KroneckerProduct/KroneckerTensorProduct.h"
#include "time.h"
#include <chrono>
#include <iomanip>
#include <string>    // useful for reading and writing
#include <fstream>   // ifstream, ofstream
#include <vector>            //<-vector用
#include <algorithm>         //<-sort用

//////////////////////////////////////////////////////////////////////////////////

using namespace Eigen;
using namespace std;

//params
const int  N=1024;   //number of
const int  K=1000;     //number of data bits
const double e = 0.5f;
static int hoge = 0;
static int hoge2 = 0;
static double hogetime = 0.0;

typedef pair<int, double> ass_arr;

#define PRINT(X) cout << #X << ":\n" << setprecision(10) << X << endl << endl;
#define ARR(array)     (sizeof(array) / sizeof(array[0]));

////////////////////////////////////////////////////////////////////////////////

void outputArray(double *x);
void makeArrayCapacityForBec(double *array);
bool sort_less(const ass_arr& left,const ass_arr& right);
bool sort_greater(const ass_arr& left,const ass_arr& right);
int compare_int(const void *a, const void *b);
void dispArray(double *x);
void dispArray(int n,int *x);
void dispArray(int n,double *x);
int vsize(VectorXi &x);
double calcW(int y, int x);
VectorXi index_o(VectorXi &x);
VectorXi index_e(VectorXi &x);
VectorXi retBinary(VectorXi &x);
VectorXi generateUi(VectorXi &x,int *u_Ac, int *A);
double calcW_i(int i, int n, VectorXi &u, VectorXi &u_i, VectorXi &y);
double calcCapacityForBec(int i, int n);

double calcL_i(int i, int n ,VectorXi &y ,VectorXi &u, int u_i_est, vector<vector<bool>> &isCache, vector<vector<double>> &cache);
VectorXi encoder(int n, VectorXi &input);
VectorXi channel(VectorXi &input);
VectorXi decoder(VectorXi &input, int *u_Ac, int *A);
double calcBhatForBec(int i, int n);
void probErrBound(double *array);
void defineFixedAndFree(int *fixed, int *free);
int ithIndexDesc(int i, double *array, double *descArray);
bool containNumInArray(int i, int n, int *array);
double errorRate(VectorXi &u, VectorXi &u_est);

//////////////////////////////////////////////////////////////////////////////////

bool containVal(int value, vector<int> m_array) {
    auto it = find(begin(m_array), m_array.end(), value);
    size_t index = distance(begin(m_array), it);
    if(index == m_array.size()) {
        return false;
    }
    return true;
}

bool sort_less(const ass_arr& left,const ass_arr& right){
    return left.second < right.second;
}

bool sort_greater(const ass_arr& left,const ass_arr& right){
    return left.second > right.second;
}

int compare_asc(const void *x, const void *y) {
    const double* a=(const double*)x;
    const double* b=(const double*)y;
    if(*a>*b)return 1;
    if(*a<*b)return -1;
    return 0;
}

int compare_desc(const void *x, const void *y) {
    const double* a=(const double*)x;
    const double* b=(const double*)y;
    if(*a>*b)return -1;
    if(*a<*b)return 1;
    return 0;
}

void dispArray(double *x){
    for(int i = 0; i < N; i++){
        printf("x[%d] = %f\n",i, x[i]);
    }
}
void dispArray(int n, double *x){
    for(int i = 0; i < n; i++){
        printf("x[%d] = %f\n",i, x[i]);
    }
}
void dispArray(int n, int *x){
    for(int i = 0; i < n; i++){
        printf("x[%d] = %d\n",i, x[i]);
    }
}
void outputArray(double *x){
    string filename = "/Users/ryotaro/labo/symmetric_capacity";
    ofstream w_file;
    w_file.open(filename, ios::out);
    for (int i = 0; i<N; i++)
    {
        w_file << i << " " << x[i]<< endl;
    }
}
int vsize(VectorXi &x){
    return (x.array() >= 0).count();
}
VectorXi index_o(VectorXi &x){
    VectorXi ret(vsize(x)/2);
    for(int i = 0; i < vsize(x); i++){
        if ( i % 2 == 0) {
            ret[i/2] = x[i];
        }
    }
    return ret;
}

VectorXi index_e(VectorXi &x){
    VectorXi ret(vsize(x)/2);
    for(int i = 0; i < vsize(x); i++){
        if (!(i % 2 == 0)) {
            ret[(i-1)/2] = x[i];
        }
    }
    return ret;
}

VectorXi retBinary(VectorXi &x) {
    VectorXi ret(vsize(x));
    for(int i = 0; i < vsize(x) ; i++){
        ret[i] = x[i] % 2;
    }
    return ret;
}

VectorXi generateUi(int set, VectorXi &x,int *u_Ac, int *A){
    VectorXi ret(N);
    srand((int) time(NULL));
    for (int i = 0; i < N; i++) {
        ret[i] = rand() % 2;
        if (set == 0) {
            ret[i] = 0;
        } else if (set == 1) {
            ret[i] = 1;
        }

    }
    return ret;
}

double calcW(int y, int x) {
    double retVal = 0.0;
    if (x == y) {
        retVal = 1 - e;
    } else if (y == 2) {
        retVal = e;
    } else {
        retVal = 0;
    }
    return retVal;
}

double calcW_i(int i, int n, VectorXi &u, int u_i, VectorXi &y) {
    double W_i = 0.0;

    if ( n == 2 ){
        if ( i == 1 ) {
            W_i = 0.5 * (calcW(y[0] ,(u_i+0) % 2) * calcW(y[1],0)
                         + calcW(y[0] ,(u_i+1) % 2) * calcW(y[1],1));
        } else {
            W_i = 0.5 * calcW(y[0] ,(u[0]+u_i) % 2) * calcW(y[1],u_i);
        }
    } else {
        VectorXi tempY1(vsize(y)/2);
        VectorXi tempY2(vsize(y)/2);
        for (int j = 0; j < vsize(y) ; j++) {
            if(j < vsize(y)/2){
                tempY1[j]= y[j];
            } else {
                tempY2[j - vsize(y)/2] = y[j];
            }
        }

        VectorXi tempU;
        VectorXi tempU_bin;
        VectorXi u_e;

        tempU = index_e(u)+index_o(u);
        u_e = index_e(u);
        tempU_bin = retBinary(tempU);

        if ( i % 2 == 0 ) {
            W_i = 0.5 * (calcW_i(i/2, n/2, tempU_bin ,(1 + u_i) % 2,tempY1) * calcW_i(i/2, n/2, u_e, 1, tempY2)
                         + calcW_i(i/2, n/2, tempU_bin ,(0 + u_i) % 2,tempY1) * calcW_i(i/2, n/2, u_e, 0, tempY2));
        } else {
            W_i = 0.5 * calcW_i((i-1)/2, n/2, tempU_bin ,(u_i+u[i-1]) % 2, tempY1)
                  * calcW_i( i/2   , n/2, u_e       , u_i            , tempY2);
        }
    }
//    cout << "W_" << "n:" << n << ",i:" << i << endl;
//    PRINT(W_i);
    return W_i;
}

double calcL_i(int i, int n ,int level ,VectorXi &y ,VectorXi &u, int u_i_est, vector<vector<bool>> &isCache , vector<vector<double>> &cache) {
    double lr = 0.0;
    if ( n == 1 ) {
        double wc = calcW(y[0],0);
        double wp = calcW(y[0],1);
        lr = wc / wp;
    } else {
        VectorXi tempY1(vsize(y)/2);
        VectorXi tempY2(vsize(y)/2);
        for (int j = 0; j < vsize(y) ; j++) {
            if(j < vsize(y)/2){
                tempY1[j]= y[j];
            } else {
                tempY2[j - vsize(y)/2] = y[j];
            }
        }

        VectorXi tempU;
        VectorXi tempU_bin;
        VectorXi u_e;

        tempU = index_e(u)+index_o(u);
        u_e = index_e(u);
        tempU_bin = retBinary(tempU);
        hoge2++;

        double temp1 = calcL_i(i/2, n/2, level+1, tempY1, tempU_bin, u_i_est, isCache, cache);
        double temp2 = calcL_i(i/2, n/2, level+1, tempY2, u_e, u_i_est, isCache, cache);

        if ( i % 2 == 0) {
            lr = ( 1 + temp1 * temp2 ) / ( temp1 + temp2 );
        } else {
            lr = pow(temp1, 1-2*u[i-1]) * temp2;
        }
    }
    if (isinf(lr) || isnan(lr)) {
        lr = 1;
    }
//    cout << i << " " << n << " " << lr  << endl;
    return lr;
}

// double errorRate(VectorXi &u, VectorXi &u_est){
//     int error_count = 0;
//     for(int i=0; i<N; i++){
//         if(u[i] != u_est[i]){
//             error_count++;
//         }
//     }
//     return (double)error_count/N;
// }


double calcCapacityForBec(int i, int n) {
    double cap =0.0;
    if ( i == 0 && n == 1 ) {
        cap = 1 - e;
    } else {
        if ( i % 2 == 0) {
            cap = pow(calcCapacityForBec(i/2, n/2),2);
        } else {
            double tempCap = calcCapacityForBec((i-1)/2, n/2);
            cap = 2 * tempCap - pow(tempCap,2);
        }
    }
    //cout << "i:" << i << ", n:" << n <<endl;
//    PRINT(cap);
    return cap;
}

double calcBhatForBec(int i, int n){
    double bha =0.0;
    if ( i == 0 && n == 1 ) {
        bha = e;
    } else {
        if ( i % 2 == 0) {
            double tempBha = calcBhatForBec(i/2, n/2);
            bha = 2 * tempBha - pow(tempBha,2);
        } else {
            bha = pow(calcBhatForBec((i-1)/2, n/2),2);
        }
    }
//    cout << "i:" << i << ", n:" << n <<endl;
//    PRINT(bha);
    return bha;
}

void makeArrayCapacityForBec(double *array) {
    for(int i = 0; i < N; i++){
        array[i] = calcCapacityForBec(i,N);
    }
}

void probErrBound(double *array) {
    int N = pow(2,18);
    int count = 0;
    double tempArr[N];
    double sumArr[N];
    double sum = 0.0;

    for(int i = 0; i < N; i++) {
        tempArr[i] = calcBhatForBec(i, N);
    }

    qsort(tempArr, N, sizeof(double), compare_asc);

    for(int i=0; i < N; i++){
        sum += tempArr[i];
        sumArr[i] = sum;
    }

    string filename = "/Users/ryotaro/labo/e18";
    ofstream w_file;
    w_file.open(filename, ios::out);
    for (int i = 0; i < N; i++)
    {
        w_file << (double)i/N << " " << sumArr[i] << " " << tempArr[i] << endl;
    }
}

int ithIndexDesc(int i, double *array, double *descArray){
    for (int j=0; j<N; j++) {
        if(descArray[i] == array[j]){
            return j;
        }
    }
    return 0;
}

void defineFixedAndFree(int *fixed, int *free){
    double cap[N] = {0};
    double cap_desc[N] = {0};
    makeArrayCapacityForBec(cap);

    vector<pair<int, double> > cap_map;
    for(int i=0; i<N; i++){
        cap_map.push_back(pair<int, double>(i, cap[i]));
    }

    //昇順ソート
    sort(begin(cap_map), end(cap_map), sort_greater);
    int i = 0;
    for(auto val : cap_map){
        if(i<K){
            free[i] = val.first;
        }else{
            fixed[i-K] = val.first;
        }
        i++;
    }
}

bool containNumInArray(int i, int n, int *array){
    for(int j=0; j<n; j++){
        if(i == array[j]){
            return true;
        }
    }
    return false;
}

double errorRate(VectorXi &u, VectorXi &u_est){
    int error_count = 0;
    for(int i=0; i<N; i++){
        if(u[i] != u_est[i]){
            error_count++;
        }
    }
    return (double)error_count/N;
}

//////////////////////////////////////////////////////////////////////////////////

#endif //CHANNEL_POLARIZATION_LIB_H
