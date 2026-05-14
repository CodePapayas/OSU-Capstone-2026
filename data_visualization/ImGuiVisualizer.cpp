#include "ImGuiVisualizer.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <GLFW/glfw3.h>

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <cstring>
#include <cstdio>

using namespace std;


static void glfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static const char* TrendArrow(double cur, double prev) {
    if (cur > prev) return reinterpret_cast<const char*>(u8"\u25B2");
    if (cur < prev) return reinterpret_cast<const char*>(u8"\u25BC");
    return "=";
}

static ImVec4 TrendColor(double cur, double prev) {
    if (cur > prev) return {0.2f, 0.9f, 0.3f, 1.0f};
    if (cur < prev) return {0.9f, 0.2f, 0.2f, 1.0f};
    return {0.6f, 0.6f, 0.6f, 1.0f};
}

static void StatCard(const char* id, const char* label, const char* value,
                     const ImVec4& valueColor, float width = 150.0f) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.12f, 0.18f, 1.0f));
    ImGui::BeginChild(id, ImVec2(width, 62), true, ImGuiWindowFlags_NoScrollbar);
    ImGui::TextColored({0.55f, 0.55f, 0.65f, 1.0f}, "%s", label);
    ImGui::Spacing();
    ImGui::TextColored(valueColor, "%s", value);
    ImGui::EndChild();
    ImGui::PopStyleColor();
}


ImGuiVisualizer::ImGuiVisualizer() = default;

ImGuiVisualizer::~ImGuiVisualizer() { cleanup(); }


bool ImGuiVisualizer::initWindow() {
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(1680, 960, "A-Life Simulation Dashboard", nullptr, nullptr);
    if (!m_window) { glfwTerminate(); return false; }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    return true;
}

void ImGuiVisualizer::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    float xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(m_window, &xscale, &yscale);
    if (xscale > 1.0f) io.FontGlobalScale = xscale;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    setupStyle();
}

void ImGuiVisualizer::setupStyle() {
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding    = 6.0f;
    s.FrameRounding     = 4.0f;
    s.GrabRounding      = 4.0f;
    s.TabRounding       = 4.0f;
    s.ScrollbarRounding = 6.0f;
    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 0.0f;
    s.PopupBorderSize   = 1.0f;
    s.WindowPadding     = {10, 10};
    s.FramePadding      = {8, 4};
    s.ItemSpacing       = {10, 8};

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]        = {0.08f, 0.08f, 0.12f, 1.00f};
    c[ImGuiCol_ChildBg]         = {0.10f, 0.10f, 0.14f, 1.00f};
    c[ImGuiCol_PopupBg]         = {0.10f, 0.10f, 0.14f, 0.95f};
    c[ImGuiCol_Border]          = {0.20f, 0.22f, 0.30f, 1.00f};
    c[ImGuiCol_FrameBg]         = {0.14f, 0.14f, 0.20f, 1.00f};
    c[ImGuiCol_FrameBgHovered]  = {0.22f, 0.22f, 0.30f, 1.00f};
    c[ImGuiCol_FrameBgActive]   = {0.26f, 0.26f, 0.36f, 1.00f};
    c[ImGuiCol_TitleBg]         = {0.06f, 0.06f, 0.10f, 1.00f};
    c[ImGuiCol_TitleBgActive]   = {0.10f, 0.14f, 0.22f, 1.00f};
    c[ImGuiCol_Tab]             = {0.12f, 0.14f, 0.20f, 1.00f};
    c[ImGuiCol_TabHovered]      = {0.20f, 0.30f, 0.45f, 1.00f};
    c[ImGuiCol_TabActive]       = {0.16f, 0.24f, 0.38f, 1.00f};
    c[ImGuiCol_Header]          = {0.16f, 0.20f, 0.30f, 1.00f};
    c[ImGuiCol_HeaderHovered]   = {0.20f, 0.28f, 0.40f, 1.00f};
    c[ImGuiCol_HeaderActive]    = {0.24f, 0.32f, 0.46f, 1.00f};
    c[ImGuiCol_Button]          = {0.16f, 0.22f, 0.34f, 1.00f};
    c[ImGuiCol_ButtonHovered]   = {0.20f, 0.30f, 0.46f, 1.00f};
    c[ImGuiCol_ButtonActive]    = {0.24f, 0.36f, 0.54f, 1.00f};
    c[ImGuiCol_TableHeaderBg]   = {0.12f, 0.14f, 0.20f, 1.00f};
    c[ImGuiCol_TableBorderLight]= {0.20f, 0.22f, 0.28f, 1.00f};
    c[ImGuiCol_TableBorderStrong]={0.24f, 0.26f, 0.34f, 1.00f};

    ImPlotStyle& ps = ImPlot::GetStyle();
    ps.PlotPadding  = {12, 12};
    ps.LabelPadding = {5, 5};
    ps.FillAlpha    = 0.20f;
    ps.LineWeight   = 2.0f;
}

void ImGuiVisualizer::cleanup() {
    if (m_window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_window);
        glfwTerminate();
        m_window = nullptr;
    }
}


bool ImGuiVisualizer::loadCSV(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        cerr << "Cannot open CSV: " << filepath << "\n";
        return false;
    }

    m_csvPath = filepath;
    strncpy(m_csvPathBuf, filepath.c_str(), sizeof(m_csvPathBuf) - 1);
    m_csvPathBuf[sizeof(m_csvPathBuf) - 1] = '\0';

    m_history.clear();
    string line;

    if (!getline(file, line)) return false;

    while (getline(file, line)) {
        if (line.empty()) continue;
        istringstream iss(line);
        string tok;
        VisSnapshot snap{};
        try {
            getline(iss, tok, ','); snap.tick          = stoull(tok);
            getline(iss, tok, ','); snap.timestamp      = stod(tok);
            getline(iss, tok, ','); snap.agentCount     = stoi(tok);
            getline(iss, tok, ','); snap.resourceCount  = stoi(tok);
            getline(iss, tok, ','); snap.totalEnergy    = stod(tok);
            getline(iss, tok, ','); snap.avgFitness     = stod(tok);
        } catch (...) {
            continue;
        }
        m_history.push_back(snap);
    }

    if (!m_history.empty()) {
        vector<VisSnapshot> deduped;
        deduped.reserve(m_history.size());
        deduped.push_back(m_history[0]);
        for (size_t i = 1; i < m_history.size(); i++) {
            if (m_history[i].tick == deduped.back().tick)
                deduped.back() = m_history[i];
            else
                deduped.push_back(m_history[i]);
        }
        m_history = move(deduped);
    }

    if (m_history.empty()) {
        cerr << "No data rows found in CSV.\n";
        return false;
    }

    rebuildPlotArrays();
    return true;
}


void ImGuiVisualizer::pushSnapshot(const VisSnapshot& snap) {
    m_history.push_back(snap);
    rebuildPlotArrays();
}

void ImGuiVisualizer::setTickCallback(function<void()> cb) {
    m_tickCallback = move(cb);
    m_simRunning = true;
}

void ImGuiVisualizer::setRestartCallback(function<void()> cb) {
    m_restartCallback = move(cb);
}

void ImGuiVisualizer::setWorldMapCallback(function<WorldMapSnapshot()> cb) {
    m_worldMapCallback = move(cb);
}

void ImGuiVisualizer::clearHistory() {
    m_history.clear();
    rebuildPlotArrays();
}

void ImGuiVisualizer::notifyEntityDied() {
    RunSummary rs;
    rs.runIndex = m_batchActive ? m_batchCurrent + 1 : (int)m_batchResults.size() + 1;
    rs.lifespan = m_history.empty() ? 0 : m_history.back().tick;

    double sumE = 0.0, maxE = 0.0, sumF = 0.0;
    int maxR = 0;
    for (auto& s : m_history) {
        sumE += s.totalEnergy;
        if (s.totalEnergy > maxE) maxE = s.totalEnergy;
        sumF += s.avgFitness;
        if (s.resourceCount > maxR) maxR = s.resourceCount;
    }
    size_t n = m_history.size();
    rs.peakEnergy    = maxE;
    rs.avgEnergy     = n > 0 ? sumE / n : 0.0;
    rs.finalEnergy   = n > 0 ? m_history.back().totalEnergy : 0.0;
    rs.avgFitness    = n > 0 ? sumF / n : 0.0;
    rs.peakResources = maxR;
    m_batchResults.push_back(rs);

    if (m_batchActive) {
        m_batchCurrent++;
        if (m_batchCurrent < m_batchTarget) {
            if (m_restartCallback) m_restartCallback();
        } else {
            m_batchActive = false;
            m_simPaused = true;
        }
    }
}


void ImGuiVisualizer::rebuildPlotArrays() {
    const size_t n = m_history.size();
    m_ticks.resize(n);
    m_agentCounts.resize(n);
    m_resourceCounts.resize(n);
    m_totalEnergies.resize(n);
    m_avgFitnesses.resize(n);

    for (size_t i = 0; i < n; i++) {
        m_ticks[i]          = static_cast<double>(m_history[i].tick);
        m_agentCounts[i]    = static_cast<double>(m_history[i].agentCount);
        m_resourceCounts[i] = static_cast<double>(m_history[i].resourceCount);
        m_totalEnergies[i]  = m_history[i].totalEnergy;
        m_avgFitnesses[i]   = m_history[i].avgFitness;
    }
}

vector<double> ImGuiVisualizer::smooth(const vector<double>& data, int w) const {
    if (w <= 1 || data.empty()) return data;
    vector<double> out(data.size());
    int half = w / 2;
    for (size_t i = 0; i < data.size(); i++) {
        double sum = 0.0;
        int count = 0;
        for (int j = -half; j <= half; j++) {
            int idx = static_cast<int>(i) + j;
            if (idx >= 0 && idx < static_cast<int>(data.size())) {
                sum += data[idx];
                count++;
            }
        }
        out[i] = sum / count;
    }
    return out;
}

ImGuiVisualizer::Stats ImGuiVisualizer::computeStats(const vector<double>& d) {
    Stats s{};
    if (d.empty()) return s;
    s.min   = *min_element(d.begin(), d.end());
    s.max   = *max_element(d.begin(), d.end());
    s.avg   = accumulate(d.begin(), d.end(), 0.0) / static_cast<double>(d.size());
    s.first = d.front();
    s.last  = d.back();
    if (d.size() >= 2)
        s.trend = (d.back() - d.front()) / static_cast<double>(d.size() - 1);
    double sq = 0.0;
    for (double v : d) sq += (v - s.avg) * (v - s.avg);
    s.stddev = sqrt(sq / static_cast<double>(d.size()));
    return s;
}


void ImGuiVisualizer::exportCSV(const string& path) const {
    ofstream f(path);
    if (!f.is_open()) {
        cerr << "Cannot write CSV: " << path << "\n";
        return;
    }
    f << "Tick,Timestamp,AgentCount,ResourceCount,TotalEnergy,AvgFitness\n";
    for (const auto& s : m_history) {
        f << s.tick << ","
          << s.timestamp << ","
          << s.agentCount << ","
          << s.resourceCount << ","
          << s.totalEnergy << ","
          << s.avgFitness << "\n";
    }
    cout << "Exported " << m_history.size() << " snapshots to " << path << "\n";
}


void ImGuiVisualizer::run() {
    if (!init()) return;
    while (!shouldClose()) {
        frameTick();
    }
}


bool ImGuiVisualizer::init() {
    if (m_initialized) return true;
    if (!initWindow()) {
        cerr << "Failed to create GLFW window.\n";
        return false;
    }
    initImGui();
    m_lastFrameTime = glfwGetTime();
    m_initialized = true;
    return true;
}

bool ImGuiVisualizer::shouldClose() const {
    return m_window && glfwWindowShouldClose(m_window);
}

bool ImGuiVisualizer::frameTick() {
    if (!m_window || glfwWindowShouldClose(m_window))
        return false;

    glfwPollEvents();

    double now = glfwGetTime();
    double dt = now - m_lastFrameTime;
    m_lastFrameTime = now;

    if (m_tickCallback && !m_simPaused && m_autoTickRate > 0) {
        m_tickAccum += dt;
        double interval = 1.0 / static_cast<double>(m_autoTickRate);
        while (m_tickAccum >= interval) {
            m_tickAccum -= interval;
            m_tickCallback();
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderFrame();

    ImGui::Render();
    int dw, dh;
    glfwGetFramebufferSize(m_window, &dw, &dh);
    glViewport(0, 0, dw, dh);
    glClearColor(0.06f, 0.06f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
    return true;
}


void ImGuiVisualizer::renderFrame() {
    renderMenuBar();
    renderOverviewPanel();
    if (m_simRunning) renderSimulationControls();
    renderPopulationPlot();
    renderEnergyPlot();
    renderFitnessPlot();
    renderResourcePlot();
    renderNormalizedPlot();
    renderDerivativesPlot();
    renderEnergyHistogram();
    renderStatisticsPanel();
    renderControlPanel();
    if (m_worldMapCallback) renderWorldMap();
    if (!m_batchResults.empty()) renderBatchResults();

    if (m_showDemoWindow) {
        ImGui::ShowDemoWindow(&m_showDemoWindow);
        ImPlot::ShowDemoWindow();
    }
}


void ImGuiVisualizer::renderMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Reload CSV") && !m_csvPath.empty())
                loadCSV(m_csvPath);
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("ImGui / ImPlot Demo", nullptr, &m_showDemoWindow);
            ImGui::EndMenu();
        }

        char info[64];
        snprintf(info, sizeof(info), "Snapshots: %zu", m_history.size());
        float w = ImGui::CalcTextSize(info).x + 20.0f;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - w);
        ImGui::TextColored({0.5f, 0.7f, 0.9f, 1.0f}, "%s", info);

        ImGui::EndMainMenuBar();
    }
}


void ImGuiVisualizer::renderOverviewPanel() {
    ImGui::SetNextWindowPos({10, 30}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({1660, 130}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Overview");

    if (m_history.empty()) {
        ImGui::TextColored({1.0f, 0.8f, 0.2f, 1.0f}, "No data loaded.  Use Controls panel to load a CSV file.");
        ImGui::End();
        return;
    }

    const auto& cur = m_history.back();
    const VisSnapshot* prev = m_history.size() >= 2 ? &m_history[m_history.size() - 2] : nullptr;

    ImGui::TextColored({0.4f, 0.8f, 1.0f, 1.0f}, "A-LIFE SIMULATION DASHBOARD");
    ImGui::SameLine(0, 30);
    ImGui::TextColored({0.4f, 0.4f, 0.5f, 1.0f}, "Tick range: %llu - %llu",
                       (unsigned long long)m_history.front().tick,
                       (unsigned long long)cur.tick);
    ImGui::Separator();
    ImGui::Spacing();

    char buf[64];

    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)cur.tick);
    StatCard("##tick", "Tick", buf, {0.8f, 0.8f, 1.0f, 1.0f});
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%d", cur.agentCount);
    StatCard("##agents", "Agents", buf, {0.3f, 0.7f, 1.0f, 1.0f});
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%d", cur.resourceCount);
    StatCard("##resources", "Resources", buf, {0.2f, 0.9f, 0.7f, 1.0f});
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%.4f", cur.totalEnergy);
    StatCard("##energy", "Total Energy", buf, {0.9f, 0.8f, 0.2f, 1.0f}, 170);
    ImGui::SameLine();

    snprintf(buf, sizeof(buf), "%.4f", cur.avgFitness);
    StatCard("##fitness", "Avg Fitness", buf, {1.0f, 0.5f, 0.2f, 1.0f}, 170);

    if (prev) {
        ImGui::SameLine(0, 30);
        ImGui::BeginGroup();
        ImGui::TextColored({0.55f, 0.55f, 0.65f, 1.0f}, "Trends");

        auto trend = [&](const char* lbl, double a, double b) {
            ImVec4 tc = TrendColor(a, b);
            ImGui::TextColored(tc, "%s %s", lbl, TrendArrow(a, b));
        };
        trend("Energy",  cur.totalEnergy, prev->totalEnergy);
        ImGui::SameLine(0, 16);
        trend("Agents",  cur.agentCount,  prev->agentCount);
        ImGui::SameLine(0, 16);
        trend("Fitness", cur.avgFitness,  prev->avgFitness);
        ImGui::EndGroup();
    }

    ImGui::End();
}


void ImGuiVisualizer::renderPopulationPlot() {
    ImGui::SetNextWindowPos({10, 260}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 340}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Agent Population");
    if (m_ticks.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    auto ys = smooth(m_agentCounts, m_smoothingWindow);
    if (ImPlot::BeginPlot("##AgentPop", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Tick", "Agent Count");
        ImPlot::SetupAxisLimits(ImAxis_X1, m_ticks.front(), m_ticks.back(), ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.3f, 0.7f, 1.0f, 0.20f));
        ImPlot::PlotShaded("Agents", m_ticks.data(), ys.data(), (int)m_ticks.size(), 0);
        ImPlot::PlotLine("Agents",   m_ticks.data(), ys.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor(2);

        ImPlot::EndPlot();
    }
    ImGui::End();
}

void ImGuiVisualizer::renderEnergyPlot() {
    ImGui::SetNextWindowPos({840, 260}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 340}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Total Energy");
    if (m_ticks.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    auto ys = smooth(m_totalEnergies, m_smoothingWindow);
    if (ImPlot::BeginPlot("##Energy", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Tick", "Energy");
        ImPlot::SetupAxisLimits(ImAxis_X1, m_ticks.front(), m_ticks.back(), ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.9f, 0.8f, 0.2f, 1.0f));
        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.9f, 0.8f, 0.2f, 0.20f));
        ImPlot::PlotShaded("Total Energy", m_ticks.data(), ys.data(), (int)m_ticks.size(), 0);
        ImPlot::PlotLine("Total Energy",   m_ticks.data(), ys.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor(2);

        double zx[2] = {m_ticks.front(), m_ticks.back()};
        double zy[2] = {0.0, 0.0};
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.5f, 0.5f, 0.5f, 0.4f));
        ImPlot::PlotLine("##zero", zx, zy, 2);
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
    ImGui::End();
}

void ImGuiVisualizer::renderFitnessPlot() {
    ImGui::SetNextWindowPos({10, 610}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 340}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Average Fitness");
    if (m_ticks.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    auto ys = smooth(m_avgFitnesses, m_smoothingWindow);
    if (ImPlot::BeginPlot("##Fitness", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Tick", "Avg Fitness");
        ImPlot::SetupAxisLimits(ImAxis_X1, m_ticks.front(), m_ticks.back(), ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(1.0f, 0.5f, 0.2f, 0.20f));
        ImPlot::PlotShaded("Avg Fitness", m_ticks.data(), ys.data(), (int)m_ticks.size(), 0);
        ImPlot::PlotLine("Avg Fitness",   m_ticks.data(), ys.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor(2);

        ImPlot::EndPlot();
    }
    ImGui::End();
}

void ImGuiVisualizer::renderResourcePlot() {
    ImGui::SetNextWindowPos({840, 610}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 340}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Resources");
    if (m_ticks.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    auto ys = smooth(m_resourceCounts, m_smoothingWindow);
    if (ImPlot::BeginPlot("##Resources", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Tick", "Resource Count");
        ImPlot::SetupAxisLimits(ImAxis_X1, m_ticks.front(), m_ticks.back(), ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.2f, 0.9f, 0.7f, 1.0f));
        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.2f, 0.9f, 0.7f, 0.20f));
        ImPlot::PlotShaded("Resources", m_ticks.data(), ys.data(), (int)m_ticks.size(), 0);
        ImPlot::PlotLine("Resources",   m_ticks.data(), ys.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor(2);

        ImPlot::EndPlot();
    }
    ImGui::End();
}


void ImGuiVisualizer::renderNormalizedPlot() {
    ImGui::SetNextWindowPos({10, 960}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 340}, ImGuiCond_FirstUseEver);
    ImGui::Begin("All Metrics (Normalized)");
    if (m_ticks.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    auto norm = [&](const vector<double>& v) {
        double mn = *min_element(v.begin(), v.end());
        double mx = *max_element(v.begin(), v.end());
        double range = (mx - mn == 0.0) ? 1.0 : mx - mn;
        vector<double> out(v.size());
        for (size_t i = 0; i < v.size(); i++) out[i] = (v[i] - mn) / range;
        return smooth(out, m_smoothingWindow);
    };

    auto nA = norm(m_agentCounts);
    auto nE = norm(m_totalEnergies);
    auto nF = norm(m_avgFitnesses);
    auto nR = norm(m_resourceCounts);

    if (ImPlot::BeginPlot("##Normalized", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Tick", "Normalized [0-1]");
        ImPlot::SetupAxisLimits(ImAxis_X1, m_ticks.front(), m_ticks.back(), ImPlotCond_Once);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.05, 1.05, ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImPlot::PlotLine("Agents",   m_ticks.data(), nA.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.9f, 0.8f, 0.2f, 1.0f));
        ImPlot::PlotLine("Energy",   m_ticks.data(), nE.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
        ImPlot::PlotLine("Fitness",  m_ticks.data(), nF.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.2f, 0.9f, 0.7f, 1.0f));
        ImPlot::PlotLine("Resources", m_ticks.data(), nR.data(), (int)m_ticks.size());
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
    ImGui::End();
}


void ImGuiVisualizer::renderDerivativesPlot() {
    ImGui::SetNextWindowPos({840, 960}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 340}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Rate of Change");
    if (m_ticks.size() < 2) { ImGui::Text("Need >= 2 data points."); ImGui::End(); return; }

    const size_t n = m_ticks.size();
    vector<double> dt(n - 1), dE(n - 1), dA(n - 1), dF(n - 1);
    for (size_t i = 1; i < n; i++) {
        double dx = m_ticks[i] - m_ticks[i - 1];
        if (dx == 0.0) dx = 1.0;
        dt[i - 1] = m_ticks[i];
        dE[i - 1] = (m_totalEnergies[i] - m_totalEnergies[i - 1]) / dx;
        dA[i - 1] = (m_agentCounts[i]   - m_agentCounts[i - 1])   / dx;
        dF[i - 1] = (m_avgFitnesses[i]  - m_avgFitnesses[i - 1])  / dx;
    }

    auto sE = smooth(dE, m_smoothingWindow);
    auto sA = smooth(dA, m_smoothingWindow);
    auto sF = smooth(dF, m_smoothingWindow);

    if (ImPlot::BeginPlot("##Derivatives", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Tick", "Change / Tick");
        ImPlot::SetupAxisLimits(ImAxis_X1, dt.front(), dt.back(), ImPlotCond_Once);

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.9f, 0.8f, 0.2f, 1.0f));
        ImPlot::PlotLine("dEnergy/dt",  dt.data(), sE.data(), (int)dt.size());
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImPlot::PlotLine("dAgents/dt",  dt.data(), sA.data(), (int)dt.size());
        ImPlot::PopStyleColor();

        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
        ImPlot::PlotLine("dFitness/dt", dt.data(), sF.data(), (int)dt.size());
        ImPlot::PopStyleColor();

        double zx[2] = {dt.front(), dt.back()};
        double zy[2] = {0.0, 0.0};
        ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.5f, 0.5f, 0.5f, 0.4f));
        ImPlot::PlotLine("##zero", zx, zy, 2);
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
    ImGui::End();
}


void ImGuiVisualizer::renderEnergyHistogram() {
    ImGui::SetNextWindowPos({10, 1310}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 300}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Energy Distribution");
    if (m_totalEnergies.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    if (ImPlot::BeginPlot("##EnergyHist", ImVec2(-1, -1))) {
        ImPlot::SetupAxes("Total Energy", "Count");

        ImPlot::PushStyleColor(ImPlotCol_Fill, ImVec4(0.9f, 0.8f, 0.2f, 0.65f));
        ImPlot::PlotHistogram("Energy", m_totalEnergies.data(), (int)m_totalEnergies.size());
        ImPlot::PopStyleColor();

        ImPlot::EndPlot();
    }
    ImGui::End();
}


void ImGuiVisualizer::renderStatisticsPanel() {
    ImGui::SetNextWindowPos({840, 1310}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 300}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Statistics");
    if (m_history.empty()) { ImGui::Text("No data."); ImGui::End(); return; }

    auto sa = computeStats(m_agentCounts);
    auto se = computeStats(m_totalEnergies);
    auto sf = computeStats(m_avgFitnesses);
    auto sr = computeStats(m_resourceCounts);

    const int flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg
                    | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("##stats", 8, flags)) {
        ImGui::TableSetupColumn("Metric");
        ImGui::TableSetupColumn("Current");
        ImGui::TableSetupColumn("Min");
        ImGui::TableSetupColumn("Max");
        ImGui::TableSetupColumn("Mean");
        ImGui::TableSetupColumn("Std Dev");
        ImGui::TableSetupColumn("Trend (/tick)");
        ImGui::TableSetupColumn("Samples");
        ImGui::TableHeadersRow();

        auto row = [&](const char* name, const Stats& st, const ImVec4& col, size_t count) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextColored(col, "%s", name);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.4f", st.last);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%.4f", st.min);
            ImGui::TableSetColumnIndex(3); ImGui::Text("%.4f", st.max);
            ImGui::TableSetColumnIndex(4); ImGui::Text("%.4f", st.avg);
            ImGui::TableSetColumnIndex(5); ImGui::Text("%.4f", st.stddev);
            ImGui::TableSetColumnIndex(6);
            ImVec4 tc = st.trend > 0 ? ImVec4(0.2f,0.9f,0.3f,1.f) :
                        st.trend < 0 ? ImVec4(0.9f,0.2f,0.2f,1.f) :
                                        ImVec4(0.6f,0.6f,0.6f,1.f);
            ImGui::TextColored(tc, "%+.6f", st.trend);
            ImGui::TableSetColumnIndex(7); ImGui::Text("%zu", count);
        };

        row("Agent Count",  sa, {0.3f, 0.7f, 1.0f, 1.f}, m_agentCounts.size());
        row("Total Energy", se, {0.9f, 0.8f, 0.2f, 1.f}, m_totalEnergies.size());
        row("Avg Fitness",  sf, {1.0f, 0.5f, 0.2f, 1.f}, m_avgFitnesses.size());
        row("Resources",    sr, {0.2f, 0.9f, 0.7f, 1.f}, m_resourceCounts.size());

        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::TextColored({0.45f, 0.45f, 0.55f, 1.0f},
        "Tick range %llu - %llu  |  %zu unique snapshots",
        (unsigned long long)m_history.front().tick,
        (unsigned long long)m_history.back().tick,
        m_history.size());

    ImGui::End();
}


void ImGuiVisualizer::renderControlPanel() {
    ImGui::SetNextWindowPos({1280, 30}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({380, 130}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls");

    ImGui::Text("CSV File:");
    ImGui::SetNextItemWidth(-80);
    ImGui::InputText("##csv", m_csvPathBuf, sizeof(m_csvPathBuf));
    ImGui::SameLine();
    if (ImGui::Button("Load")) {
        if (strlen(m_csvPathBuf) > 0)
            loadCSV(m_csvPathBuf);
    }

    ImGui::SetNextItemWidth(200);
    ImGui::SliderInt("Smoothing", &m_smoothingWindow, 1, 21, "%d ticks");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Moving-average window. 1 = raw data.");

    ImGui::End();
}


void ImGuiVisualizer::renderSimulationControls() {
    ImGui::SetNextWindowPos({10, 170}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({1660, 110}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Simulation Controls");

    if (m_simPaused) {
        if (ImGui::Button("Resume", ImVec2(80, 0)))
            m_simPaused = false;
    } else {
        if (ImGui::Button("Pause", ImVec2(80, 0)))
            m_simPaused = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Step", ImVec2(60, 0))) {
        if (m_tickCallback) m_tickCallback();
    }
    ImGui::SameLine();
    if (ImGui::Button("x10", ImVec2(45, 0))) {
        if (m_tickCallback) for (int i = 0; i < 10; i++) m_tickCallback();
    }
    ImGui::SameLine();
    if (ImGui::Button("x100", ImVec2(50, 0))) {
        if (m_tickCallback) for (int i = 0; i < 100; i++) m_tickCallback();
    }

    ImGui::SameLine(0, 20);
    ImGui::SetNextItemWidth(180);
    ImGui::SliderInt("Tick Rate", &m_autoTickRate, 0, 120, "%d t/s");

    ImGui::SameLine(0, 20);
    ImGui::TextColored(
        m_simPaused ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.2f, 0.9f, 0.3f, 1.0f),
        m_simPaused ? "PAUSED" : "RUNNING");

    ImGui::SameLine(0, 20);
    ImGui::TextColored({0.5f, 0.5f, 0.6f, 1.0f}, "%zu snapshots", m_history.size());

    ImGui::SameLine(0, 30);
    if (ImGui::Button("Export CSV")) {
        string path = m_csvPath.empty() ? "live_simulation_out.csv" : m_csvPath;
        exportCSV(path);
    }
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("Save all snapshots to CSV");

    if (m_restartCallback) {
        ImGui::SameLine(0, 30);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Restart Simulation")) {
            m_restartCallback();
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Reinitialize simulation with new random seed");
    }

    if (m_restartCallback) {
        ImGui::Separator();
        ImGui::SetNextItemWidth(100);
        ImGui::InputInt("##BatchCount", &m_batchTarget, 1, 10);
        if (m_batchTarget < 1) m_batchTarget = 1;
        ImGui::SameLine();

        if (!m_batchActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.55f, 0.85f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.65f, 0.95f, 1.0f));
            if (ImGui::Button("Run Batch", ImVec2(100, 0))) {
                m_batchResults.clear();
                m_batchCurrent = 0;
                m_batchActive = true;
                m_simPaused = false;
                if (m_restartCallback) m_restartCallback();
            }
            ImGui::PopStyleColor(2);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Run %d simulations back-to-back and collect results", m_batchTarget);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.1f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.2f, 1.0f));
            if (ImGui::Button("Stop Batch", ImVec2(100, 0))) {
                m_batchActive = false;
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();
            ImGui::TextColored({0.3f, 0.8f, 1.0f, 1.0f}, "Run %d / %d", m_batchCurrent + 1, m_batchTarget);

            ImGui::SameLine(0, 20);
            float progress = (float)m_batchCurrent / (float)m_batchTarget;
            ImGui::SetNextItemWidth(200);
            ImGui::ProgressBar(progress, ImVec2(200, 0));
        }

        if (!m_batchResults.empty()) {
            ImGui::SameLine(0, 20);
            ImGui::TextColored({0.6f, 0.6f, 0.7f, 1.0f}, "%zu completed runs", m_batchResults.size());
        }
    }

    ImGui::End();
}


void ImGuiVisualizer::renderWorldMap() {
    if (m_worldMapCallback) m_worldMap = m_worldMapCallback();

    ImGui::SetNextWindowPos({10, 1620}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 500}, ImGuiCond_FirstUseEver);
    ImGui::Begin("World Map");

    const int gs = m_worldMap.gridSize;
    if (gs <= 0 || (int)m_worldMap.tileValues.size() != gs * gs) {
        ImGui::Text("No world data.");
        ImGui::End();
        return;
    }

    if (ImPlot::BeginPlot("##WorldMap", ImVec2(-1, -1), ImPlotFlags_Equal)) {
        ImPlot::SetupAxes("X", "Y", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxisLimits(ImAxis_X1, -0.5, gs - 0.5);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, gs - 0.5);

        ImPlot::PushColormap(ImPlotColormap_Viridis);
        ImPlot::PlotHeatmap("Tiles",
                            m_worldMap.tileValues.data(),
                            gs, gs,
                            0.0, 1.0,
                            nullptr,
                            ImPlotPoint(0, 0),
                            ImPlotPoint(gs, gs));
        ImPlot::PopColormap();

        if (!m_worldMap.entityPositions.empty()) {
            vector<double> ex, ey;
            for (auto& [px, py] : m_worldMap.entityPositions) {
                ex.push_back(px + 0.5);
                ey.push_back(py + 0.5);
            }
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Cross, 10, ImVec4(1, 1, 1, 1), 3);
            ImPlot::PlotScatter("Entity", ex.data(), ey.data(), (int)ex.size());
        }

        if (!m_worldMap.foodPositions.empty()) {
            vector<double> fx, fy;
            for (auto& [px, py] : m_worldMap.foodPositions) {
                fx.push_back(px + 0.5);
                fy.push_back(py + 0.5);
            }
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4, ImVec4(1.0f, 0.6f, 0.1f, 0.8f), 1);
            ImPlot::PlotScatter("Food", fx.data(), fy.data(), (int)fx.size());
        }

        if (!m_worldMap.waterPositions.empty()) {
            vector<double> wx, wy;
            for (auto& [px, py] : m_worldMap.waterPositions) {
                wx.push_back(px + 0.5);
                wy.push_back(py + 0.5);
            }
            ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 4, ImVec4(0.2f, 0.6f, 1.0f, 0.8f), 1);
            ImPlot::PlotScatter("Water", wx.data(), wy.data(), (int)wx.size());
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}


void ImGuiVisualizer::renderBatchResults() {
    ImGui::SetNextWindowPos({840, 1620}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize({820, 500}, ImGuiCond_FirstUseEver);
    ImGui::Begin("Batch Results");

    if (m_batchResults.empty()) {
        ImGui::Text("No completed runs.");
        ImGui::End();
        return;
    }

    const size_t n = m_batchResults.size();

    double sumLife = 0, sumAvgE = 0, sumPeakE = 0, sumFinalE = 0, sumFit = 0;
    uint64_t minLife = UINT64_MAX, maxLife = 0;
    double bestFit = 0, worstFit = 1e18;
    for (auto& r : m_batchResults) {
        sumLife  += r.lifespan;
        sumAvgE  += r.avgEnergy;
        sumPeakE += r.peakEnergy;
        sumFinalE += r.finalEnergy;
        sumFit   += r.avgFitness;
        if (r.lifespan < minLife) minLife = r.lifespan;
        if (r.lifespan > maxLife) maxLife = r.lifespan;
        if (r.avgFitness > bestFit) bestFit = r.avgFitness;
        if (r.avgFitness < worstFit) worstFit = r.avgFitness;
    }
    double avgLife = sumLife / n;

    ImGui::TextColored({0.3f, 0.85f, 1.0f, 1.0f}, "Aggregate (%zu runs)", n);
    ImGui::Separator();
    ImGui::Columns(4, "##AggCols", true);
    ImGui::Text("Avg Lifespan");  ImGui::NextColumn();
    ImGui::Text("Min / Max");     ImGui::NextColumn();
    ImGui::Text("Avg Energy");    ImGui::NextColumn();
    ImGui::Text("Avg Fitness");   ImGui::NextColumn();
    ImGui::Separator();

    ImGui::TextColored({1.0f, 1.0f, 0.4f, 1.0f}, "%.1f ticks", avgLife);  ImGui::NextColumn();
    ImGui::Text("%llu / %llu", (unsigned long long)minLife, (unsigned long long)maxLife); ImGui::NextColumn();
    ImGui::Text("%.4f", sumAvgE / n);  ImGui::NextColumn();
    ImGui::Text("%.4f", sumFit / n);   ImGui::NextColumn();
    ImGui::Columns(1);

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##BatchTabs")) {
        if (ImGui::BeginTabItem("Table")) {
            if (ImGui::BeginTable("##RunTable", 7,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                ImVec2(0, 200))) {

                ImGui::TableSetupColumn("Run");
                ImGui::TableSetupColumn("Lifespan");
                ImGui::TableSetupColumn("Peak Energy");
                ImGui::TableSetupColumn("Avg Energy");
                ImGui::TableSetupColumn("Final Energy");
                ImGui::TableSetupColumn("Avg Fitness");
                ImGui::TableSetupColumn("Peak Resources");
                ImGui::TableHeadersRow();

                for (auto& r : m_batchResults) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::Text("%d", r.runIndex);
                    ImGui::TableNextColumn(); ImGui::Text("%llu", (unsigned long long)r.lifespan);
                    ImGui::TableNextColumn(); ImGui::Text("%.4f", r.peakEnergy);
                    ImGui::TableNextColumn(); ImGui::Text("%.4f", r.avgEnergy);
                    ImGui::TableNextColumn(); ImGui::Text("%.4f", r.finalEnergy);
                    ImGui::TableNextColumn(); ImGui::Text("%.4f", r.avgFitness);
                    ImGui::TableNextColumn(); ImGui::Text("%d", r.peakResources);
                }

                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Lifespan Plot")) {
            if (ImPlot::BeginPlot("##LifespanPlot", ImVec2(-1, -1))) {
                ImPlot::SetupAxes("Run #", "Ticks Survived");
                vector<double> xs(n), ys(n);
                for (size_t i = 0; i < n; i++) {
                    xs[i] = m_batchResults[i].runIndex;
                    ys[i] = static_cast<double>(m_batchResults[i].lifespan);
                }
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5);
                ImPlot::PlotBars("Lifespan", xs.data(), ys.data(), (int)n, 0.6);

                double avg = avgLife;
                double xmin = 0.5, xmax = n + 0.5;
                ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(1, 0.8f, 0.2f, 1));
                ImPlot::PlotLine("Avg", &xmin, &avg, 1);
                double xs2[2] = {xmin, xmax};
                double ys2[2] = {avg, avg};
                ImPlot::PlotLine("Avg##line", xs2, ys2, 2);
                ImPlot::PopStyleColor();

                ImPlot::EndPlot();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Energy Plot")) {
            if (ImPlot::BeginPlot("##BatchEnergyPlot", ImVec2(-1, -1))) {
                ImPlot::SetupAxes("Run #", "Energy");
                vector<double> xs(n), peakE(n), avgE(n), finalE(n);
                for (size_t i = 0; i < n; i++) {
                    xs[i]    = m_batchResults[i].runIndex;
                    peakE[i] = m_batchResults[i].peakEnergy;
                    avgE[i]  = m_batchResults[i].avgEnergy;
                    finalE[i]= m_batchResults[i].finalEnergy;
                }
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 4);
                ImPlot::PlotLine("Peak", xs.data(), peakE.data(), (int)n);
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, 4);
                ImPlot::PlotLine("Avg", xs.data(), avgE.data(), (int)n);
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Diamond, 4);
                ImPlot::PlotLine("Final", xs.data(), finalE.data(), (int)n);
                ImPlot::EndPlot();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fitness Plot")) {
            if (ImPlot::BeginPlot("##BatchFitnessPlot", ImVec2(-1, -1))) {
                ImPlot::SetupAxes("Run #", "Avg Fitness");
                vector<double> xs(n), fit(n);
                for (size_t i = 0; i < n; i++) {
                    xs[i]  = m_batchResults[i].runIndex;
                    fit[i] = m_batchResults[i].avgFitness;
                }
                ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5);
                ImPlot::PlotBars("Fitness", xs.data(), fit.data(), (int)n, 0.6);
                ImPlot::EndPlot();
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
