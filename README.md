# StandartVsContinuous
Simulation comparing a classic limit order exchange and a fully continuous one. Based on “Toward a Fully Continuous Exchange” (A.S. Kyle, J. Lee, 2017). Includes Python brokers, C++ market exchanges, and live visualization of liquidity and volatility.


A simulation comparing two types of markets — a **classic limit order exchange** and a **fully continuous exchange**, based on the paper  
**"Toward a Fully Continuous Exchange" (Albert S. Kyle, Jeongmin Lee, 2017)**.

---

## Project Overview

This project simulates how two exchanges behave under similar trading conditions:
- One follows the **traditional limit order model**.
- The other implements a **continuous-time, continuous-price exchange**.

Each exchange receives the same stream of randomly generated orders with:
- Maximum volume `qmax`
- Lifetime `tmax`
- Price bounds (upper/lower)
- Random price forecasts based on stochastic movements

The goal is to analyze and compare:
- **Price volatility** (standard deviation over time)
- **Total traded volume**
- **Liquidity and predictability**
- **Response to parameters** (e.g., `tmax`, `umax`)

---

## Architecture

### 1. Python — Broker & Visualizer
- Generates random buy/sell orders with Poisson-distributed timing (mean λ per second).
- Sends orders to both exchanges.
- Reads back prices and order book snapshots.
- Plots:
  - Price evolution of both exchanges
  - Liquidity (bid/ask curves)
  - Derived metrics (e.g., volatility, volume)

### 2. C++ — Market Engines
Two separate C++ programs simulate the exchanges.

#### Limit Exchange
- Maintains **two sorted lists** (buy/sell orders).
- Orders are matched if overlapping prices exist.
- Execution occurs **at the price of existing opposite orders**.
- Remaining unfilled orders enter the order book.

#### Continuous Exchange
- Uses **binary search** to find equilibrium price `p*` where total demand = total supply.
- Each trader defines linear demand/supply functions:
  - 0 volume below lower bound
  - Max volume above upper bound
  - Linear growth in between
- Price updates occur when:
  - A new order arrives, or
  - An old order expires (`tmax`), or
  - The total executed volume reaches `qmax`.

---

## Output

The Python broker records and visualizes:
- Price trajectories (`p_limit`, `p_continuous`)
- Execution volumes
- Real-time order book depth
- Volatility and liquidity differences

This allows a side-by-side comparison of discrete and continuous market dynamics.

---

## Key Ideas

The continuous exchange eliminates the **discreteness** that high-frequency traders exploit:
- **Time continuity:** small time advantages yield proportionally small benefits.
- **Price continuity:** prices can form at any value.
- **Volume continuity:** trades execute fractionally and smoothly over time.

---

## Reference

> Albert S. Kyle, Jeongmin Lee (2017)  
> *Toward a Fully Continuous Exchange*  
> [https://papers.ssrn.com/sol3/papers.cfm?abstract_id=2920460]

---

## Tech Stack

- **Python 3** — for simulation orchestration and visualization  
- **C++** — for high-performance market logic  
- **Matplotlib / NumPy** — for analysis and plotting  

---

## Future Plans
- Add reinforcement-learning agents for adaptive trading strategies  
- Introduce latency and network effects  
- Compare equilibrium price stability under various λ and `umax` parameters  


