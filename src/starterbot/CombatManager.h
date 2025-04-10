#pragma once

#include "Tools.h"
#include "UnitManager.h"
#include "UnitTools.h"

// This class is in charge of all defensive and offensive operation for the bot. It
// automatically reserves fighter units for defensive purposes and defends the base
// against attacking enemy units. Additionally, when the class is instructed to attack, it
// sends these units to the enemy base, where they attack the enemy units according to the
// strategic priorities of the class.
class CombatManager : public EventReceiver {
private:
    UnitManager& m_unitManager;

    // The ideal and maximum sizes that we cluster units into for combat purposes.
    static constexpr int IDEAL_CLUSTER = 7;
    static constexpr int MAX_CLUSTER = 10;

    // How far away from the enemy base attacking units should stop and wait to regroup.
    static constexpr int WAIT_RADIUS = 512;
    // How close all the units in the group need to be to be considered regrouped.
    static constexpr int GROUP_RADIUS = 192;

    // How large a radius around our buildings is considered to be part of our base.
    static constexpr int BASE_RADIUS = 768;
    // How close enemy units must be to our base for our defensive units to attack.
    static constexpr int DANGER_RADIUS = 256;

    // The number of frames we delay before choosing new targets for our units to attack
    // in order to prevent switching between targets too quickly to get any progress.
    static constexpr int RETARGET_TIME = 50;

    // The countdown timer between recomputing which units are in each cluster. We do this
    // on a timeout to prevent clusters from changing too rapidly as units move around.
    int m_clusterTimer;

    // The countdown timers before choosing new targets for our defensive/attacking units.
    int m_defenseTimer;
    int m_offenseTimer;

    // Whether the bot should defend against attackers, which is recomputed each frame.
    bool m_isDefending;
    // Whether the bot is in the middle of an attack, which is set via startAttack() and
    // reset if the entire attacking army is killed.
    bool m_isAttacking;
    // Whether the bot is moving units to the enemy base or waiting for units to regroup
    // before commencing the attack itself.
    bool m_isWaiting;

    // Sets containing the reserved units for defense and offense. All new fighter units
    // are reserved for defense upon creation. Defensive troops are transferred to become
    // offensive troops when an attack is started.
    bw::Unitset m_defenseUnits;
    bw::Unitset m_offenseUnits;
    // Set containing all enemy units considered dangerous for the purposes of defense,
    // namely mobile troops (not including buildings) that can attack us. This set is
    // recomputed every frame, and primarily exists for clustering purposes.
    bw::Unitset m_dangerUnits;

    // Clusterings of the previous defensive, offensive, and dangerous enemy sets.
    std::vector<Cluster> m_defenseClusters;
    std::vector<Cluster> m_offenseClusters;
    std::vector<Cluster> m_dangerClusters;

    // Arbitrary units that are chosen to be the "leaders" for the defensive and offensive
    // parts of the army, primarily to have a fixed unit that the other units can follow
    // and group around when transitioning between places.
    bw::Unit m_defenseLeader;
    bw::Unit m_offenseLeader;

public:
    CombatManager(UnitManager& unitManager);

    // Instructs the combat code to start attacking the enemy base using all the current
    // units assigned as defensive units.
    void startAttack();

protected:
    virtual void onStart() override;
    virtual void onFrame() override;
    virtual void onDraw() override;
    virtual void onUnitDestroy(bw::Unit unit) override;

private:
    // Helper functions that manage each major portion of combat, namely common unit
    // tasks shared across the whole class, defense, and offense.
    void updateUnits();

    void updateDefense();
    void updatePassiveDefense();
    void updateActiveDefense();

    void updateOffense();
    void updateWaitingOffense();
    void updateAttackOffense();

    // Gets all units that are within the base radius of any buildings for a specific
    // player, used to determine which units are a safe distance away from a base.
    bw::Unitset getBaseUnits(bw::Player player);

    // Draws lines and circles to give a visual demonstration of the clusters in a base.
    void drawClusters(const std::vector<Cluster>& clusters, int radius, bw::Color color);
};