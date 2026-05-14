/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/

#include "LiveVisualizer.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <csignal>

using namespace std;

// Global flag flipped by SIGINT (Ctrl+C) so the loop exits cleanly
static volatile sig_atomic_t g_running = 1;
static void handleSignal(int) { g_running = 0; }

LiveVisualizer::LiveVisualizer(shared_ptr<DBConnector> db, int pollIntervalSeconds, int historyLimit)
    : m_db(db)
    , m_viz(db)
    , m_pollInterval(pollIntervalSeconds)
    , m_historyLimit(historyLimit)
{}

void LiveVisualizer::run() {
    signal(SIGINT, handleSignal);

    cout << "\033[?25l";  // Hide cursor for cleaner redraws

    while (g_running) {
        vector<SnapshotData> data = m_viz.fetchHistory(m_historyLimit);

        clearScreen();

        cout << "\033[1;36m";  // Bold cyan header
        cout << "╔══════════════════════════════════════════════════════╗\n";
        cout << "║        A-LIFE SIMULATION LIVE STATS MONITOR         ║\n";
        cout << "║  Poll interval: " << m_pollInterval << "s  |  Showing last "
             << setw(3) << m_historyLimit << " ticks          ║\n";
        cout << "║  Press Ctrl+C to exit                               ║\n";
        cout << "╚══════════════════════════════════════════════════════╝\n";
        cout << "\033[0m";  // Reset colour

        if (data.empty()) {
            cout << "\n  \033[33mWaiting for simulation data...\033[0m  (Is main_exe running?)\n";
        } else {
            drawSummaryTable(data);
            drawCharts(data);
        }

        this_thread::sleep_for(chrono::seconds(m_pollInterval));
    }

    cout << "\033[?25h";   // Restore cursor
    cout << "\n\033[32mMonitor stopped.\033[0m\n";
}

void LiveVisualizer::clearScreen() const {
    // ANSI: clear screen + move cursor to home position
    cout << "\033[2J\033[H" << flush;
}

void LiveVisualizer::drawSummaryTable(const vector<SnapshotData>& data) const {
    const SnapshotData& latest = data.back();

    cout << "\n\033[1mLatest Snapshot\033[0m\n";
    cout << "  Tick           : " << latest.tick          << "\n";
    cout << "  Agents Alive   : " << latest.agentCount    << "\n";
    cout << "  Resources      : " << latest.resourceCount << "\n";
    cout << fixed << setprecision(4);
    cout << "  Total Energy   : " << latest.totalEnergy   << "\n";
    cout << "  Avg Fitness    : " << latest.avgFitness    << "\n";

    // Trend arrow: compare latest two points
    if (data.size() >= 2) {
        double delta = data.back().totalEnergy - data[data.size()-2].totalEnergy;
        string arrow = (delta > 0) ? "\033[32m▲\033[0m" : (delta < 0) ? "\033[31m▼\033[0m" : "=";
        cout << "  Energy Trend   : " << arrow << "\n";
    }
    cout << "\n";
}

void LiveVisualizer::drawCharts(const vector<SnapshotData>& data) const {
    m_viz.plotAgentCount(data);
    m_viz.plotTotalEnergy(data);
}
