#include "../lib/Analysor.h"

Analysor::Analysor(){

}

Analysor::~Analysor(){

}

void Analysor::errorCount(vector<int> &u, vector<int> &u_est, int* error_count){
    for(int i=0; i<Params::get_N(); i++){
        if(u[i] != u_est[i]){
            *error_count = *error_count + 1;
        }
    }
}

double Analysor::errorCalc(vector<int> &u, vector<int> &u_est, int* error_count){
    for(int i=0; i<Params::get_N(); i++){
        if(u[i] != u_est[i]){
            *error_count = *error_count + 1;
        }
    }

    return (double)(*error_count)/Params::get_N();
}

double Analysor::calcCapacity(int i, int n) {
    double cap =0.0;
    if(Params::get_s() == BEC){
        if ( i == 0 && n == 1 ) {
            cap = 1 - Params::get_e();
        } else {
            if ( i % 2 == 0) {
                cap = pow(Analysor::calcCapacity(i/2, n/2),2);
            } else {
                double tempCap = Analysor::calcCapacity((i-1)/2, n/2);
                cap = 2 * tempCap - pow(tempCap,2);
            }
        }
    } else if (Params::get_s() == BSC){

    }
    return cap;
}

double Analysor::calcBhatforBEC(int i, int n){
    double bha =0.0;

    if ( i == 0 && n == 1 ) {
        bha = Params::get_e();
    } else {
        if ( i % 2 == 0) {
            double tempBha = Analysor::calcBhatforBEC(i/2, n/2);
            bha = 2 * tempBha - pow(tempBha, 2);
        } else {
            bha = pow(Analysor::calcBhatforBEC((i-1)/2, n/2),2);
        }
    }
    return bha;
}

void Analysor::makeArrayCapacity(vector<double> &array) {
    for(int i = 0; i < Params::get_N(); i++) {
        array[i] = Analysor::calcCapacity(i, Params::get_N());
    }
}

double Analysor::calc_inv(double y){
    double x1 = 10.0;
    double x2 = 500.0;
    double x3 = 0.0;
    double fx = 0.0;
    int count = 1;
    while (abs(x1 - x2) >= 0.00001) {
        x3 = (x1+x2) / 2.0;
        fx = sqrt(M_PI/x3) * exp(-1.0 * (x3/4.0)) * (1.0 - 10.0/(7.0 * x3));
        if(fx > y){
            x1 = x3;
        } else {
            x2 = x3;
        }
        count++;
//        cout << "est:" <<  x1 << " " << x2 << " " << m_func(x3)<< " true:" << y << endl;
    }
    return x3;
}

double Analysor::inv_m_func(double y){
    double ret = 0.0;
    const double a = -0.4527;
    const double b = 0.0218;
    const double c = 0.86;
    double temp = 0.0;
    //0.03847)
    if (y > 0.03847){
        ret = pow((log(y)-b)/a, 1.0/c);
    } else if(y < 0.03847) {
        ret = calc_inv(y);
    } else {
//        ret = __nan();
    }
    return ret;
}

double Analysor::m_func(double x){
    double ret = 0.0;
    const double a = -0.4527;
    const double b = 0.0218;
    const double c = 0.86;
    double temp1 = 0.0;
    double temp2 = 0.0;
    if (x < 10.0) {
        ret = exp(a * pow(x,c) + b);
    } else if (x > 10.0) {
        temp1 = 1.0 - 3.0/x;
        temp2 = 1.0 + 1.0/(7.0 * x);
        ret = 0.5 * sqrt(M_PI/x) * exp(-1.0 * x / 4.0)  * (temp1 + temp2);
    } else {
//        ret = __nan();
    }
    return ret;
}

double Analysor::calc_m_in(int i, int n){
    double ret = 0.0;
    double temp1 = 0.0;
    double temp2 = 0.0;
    int temp_i = (i/2 == 0)? 1 : i/2;
    if (n == 1){
        ret = 2.0 / Params::get_e();
    } else {
        temp1 = Analysor::calc_m_in(temp_i, n/2);
        if (i % 2 == 0){
            ret = 2.0 * temp1;
        } else {
            temp2 = 1.0 - pow((1.0 - Analysor::m_func(temp1)), 2);
            ret = Analysor::inv_m_func(temp2);
        }
    }
//    cout << "[" << temp_i << "][" << n  << "] "  << ret << endl;
    return ret;
}

void Analysor::makeArrayBhat(vector<double> &array) {
    if ( Params::get_s() == BSC) {
        Performance performance;
        Decoder decoder;
        Encoder encoder;
        Logger logger;

        vector<int> u(Params::get_N(), 0);
        vector<int> x(Params::get_N(), 0);
        vector<double> y(Params::get_N(), 0.0);
        int u_n = 0;

        double sumbha = 0.0;
        double tempbha = 0.0;
        int size = log2(Params::get_N());

        vector<vector<bool> > isCache (size, vector<bool>(Params::get_N(),false));
        vector<vector<double> > cache (size, vector<double>(Params::get_N(),0.0));

        double lr;
        int cache_i = 0;
        int repeatNum = Params::get_monteNum();

        for (int m = 1; m <= repeatNum; m++) {
            performance.startTimer();
//            cout << "[" << m << "]" << endl;
            Preseter::preset_u(RAND, u);
            vector<int> temp_u = u;
            x = encoder.encode(Params::get_N(), u);

            for (int i = 0; i < Params::get_N(); i++) {
                temp_u = u;
                u_n = temp_u[i];
                temp_u.resize(i);

                y = Channel::channel_output(x);
                cache_i = decoder.makeTreeIndex(Params::get_N())[i] - 1;
                lr = exp(decoder.calcL_i(i+1, Params::get_N(), cache_i, 0, y, temp_u, isCache, cache));
                lr = pow(lr, -1+2 * u_n);
                tempbha = sqrt(lr);
                if (tempbha > 1.0) tempbha = 1.0/tempbha;
                if (isnan(tempbha)) tempbha = 0.0;
                array[i] += 1.0 * tempbha/repeatNum;
            }
        }
    } else {
        vector<double> m_in(Params::get_N(), 0.0);
        vector<double> sigma2_in(Params::get_N(), 0.0);
        double temp = 0.0;
        for (int i = 0; i < Params::get_N(); i++) {
            temp = Analysor::calc_m_in(i+1, Params::get_N());
//            if(isnan(temp)){
//                m_in[i] = 0.0;
//            } else {
                m_in[i] = temp;
//            }
//            cout << i << " mean " << m_in[i] <<endl;
            sigma2_in[i] = 2.0/m_in[i];
            array[i] = exp(-1.0/(2.0 * sigma2_in[i]));
        }
    }

    string filename = Params::get_rvbDir() + " Bhat";
    ofstream w_file;
    w_file.open(filename, ios::out);
    for (int i = 0; i < Params::get_N(); i++)
    {
        w_file << i << " " << array[i] << endl;
        cout << i << " " << array[i] << endl;
    }
    Common::bar();
}

void Analysor::probErrBound() {
    int count = 0;
    vector<double> tempArr;
    vector<double> sumArr;
    double sum = 0.0;

    for(int i = 0; i < Params::get_N(); i++) {
        tempArr.push_back(Analysor::calcBhatforBEC(i, Params::get_N()));
    }

    sort(tempArr.begin(), tempArr.end(), greater<int>());

    for(int i=0; i < Params::get_N(); i++){
        sum += tempArr[i];
        sumArr.push_back(sum);
    }

    string filename = "log/eb10";
    ofstream w_file;
    w_file.open(filename, ios::out);
    for (int i = 0; i < Params::get_N(); i++)
    {
        w_file << (double)i/Params::get_N() << " " << sumArr[i] << " " << tempArr[i] << endl;
        cout << (double)i/Params::get_N() << " " << sumArr[i] << " " << tempArr[i] << endl;
    }
}

//rate vs BERのグラフ作成用
void Analysor::calcBlockErrorRate(MODE mode) {
    Performance performance;
    Decoder decoder;
    Encoder encoder;
    Logger logger;
    logger.setRvbDir(Params::get_rvbDir());
    vector<int> A(Params::get_K(), -1);
    vector<int> u_n(Params::get_N(), 0);
//    if(false){
    vector<int> x_n(Params::get_N(), 0);
    vector<double> y_n(Params::get_N(), 0);
//    } else {
//
//    }
//    vector<vector<int> > x_n(log2(Params::get_N()), vector<int>(Params::get_N()) );
//    vector<vector<double> > y_n(log2(Params::get_N()), vector<double>(Params::get_N()) );

    vector<int> u_est(Params::get_N(), 0);
    vector<pair<int, double> > cap_map;
    Preseter::makeMutualInfoArray(cap_map);

    double BER = 0.0;
    double sumBER = 0.0;
    double rate = 0.0;
    int n = Params::get_N();
    int error_count = 0;
    int block_error_count = 0;
    int loopi = 0;
    int repeatNum = Params::get_blockNum();

    int tmp = Params::get_N()/Params::get_K();
    int tmpK = Params::get_K();
    for (int i = 1; i <= (tmp+1)/2; i++) {

        performance.startTimer();
        Params::set_K(i * tmpK);
        A.resize(Params::get_K(), -1);
        Preseter::represet_A(A, cap_map);
        Preseter::preset_u(RAND, u_n);
        x_n = encoder.encode(Params::get_N(), u_n);

        while (block_error_count < Params::get_upperBlockErrorNum()) {
            loopi++;

            y_n.assign(Params::get_N(), 0);
            u_est.assign(Params::get_N(), 0);
            y_n = Channel::channel_output(x_n);
//            u_est = (Params::get_decode_mode() == BP)?decoder.BP(Params::get_rp(), y_n, u_n, A):decoder.decode(y_n, u_n, A);

//            Common::pp(u_est);
            Analysor::errorCount(u_n, u_est, &error_count);
            if(error_count > 0) block_error_count++;
            if (loopi % 1 == 0 ) cout << loopi << " " << error_count << " " << block_error_count  << (double)block_error_count/loopi << endl;
            error_count = 0;
            rate = (double) Params::get_K() / Params::get_N();

//            if(loopi >= repeatNum || i * tmpK < Params::get_N()/7) break;
            if(loopi >= repeatNum) break;
        }
        performance.stopTimer();

        cout << Params::get_rvbDir() << endl;
        BER = (double)block_error_count/loopi;

        logger.outLog("=================================");
        logger.outLog(performance.outTime("処理時間"));
        logger.outLog("(N,K) = (" + to_string(Params::get_N()) + "," + to_string(Params::get_K()) + ")");
        logger.outLog("BER:" + to_string(BER));
        logger.outLog("Rate:" + to_string(rate));
        logger.outLog(encoder.outCount("encoder_count"));
        logger.outLog(decoder.outCount("decoder_count"));
        performance.outHMS();

        logger.outLogRVB(rate, BER);
        loopi = 0;
        sumBER = 0.0;
        block_error_count = 0;
        if(mode == TEST) break;
    }
}

//rate vs BERのグラフ作成用
void Analysor::calcBlockErrorRate_BP(MODE mode) {
    Performance performance;
    Decoder decoder;
    Encoder encoder;
    Logger logger;
    logger.setRvbDir(Params::get_rvbDir());
    vector<int> u_n(Params::get_N(), 0);
    vector<int> x_n(Params::get_N(), 0);
    vector<double> y_n(Params::get_N(), 0);

    vector<int> u_est(Params::get_N(), 0);
    vector<pair<int, double> > cap_map;
    Preseter::makeMutualInfoArray(cap_map);

    vector<int> A;
    vector<int> Ac;
    Preseter::preset_A_Ac(A,Ac);

    double BER = 0.0;
    double sumBER = 0.0;
    double rate = 0.0;
    int n = Params::get_N();
    int error_count = 0;
    int block_error_count = 0;
    int loopi = 0;

    int tmp = Params::get_N()/Params::get_K();
    int tmpK = Params::get_K();
    vector<int> param(2,0);

    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    string itrfn = "log/"
                   + to_string(pnow->tm_year+1900) + to_string(pnow->tm_mon + 1) + to_string(pnow->tm_mday)
                   + "/N=" + to_string(Params::get_N())
                   + " e=" + to_string(Params::get_e())
                   + " channel=" + (Params::get_s()?"BSC":"BEC") + " mean_itr";
    if(Params::get_M() > 0) itrfn = itrfn + " punc";
    ofstream itrfile;
    itrfile.open(itrfn, ios::out);
    double mitr = 0.0;
    int itr = 0;

    for (int i = 10; i <= 13; i++) {
        loopi = 0;
        sumBER = 0.0;
        mitr = 0.0;
        block_error_count = 0;
        performance.startTimer();
        Params::set_K(i * tmpK);
        A.resize(Params::get_K(), -1);
        Preseter::represet_A(A, cap_map);
        Preseter::preset_u(RAND, u_n);
        x_n = encoder.encode(Params::get_N(), u_n);

        while (block_error_count < Params::get_upperBlockErrorNum()) {
            loopi++;

            y_n.assign(Params::get_N(), 0);
            u_est.assign(Params::get_N(), 0);
            y_n = Channel::channel_output(x_n);
            u_est = decoder.calcBP(param, y_n, u_n, A, Ac);

            itr = param[0];
            Analysor::errorCount(u_n, u_est, &error_count);
            if(error_count > 0) block_error_count++;
            if (loopi % 1 == 0 ) cout << loopi << " " << error_count << " " << block_error_count  << " " << itr << " " << (double)block_error_count/loopi << endl;
            error_count = 0;
            rate = (double) Params::get_K() / (Params::get_N());

            mitr += itr; // 0:itr, 1:nochecked

            if(loopi >= Params::get_blockNum()) break;
        }
        mitr = mitr/loopi;
        itrfile << rate << " " << mitr << endl;
        cout << "mitr : "<< mitr << endl;
        cout << "rate : "<< rate << endl;
        performance.stopTimer();

        cout << Params::get_rvbDir() << endl;
        BER = (double)block_error_count/loopi;

        logger.outLog("=================================");
        logger.outLog(performance.outTime("処理時間"));
        logger.outLog("(N,K) = (" + to_string(Params::get_N()) + "," + to_string(Params::get_K()) + ")");
        logger.outLog("BER:" + to_string(BER));
        logger.outLog("Rate:" + to_string(rate));
        logger.outLog(encoder.outCount("encoder_count"));
        logger.outLog(decoder.outCount("decoder_count"));
        performance.outHMS();

        logger.outLogRVB(rate, BER);
        if(mode == TEST) break;
    }
}

//rate vs BERのグラフ作成用
void Analysor::calcBlockErrorRate_mid_BP(MODE mode) {
    Performance performance;
    Decoder decoder;
    Encoder encoder;
    Logger logger;
    logger.setRvbDir(Params::get_rvbDir());

    int size = log2(Params::get_N())+1;
    vector<int> A(Params::get_K(), -1);
    vector<int> u_n(Params::get_N(), 0);

    vector<vector<int> > tmp_x_n(log2(Params::get_N()), vector<int>(Params::get_N(),0) );
    vector<vector<int> > x_n(size, vector<int>(Params::get_N(),0) );
    vector<vector<double> > y_n(size, vector<double>(Params::get_N(), 0.0));

    vector<int> u_est(Params::get_N(), 0);
    vector<pair<int, double> > cap_map;
    Preseter::makeMutualInfoArray(cap_map);

    double BER = 0.0;
    double sumBER = 0.0;
    double rate = 0.0;
    int n = Params::get_N();
    int error_count = 0;
    int block_error_count = 0;
    int loopi = 0;
    vector<int> param(2, 0);

    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    string itrfn = "log/"
                   + to_string(pnow->tm_year+1900) + to_string(pnow->tm_mon + 1) + to_string(pnow->tm_mday)
                   + "/N=" + to_string(Params::get_N())
                   + " e=" + to_string(Params::get_e())
                   + " channel=" + (Params::get_s()?"BSC":"BEC") + " mean_itr mid";
    ofstream itrfile;
    itrfile.open(itrfn, ios::out);
    double mitr = 0;
    int itr = 0;

    int tmp = Params::get_N()/Params::get_K();
    int tmpK = Params::get_K();
    for (int i = 10; i <= 13; i++) {
        mitr = 0.0;
        loopi = 0;
        sumBER = 0.0;
        block_error_count = 0;
        performance.startTimer();
        Params::set_K(i * tmpK);
        A.resize(Params::get_K(), -1);
        Preseter::represet_A(A, cap_map);
        Preseter::preset_u(RAND, u_n);
        tmp_x_n[log2(Params::get_N()) - 1] = encoder.encode_m(Params::get_N(), 0, 0, u_n, tmp_x_n);
        //uとxをまとめて送る
        for (int i = 0; i < log2(Params::get_N()) + 1; i++) {
            for (int j = 0; j < Params::get_N(); j++) {
                if (i == 0) {
                    x_n[i][j] = u_n[j];
                } else {
                    x_n[i][j] = tmp_x_n[i - 1][j];
                }
            }
        }

        while (block_error_count < Params::get_upperBlockErrorNum()) {
            loopi++;
            u_est.assign(Params::get_N(), 0);

            Channel::channel_output_m(x_n, y_n);
            u_est = decoder.calcBP_m(param, y_n, u_n, A);

            itr = param[0];
            Analysor::errorCount(u_n, u_est, &error_count);
            if(error_count > 0) block_error_count++;
            if (loopi % 1 == 0 ) cout << loopi << " " << error_count << " " << block_error_count  << " " << itr << " " << (double)block_error_count/loopi << endl;
            error_count = 0;
            rate = (double) Params::get_K()/(Params::get_N()+Params::get_M());
            mitr += itr; // 0:itr, 1:nochecked

            if(loopi >= Params::get_blockNum()) break;
        }
        mitr = (mitr/loopi)>=70 ? 70 : mitr/loopi;
        itrfile << rate << " " << mitr << endl;
        cout << "mitr : "<< mitr << endl;
        cout << "rate : "<< rate << endl;

        performance.stopTimer();

        cout << Params::get_rvbDir() << endl;
        BER = (double)block_error_count/loopi;

        logger.outLog("=================================");
        logger.outLog(performance.outTime("処理時間"));
        logger.outLog("(N,K) = (" + to_string(Params::get_N()) + "," + to_string(Params::get_K()) + ")");
        logger.outLog("BER:" + to_string(BER));
        logger.outLog("Rate:" + to_string(rate));
        logger.outLog(encoder.outCount("encoder_count"));
        logger.outLog(decoder.outCount("decoder_count"));
        performance.outHMS();

        logger.outLogRVB(rate, BER);
        if(mode == TEST) break;
    }
}

inline vector<int>Analysor::makeTable(int n){
    vector<int> ret(n);
    if (n == 1) {
        ret[0] = 1;
    } else {
        vector<int> temp = makeTable(n/2);
        for (int i = 0; i < n ; i++) {
            if(i < n/2){
                ret[i] = 2*temp[i]-1;
            } else {
                ret[i] = 2*temp[i-n/2];
            }
        }
    }
    return ret;
}

vector<int> Analysor::get_P(vector<int> p_0){
    vector<int> p;
    vector<int> table = makeTable(Params::get_N());
    for (int i = 0; i < p_0.size(); i++) {
        p.push_back(table[p_0[i]-1]-1);
    }
    return p;
}

void Analysor::set_params(vector<pair<int, double> > &cap_map,vector<int> &A, vector<int> &Ac, vector<int> &p_0, vector<int> &p){
    A.resize(Params::get_K(), -1);
    Ac.resize(Params::get_N()-Params::get_K(), -1);
    for (int i = 0; i < Params::get_M(); i++) {
        p_0[i] = Params::get_N()-i;
        Ac[i] = Params::get_N()-i-1;
    }
    p = Analysor::get_P(p_0);
    Preseter::represet_A_wang(A, Ac, cap_map);
}

//rate vs BERのグラフ作成用
void Analysor::calcBlockErrorRate_BP_wang(MODE mode) {
    Performance performance;
    Decoder decoder;
    Encoder encoder;
    Logger logger;
    logger.setRvbDir(Params::get_rvbDir());
    vector<int> u_n(Params::get_N(), 0);
    vector<int> x_n(Params::get_N(), 0);
    vector<double> y_n(Params::get_N(), 0);

    vector<int> u_est(Params::get_N(), 0);
    vector<pair<int, double> > cap_map;
    Preseter::makeMutualInfoArray(cap_map);

    vector<int> A;
    vector<int> Ac;
//    Preseter::preset_A_Ac(A,Ac);
    vector<int> p_0(Params::get_M());
//    for (int i = 0; i < Params::get_M(); i++) {
//        p_0[i] = Params::get_N()-i;
//    }
    vector<int> p = Analysor::get_P(p_0);

    double BER = 0.0;
    double sumBER = 0.0;
    double rate = 0.0;
    int n = Params::get_N();
    int error_count = 0;
    int block_error_count = 0;
    int loopi = 0;

    int tmp = Params::get_N()/Params::get_K();
    int tmpK = Params::get_K();
    vector<int> param(2,0);

    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    string itrfn = "log/"
                   + to_string(pnow->tm_year+1900) + to_string(pnow->tm_mon + 1) + to_string(pnow->tm_mday)
                   + "/N=" + to_string(Params::get_N())
                   + " e=" + to_string(Params::get_e())
                   + " channel=" + (Params::get_s()?"BSC":"BEC") + " mean_itr wang";
    if(Params::get_M() > 0) itrfn = itrfn + " punc";
    ofstream itrfile;
    itrfile.open(itrfn, ios::out);
    double mitr = 0.0;
    int itr = 0;

    for (int i = 10; i <= 13; i++) {
        loopi = 0;
        sumBER = 0.0;
        mitr = 0.0;
        block_error_count = 0;
        performance.startTimer();
        Params::set_K(i * tmpK);

        Analysor::set_params(cap_map,A,Ac,p_0,p);

        Preseter::preset_u(RAND, u_n);
        x_n = encoder.encode(Params::get_N(), u_n);

        while (block_error_count < Params::get_upperBlockErrorNum()) {
            loopi++;

            y_n.assign(Params::get_N(), 0);
            u_est.assign(Params::get_N(), 0);
            y_n = Channel::channel_output(x_n);
            u_est = decoder.calcBP_wang(p, param, x_n, y_n, u_n, A, Ac);

            itr = param[0];
            Analysor::errorCount(u_n, u_est, &error_count);
            if(error_count > 0) block_error_count++;
            if (loopi % 1 == 0 ) cout << loopi << " " << error_count << " " << block_error_count  << " " << itr << " " << (double)block_error_count/loopi << endl;
            error_count = 0;
            rate = (double) Params::get_K() / (Params::get_N()-Params::get_M());

            mitr += itr; // 0:itr, 1:nochecked

            if(loopi >= Params::get_blockNum()) break;
        }
        mitr = (mitr/loopi)>=70 ? 70 : mitr/loopi;
        itrfile << rate << " " << mitr << endl;

        cout << "mitr : "<< mitr << endl;
        cout << "rate : "<< rate << endl;
        performance.stopTimer();

        cout << Params::get_rvbDir() << endl;
        BER = (double)block_error_count/loopi;

        logger.outLog("=================================");
        logger.outLog(performance.outTime("処理時間"));
        logger.outLog("(N,K) = (" + to_string(Params::get_N()) + "," + to_string(Params::get_K()) + ")");
        logger.outLog("BER:" + to_string(BER));
        logger.outLog("Rate:" + to_string(rate));
        logger.outLog(encoder.outCount("encoder_count"));
        logger.outLog(decoder.outCount("decoder_count"));
        performance.outHMS();

        logger.outLogRVB(rate, BER);
        if(mode == TEST) break;
    }
}

//rate vs BERのグラフ作成用
void Analysor::calcBlockErrorRate_mid_wang_BP(MODE mode) {
    Performance performance;
    Decoder decoder;
    Encoder encoder;
    Logger logger;
    logger.setRvbDir(Params::get_rvbDir());

    int size = log2(Params::get_N())+1;
    vector<int> u_n(Params::get_N(), 0);

    vector<int> A;
    vector<int> Ac;

    vector<int> p_0(Params::get_M());
    vector<int> p = Analysor::get_P(p_0);

    vector<vector<int> > tmp_x_n(log2(Params::get_N()), vector<int>(Params::get_N(),0) );
    vector<vector<int> > x_n(size, vector<int>(Params::get_N(),0) );
    vector<vector<double> > y_n(size, vector<double>(Params::get_N(), 0.0));

    vector<int> u_est(Params::get_N(), 0);
    vector<pair<int, double> > cap_map;
    Preseter::makeMutualInfoArray(cap_map);

    double BER = 0.0;
    double sumBER = 0.0;
    double rate = 0.0;
    int n = Params::get_N();
    int error_count = 0;
    int block_error_count = 0;
    int loopi = 0;
    vector<int> param(2, 0);

    time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
    string itrfn = "log/"
                   + to_string(pnow->tm_year+1900) + to_string(pnow->tm_mon + 1) + to_string(pnow->tm_mday)
                   + "/N=" + to_string(Params::get_N())
                   + " e=" + to_string(Params::get_e())
                   + " channel=" + (Params::get_s()?"BSC":"BEC") + " mean_itr midwang";
    ofstream itrfile;
    itrfile.open(itrfn, ios::out);
    double mitr = 0;
    int itr = 0;

    int tmp = Params::get_N()/Params::get_K();
    int tmpK = Params::get_K();
    for (int i = 10; i <= 13; i++) {
        mitr = 0.0;
        loopi = 0;
        sumBER = 0.0;
        block_error_count = 0;
        performance.startTimer();
        Params::set_K(i * tmpK);
        A.resize(Params::get_K(), -1);

        Analysor::set_params(cap_map, A, Ac, p_0, p);

        Preseter::preset_u(RAND, u_n);
        tmp_x_n[log2(Params::get_N()) - 1] = encoder.encode_m(Params::get_N(), 0, 0, u_n, tmp_x_n);

        //uとxをまとめて送る
        for (int i = 0; i < log2(Params::get_N()) + 1; i++) {
            for (int j = 0; j < Params::get_N(); j++) {
                if (i == 0) {
                    x_n[i][j] = u_n[j];
                } else {
                    x_n[i][j] = tmp_x_n[i - 1][j];
                }
            }
        }

        while (block_error_count < Params::get_upperBlockErrorNum()) {
            loopi++;
            u_est.assign(Params::get_N(), 0);

            Channel::channel_output_m(x_n, y_n);
            u_est = decoder.calcBP_m_wang(p, param, x_n, y_n, u_n, A);

            itr = param[0];
            Analysor::errorCount(u_n, u_est, &error_count);
            if(error_count > 0) block_error_count++;
            if (loopi % 1 == 0 ) cout << loopi << " " << error_count << " " << block_error_count  << " " << itr << " " << (double)block_error_count/loopi << endl;
            error_count = 0;
            rate = (double) Params::get_K()/(Params::get_N());
            mitr += itr; // 0:itr, 1:no_checked

            if(loopi >= Params::get_blockNum()) break;
        }

        mitr = (mitr/loopi)>=70 ? 70 : mitr/loopi;
        itrfile << rate << " " << mitr << endl;
        cout << "mitr : "<< mitr << endl;
        cout << "rate : "<< rate << endl;

        performance.stopTimer();

        cout << Params::get_rvbDir() << endl;
        BER = (double)block_error_count/loopi;

        logger.outLog("=================================");
        logger.outLog(performance.outTime("処理時間"));
        logger.outLog("(N,K) = (" + to_string(Params::get_N()) + "," + to_string(Params::get_K()) + ")");
        logger.outLog("BER:" + to_string(BER));
        logger.outLog("Rate:" + to_string(rate));
        logger.outLog(encoder.outCount("encoder_count"));
        logger.outLog(decoder.outCount("decoder_count"));
        performance.outHMS();

        logger.outLogRVB(rate, BER);
        if(mode == TEST) break;
    }
}