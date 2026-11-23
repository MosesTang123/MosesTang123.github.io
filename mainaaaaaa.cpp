#include <algorithm>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstring>
#include <deque>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
    #define _WINSOCK_DEPRECATED_NO_WARNINGS
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

// ==========================================
// 1. MARKET & PORTFOLIO STATE
// ==========================================
struct MarketState {
    double currentPrice = 120.00;
    double prevPrice = 120.00;
    double cash = 10000.00;
    int shares = 0;
    double avgBuyPrice = 0.0;
    std::string headline = "MARKET OPEN: $AIDX begins trading amid extreme speculation.";
    std::string shockType = "neutral"; // "up", "down", "neutral"
    std::deque<double> history;
};

MarketState state;
std::mutex stateMutex;
bool running = true;

// ==========================================
// 2. UNSTABLE AI STOCK SIMULATION ENGINE
// ==========================================
void marketSimulationLoop() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> baseNoise(0.0, 0.065);      // High baseline volatility (6.5% per tick)
    std::uniform_real_distribution<> eventChance(0.0, 1.0); // Trigger for extreme jumps
    std::uniform_int_distribution<> eventPicker(0, 7);

    const std::vector<std::pair<std::string, double>> chaosEvents = {
        {"BREAKING: AGI breakthrough leaked on GitHub! Short squeeze triggered!", 0.42},
        {"FLASH CRASH: AI model starts outputting exclusively in binary during keynote!", -0.36},
        {"HYPE SURGE: Sovereign wealth fund buys 5M shares of $AIDX!", 0.31},
        {"PANIC SELL: Lead architect resigns via cryptic emoji tweet!", -0.29},
        {"RUMOR: Secret government quantum-AI contract worth $80B!", 0.27},
        {"OUTAGE: Primary GPU data center trips local power grid!", -0.24},
        {"ANALYST UPGRADE: Price target raised to $1,000 ('Pure hyper-growth')!", 0.22},
        {"REGULATORY SHOCK: Emergency Senate hearing called over $AIDX trading bot!", -0.26}
    };

    {
        std::lock_guard<std::mutex> lock(stateMutex);
        for (int i = 0; i < 60; ++i) {
            state.history.push_back(state.currentPrice);
        }
    }

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(650));

        std::lock_guard<std::mutex> lock(stateMutex);
        state.prevPrice = state.currentPrice;

        double pctChange = baseNoise(gen);
        double roll = eventChance(gen);

        // 16% chance per tick for a wild news shock
        if (roll < 0.16) {
            int idx = eventPicker(gen);
            pctChange += chaosEvents[idx].second;
            state.headline = chaosEvents[idx].first;
            state.shockType = (chaosEvents[idx].second >= 0) ? "up" : "down";
        } else {
            state.shockType = "neutral";
        }

        // Apply price movement
        state.currentPrice *= (1.0 + pctChange);

        // Prevent price from hitting $0.00 via emergency volatile bounce
        if (state.currentPrice < 3.50) {
            state.currentPrice += 18.50;
            state.headline = "VOLATILITY HALT & BAILOUT: Venture syndicate injects emergency capital!";
            state.shockType = "up";
        }

        state.history.push_back(state.currentPrice);
        if (state.history.size() > 60) {
            state.history.pop_front();
        }
    }
}

// ==========================================
// 3. JSON SERIALIZATION & TRADE EXECUTION
// ==========================================
std::string buildStateJson() {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "{"
        << "\"price\":" << state.currentPrice << ","
        << "\"prevPrice\":" << state.prevPrice << ","
        << "\"cash\":" << state.cash << ","
        << "\"shares\":" << state.shares << ","
        << "\"avgBuyPrice\":" << state.avgBuyPrice << ","
        << "\"headline\":\"" << state.headline << "\","
        << "\"shockType\":\"" << state.shockType << "\","
        << "\"history\":[";

    for (size_t i = 0; i < state.history.size(); ++i) {
        oss << state.history[i];
        if (i + 1 < state.history.size()) oss << ",";
    }
    oss << "]}";
    return oss.str();
}

void processTrade(const std::string& action, const std::string& qtyStr) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (action == "buy") {
        int qty = 0;
        if (qtyStr == "max") {
            qty = static_cast<int>(std::floor(state.cash / state.currentPrice));
        } else {
            qty = std::stoi(qtyStr);
        }

        double cost = qty * state.currentPrice;
        if (qty > 0 && state.cash >= cost) {
            double totalCostBasis = (state.shares * state.avgBuyPrice) + cost;
            state.shares += qty;
            state.cash -= cost;
            state.avgBuyPrice = totalCostBasis / state.shares;
        }
    } else if (action == "sell") {
        int qty = 0;
        if (qtyStr == "max") {
            qty = state.shares;
        } else {
            qty = std::min(state.shares, std::stoi(qtyStr));
        }

        if (qty > 0 && state.shares >= qty) {
            state.shares -= qty;
            state.cash += qty * state.currentPrice;
            if (state.shares == 0) {
                state.avgBuyPrice = 0.0;
            }
        }
    } else if (action == "reset") {
        state.currentPrice = 120.00;
        state.prevPrice = 120.00;
        state.cash = 10000.00;
        state.shares = 0;
        state.avgBuyPrice = 0.0;
        state.headline = "ACCOUNT RESET: Fresh $10,000.00 margin account initialized.";
        state.shockType = "neutral";
        state.history.assign(60, 120.00);
    }
}

// ==========================================
// 4. EMBEDDED WEB FRONTEND (HTML/CSS/JS)
// ==========================================
const std::string HTML_PAGE = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>$AIDX | Unstable AI Stock Terminal</title>
<style>
  :root {
    --bg: #0b0e14;
    --panel: #131722;
    --border: #2a2e39;
    --text: #d1d4dc;
    --muted: #787b86;
    --green: #00e676;
    --red: #ff1744;
    --accent: #2962ff;
  }
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body {
    background: var(--bg);
    color: var(--text);
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, monospace;
    padding: 20px;
    display: flex;
    flex-direction: column;
    align-items: center;
  }
  .container { width: 100%; max-width: 1050px; display: grid; gap: 16px; }
  .header {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 16px 24px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    flex-wrap: wrap;
    gap: 12px;
  }
  .ticker-title { display: flex; align-items: baseline; gap: 12px; }
  .ticker-symbol { font-size: 1.6rem; font-weight: 800; color: #fff; }
  .ticker-name { color: var(--muted); font-size: 0.9rem; }
  .price-box { text-align: right; }
  .price { font-size: 2rem; font-weight: 800; font-variant-numeric: tabular-nums; }
  .delta { font-size: 0.95rem; font-weight: 600; }
  .up { color: var(--green); }
  .down { color: var(--red); }

  .news-banner {
    background: #1a1f2e;
    border-left: 4px solid var(--accent);
    padding: 12px 18px;
    border-radius: 6px;
    font-size: 0.95rem;
    font-weight: 600;
    transition: background 0.25s;
  }
  .news-up { border-left-color: var(--green); background: rgba(0, 230, 118, 0.12); color: #b9f6ca; }
  .news-down { border-left-color: var(--red); background: rgba(255, 23, 68, 0.14); color: #ffcdd2; }

  .main-grid { display: grid; grid-template-columns: 2fr 1fr; gap: 16px; }
  @media (max-width: 800px) { .main-grid { grid-template-columns: 1fr; } }

  .card {
    background: var(--panel);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 18px;
  }
  canvas { width: 100%; height: 340px; display: block; }

  .stats { display: grid; gap: 12px; margin-bottom: 18px; }
  .stat-row {
    display: flex;
    justify-content: space-between;
    padding-bottom: 8px;
    border-bottom: 1px solid var(--border);
    font-size: 0.95rem;
  }
  .stat-label { color: var(--muted); }
  .stat-val { font-weight: 700; font-variant-numeric: tabular-nums; }

  .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
  button {
    border: none;
    border-radius: 6px;
    padding: 12px;
    font-size: 0.9rem;
    font-weight: 700;
    cursor: pointer;
    transition: transform 0.08s, opacity 0.15s;
    color: #fff;
  }
  button:active { transform: scale(0.97); }
  button:hover { opacity: 0.9; }
  .btn-buy { background: #00c853; color: #051b0e; }
  .btn-sell { background: #ff1744; }
  .btn-reset {
    grid-column: span 2;
    background: #2a2e39;
    color: var(--muted);
    margin-top: 6px;
    font-size: 0.8rem;
  }
</style>
</head>
<body>
<div class="container">
  <div class="header">
    <div class="ticker-title">
      <span class="ticker-symbol">$AIDX</span>
      <span class="ticker-name">NeuralCore AI Corp • EXTREME VOLATILITY</span>
    </div>
    <div class="price-box">
      <div id="price" class="price">$120.00</div>
      <div id="delta" class="delta">0.00%</div>
    </div>
  </div>

  <div id="news" class="news-banner">Connecting to C++ market engine...</div>

  <div class="main-grid">
    <div class="card">
      <canvas id="chart"></canvas>
    </div>

    <div class="card">
      <div class="stats">
        <div class="stat-row"><span class="stat-label">Buying Power</span><span id="cash" class="stat-val">$10,000.00</span></div>
        <div class="stat-row"><span class="stat-label">Shares Held</span><span id="shares" class="stat-val">0</span></div>
        <div class="stat-row"><span class="stat-label">Avg Buy Price</span><span id="avgPrice" class="stat-val">$0.00</span></div>
        <div class="stat-row"><span class="stat-label">Position Value</span><span id="posVal" class="stat-val">$0.00</span></div>
        <div class="stat-row"><span class="stat-label">Unrealized P&L</span><span id="pnl" class="stat-val">$0.00</span></div>
        <div class="stat-row"><span class="stat-label">Total Net Worth</span><span id="netWorth" class="stat-val">$10,000.00</span></div>
      </div>

      <div class="btn-grid">
        <button class="btn-buy" onclick="trade('buy', '1')">Buy 1</button>
        <button class="btn-sell" onclick="trade('sell', '1')">Sell 1</button>
        <button class="btn-buy" onclick="trade('buy', '10')">Buy 10</button>
        <button class="btn-sell" onclick="trade('sell', '10')">Sell 10</button>
        <button class="btn-buy" onclick="trade('buy', 'max')">YOLO All-In</button>
        <button class="btn-sell" onclick="trade('sell', 'max')">Panic Sell All</button>
        <button class="btn-reset" onclick="trade('reset', '0')">Reset Simulation ($10k)</button>
      </div>
    </div>
  </div>
</div>

<script>
const fmt = (n) => '$' + n.toLocaleString('en-US', { minimumFractionDigits: 2, maximumFractionDigits: 2 });

function drawChart(history, avgBuyPrice) {
  const canvas = document.getElementById('chart');
  const dpr = window.devicePixelRatio || 1;
  const rect = canvas.getBoundingClientRect();
  canvas.width = rect.width * dpr;
  canvas.height = rect.height * dpr;

  const ctx = canvas.getContext('2d');
  ctx.scale(dpr, dpr);
  const w = rect.width;
  const h = rect.height;

  ctx.clearRect(0, 0, w, h);
  if (!history || history.length < 2) return;

  let min = Math.min(...history);
  let max = Math.max(...history);
  if (avgBuyPrice > 0) {
    min = Math.min(min, avgBuyPrice);
    max = Math.max(max, avgBuyPrice);
  }
  const pad = (max - min) * 0.12 || 5;
  min = Math.max(0, min - pad);
  max += pad;

  const getY = (val) => h - 24 - ((val - min) / (max - min)) * (h - 44);
  const getX = (idx) => 55 + (idx / (history.length - 1)) * (w - 70);

  // Horizontal grid lines & Y-axis labels
  ctx.strokeStyle = '#1e222d';
  ctx.fillStyle = '#787b86';
  ctx.font = '11px monospace';
  ctx.lineWidth = 1;
  for (let i = 0; i <= 4; i++) {
    const val = min + (i / 4) * (max - min);
    const y = getY(val);
    ctx.beginPath();
    ctx.moveTo(55, y);
    ctx.lineTo(w - 15, y);
    ctx.stroke();
    ctx.fillText('$' + val.toFixed(1), 6, y + 4);
  }

  // Average Buy Price dashed line
  if (avgBuyPrice > 0) {
    const avgY = getY(avgBuyPrice);
    ctx.save();
    ctx.setLineDash([5, 5]);
    ctx.strokeStyle = '#ffd54f';
    ctx.beginPath();
    ctx.moveTo(55, avgY);
    ctx.lineTo(w - 15, avgY);
    ctx.stroke();
    ctx.fillStyle = '#ffd54f';
    ctx.fillText('AVG', w - 42, avgY - 5);
    ctx.restore();
  }

  // Price line
  const isUp = history[history.length - 1] >= history[0];
  const lineColor = isUp ? '#00e676' : '#ff1744';

  ctx.beginPath();
  history.forEach((val, i) => {
    const x = getX(i);
    const y = getY(val);
    if (i === 0) ctx.moveTo(x, y);
    else ctx.lineTo(x, y);
  });
  ctx.strokeStyle = lineColor;
  ctx.lineWidth = 2.5;
  ctx.stroke();

  // Gradient fill
  ctx.lineTo(getX(history.length - 1), h - 24);
  ctx.lineTo(getX(0), h - 24);
  ctx.closePath();
  const grad = ctx.createLinearGradient(0, 0, 0, h);
  grad.addColorStop(0, isUp ? 'rgba(0, 230, 118, 0.25)' : 'rgba(255, 23, 68, 0.25)');
  grad.addColorStop(1, 'rgba(0, 0, 0, 0)');
  ctx.fillStyle = grad;
  ctx.fill();
}

function updateUI(data) {
  const priceEl = document.getElementById('price');
  const deltaEl = document.getElementById('delta');
  const diff = data.price - data.prevPrice;
  const pct = data.prevPrice > 0 ? (diff / data.prevPrice) * 100 : 0;

  priceEl.textContent = fmt(data.price);
  priceEl.className = 'price ' + (diff >= 0 ? 'up' : 'down');
  deltaEl.textContent = `${diff >= 0 ? '+' : ''}${fmt(diff)} (${pct >= 0 ? '+' : ''}${pct.toFixed(2)}%)`;
  deltaEl.className = 'delta ' + (diff >= 0 ? 'up' : 'down');

  const newsEl = document.getElementById('news');
  newsEl.textContent = data.headline;
  newsEl.className = 'news-banner ' + (data.shockType === 'up' ? 'news-up' : data.shockType === 'down' ? 'news-down' : '');

  const posVal = data.shares * data.price;
  const pnl = data.shares > 0 ? (data.price - data.avgBuyPrice) * data.shares : 0;
  const netWorth = data.cash + posVal;

  document.getElementById('cash').textContent = fmt(data.cash);
  document.getElementById('shares').textContent = data.shares;
  document.getElementById('avgPrice').textContent = fmt(data.avgBuyPrice);
  document.getElementById('posVal').textContent = fmt(posVal);

  const pnlEl = document.getElementById('pnl');
  pnlEl.textContent = `${pnl >= 0 ? '+' : ''}${fmt(pnl)}`;
  pnlEl.className = 'stat-val ' + (pnl > 0 ? 'up' : pnl < 0 ? 'down' : '');

  document.getElementById('netWorth').textContent = fmt(netWorth);

  drawChart(data.history, data.avgBuyPrice);
}

async function fetchState() {
  try {
    const res = await fetch('/api/state');
    const data = await res.json();
    updateUI(data);
  } catch (e) {}
}

async function trade(action, qty) {
  try {
    const res = await fetch(`/api/trade?action=${action}&qty=${qty}`);
    const data = await res.json();
    updateUI(data);
  } catch (e) {}
}

setInterval(fetchState, 650);
fetchState();
</script>
</body>
</html>)HTML";

// ==========================================
// 5. LIGHTWEIGHT C++ HTTP SERVER
// ==========================================
std::string extractQueryParam(const std::string& path, const std::string& key) {
    size_t pos = path.find(key + "=");
    if (pos == std::string::npos) return "";
    pos += key.length() + 1;
    size_t end = path.find('&', pos);
    return path.substr(pos, end == std::string::npos ? std::string::npos : end - pos);
}

void sendHttpResponse(SOCKET clientSock, const std::string& contentType, const std::string& body) {
    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n"
        << "Content-Type: " << contentType << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Cache-Control: no-store\r\n"
        << "Connection: close\r\n\r\n"
        << body;
    std::string response = oss.str();
    send(clientSock, response.c_str(), static_cast<int>(response.size()), 0);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return 1;
    }
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif

    std::thread simThread(marketSimulationLoop);
    simThread.detach();

    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket.\n";
        return 1;
    }

    int opt = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSock, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed on port 8080.\n";
        closesocket(serverSock);
        return 1;
    }

    listen(serverSock, 10);
    std::cout << "====================================================\n";
    std::cout << "  $AIDX Trading Simulator running in C++!\n";
    std::cout << "  Open your browser to: http://localhost:8080\n";
    std::cout << "====================================================\n";

    while (running) {
        sockaddr_in clientAddr{};
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSock = accept(serverSock, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (clientSock == INVALID_SOCKET) continue;

        char buffer[4096] = {0};
        int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            std::string req(buffer, bytesRead);
            std::istringstream reqStream(req);
            std::string method, path;
            reqStream >> method >> path;

            if (path.rfind("/api/state", 0) == 0) {
                std::lock_guard<std::mutex> lock(stateMutex);
                sendHttpResponse(clientSock, "application/json", buildStateJson());
            } else if (path.rfind("/api/trade", 0) == 0) {
                std::string action = extractQueryParam(path, "action");
                std::string qty = extractQueryParam(path, "qty");
                processTrade(action, qty);

                std::lock_guard<std::mutex> lock(stateMutex);
                sendHttpResponse(clientSock, "application/json", buildStateJson());
            } else {
                sendHttpResponse(clientSock, "text/html; charset=utf-8", HTML_PAGE);
            }
        }
        closesocket(clientSock);
    }

    closesocket(serverSock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}