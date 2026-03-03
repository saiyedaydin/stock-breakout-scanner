#pragma once
#include <string>

struct Candle {
    std::string date;
    double open;
    double high;
    double low;
    double close;
    long long volume;

    double range() const { return high - low; }
    double rangePercent() const { return (close > 0) ? (range() / close) * 100.0 : 0.0; }
    double dollarVolume() const { return close * volume; }
};
