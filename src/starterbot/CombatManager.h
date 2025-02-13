#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "UnitTools.h"

// This class manages the combat of the bot. When instructed to attack, it uses a greedy
// search to find the best groups of enemy units to attack. It utilizes a basic heuristic
// function that tries to gauge how good or bad the results of a confrontation will be.
class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // The time in frames that the retarget timer is reset to after it hits zero.
    static constexpr int RETARGET_TIME = 300;

    // The ideal and maximum sizes that we cluster units into for combat purposes.
    static constexpr int IDEAL_CLUSTER = 7;
    static constexpr int MAX_CLUSTER = 10;

    // Indicates whether the bot is currently attacking.
    bool m_attacking;

    // Contains the remaining amount of frames before the heuristic function should be run
    // to determine which enemies to attack with which soldiers.
    int m_retargetTimer;

    // The set of all units reserved to be soldiers to attack the enemy.
    bw::Unitset m_soldiers;

    // The clusterings of units calculated at the last retarget time.
    std::vector<Cluster> m_soldierClusters;
    std::vector<Cluster> m_enemyClusters;

public:
    CombatManager(UnitManager& unitManager);

    // Instructs the combat manager to attack. Once called, the bot will attack with full
    // force, and the attack cannot be cancelled.
    void attack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onDraw() override;
    virtual void onUnitDestroy(bw::Unit unit) override;

private:
    // Recalculates soldier and enemy clusterings and runs the heuristic function to
    // determine which soldiers should attack which enemies.
    void chooseSoldierTargets();

    // Calculates the goodness of a battle between a clustering of soldiers and targets.
    double calcClusterHeuristic(const Cluster& soldiers, const Cluster& targets);
    // Calculates the goodness of a battle between a single pair of units.
    double calcPairwiseHeuristic(const bw::Unit& soldier, const bw::Unit& target);

    // Draws lines between units in each cluster in a list of clusterings.
    void drawClusters(const std::vector<Cluster>& clusters, bw::Color color);
};