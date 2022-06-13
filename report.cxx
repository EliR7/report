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
 */

enum securityInput {
    TIMESTAMP,
    TICKER,
    QUANTITY,
    PRICE
};

struct tickerData {
    uint64_t ts;
    uint64_t qty;
    uint64_t price;
};

struct tickerReport {
    uint64_t totalVol;
    uint64_t maxPrice;
    uint64_t totalPVol;
    uint64_t maxTimeGap;
    uint64_t prevTime;

    tickerReport(){}
    tickerReport(const tickerData& d) : totalVol(d.qty),
                                        maxPrice(d.price),
                                        totalPVol(d.qty*d.price), 
                                        prevTime(d.ts){}
    void update(const tickerData& d) {
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

    //uint64_t wap(){ return totalPVol/totalVol; }
};

std::ostream& operator<<(std::ostream& os, const tickerReport& tr) {
    os << tr.maxTimeGap 
       << "," 
       << tr.totalVol 
       << "," 
       << tr.totalPVol/tr.totalVol 
       << ","
       << tr.maxPrice;
    return os;
}

typedef std::map<std::string, std::unique_ptr<tickerReport> > tReport; 

struct reportBook {
    tReport rpt;
    void report() {
        for (const auto &p: rpt){
            std::cout << p.first << "," << *(p.second) << "\n";
        }
    }
    void process (const std::string& k, const tickerData& v) {
        auto x = rpt.find(k);
        if (x == rpt.end()) {
            rpt.emplace(k,std::make_unique<tickerReport>(v));
        } else {
            x->second->update(v);
        }
    }
};

int main(int argc, char *argv[]) {
    reportBook book;
    std::string line;

    for (; std::getline(std::cin, line); ){
        std::stringstream ss(line);
        tickerData d;
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
