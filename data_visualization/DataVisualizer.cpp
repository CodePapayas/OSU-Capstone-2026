/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462
*/

#include "DataVisualizer.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cmath>

using namespace std;

DataVisualizer::DataVisualizer(shared_ptr<DBConnector> db) : m_db(db) {
    if (!m_db || !m_db->isConnected()) {
        throw runtime_error("DataVisualizer requires an active DB connection.");
    }
}

// Fetch the N most recent history entries, sorted by tick ascending
vector<SnapshotData> DataVisualizer::fetchHistory(int limit) {
    vector<SnapshotData> results;
    
    // We get the most recent ticks, up to 'limit', then reverse them so they are chronological
    string qLimit = to_string(limit);
    string query = 
        "SELECT tick, real_timestamp, agent_count, total_resources, total_energy, avg_fitness "
        "FROM simulation_state_history "
        "ORDER BY tick DESC LIMIT " + qLimit + ";";

    PGResultGuard res(m_db->exec(query));
    
    for (int i = 0; i < res.rows(); i++) {
        SnapshotData sn;
        sn.tick = stoull(res.val(i, 0));
        sn.timestamp = stod(res.val(i, 1));
        sn.agentCount = stoi(res.val(i, 2));
        sn.resourceCount = stoi(res.val(i, 3));
        sn.totalEnergy = stod(res.val(i, 4));
        sn.avgFitness = stod(res.val(i, 5));
        results.push_back(sn);
    }

    // Now reverse map it back ascending order for visualization
    reverse(results.begin(), results.end());
    
    return results;
}

// Visualize agent count
void DataVisualizer::plotAgentCount(const vector<SnapshotData>& data) const {
    if (data.empty()) return;

    vector<double> vals;
    vals.reserve(data.size());
    for (const auto& item : data) {
        vals.push_back(item.agentCount);
    }
    
    renderAsciiChart(vals, "Simulation Agent Count Over Time", "Agents Count");
}

// Visualize total energy
void DataVisualizer::plotTotalEnergy(const vector<SnapshotData>& data) const {
    if (data.empty()) return;

    vector<double> vals;
    vals.reserve(data.size());
    for (const auto& item : data) {
        vals.push_back(item.totalEnergy);
    }
    
    renderAsciiChart(vals, "Simulation Total Environment Energy", "Joules / Energy");
}

// Basic generic terminal ASCII bar/line chart renderer based on scaled values
void DataVisualizer::renderAsciiChart(const vector<double>& values, const string& title, const string& yAxisLabel) const {
    if (values.empty()) {
        cout << " [No Data Available for " << title << "]" << endl;
        return;
    }
    
    double maxVal = *max_element(values.begin(), values.end());
    double minVal = *min_element(values.begin(), values.end());
    
    // Fallback scaling bounds
    if (maxVal == minVal) {
        maxVal = minVal + 1.0;
        minVal = maxVal - 2.0;
    }
    
    int const rows = 15;
    int const cols = values.size();
    
    cout << "\n======================================================\n";
    cout << "  " << title << "\n";
    cout << "======================================================\n";
    cout << " Y: " << yAxisLabel << " | X: Tick Progression (" << cols << " steps)\n\n";

    // Charting vertically top down limits to 'rows' size screen space
    for (int r = rows; r >= 0; --r) {
        double threshold = minVal + (maxVal - minVal) * (1.0 * r / rows);
        
        // Print y-axis labels on 3 specific rows, top, middle, bottom
        if (r == rows) {
            cout << setw(8) << right << (int)maxVal << " | ";
        } else if (r == rows / 2) {
            cout << setw(8) << right << (int)(minVal + (maxVal - minVal) / 2) << " | ";
        } else if (r == 0) {
            cout << setw(8) << right << (int)minVal << " | ";
        } else {
            cout << "         | ";
        }
        
        for (int c = 0; c < cols; ++c) {
            double v = values[c];
            if (v >= threshold) {
                cout << "#";
            } else {
                cout << " ";
            }
        }
        cout << "\n";
    }
    
    // Draw bottom X Axis
    cout << "         +";
    for(int i=0; i < cols; ++i) cout << "-";
    cout << ">\n\n";
}

void DataVisualizer::exportToCSV(const vector<SnapshotData>& data, const string& filepath) const {
    ofstream ofs(filepath);
    if (!ofs.is_open()) {
        cerr << "Failed to open CSV output string for writing: " << filepath << endl;
        return;
    }
    
    ofs << "Tick,Timestamp,AgentCount,ResourceCount,TotalEnergy,AvgFitness\n";
    for (const auto& sn : data) {
        ofs << sn.tick << ","
            << setprecision(10) << sn.timestamp << ","
            << sn.agentCount << ","
            << sn.resourceCount << ","
            << sn.totalEnergy << ","
            << setprecision(4) << sn.avgFitness << "\n";
    }
    
    cout << "Successfully exported simulation stats to " << filepath << endl;
}
