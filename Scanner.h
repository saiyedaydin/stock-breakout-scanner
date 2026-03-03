#pragma once
#include "Candle.h"
#include <vector>
#include <string>

struct BreakoutResult {
    bool patternFound      = false;
    std::string breakoutDate;
    double entryPrice      = 0.0;
    double stopLoss        = 0.0;
    double riskPerShare    = 0.0;
    double accountRisk     = 0.0;
    int    suggestedShares = 0;
    double target5R        = 0.0;
    double target8R        = 0.0;
    bool   smaExitActive   = false;
};

class Scanner {
public:
    // Configuration
    static constexpr double MIN_PRICE          = 1.0;
    static constexpr double MIN_AVG_RANGE_PCT  = 5.0;
    static constexpr double MIN_AVG_DOLLAR_VOL = 30'000'000.0;
    static constexpr double ACCOUNT_SIZE       = 50'000.0;
    static constexpr double RISK_PCT           = 0.01;   // 1% of account
    static constexpr int    SMA_PERIOD         = 10;
    static constexpr int    CONTRACTION_WINDOW = 5;      // candles to look back

    explicit Scanner(const std::string& csvPath);

    BreakoutResult scan();

private:
    std::vector<Candle> candles_;

    bool        loadCSV(const std::string& path);
    bool        meetsBaseline() const;
    double      avgRangePct(int lookback = 20) const;
    double      avgDollarVolume(int lookback = 20) const;
    double      sma10(int endIndex) const;
    bool        hasContractionPattern(int endIndex) const;
    bool        hasVolumeDryUp(int endIndex) const;
    bool        isBreakoutCandle(int index) const;
};
