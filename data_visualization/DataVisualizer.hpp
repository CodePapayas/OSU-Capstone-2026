/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/
#pragma once

#include <string>
#include <vector>
#include <memory>
#include "db_connector.h"

struct SnapshotData {
    uint64_t tick;
    double timestamp;
    int agentCount;
    int resourceCount;
    double totalEnergy;
    double avgFitness;
};

class DataVisualizer {
public:
    DataVisualizer(std::shared_ptr<DBConnector> db);
    ~DataVisualizer() = default;

    // Fetch the recent history across saves or limit to a specific save
    std::vector<SnapshotData> fetchHistory(int limit = 50);

    // Prints an ASCII line chart for Agent Count over time
    void plotAgentCount(const std::vector<SnapshotData>& data) const;
    
    // Prints an ASCII line chart for Total Energy over time
    void plotTotalEnergy(const std::vector<SnapshotData>& data) const;

    // Export history to CSV for external tools (Excel, Python, etc.)
    void exportToCSV(const std::vector<SnapshotData>& data, const std::string& filepath) const;

private:
    std::shared_ptr<DBConnector> m_db;
    
    // Internal helper for generic ASCII plotting
    void renderAsciiChart(const std::vector<double>& values, const std::string& title, const std::string& yAxisLabel) const;
};
