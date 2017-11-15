#include "../lib/Common.h"

void Common::bar(){
    cout << "======================" << endl;
}

double Common::gauss_dist(double x, double mean, double sigma){
    return exp(-1.0*pow(x - mean, 2)/(2 * sigma)) / (sqrt(2*M_PI*sigma));
}

bool Common::containVal(int value, vector<int> m_array) {
    auto it = find(begin(m_array), m_array.end(), value);
    size_t index = distance(begin(m_array), it);
    if(index == m_array.size()) {
        return false;
    }
    return true;
}

bool Common::sort_less(const ass_arr& left,const ass_arr& right){
    return left.second < right.second;
}

bool Common::sort_greater(const ass_arr& left,const ass_arr& right){
    return left.second > right.second;
}

int Common::compare_asc(const void *x, const void *y) {
    const double* a=(const double*)x;
    const double* b=(const double*)y;
    if(*a>*b)return 1;
    if(*a<*b)return -1;
    return 0;
}

int Common::compare_desc(const void *x, const void *y) {
    const double* a=(const double*)x;
    const double* b=(const double*)y;
    if(*a>*b)return -1;
    if(*a<*b)return 1;
    return 0;
}

void Common::dispArray(double *x){
    for(int i = 0; i < Params::get_N(); i++){
        printf("x[%d] = %f\n",i, x[i]);
    }
}
void Common::dispArray(int n, double *x){
    for(int i = 0; i < n; i++){
        printf("x[%d] = %f\n",i, x[i]);
    }
}

void Common::dispArray(int n, int *x){
    for(int i = 0; i < n; i++){
        printf("x[%d] = %d\n",i, x[i]);
    }
}

void Common::pp(vector<int> &a){
    bar();
    cout << "{";
    int count = 1;
    for(auto var : a){
        cout << var;
        if(count != a.size()) cout << ",";
        count++;
    }
    cout << "}" << endl;
}

void Common::pp(vector<double> &a){
    bar();
    cout << "{";
    int count = 1;
    for(auto var : a){
        cout << var;
        if(count != a.size()) cout << ",";
        count++;
    }
    cout << "}" << endl;
}

void Common::outputArray(double *x){
    string filename = "/Users/ryotaro/labo/symmetric_capacity";
    ofstream w_file;
    w_file.open(filename, ios::out);
    for (int i = 0; i<Params::get_N(); i++)
    {
        w_file << i << " " << x[i]<< endl;
    }
}

vector<int> Common::index_o(int n, vector<int> &x){
    vector<int> ret(n/2);
    for(int i = 0; i < n; i++){
        if (i % 2 == 0) {
            ret[i/2] = x[i];
        }
    }
    return ret;
}

vector<int> Common::index_e(int n, vector<int> &x){
    vector<int> ret(n/2);
    for(int i = 0; i < n; i++){
        if (i % 2 == 1) {
            ret[i/2] = x[i];
        }
    }
    return ret;
}

vector<int> Common::retBinary(int n, vector<int> &x) {
    vector<int> ret(n);
    for(int i = 0; i < n; i++){
        ret[i] = x[i]%2;
    }
    return ret;
}

int Common::ithIndexDesc(int i, double *array, double *descArray){
    for (int j=0; j<Params::get_N(); j++) {
        if(descArray[i] == array[j]){
            return j;
        }
    }
    return 0;
}


bool Common::containNumInArray(int i, int n, vector<int> &array){
    for(auto val: array ){
        if(i == val){
            return true;
        }
    }
    return false;
}

double Common::get_rate(){
    double rate;
    int n = Params::get_N();
    int k = Params::get_K();
    int m = Params::get_M();
    int mn = Params::get_MN();
    switch (Params::get_exp_mode()){
        case NORMAL:
            rate = (double)k/n;
            break;
        case PUNC:
        case QUP:
        case WANG:
        case VALERIO_P:
            rate = (double)k/(n-m);
            break;
        case VALERIO_S:
            rate = (double)(k-m)/(n-m);
            break;
        case MID:
            rate = (double)k/(n+mn);
            break;
        case M_WANG:
        case M_QUP:
        case M_VALERIO_P:
            rate = (double)k/(n-m+mn);
            break;
        case M_VALERIO_S:
            rate = (double)(k-m)/(n-m+mn);
            break;
        default:
            rate = 0.0;
            break;
    }
    return rate;
}

bool Common::is_mid_send(){
    switch (Params::get_exp_mode()){
        case NORMAL:
        case PUNC:
        case QUP:
        case WANG:
        case VALERIO_P:
        case VALERIO_S:
            return false;
        case MID:
        case M_WANG:
        case M_QUP:
        case M_VALERIO_P:
        case M_VALERIO_S:
            return true;
        default:
            break;
    }
}