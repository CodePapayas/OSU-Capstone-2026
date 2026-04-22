/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/
#pragma once

#include <memory>
#include <vector>
#include <string>
#include "db_connector.h"
#include "DataVisualizer.hpp"

class LiveVisualizer {
public:
    // pollIntervalSeconds: how often to re-query the DB and redraw
    // historyLimit: how many recent ticks to show in the charts
    LiveVisualizer(std::shared_ptr<DBConnector> db, int pollIntervalSeconds = 2, int historyLimit = 60);
    ~LiveVisualizer() = default;

    // Blocks and loops until the user presses Ctrl+C
    void run();

private:
    std::shared_ptr<DBConnector> m_db;
    DataVisualizer m_viz;
    int m_pollInterval;
    int m_historyLimit;

    // Clear terminal and jump cursor to top-left
    void clearScreen() const;

    // Draw the summary stats table at the top
    void drawSummaryTable(const std::vector<SnapshotData>& data) const;

    // Draw both charts back-to-back below the summary
    void drawCharts(const std::vector<SnapshotData>& data) const;
};
