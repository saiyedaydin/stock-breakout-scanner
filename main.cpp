#include "Scanner.h"
#include <iostream>
#include <iomanip>
#include <string>

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: ./scanner <path/to/data.csv>\n";
        return 1;
    }

    std::string csvPath = argv[1];

    Scanner scanner(csvPath);
    BreakoutResult result = scanner.scan();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\nStock Breakout Scanner\n";
    std::cout << "----------------------\n";
    std::cout << "Pattern Found:      " << (result.patternFound ? "Yes" : "No") << "\n";

    if (result.patternFound) {
        std::cout << "Breakout Date:      " << result.breakoutDate      << "\n";
        std::cout << "Entry Price:        " << result.entryPrice         << "\n";
        std::cout << "Stop Loss:          " << result.stopLoss           << "\n";
        std::cout << "Risk Per Share:     " << result.riskPerShare       << "\n";
        std::cout << "Account Risk:       " << result.accountRisk        << "\n";
        std::cout << "Suggested Shares:   " << result.suggestedShares    << "\n";
        std::cout << "5R Target:          " << result.target5R           << "\n";
        std::cout << "8R Target:          " << result.target8R           << "\n";
        std::cout << "10-day SMA Exit Active: " << (result.smaExitActive ? "Yes" : "No") << "\n";
    }

    std::cout << "\n";
    return 0;
}
