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
#include <numeric>
#include <iomanip>

using namespace std;

class buyer;
class seller;

mutex mtx;
condition_variable cv;

chrono::microseconds time_to_update(1000000);//10000000000000);
bool timer_updated = false;
atomic<bool> update_running = false;

double epsilon = 0.00000000001;
double max_price_double = 999999;

double big_epsilon = 0.0002;

int cnt = 0; 

chrono::steady_clock::time_point last_update;
vector<double> prices;
vector<double> times;
vector<double> us;
double all_value = 0;

set<buyer> buyers;
set<seller> sellers;

void update_timer(double seconds){
    using namespace std::chrono;
    duration<double> sec_duration(seconds);
    microseconds duration = duration_cast<microseconds>(sec_duration);
    //lock_guard<mutex> lock(mtx);
    time_to_update = duration;
    timer_updated = true;
    cv.notify_all();
}

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
    double p_l;
    double p_h;
    double u_max;
    double q_max;
    double t_max;
    double t = -1;
    double q = 0;

    double u_demand(double p){
        if(p >= p_h){
            return 0;
        }
        if(p <= p_l){
            return u_max;
        }
        double u = u_max * (p_h - p) / (p_h - p_l);
        return u;
    }
    buyer(double pl, double ph, double umax, double qmax, double tmax) 
        : p_l(pl), p_h(ph), u_max(umax), q_max(qmax), t_max(tmax) {}
    
    bool operator<(const buyer& other) const {
        if (p_l != other.p_l) return p_l < other.p_l;
        if (p_h != other.p_h) return p_h < other.p_h;
        if (u_max != other.u_max) return u_max < other.u_max;
        if (q_max != other.q_max) return q_max < other.q_max;
        return t_max < other.t_max;
    }

    
};
class seller{
public:
    double p_l;
    double p_h;
    double u_max;
    double q_max;
    double t_max;
    double t = -1;
    double q = q_max;
    
    double u_supply(double p){
        if(p <= p_l){
            return 0;
        }
        if(p >= p_h){
            return u_max;
        }
        double u = u_max * (p - p_l) / (p_h - p_l);
        return u;
    }
    seller(double pl, double ph, double umax, double qmax, double tmax) 
        : p_l(pl), p_h(ph), u_max(umax), q_max(qmax), t_max(tmax) {}
    

    bool operator<(const seller& other) const {
        if (p_l != other.p_l) return p_l < other.p_l;
        if (p_h != other.p_h) return p_h < other.p_h;
        if (u_max != other.u_max) return u_max < other.u_max;
        if (q_max != other.q_max) return q_max < other.q_max;
        return t_max < other.t_max;
    }
};



double overage(double price, set<buyer>& set_buyers, set<seller>& set_sellers){
    double sum = 0;

    for(buyer order : set_buyers){
        sum -= order.u_demand(price);
    }
    for(seller order : set_sellers){
        sum += order.u_supply(price);
    }

    return sum;
}





void update_stocks() {
    //lock_guard<mutex> lock(mtx);
    update_running = true;

    double value = 0;
    double u = 0;
    //cout_time();

    double price = prices.back();
    auto now = chrono::steady_clock::now();
    chrono::duration<double> time_passed2 = now - last_update;
    double time_passed = time_passed2.count();
    //cout << "time passed " << time_passed << endl;
    last_update = now;

    //удаление исполнившихся
    for (auto it = buyers.begin(); it != buyers.end();){
        buyer order = *it; 
        it = buyers.erase(it);
        if (order.t != -1) {
            order.q += order.u_demand(price) * time_passed;
            order.t += time_passed;
            //if(order.q > order.q_max + epsilon or order.t > order.t_max + epsilon){
            //    cout << "AAAAAA " << order.q << " " << order.q_max << " " << order.t << " " << order.t_max << endl;
            //}
            if ((order.t_max - order.t < big_epsilon) or (order.q_max - order.q < big_epsilon)) {
                ofstream outfile_value("part_compl_cont.txt", ios::app);
                //cout << double(int(order.q / order.q_max * 1000)) / 10 << "%" << endl;
                outfile_value << double(int(order.q / order.q_max * 100000)) / 1000 << " " << order.q_max  << " " << min(order.t, order.t_max) <<"\n";
                outfile_value.close();
                continue; 
            }
        }
        else {
            order.t = 0;
        }
        buyers.insert(order);
    }
    for (auto it = sellers.begin(); it != sellers.end();){
        seller order = *it; 
        it = sellers.erase(it);
        if (order.t != -1) {
            order.q -= order.u_supply(price) * time_passed;
            order.t += time_passed;
            //if(order.q < -epsilon or order.t > order.t_max + epsilon){
            //    cout << "AAAAAA " << order.q << " " << order.t << " " << order.t_max << endl;
            //}
            if ((order.t_max - order.t < big_epsilon) or (order.q < big_epsilon)) {
                ofstream outfile_value("part_compl_cont.txt", ios::app);
                //cout << double(int((order.q_max - order.q) / order.q_max * 1000)) / 10 << "%" << endl;
                outfile_value << double(int((order.q_max - order.q) / order.q_max * 100000)) / 1000 << " " << order.q_max << " " << min(order.t, order.t_max) << "\n";
                outfile_value.close(); 
                continue; 
            }
        }
        else {
            order.t = 0;
        }
        sellers.insert(order);
    }

    //поиск равновесной

    double left_p = 0;
    double right_p = max_price_double;

    while (right_p - left_p > epsilon){
        double m = (left_p + right_p) / 2;

        if(overage(m, buyers, sellers) > 0){
            right_p = m;
        }
        else{
            left_p = m;
        }
    } 

    for(buyer order : buyers){
        u += order.u_demand(prices.back());
    }
    us.push_back(u);
    prices.push_back(right_p);
    times.push_back(times.back() + time_passed);
    
    //поиск времени до нового обновления
    double min_update_time = 99999999;
    
    for(auto order : buyers){
        double min_time = order.t_max - order.t;
        if(order.u_demand(prices.back()) > epsilon){
            min_time = min(order.t_max - order.t, 
                (order.q_max - order.q) / order.u_demand(prices.back()));

        }
        min_update_time = min(min_update_time, min_time);
    }
    for(auto order : sellers){
        double min_time = order.t_max - order.t;
        if(order.u_supply(prices.back()) > epsilon){
            min_time = min(order.t_max - order.t,
                order.q / order.u_supply(prices.back()));
        }
        min_update_time = min(min_update_time, min_time);
    }
    ofstream outfile1("to_cont_order_book.txt", ios::app);
    for(auto order : buyers){
        outfile1 << "b " << order.p_l << " " << order.p_h << " " << order.u_max << "\n";
    }
    for(auto order : sellers){
        outfile1 << "s " << order.p_l << " " << order.p_h << " " << order.u_max << "\n";
    }
    outfile1.close();
    now = chrono::steady_clock::now();
    auto delta = now - last_update;
    min_update_time = max(0.0, min_update_time - chrono::duration<double>(delta).count());

    //cout << min_update_time << " mut" << endl;

    update_timer(min_update_time);

    float sum = 0;
    for(seller order : sellers){
        sum += order.u_supply(price);
    }
    //cout << prices.back() << " "  << std::fixed << std::setprecision(15) << times.back() << endl;

    ofstream outfile("continious_to_broker.txt", ios::app);
    all_value += us[us.size() - 2] * time_passed;
    cout << "+ " << us[us.size() - 2] * time_passed << " = " << all_value << endl;
    //cout << prices.back()  << " " << u << endl;
    outfile <<  std::fixed << std::setprecision(15) << prices.back() << " " << std::fixed << std::setprecision(15) << times.back() << "\n";
    //cout << "cnt " << cnt << endl;
    update_running = false;
    
}


void make_order(char type, double p_l, double p_h, double u_max, double q_max, double t_max){
    if(type == 'b'){
        buyers.emplace(p_l, p_h, u_max, q_max, t_max);
    }
    else if(type == 's'){
        sellers.emplace(p_l, p_h, u_max, q_max, t_max);
    }
    else{
        cout << "!!!!!!!!!!!" << endl;
    }
    if(!update_running){
        //cout << "make order update" << endl;
        update_stocks();
    }
}





void timer(){
    unique_lock<mutex> lock(mtx);

    auto now = chrono::steady_clock::now();
    auto next_wake = now + time_to_update;
    while(true){
        cv.wait_until(lock, next_wake, [] {return timer_updated;});

        if (timer_updated){
            timer_updated = false;
            next_wake = chrono::steady_clock::now() + time_to_update;
            continue;
        }
        if(!update_running){
            //cout << "timer update" << endl;
            update_stocks();
        }
    }
}

void check_file(){
    //lock_guard<mutex> lock(mtx);
    while(true){
        while(true){
            this_thread::sleep_for(0.01ms);
            if (filesystem::file_size("broker_to_continious.txt") != 0){
                break;
            }
            
        }
        ifstream fin("broker_to_continious.txt");
        char type;
        double p_l,p_h,u_max,q_max,t_max;
        fin >> type >> p_l >> p_h >> u_max >> q_max >> t_max;
        //cout << type << " " << p_l << " " << p_h << endl;
        ofstream("broker_to_continious.txt");
        cnt ++;
        //cout << "cnt " << cnt << endl;
        make_order(type,p_l,p_h,u_max,q_max,t_max);
        
        if(!update_running){
            //cout << "file update" << endl;
            update_stocks();
        }
    }
}





int main() {
    last_update = chrono::steady_clock::now();
    prices.push_back(100.0);
    times.push_back(0.0);
    us.push_back(0.0);


    

    thread timer_thread(timer);
    thread check_file_thread(check_file);

    timer_thread.join();
    check_file_thread.join();
    
    return 0;
}
