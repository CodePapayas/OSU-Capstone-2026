#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <functional>

using namespace std;

struct GLFWwindow;

struct VisSnapshot {
    uint64_t tick = 0;
    double   timestamp = 0.0;
    int      agentCount = 0;
    int      resourceCount = 0;
    double   totalEnergy = 0.0;
    double   avgFitness = 0.0;
};

struct WorldMapSnapshot {
    int gridSize = 0;
    vector<float> tileValues;
    vector<pair<int,int>> entityPositions;
    vector<pair<int,int>> foodPositions;
    vector<pair<int,int>> waterPositions;
};

struct RunSummary {
    int      runIndex      = 0;
    uint64_t lifespan      = 0;
    double   peakEnergy    = 0.0;
    double   avgEnergy     = 0.0;
    double   finalEnergy   = 0.0;
    double   avgFitness    = 0.0;
    int      peakResources = 0;
};

class ImGuiVisualizer {
public:
    ImGuiVisualizer();
    ~ImGuiVisualizer();

    ImGuiVisualizer(const ImGuiVisualizer&) = delete;
    ImGuiVisualizer& operator=(const ImGuiVisualizer&) = delete;

    bool loadCSV(const string& filepath);
    void pushSnapshot(const VisSnapshot& snap);
    void run();

    bool init();
    bool frameTick();
    bool shouldClose() const;

    void setTickCallback(function<void()> cb);
    void setRestartCallback(function<void()> cb);
    void setWorldMapCallback(function<WorldMapSnapshot()> cb);
    void clearHistory();
    void notifyEntityDied();

private:
    GLFWwindow* m_window = nullptr;
    string m_csvPath;

    vector<VisSnapshot> m_history;
    vector<double> m_ticks;
    vector<double> m_agentCounts;
    vector<double> m_resourceCounts;
    vector<double> m_totalEnergies;
    vector<double> m_avgFitnesses;

    bool m_showDemoWindow  = false;
    int  m_smoothingWindow = 1;
    char m_csvPathBuf[512] = {};
    bool m_initialized     = false;

    bool m_simPaused       = false;
    bool m_simRunning      = false;
    int  m_autoTickRate    = 10;
    double m_tickAccum     = 0.0;
    double m_lastFrameTime = 0.0;
    function<void()> m_tickCallback;
    function<void()> m_restartCallback;
    function<WorldMapSnapshot()> m_worldMapCallback;
    WorldMapSnapshot m_worldMap;
    int  m_csvAutoExportInterval = 0;

    bool m_batchActive        = false;
    int  m_batchTarget        = 10;
    int  m_batchCurrent       = 0;
    vector<RunSummary> m_batchResults;

    bool initWindow();
    void initImGui();
    void setupStyle();
    void cleanup();

    void rebuildPlotArrays();
    void exportCSV(const string& path) const;
    vector<double> smooth(const vector<double>& data, int windowSize) const;

    struct Stats {
        double min = 0, max = 0, avg = 0;
        double first = 0, last = 0;
        double trend = 0;
        double stddev = 0;
    };
    static Stats computeStats(const vector<double>& data);

    void renderFrame();
    void renderMenuBar();
    void renderOverviewPanel();
    void renderPopulationPlot();
    void renderEnergyPlot();
    void renderFitnessPlot();
    void renderResourcePlot();
    void renderNormalizedPlot();
    void renderDerivativesPlot();
    void renderEnergyHistogram();
    void renderStatisticsPanel();
    void renderControlPanel();
    void renderSimulationControls();
    void renderWorldMap();
    void renderBatchResults();
};
