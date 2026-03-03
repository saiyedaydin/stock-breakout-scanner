#include "Scanner.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>

// ─── Constructor ─────────────────────────────────────────────────────────────

Scanner::Scanner(const std::string& csvPath) {
    if (!loadCSV(csvPath)) {
        std::cerr << "Error: could not load CSV file: " << csvPath << "\n";
    }
}

// ─── CSV Loader ───────────────────────────────────────────────────────────────
// Expects header: date,open,high,low,close,volume

bool Scanner::loadCSV(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;

    std::string line;
    bool firstLine = true;

    while (std::getline(file, line)) {
        if (firstLine) { firstLine = false; continue; }  // skip header
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;
        Candle c;

        try {
            std::getline(ss, c.date,   ',');
            std::getline(ss, token,    ','); c.open   = std::stod(token);
            std::getline(ss, token,    ','); c.high   = std::stod(token);
            std::getline(ss, token,    ','); c.low    = std::stod(token);
            std::getline(ss, token,    ','); c.close  = std::stod(token);
            std::getline(ss, token,    ','); c.volume = std::stoll(token);
        } catch (...) {
            std::cerr << "Warning: skipping malformed row: " << line << "\n";
            continue;
        }

        candles_.push_back(c);
    }

    return !candles_.empty();
}

// ─── Baseline Filter ──────────────────────────────────────────────────────────

bool Scanner::meetsBaseline() const {
    if (candles_.empty()) return false;

    double lastClose = candles_.back().close;
    if (lastClose <= MIN_PRICE)                   return false;
    if (avgRangePct()    < MIN_AVG_RANGE_PCT)     return false;
    if (avgDollarVolume() < MIN_AVG_DOLLAR_VOL)   return false;

    return true;
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

double Scanner::avgRangePct(int lookback) const {
    int n = static_cast<int>(candles_.size());
    int start = std::max(0, n - lookback);
    double sum = 0.0;
    int count = 0;
    for (int i = start; i < n; ++i) {
        sum += candles_[i].rangePercent();
        ++count;
    }
    return (count > 0) ? sum / count : 0.0;
}

double Scanner::avgDollarVolume(int lookback) const {
    int n = static_cast<int>(candles_.size());
    int start = std::max(0, n - lookback);
    double sum = 0.0;
    int count = 0;
    for (int i = start; i < n; ++i) {
        sum += candles_[i].dollarVolume();
        ++count;
    }
    return (count > 0) ? sum / count : 0.0;
}

double Scanner::sma10(int endIndex) const {
    if (endIndex < SMA_PERIOD - 1) return 0.0;
    double sum = 0.0;
    for (int i = endIndex - SMA_PERIOD + 1; i <= endIndex; ++i) {
        sum += candles_[i].close;
    }
    return sum / SMA_PERIOD;
}

// ─── Pattern Detection ────────────────────────────────────────────────────────

// Looks for lower highs AND higher lows in a window → price is contracting
bool Scanner::hasContractionPattern(int endIndex) const {
    int start = endIndex - CONTRACTION_WINDOW + 1;
    if (start < 1) return false;

    bool lowerHighs = true;
    bool higherLows = true;

    for (int i = start + 1; i <= endIndex; ++i) {
        if (candles_[i].high  >= candles_[i - 1].high) lowerHighs = false;
        if (candles_[i].low   <= candles_[i - 1].low)  higherLows = false;
    }

    return lowerHighs && higherLows;
}

// Volume has been declining over the contraction window
bool Scanner::hasVolumeDryUp(int endIndex) const {
    int start = endIndex - CONTRACTION_WINDOW + 1;
    if (start < 1) return false;

    // Average volume in the window should be below the 20-day average
    double windowVol = 0.0;
    for (int i = start; i <= endIndex; ++i) {
        windowVol += candles_[i].volume;
    }
    windowVol /= CONTRACTION_WINDOW;

    double baselineVol = 0.0;
    int baseStart = std::max(0, endIndex - 20);
    int count = 0;
    for (int i = baseStart; i <= endIndex; ++i) {
        baselineVol += candles_[i].volume;
        ++count;
    }
    baselineVol /= count;

    return windowVol < (baselineVol * 0.75);   // window volume < 75 % of baseline
}

// A "thin" candle: range is narrower than 50 % of the 5-day average range
bool Scanner::isBreakoutCandle(int index) const {
    if (index < 1) return false;
    const Candle& prev = candles_[index - 1];
    const Candle& curr = candles_[index];

    // Breakout: current close moves above previous high
    return curr.close > prev.high;
}

// ─── Main Scan ────────────────────────────────────────────────────────────────

BreakoutResult Scanner::scan() {
    BreakoutResult result;

    if (candles_.size() < static_cast<size_t>(SMA_PERIOD + CONTRACTION_WINDOW + 2)) {
        std::cerr << "Not enough candle data to scan.\n";
        return result;
    }

    if (!meetsBaseline()) {
        std::cerr << "Stock does not meet baseline filter criteria.\n";
        return result;
    }

    int n = static_cast<int>(candles_.size());

    // Scan from the earliest valid index up to the last candle
    for (int i = SMA_PERIOD + CONTRACTION_WINDOW; i < n; ++i) {

        // The "setup" window ends at i-1; the breakout candle is i
        int setupEnd = i - 1;

        if (!hasContractionPattern(setupEnd)) continue;
        if (!hasVolumeDryUp(setupEnd))        continue;
        if (!isBreakoutCandle(i))             continue;

        // ── Signal found ──────────────────────────────────────────────────
        const Candle& setupCandle    = candles_[setupEnd];
        const Candle& breakoutCandle = candles_[i];

        double entry      = setupCandle.high + 0.01;     // penny above prior high
        double stop       = breakoutCandle.low;
        double riskPerShr = entry - stop;

        if (riskPerShr <= 0.0) continue;                 // invalid setup

        double accountRisk     = ACCOUNT_SIZE * RISK_PCT;
        int    suggestedShares = static_cast<int>(accountRisk / riskPerShr);

        double sma = sma10(i);
        bool   smaExit = breakoutCandle.close > sma;    // price is still above SMA

        result.patternFound    = true;
        result.breakoutDate    = breakoutCandle.date;
        result.entryPrice      = entry;
        result.stopLoss        = stop;
        result.riskPerShare    = riskPerShr;
        result.accountRisk     = accountRisk;
        result.suggestedShares = suggestedShares;
        result.target5R        = entry + (5.0 * riskPerShr);
        result.target8R        = entry + (8.0 * riskPerShr);
        result.smaExitActive   = smaExit;

        // Return the most recent valid signal
    }

    return result;
}
