# Stock Breakout Scanner

A C++ command-line project that scans historical stock CSV data for breakout setups based on price contraction, higher lows, lower highs, volume dry-ups, and breakout confirmation.

This project models a rule-based trading strategy by detecting possible entry signals, calculating stop loss levels, estimating position size based on account risk, and tracking basic trade management rules.

## Why I Built This

I built this project to combine my interest in stock trading and technical analysis with my C++ programming background. Instead of making a generic market-data project, I wanted to create something based on a real breakout strategy with clear rules for entries, stop losses, risk sizing, and exits.

## Strategy Rules

The scanner looks for stocks that meet the following baseline conditions:

- Price > $1
- Average daily range % >= 5%
- Average dollar volume >= $30,000,000

The setup is based on these ideas:

- Price begins to tighten through lower highs and higher lows
- Volume starts to decrease during the contraction
- A narrow “thin” candle appears before the breakout
- A breakout is confirmed when the next candle moves above the previous day’s high

## Trade Logic

When a breakout is detected:

- Entry is triggered above the previous day’s high
- Stop loss is placed at the low of the breakout day
- Risk per trade is limited to 1% of account size
- Position size is calculated using:

`Shares = Risk / (Entry - Stop)`

Trade management rules:

- After reaching 5R profit, trim part of the position
- After trimming, move the stop loss to break-even
- If profit reaches 7R to 10R, trim more into strength
- Exit the remaining position when price closes below the 10-day SMA

## Features

- Reads OHLCV stock data from CSV files
- Parses daily candle data into C++ objects
- Computes:
  - average daily range %
  - average dollar volume
  - 10-day simple moving average
- Detects:
  - higher lows
  - lower highs
  - contraction patterns
  - volume dry-ups
  - breakout candles
- Calculates:
  - entry
  - stop loss
  - per-share risk
  - position size
  - profit targets based on R multiples

## Build and Run

```bash
make
./scanner data/sample.csv
```
## Example Output
```text
Stock Breakout Scanner
----------------------
Pattern Found: Yes
Breakout Date: 2026-02-20
Entry Price: 106.20
Stop Loss: 101.80
Risk Per Share: 4.40
Account Risk: 500.00
Suggested Shares: 113
5R Target: 128.20
8R Target: 141.40
10-day SMA Exit Active: Yes
```

