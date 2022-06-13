#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <memory>

/*
 * compile using g++, e.g.
 * % g++ -o report report.cxx
 * or using Intel compiler e.g.
 * % icc -x c++ -o report report.cxx
 * run % cat input.csv | ./report > output.csv
 * test % ./test.sh output.csv input.csv
 * I spend below 2h on the problem, I tested my results first manually
 * for every column of output file using pieces of 'test.sh' 
 * so I knew that code works at least on the data supplied
 * but when I combined together all the pieces of my test,
 * I found out that last column (max price) comparison does not 
 * work because of extra character and I spent more than an hour 
 * to find it out.
 * uint64_t or unsigned 64 bit integer is the largest type 
 * supported by C++
 * however its upper boundary is not large enough to serve the purpose
 * of this task, i.e. it can be overwritten by any calculation involving summation.
 * Therefore it would be  benefitial to use GNU multiprecision library to handle 
 * integer operations with upper limit higher than what can provide uint64_t.
 * I am going to make this code type dependent, so that it can be extended by using GMP 
 * or any other library.
 */

enum securityInput {
    TIMESTAMP,
    TICKER,
    QUANTITY,
    PRICE
};

template <typename T> struct tickerData {
    T ts;
    T qty;
    T price;
};

template <typename T> struct tickerReport {
    T totalVol;
    T maxPrice;
    T totalPVol;
    T maxTimeGap;
    T prevTime;

    tickerReport(){}
    tickerReport(const tickerData<T>& d);
    void update(const tickerData<T>& d); 

    //uint64_t wap(){ return totalPVol/totalVol; }
};

template<>
tickerReport<uint64_t>::tickerReport(const tickerData<uint64_t>& d): totalVol(d.qty),
                                        maxPrice(d.price),
                                        totalPVol(d.qty*d.price), 
                                        prevTime(d.ts){}

template<>
void tickerReport<uint64_t>::update(const tickerData<uint64_t>& d) {
        totalVol += d.qty;
        totalPVol += d.qty * d.price;
        if (d.price > maxPrice) {
            maxPrice = d.price;
        }
        auto gap = d.ts - prevTime;
        if (gap > maxTimeGap) {
            maxTimeGap = gap;
        }
        prevTime = d.ts;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const tickerReport<T>& tr);

template<>
std::ostream& operator<<(std::ostream& os, const tickerReport<uint64_t>& tr) {
    os << tr.maxTimeGap 
       << "," 
       << tr.totalVol 
       << "," 
       << tr.totalPVol/tr.totalVol 
       << ","
       << tr.maxPrice;
    return os;
}

// typedef std::map<std::string, std::unique_ptr<tickerReport> > tReport; 

template <typename T>
struct reportBook {
    std::map<std::string, std::unique_ptr<tickerReport<T> > > rpt;
    void report();
    void process (const std::string& k, const tickerData<T>& v); 
};

template <>
void reportBook<uint64_t>::report() {
    for (const auto &p: rpt){
        std::cout << p.first << "," << *(p.second) << "\n";
   }
}

template <>
void reportBook<uint64_t>::process(const std::string& k, const tickerData<uint64_t>& v) {
    auto x = rpt.find(k);
    if (x == rpt.end()) {
        rpt.emplace(k,std::make_unique<tickerReport<uint64_t> >(v));
    } else {
        x->second->update(v);
    }
}

int main(int argc, char *argv[]) {
    reportBook<uint64_t> book;
    std::string line;

    for (; std::getline(std::cin, line); ){
        std::stringstream ss(line);
        tickerData<uint64_t> d;
        std::string ticker;
        for (int i=0; ss.good(); i++) {
            std::string substr;
            std::getline(ss, substr,',');
            switch (i) {
                case TIMESTAMP:
                    d.ts = std::stoull(substr,nullptr,0);
                    break;
                case TICKER:
                    ticker = substr;
                    break;
                case QUANTITY:
                    d.qty = std::stoull(substr,nullptr,0);
                    break;
                case PRICE:
                    d.price = std::stoull(substr,nullptr,0);
                    break;
                default:
                    std::cerr << "Error: " << line << "\n";
                    return 1;
            }
        }
        book.process(ticker, d);
    }

    book.report();
    return 0;
}
