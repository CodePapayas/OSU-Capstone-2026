/*
  Author: Kai Lindskog-Coffin
  Oregon State University
  CS 462 
*/


#include "../include/circular_buffer.h"
#include "../include/simulation_state.h"
#include "../include/fitness_calculator.h"
#include "../include/resource_node.h"
#include <iostream>
#include <cmath>

using namespace std;

// Original test suite for A-Life simulation core components
// Implements validation for circular buffer, fitness evaluation, and resources

int totalTests = 0, passedTests = 0;

#define TEST(name) totalTests++; cout << name << "... "; bool ok = true;
#define CHECK(cond) if (!(cond)) { ok = false; cout << "FAIL@" << __LINE__ << " "; }
#define END_TEST() if (ok) { cout << "PASS"; passedTests++; } cout << endl;

// Circular buffer validation
void testCircularBuffer() {
    TEST("CircularBuffer")
    
    // Test basic operations
    CircularBuffer<int> buf(3);
    CHECK(buf.empty());
    
    buf.push(1); buf.push(2); buf.push(3);
    CHECK(buf.full() && buf.latest() == 3);
    
    // Test overflow behavior (should drop oldest = 1)
    buf.push(4);
    CHECK(buf.get(0) == 2 && buf.rewind(0) == 4);
    
    // Test clear
    buf.clear();
    CHECK(buf.empty());
    
    // Test with actual simulation state
    CircularBuffer<SimulationState> hist(5);
    for (int i = 0; i < 3; i++) {
        SimulationState s;
        s.tick = i;
        s.agentCount = 10 + i;
        hist.push(s);
    }
    CHECK(hist.latest().tick == 2 && hist.rewind(2).agentCount == 10);
    
    END_TEST()
}

// Fitness calculation validation
void testFitnessCalculator() {
    TEST("FitnessCalculator")
    
    // Test individual component scores
    CHECK(abs(FitnessCalculator::energyScore(75.0, 100.0) - 0.75) < 0.01);
    CHECK(FitnessCalculator::survivalScore(100) > FitnessCalculator::survivalScore(10));
    CHECK(FitnessCalculator::efficiencyScore(200.0, 100.0) > 0.5);
    CHECK(FitnessCalculator::reproductionScore(5) > 0.0);
    
    // Test full fitness calculation: healthy agent should beat struggling agent
    double fit1 = FitnessCalculator::calculateFitness(80.0, 100.0, 150, 500.0, 300.0, 2);
    double fit2 = FitnessCalculator::calculateFitness(20.0, 100.0, 50, 200.0, 250.0, 0);
    CHECK(fit1 > fit2 && fit1 <= 1.0);
    
    // Test survival thresholds
    CHECK(FitnessCalculator::meetsMinimumSurvival(50.0, 10.0));
    CHECK(!FitnessCalculator::canReproduce(40.0, 50.0));
    
    // Test custom weights: 100% energy weight should equal energy score
    FitnessCalculator::FitnessWeights w;
    w.energyWeight = 1.0; w.survivalWeight = 0.0;
    w.efficiencyWeight = 0.0; w.reproductionWeight = 0.0;
    double fit = FitnessCalculator::calculateFitness(75.0, 100.0, 100, 500.0, 300.0, 5, w);
    CHECK(abs(fit - 0.75) < 0.01);
    
    END_TEST()
}

// Resource node validation
void testResourceNode() {
    TEST("ResourceNode")
    
    // Create renewable resource (like a plant)
    ResourceNode res(Position(10, 10), ResourceType::PLANT, 100.0, true);
    CHECK(res.getPosition() == Position(10, 10));
    CHECK(abs(res.getEnergyValue() - 100.0) < 0.01);
    CHECK(res.isRenewable());
    
    // Test consumption
    double consumed = res.consume(30.0);
    CHECK(abs(consumed - 30.0) < 0.01 && abs(res.getEnergyValue() - 70.0) < 0.01);
    
    // Test regeneration (should increase from 70.0)
    res.update(10.0);
    CHECK(res.getEnergyValue() > 70.0);
    
    // Test range detection
    CHECK(res.isInRange(Position(11, 10), 1));    // Adjacent, in range
    CHECK(!res.isInRange(Position(20, 20), 1));   // Too far
    
    END_TEST()
}

// Resource manager validation
void testResourceManager() {
    TEST("ResourceManager")
    
    // Create multiple resources
    ResourceManager mgr;
    mgr.createResource(Position(0, 0), ResourceType::FOOD, 50.0, false);
    mgr.createResource(Position(10, 10), ResourceType::WATER, 30.0, true);
    mgr.createResource(Position(20, 20), ResourceType::MINERAL, 40.0, false);
    
    CHECK(mgr.getResourceCount() == 3);
    CHECK(abs(mgr.getTotalEnergy() - 120.0) < 0.01);
    
    // Test finding nearest (from 5,5, nearest should be 0,0)
    ResourceNode* nearest = mgr.findNearestResource(Position(5, 5));
    CHECK(nearest != nullptr && nearest->getPosition() == Position(0, 0));
    
    // Test range queries
    auto inRange = mgr.findResourcesInRange(Position(0, 0), 2);
    CHECK(inRange.size() >= 1);
    
    // Test depletion and cleanup
    ResourceNode* r = mgr.createResource(Position(1, 1), ResourceType::FOOD, 20.0, false);
    r->consume(20.0);  // Fully deplete it
    CHECK(r->isDepleted());
    
    size_t removed = mgr.removeDepletedResources();
    CHECK(removed >= 1);  // Should remove the depleted non-renewable
    
    mgr.clear();
    CHECK(mgr.getResourceCount() == 0);
    
    END_TEST()
}

// Position distance validation
void testPositionDistances() {
    TEST("Position distances")
    
    Position p1(0, 0), p2(3, 4);
    CHECK(p1.manhattanDistance(p2) == 7);
    CHECK(abs(p1.euclideanDistance(p2) - 5.0) < 0.01);
    CHECK(p1 == Position(0, 0) && !(p1 == p2));
    
    END_TEST()
}

int main() {
    cout << "=======================" << endl;
    cout << "A-LIFE TEST SUITE" << endl;
    cout << "=======================" << endl;
    
    testCircularBuffer();
    testFitnessCalculator();
    testResourceNode();
    testResourceManager();
    testPositionDistances();
    
    cout << "=======================" << endl;
    cout << passedTests << "/" << totalTests << " tests passed";
    if (passedTests == totalTests) cout << " ✓";
    cout << endl << "=======================" << endl;
    
    return passedTests == totalTests ? 0 : 1;
}
