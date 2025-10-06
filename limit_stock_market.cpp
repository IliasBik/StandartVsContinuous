#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream> 
#include<vector>
#include<string>
#include<algorithm>
#include<set>
#include <iomanip>

using namespace std;


class buyer;
class seller;

mutex mtx;
condition_variable cv;


double epsilon = 0.00000000001;
double max_price_double = 999999;

double big_epsilon = 0.02;

int cnt = 0;
int id = 1;

vector<double> prices;
vector<double> times;

double all_value = 0;

chrono::steady_clock::time_point last_update;

set<buyer> buyers;
set<seller> sellers;

void cout_time(){
    auto now = chrono::system_clock::now();
    auto now_time = chrono::system_clock::to_time_t(now);
    auto now_ms = chrono::time_point_cast<chrono::milliseconds>(now);

    auto now_c = chrono::system_clock::to_time_t(now);
    auto ms = now_ms.time_since_epoch().count() % 1000;

    cout << "update_stocks at: " 
              << put_time(localtime(&now_time), "%H:%M:%S")
              << '.' << setfill('0') << setw(3) << ms
              << endl;  
}


class buyer{
public:
    double p;
    double q_max;
    double t_max;
    chrono::steady_clock::time_point start;
    double q_start;
    double t = -1;
    int id;
    buyer(double p, double q_max, double t_max, chrono::steady_clock::time_point start, double q_start, int id)
        : p(p), q_max(q_max), t_max(t_max), start(start), q_start(q_start), id(id) {}
    
    bool operator<(const buyer& other) const{
        if(p != other.p) return p > other.p;
        return start < other.start;
    }
};

class seller{
public:
    double p;
    double q_max;
    double t_max;
    chrono::steady_clock::time_point start;
    double q_start;
    double t = -1;
    int id;

    seller(double p, double q_max, double t_max, chrono::steady_clock::time_point start, double q_start,int id)
        : p(p), q_max(q_max), t_max(t_max), start(start), q_start(q_start), id(id) {}
    
    bool operator<(const seller& other) const{
        if(p != other.p) return p < other.p;
        return start < other.start;
    }
};

void update_stocks(char type, double p, double q_max, double t_max, int id){
    auto nw = chrono::steady_clock::now();
    chrono::duration<double> time_passed2 = nw - last_update;
    double time_passed = time_passed2.count();
    last_update = nw;
    
    double last_price = prices.back();
    double value = 0;
    if(type == 'b'){
        if(sellers.empty()){
            buyers.emplace(p, q_max, t_max,nw, q_max, id);
        }
        else if((*sellers.begin()).p > p){
            buyers.emplace(p, q_max, t_max,nw, q_max, id);
        }
        else{
            double q = q_max;
            auto it = sellers.begin();
            bool flag = true;
            while(it != sellers.end()){
                seller order = *it;
                chrono::duration<double> elapsed_seconds = nw - order.start;
                double left = elapsed_seconds.count();
                if(left > order.t_max){
                    ofstream outfile_value("part_compl_lim.txt", ios::app);
                    //cout << int((order.q_start - order.q_max) / order.q_start * 100) << "%" << endl;
                    outfile_value << double(int((order.q_start - order.q_max) / order.q_start * 100000)) / 1000 << " " << order.q_start << " " << order.t_max << "\n";
                    outfile_value.close();
                    it++;
                }
                else if(order.p > p){
                    buyers.emplace(p, q, t_max,nw,q_max,id);
                    it++;
                    sellers.erase(sellers.begin(), it);
                    sellers.insert(order);
                    flag = false;
                    break;
                }
                else{
                    if(q > order.q_max){
                        ofstream outfile_value("part_compl_lim.txt", ios::app);
                        //cout << int((order.q_start - order.q_max) / order.q_start * 100) << "%" << endl;
                        //outfile_value << double(int((order.q_start - order.q_max) / order.q_start * 1000)) / 10 << " " << order.q_start << "\n";
                        chrono::duration<double> elapsed_seconds = nw - order.start;
                        double left = elapsed_seconds.count();
                        outfile_value << "100 " << order.q_start << " " << left << "\n";
                        outfile_value.close();
                        it++;
                        value += order.q_max;
                        q -= order.q_max;
                        last_price = order.p;
                    }
                    else if(q < order.q_max){
                        ofstream outfile_value("part_compl_lim.txt", ios::app);
                        //cout << "100%" << endl;
                        outfile_value << "100 " << q_max << " 0" << "\n";
                        outfile_value.close();
                        value += q;
                        order.q_max -= q;
                        last_price = order.p;
                        it++;
                        sellers.erase(sellers.begin(), it);
                        sellers.insert(order);
                        flag = false;
                        break;
                    }
                    else{ //q = q_max
                        //cout << "100%" << endl;
                        ofstream outfile_value("part_compl_lim.txt", ios::app);
                        //cout << int((order.q_start - order.q_max) / order.q_start * 100) << "%" << endl;
                        //outfile_value << double(int((order.q_start - order.q_max) / order.q_start * 1000)) / 10 << " " << order.q_start << "\n"; 
                        outfile_value << "100 " << q_max << " 0" << "\n";
                        chrono::duration<double> elapsed_seconds = nw - order.start;
                        double left = elapsed_seconds.count();
                        outfile_value << "100 " << order.q_start << left <<"\n";
                        outfile_value.close();
                        value += q;
                        last_price = order.p;
                        it++;
                        sellers.erase(sellers.begin(), it);
                        flag = false;
                        break;
                    }
                }
            }
            if(flag){
                sellers.erase(sellers.begin(), sellers.end());
                buyers.emplace(p, q, t_max,nw,q_max,id);
            }
            
        }
    }
    else{
        if(buyers.empty()){
            sellers.emplace(p, q_max, t_max,nw,q_max,id);
        }
        else if((*buyers.begin()).p < p){
            sellers.emplace(p, q_max, t_max,nw,q_max,id);
        }
        else{
            double q = q_max;
            auto it = buyers.begin();
            bool flag = true;
            while(it != buyers.end()){
                buyer order = *it;
                chrono::duration<double> elapsed_seconds = nw - order.start;
                double left = elapsed_seconds.count();
                if(left > order.t_max){
                    ofstream outfile_value("part_compl_lim.txt", ios::app);
                    //cout << int((order.q_start - order.q_max) / order.q_start * 100) << "%" << endl;
                    outfile_value << double(int((order.q_start - order.q_max) / order.q_start * 100000)) / 1000 << " " << order.q_start << " " << t_max << "\n";
                    outfile_value.close();
                    it++;
                }
                else if(order.p < p){
                    sellers.emplace(p, q, t_max,nw,q_max,id);
                    it++;
                    buyers.erase(buyers.begin(), it);
                    buyers.insert(order);
                    flag = false;
                    break;

                }
                else{
                    if(q > order.q_max){
                        ofstream outfile_value("part_compl_lim.txt", ios::app);
                        //cout << int((order.q_start - order.q_max) / order.q_start * 100) << "%";
                        //outfile_value << double(int((order.q_start - order.q_max) / order.q_start * 1000)) / 10 << " " << order.q_start << "\n";
                        chrono::duration<double> elapsed_seconds = nw - order.start;
                        double left = elapsed_seconds.count();
                        outfile_value << "100 " << order.q_start << " " << left << "\n";
                        outfile_value.close();
                        it++;
                        value += order.q_max;
                        q -= order.q_max;
                        last_price = order.p;
                    }
                    else if(q < order.q_max){
                        ofstream outfile_value("part_compl_lim.txt", ios::app);
                        //cout << "100%" << endl;
                        outfile_value << "100 " << q_max << " 0" << "\n";
                        outfile_value.close();
                        value += q;
                        order.q_max -= q;
                        last_price = order.p;
                        it++;
                        buyers.erase(buyers.begin(), it);
                        buyers.insert(order);
                        flag = false;
                        break;
                    }
                    else{ //q = q_max
                        //cout << "100%" << endl;
                        ofstream outfile_value("part_compl_lim.txt", ios::app);
                        //cout << int((order.q_start - order.q_max) / order.q_start * 100) << "%" << endl;
                        //outfile_value << double(int((order.q_start - order.q_max) / order.q_start * 1000)) / 10 << " " << order.q_start << "\n";
                        chrono::duration<double> elapsed_seconds = nw - order.start;
                        double left = elapsed_seconds.count();
                        outfile_value << "100 " << order.q_start << " " << left << "\n";
                        outfile_value << "100 " << q_max << " 0" << "\n";
                        outfile_value.close();
                        value += q;
                        last_price = order.p;
                        it++;
                        buyers.erase(buyers.begin(), it);
                        flag = false;
                        break;
                    }
                }
            }
            if(flag){
                buyers.erase(buyers.begin(), buyers.end());
                sellers.emplace(p, q, t_max,nw,q_max,id);
            }
            
        }
    }
    int cnt_b = 0;
    ofstream outfile1("to_lim_order_book.txt");
    for(auto order : buyers){
        chrono::duration<double> elapsed_seconds = nw - order.start;
        double left = elapsed_seconds.count();
        if(left <= order.t_max){
            outfile1 << "b " << order.p << " " << order.q_max << "\n";
            //cout << "b " << order.p << " " << order.q_max << "\n";
            cnt_b++;
        }
    }
    int cnt_s = 0;
    for(auto order : sellers){
        chrono::duration<double> elapsed_seconds = nw - order.start;
        double left = elapsed_seconds.count();
        if(left <= order.t_max){
            outfile1 << "s " << order.p << " " << order.q_max << "\n";
            //cout << "s " << order.p << " " << order.q_max << "\n";
            cnt_s ++;
        }
    }
    outfile1.close();

    //cout << cnt_b << " + " << cnt_s << endl;



    all_value += value;
    cout << "+ " << value << " = " << all_value << endl;
    //cout << last_price << endl;
    prices.push_back(last_price);
    times.push_back(times.back() + time_passed);

    ofstream outfile("limit_to_broker.txt", ios::app);
    outfile <<  std::fixed << std::setprecision(15) << prices.back() << " " << std::fixed << std::setprecision(15) << times.back() << "\n";
}



void check_file(){
    while(true){
        while(true){
            this_thread::sleep_for(0.01ms);
            if (filesystem::file_size("broker_to_limit.txt") != 0){
                break;
            }
            
        }
        ifstream fin("broker_to_limit.txt");
        char type;
        double p, q_max, t_max;
        fin >> type >> p >> q_max >> t_max;
        ofstream("broker_to_limit.txt");
        cnt ++;
        //cout << "cnt " << cnt << endl;
        update_stocks(type,p,q_max,t_max, id);
        id++;
        cout << "NEW TIME" << endl;
        for(auto i : buyers){
            cout << double(int((i.q_start - i.q_max) / i.q_start * 100000)) / 1000 << " " << i.q_start << " " << i.t_max << endl;
        }
        for(auto i : sellers){
            cout << double(int((i.q_start - i.q_max) / i.q_start * 100000)) / 1000 << " " << i.q_start << " " << i.t_max << endl;
        }
    }
}

int main(){
    last_update = chrono::steady_clock::now();

    prices.push_back(100.0);
    times.push_back(0.0);
    


    check_file();
    
    return 0;
}
