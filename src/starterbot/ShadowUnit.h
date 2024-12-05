#pragma once

#include "Tools.h"

// When complete map information is not enabled, all information about invisible enemy
// units is lost with no way to access the last known information about any unit. This
// class solves that problem by pretending to be a normal unit (by implementing the
// bw::UnitInterface interface) that "shadows" some real unit, saving all relevant unit
// fields every frame that the real unit is visible and continuing to provide that same
// information even after the real unit becomes invisible.
//
// It is important to note that shadow units cannot be commanded or targeted or otherwise
// passed into BWAPI in any way: doing so may result in a crash! For example, these are
// forbidden:
//
//     bw::Unit shadow = ...; // Get a ShadowUnit pointer somewhere.
//
//     shadow->attack(unit); // Commanding a shadow unit will do nothing.
//     otherUnit->rightClick(shadow); // Targeting a shadow unit will probably crash.
//
// To get the real unit associated with a shadow unit, use the UnitManager::getReal()
// method. To get the shadow unit for a real unit, use UnitManager::getShadow().
//
// Note that shadow units also provide setter methods for each saved field. This allows
// shadow units to be copied and modified to simulate a different game state.
class ShadowUnitImpl : public bw::UnitInterface {
private:
    // The real unit associated with this shadow unit.
    bw::Unit m_real;

    // These are all the fields that are saved every frame that the real unit is visible.
    bw::Player         m_player;
    bw::UnitType       m_type;
    bw::Position       m_position;
    double             m_angle;
    double             m_velocityX;
    double             m_velocityY;
    int                m_hitPoints;
    int                m_shields;
    int                m_energy;
    int                m_resources;
    int                m_resourceGroup;
    int                m_lastCommandFrame;
    bw::UnitCommand    m_lastCommand;
    bw::Player         m_lastAttackingPlayer;
    int                m_killCount;
    int                m_acidSporeCount;
    int                m_interceptorCount;
    int                m_scarabCount;
    int                m_spiderMineCount;
    int                m_groundWeaponCooldown;
    int                m_airWeaponCooldown;
    int                m_spellCooldown;
    int                m_defenseMatrixPoints;
    int                m_defenseMatrixTimer;
    int                m_ensnareTimer;
    int                m_irradiateTimer;
    int                m_lockdownTimer;
    int                m_maelstromTimer;
    int                m_orderTimer;
    int                m_plagueTimer;
    int                m_removeTimer;
    int                m_stasisTimer;
    int                m_stimTimer;
    bw::UnitType       m_buildType;
    bw::UnitType::list m_trainingQueue;
    bw::TechType       m_tech;
    bw::UpgradeType    m_upgrade;
    int                m_remainingBuildTime;
    int                m_remainingTrainTime;
    int                m_remainingResearchTime;
    int                m_remainingUpgradeTime;
    bw::Unit           m_buildUnit;
    bw::Unit           m_target;
    bw::Position       m_targetPosition;
    bw::Order          m_order;
    bw::Order          m_secondaryOrder;
    bw::Unit           m_orderTarget;
    bw::Position       m_orderTargetPosition;
    bw::Position       m_rallyPosition;
    bw::Unit           m_rallyUnit;
    bw::Unit           m_addon;
    bw::Unit           m_nydusExit;
    bw::Unit           m_powerUp;
    bw::Unit           m_transport;
    bw::Unitset        m_loadedUnits;
    bw::Unit           m_carrier;
    bw::Unitset        m_interceptors;
    bw::Unit           m_hatchery;
    bw::Unitset        m_larva;
    bool               m_hasNuke;
    bool               m_isAccelerating;
    bool               m_isAttacking;
    bool               m_isAttackFrame;
    bool               m_isBeingGathered;
    bool               m_isBeingHealed;
    bool               m_isBlind;
    bool               m_isBraking;
    bool               m_isBurrowed;
    bool               m_isCarryingGas;
    bool               m_isCarryingMinerals;
    bool               m_isCloaked;
    bool               m_isCompleted;
    bool               m_isConstructing;
    bool               m_isDetected;
    bool               m_isGatheringGas;
    bool               m_isGatheringMinerals;
    bool               m_isHallucination;
    bool               m_isIdle;
    bool               m_isInterruptible;
    bool               m_isInvincible;
    bool               m_isLifted;
    bool               m_isMorphing;
    bool               m_isMoving;
    bool               m_isParasited;
    bool               m_isSelected;
    bool               m_isStartingAttack;
    bool               m_isStuck;
    bool               m_isTraining;
    bool               m_isUnderAttack;
    bool               m_isUnderDarkSwarm;
    bool               m_isUnderDisruptionWeb;
    bool               m_isUnderStorm;
    bool               m_isPowered;
    bool               m_isTargetable;

public:
    // Create a new shadow unit from the real unit. If the real unit is not visible, the
    // shadow unit's fields will be set to the fields provided by the real unit anyway.
    ShadowUnitImpl(bw::Unit real) : m_real(real) {
        forceUpdate();
    }

    // Returns the real unit associated with this shadow unit.
    bw::Unit getRealUnit() const {
        return m_real;
    }

    // If the real unit is currently visible, this will cause the shadow unit's fields to
    // be updated accordingly. Otherwise, this method does nothing.
    void updateFields() {
        if (m_real->exists()) {
            forceUpdate();
        }
    }

    // These methods always work as intended on normal units, so they just call the
    // corresponding method on the real unit.
    virtual int                getID()                    const override { return m_real->getID(); }
    virtual bool               exists()                   const override { return m_real->exists(); }
    virtual int                getReplayID()              const override { return m_real->getReplayID(); }
    virtual bw::UnitType       getInitialType()           const override { return m_real->getInitialType(); }
    virtual bw::Position       getInitialPosition()       const override { return m_real->getInitialPosition(); }
    virtual bw::TilePosition   getInitialTilePosition()   const override { return m_real->getInitialTilePosition(); }
    virtual int                getInitialHitPoints()      const override { return m_real->getInitialHitPoints(); }
    virtual int                getInitialResources()      const override { return m_real->getInitialResources(); }
    virtual bool isVisible(bw::Player player = nullptr)   const override { return m_real->isVisible(player); }

    // These methods return each corresponding saved field stored in the shadow unit.
    virtual bw::Player         getPlayer()                const override { return m_player; }
    virtual bw::UnitType       getType()                  const override { return m_type; }
    virtual bw::Position       getPosition()              const override { return m_position; }
    virtual double             getAngle()                 const override { return m_angle; }
    virtual double             getVelocityX()             const override { return m_velocityX; }
    virtual double             getVelocityY()             const override { return m_velocityX; }
    virtual int                getHitPoints()             const override { return m_hitPoints; }
    virtual int                getShields()               const override { return m_shields; }
    virtual int                getEnergy()                const override { return m_energy; }
    virtual int                getResources()             const override { return m_resources; }
    virtual int                getResourceGroup()         const override { return m_resourceGroup; }
    virtual int                getLastCommandFrame()      const override { return m_lastCommandFrame; }
    virtual bw::UnitCommand    getLastCommand()           const override { return m_lastCommand; }
    virtual bw::Player         getLastAttackingPlayer()   const override { return m_lastAttackingPlayer; }
    virtual int                getKillCount()             const override { return m_killCount; }
    virtual int                getAcidSporeCount()        const override { return m_acidSporeCount; }
    virtual int                getInterceptorCount()      const override { return m_interceptorCount; }
    virtual int                getScarabCount()           const override { return m_scarabCount; }
    virtual int                getSpiderMineCount()       const override { return m_spiderMineCount; }
    virtual int                getGroundWeaponCooldown()  const override { return m_groundWeaponCooldown; }
    virtual int                getAirWeaponCooldown()     const override { return m_airWeaponCooldown; }
    virtual int                getSpellCooldown()         const override { return m_spellCooldown; }
    virtual int                getDefenseMatrixPoints()   const override { return m_defenseMatrixPoints; }
    virtual int                getDefenseMatrixTimer()    const override { return m_defenseMatrixTimer; }
    virtual int                getEnsnareTimer()          const override { return m_ensnareTimer; }
    virtual int                getIrradiateTimer()        const override { return m_irradiateTimer; }
    virtual int                getLockdownTimer()         const override { return m_lockdownTimer; }
    virtual int                getMaelstromTimer()        const override { return m_maelstromTimer; }
    virtual int                getOrderTimer()            const override { return m_orderTimer; }
    virtual int                getPlagueTimer()           const override { return m_plagueTimer; }
    virtual int                getRemoveTimer()           const override { return m_removeTimer; }
    virtual int                getStasisTimer()           const override { return m_stasisTimer; }
    virtual int                getStimTimer()             const override { return m_stimTimer; }
    virtual bw::UnitType       getBuildType()             const override { return m_buildType; }
    virtual bw::UnitType::list getTrainingQueue()         const override { return m_trainingQueue; }
    virtual bw::TechType       getTech()                  const override { return m_tech; }
    virtual bw::UpgradeType    getUpgrade()               const override { return m_upgrade; }
    virtual int                getRemainingBuildTime()    const override { return m_remainingBuildTime; }
    virtual int                getRemainingTrainTime()    const override { return m_remainingTrainTime; }
    virtual int                getRemainingResearchTime() const override { return m_remainingResearchTime; }
    virtual int                getRemainingUpgradeTime()  const override { return m_remainingUpgradeTime; }
    virtual bw::Unit           getBuildUnit()             const override { return m_buildUnit; }
    virtual bw::Unit           getTarget()                const override { return m_target; }
    virtual bw::Position       getTargetPosition()        const override { return m_targetPosition; }
    virtual bw::Order          getOrder()                 const override { return m_order; }
    virtual bw::Order          getSecondaryOrder()        const override { return m_secondaryOrder; }
    virtual bw::Unit           getOrderTarget()           const override { return m_orderTarget; }
    virtual bw::Position       getOrderTargetPosition()   const override { return m_orderTargetPosition; }
    virtual bw::Position       getRallyPosition()         const override { return m_rallyPosition; }
    virtual bw::Unit           getRallyUnit()             const override { return m_rallyUnit; }
    virtual bw::Unit           getAddon()                 const override { return m_addon; }
    virtual bw::Unit           getNydusExit()             const override { return m_nydusExit; }
    virtual bw::Unit           getPowerUp()               const override { return m_powerUp; }
    virtual bw::Unit           getTransport()             const override { return m_transport; }
    virtual bw::Unitset        getLoadedUnits()           const override { return m_loadedUnits; }
    virtual bw::Unit           getCarrier()               const override { return m_carrier; }
    virtual bw::Unitset        getInterceptors()          const override { return m_interceptors; }
    virtual bw::Unit           getHatchery()              const override { return m_hatchery; }
    virtual bw::Unitset        getLarva()                 const override { return m_larva; }
    virtual bool               hasNuke()                  const override { return m_hasNuke; }
    virtual bool               isAccelerating()           const override { return m_isAccelerating; }
    virtual bool               isAttacking()              const override { return m_isAttacking; }
    virtual bool               isAttackFrame()            const override { return m_isAttackFrame; }
    virtual bool               isBeingGathered()          const override { return m_isBeingGathered; }
    virtual bool               isBeingHealed()            const override { return m_isBeingHealed; }
    virtual bool               isBlind()                  const override { return m_isBlind; }
    virtual bool               isBraking()                const override { return m_isBraking; }
    virtual bool               isBurrowed()               const override { return m_isBurrowed; }
    virtual bool               isCarryingGas()            const override { return m_isCarryingGas; }
    virtual bool               isCarryingMinerals()       const override { return m_isCarryingMinerals; }
    virtual bool               isCloaked()                const override { return m_isCloaked; }
    virtual bool               isCompleted()              const override { return m_isCompleted; }
    virtual bool               isConstructing()           const override { return m_isConstructing; }
    virtual bool               isDetected()               const override { return m_isDetected; }
    virtual bool               isGatheringGas()           const override { return m_isGatheringGas; }
    virtual bool               isGatheringMinerals()      const override { return m_isGatheringMinerals; }
    virtual bool               isHallucination()          const override { return m_isHallucination; }
    virtual bool               isIdle()                   const override { return m_isIdle; }
    virtual bool               isInterruptible()          const override { return m_isInterruptible; }
    virtual bool               isInvincible()             const override { return m_isInvincible; }
    virtual bool               isLifted()                 const override { return m_isLifted; }
    virtual bool               isMorphing()               const override { return m_isMorphing; }
    virtual bool               isMoving()                 const override { return m_isMoving; }
    virtual bool               isParasited()              const override { return m_isParasited; }
    virtual bool               isSelected()               const override { return m_isSelected; }
    virtual bool               isStartingAttack()         const override { return m_isStartingAttack; }
    virtual bool               isStuck()                  const override { return m_isStuck; }
    virtual bool               isTraining()               const override { return m_isTraining; }
    virtual bool               isUnderAttack()            const override { return m_isUnderAttack; }
    virtual bool               isUnderDarkSwarm()         const override { return m_isUnderDarkSwarm; }
    virtual bool               isUnderDisruptionWeb()     const override { return m_isUnderDisruptionWeb; }
    virtual bool               isUnderStorm()             const override { return m_isUnderStorm; }
    virtual bool               isPowered()                const override { return m_isPowered; }
    virtual bool               isTargetable()             const override { return m_isTargetable; }

    // These methods can be used to set each saved field stored in the shadow unit.
    void setPlayer               (bw::Player         player)                 { m_player                = player; }
    void setType                 (bw::UnitType       type)                   { m_type                  = type; }
    void setPosition             (bw::Position       position)               { m_position              = position; }
    void setAngle                (double             angle)                  { m_angle                 = angle; }
    void setVelocityX            (double             velocityX)              { m_velocityX             = velocityX; }
    void setVelocityY            (double             velocityY)              { m_velocityY             = velocityY; }
    void setHitPoints            (int                hitPoints)              { m_hitPoints             = hitPoints; }
    void setShields              (int                shields)                { m_shields               = shields; }
    void setEnergy               (int                energy)                 { m_energy                = energy; }
    void setResources            (int                resources)              { m_resources             = resources; }
    void setResourceGroup        (int                resourceGroup)          { m_resourceGroup         = resourceGroup; }
    void setLastCommandFrame     (int                lastCommandFrame)       { m_lastCommandFrame      = lastCommandFrame; }
    void setLastCommand          (bw::UnitCommand    lastCommand)            { m_lastCommand           = lastCommand; }
    void setLastAttackingPlayer  (bw::Player         lastAttackingPlayer)    { m_lastAttackingPlayer   = lastAttackingPlayer; }
    void setKillCount            (int                killCount)              { m_killCount             = killCount; }
    void setAcidSporeCount       (int                acidSporeCount)         { m_acidSporeCount        = acidSporeCount; }
    void setInterceptorCount     (int                interceptorCount)       { m_interceptorCount      = interceptorCount; }
    void setScarabCount          (int                scarabCount)            { m_scarabCount           = scarabCount; }
    void setSpiderMineCount      (int                spiderMineCount)        { m_spiderMineCount       = spiderMineCount; }
    void setGroundWeaponCooldown (int                groundWeaponCooldown)   { m_groundWeaponCooldown  = groundWeaponCooldown; }
    void setAirWeaponCooldown    (int                airWeaponCooldown)      { m_airWeaponCooldown     = airWeaponCooldown; }
    void setSpellCooldown        (int                spellCooldown)          { m_spellCooldown         = spellCooldown; }
    void setDefenseMatrixPoints  (int                defenseMatrixPoints)    { m_defenseMatrixPoints   = defenseMatrixPoints; }
    void setDefenseMatrixTimer   (int                defenseMatrixTimer)     { m_defenseMatrixTimer    = defenseMatrixTimer; }
    void setEnsnareTimer         (int                ensnareTimer)           { m_ensnareTimer          = ensnareTimer; }
    void setIrradiateTimer       (int                irradiateTimer)         { m_irradiateTimer        = irradiateTimer; }
    void setLockdownTimer        (int                lockdownTimer)          { m_lockdownTimer         = lockdownTimer; }
    void setMaelstromTimer       (int                maelstromTimer)         { m_maelstromTimer        = maelstromTimer; }
    void setOrderTimer           (int                orderTimer)             { m_orderTimer            = orderTimer; }
    void setPlagueTimer          (int                plagueTimer)            { m_plagueTimer           = plagueTimer; }
    void setRemoveTimer          (int                removeTimer)            { m_removeTimer           = removeTimer; }
    void setStasisTimer          (int                stasisTimer)            { m_stasisTimer           = stasisTimer; }
    void setStimTimer            (int                stimTimer)              { m_stimTimer             = stimTimer; }
    void setBuildType            (bw::UnitType       buildType)              { m_buildType             = buildType; }
    void setTrainingQueue        (bw::UnitType::list trainingQueue)          { m_trainingQueue         = std::move(trainingQueue); }
    void setTech                 (bw::TechType       tech)                   { m_tech                  = tech; }
    void setUpgrade              (bw::UpgradeType    upgrade)                { m_upgrade               = upgrade; }
    void setRemainingBuildTime   (int                remainingBuildTime)     { m_remainingBuildTime    = remainingBuildTime; }
    void setRemainingTrainTime   (int                remainingTrainTime)     { m_remainingTrainTime    = remainingTrainTime; }
    void setRemainingResearchTime(int                remainingResearchTime)  { m_remainingResearchTime = remainingResearchTime; }
    void setRemainingUpgradeTime (int                remainingUpgradeTime)   { m_remainingUpgradeTime  = remainingUpgradeTime; }
    void setBuildUnit            (bw::Unit           buildUnit)              { m_buildUnit             = buildUnit; }
    void setTarget               (bw::Unit           target)                 { m_target                = target; }
    void setTargetPosition       (bw::Position       targetPosition)         { m_targetPosition        = targetPosition; }
    void setOrder                (bw::Order          order)                  { m_order                 = order; }
    void setSecondaryOrder       (bw::Order          secondaryOrder)         { m_secondaryOrder        = secondaryOrder; }
    void setOrderTarget          (bw::Unit           orderTarget)            { m_orderTarget           = orderTarget; }
    void setOrderTargetPosition  (bw::Position       orderTargetPosition)    { m_orderTargetPosition   = orderTargetPosition; }
    void setRallyPosition        (bw::Position       rallyPosition)          { m_rallyPosition         = rallyPosition; }
    void setRallyUnit            (bw::Unit           rallyUnit)              { m_rallyUnit             = rallyUnit; }
    void setAddon                (bw::Unit           addon)                  { m_addon                 = addon; }
    void setNydusExit            (bw::Unit           nydusExit)              { m_nydusExit             = nydusExit; }
    void setPowerUp              (bw::Unit           powerUp)                { m_powerUp               = powerUp; }
    void setTransport            (bw::Unit           transport)              { m_transport             = transport; }
    void setLoadedUnits          (bw::Unitset        loadedUnits)            { m_loadedUnits           = loadedUnits; }
    void setCarrier              (bw::Unit           carrier)                { m_carrier               = carrier; }
    void setInterceptors         (bw::Unitset        interceptors)           { m_interceptors          = interceptors; }
    void setHatchery             (bw::Unit           hatchery)               { m_hatchery              = hatchery; }
    void setLarva                (bw::Unitset        larva)                  { m_larva                 = larva; }
    void setHasNuke              (bool               hasNuke)                { m_hasNuke               = hasNuke; }
    void setIsAccelerating       (bool               isAccelerating)         { m_isAccelerating        = isAccelerating; }
    void setIsAttacking          (bool               isAttacking)            { m_isAttacking           = isAttacking; }
    void setIsAttackFrame        (bool               isAttackFrame)          { m_isAttackFrame         = isAttackFrame; }
    void setIsBeingGathered      (bool               isBeingGathered)        { m_isBeingGathered       = isBeingGathered; }
    void setIsBeingHealed        (bool               isBeingHealed)          { m_isBeingHealed         = isBeingHealed; }
    void setIsBlind              (bool               isBlind)                { m_isBlind               = isBlind; }
    void setIsBraking            (bool               isBraking)              { m_isBraking             = isBraking; }
    void setIsBurrowed           (bool               isBurrowed)             { m_isBurrowed            = isBurrowed; }
    void setIsCarryingGas        (bool               isCarryingGas)          { m_isCarryingGas         = isCarryingGas; }
    void setIsCarryingMinerals   (bool               isCarryingMinerals)     { m_isCarryingMinerals    = isCarryingMinerals; }
    void setIsCloaked            (bool               isCloaked)              { m_isCloaked             = isCloaked; }
    void setIsCompleted          (bool               isCompleted)            { m_isCompleted           = isCompleted; }
    void setIsConstructing       (bool               isConstructing)         { m_isConstructing        = isConstructing; }
    void setIsDetected           (bool               isDetected)             { m_isDetected            = isDetected; }
    void setIsGatheringGas       (bool               isGatheringGas)         { m_isGatheringGas        = isGatheringGas; }
    void setIsGatheringMinerals  (bool               isGatheringMinerals)    { m_isGatheringMinerals   = isGatheringMinerals; }
    void setIsHallucination      (bool               isHallucination)        { m_isHallucination       = isHallucination; }
    void setIsIdle               (bool               isIdle)                 { m_isIdle                = isIdle; }
    void setIsInterruptible      (bool               isInterruptible)        { m_isInterruptible       = isInterruptible; }
    void setIsInvincible         (bool               isInvincible)           { m_isInvincible          = isInvincible; }
    void setIsLifted             (bool               isLifted)               { m_isLifted              = isLifted; }
    void setIsMorphing           (bool               isMorphing)             { m_isMorphing            = isMorphing; }
    void setIsMoving             (bool               isMoving)               { m_isMoving              = isMoving; }
    void setIsParasited          (bool               isParasited)            { m_isParasited           = isParasited; }
    void setIsSelected           (bool               isSelected)             { m_isSelected            = isSelected; }
    void setIsStartingAttack     (bool               isStartingAttack)       { m_isStartingAttack      = isStartingAttack; }
    void setIsStuck              (bool               isStuck)                { m_isStuck               = isStuck; }
    void setIsTraining           (bool               isTraining)             { m_isTraining            = isTraining; }
    void setIsUnderAttack        (bool               isUnderAttack)          { m_isUnderAttack         = isUnderAttack; }
    void setIsUnderDarkSwarm     (bool               isUnderDarkSwarm)       { m_isUnderDarkSwarm      = isUnderDarkSwarm; }
    void setIsUnderDisruptionWeb (bool               isUnderDisruptionWeb)   { m_isUnderDisruptionWeb  = isUnderDisruptionWeb; }
    void setIsUnderStorm         (bool               isUnderStorm)           { m_isUnderStorm          = isUnderStorm; }
    void setIsPowered            (bool               isPowered)              { m_isPowered             = isPowered; }
    void setIsTargetable         (bool               isTargetable)           { m_isTargetable          = isTargetable; }

    // All the methods having to do with commanding and targeting units do nothing.
    virtual bool issueCommand(bw::UnitCommand command) override { return false; }
    virtual bool canIssueCommand(bw::UnitCommand command, bool checkCanUseTechPositionOnPositions = true, bool checkCanUseTechUnitOnUnits = true, bool checkCanBuildUnitType = true, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canIssueCommandGrouped(bw::UnitCommand command, bool checkCanUseTechPositionOnPositions = true, bool checkCanUseTechUnitOnUnits = true, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canCommand() const override { return false; }
    virtual bool canCommandGrouped(bool checkCommandibility = true) const override { return false; }
    virtual bool canIssueCommandType(bw::UnitCommandType ct, bool checkCommandibility = true) const override { return false; }
    virtual bool canIssueCommandTypeGrouped(bw::UnitCommandType ct, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canTargetUnit(bw::Unit targetUnit, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttack(bool checkCommandibility = true) const override { return false; }
    virtual bool canAttack(bw::Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttack(bw::Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackGrouped(bw::Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackGrouped(bw::Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackMove(bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackMoveGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackUnit(bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackUnit(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackUnitGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canAttackUnitGrouped(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canBuild(bool checkCommandibility = true) const override { return false; }
    virtual bool canBuild(bw::UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canBuild(bw::UnitType uType, bw::TilePosition tilePos, bool checkTargetUnitType = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canBuildAddon(bool checkCommandibility = true) const override { return false; }
    virtual bool canBuildAddon(bw::UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canTrain(bool checkCommandibility = true) const override { return false; }
    virtual bool canTrain(bw::UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canMorph(bool checkCommandibility = true) const override { return false; }
    virtual bool canMorph(bw::UnitType uType, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canResearch(bool checkCommandibility = true) const override { return false; }
    virtual bool canResearch(bw::TechType type, bool checkCanIssueCommandType = true) const override { return false; }
    virtual bool canUpgrade(bool checkCommandibility = true) const override { return false; }
    virtual bool canUpgrade(bw::UpgradeType type, bool checkCanIssueCommandType = true) const override { return false; }
    virtual bool canSetRallyPoint(bool checkCommandibility = true) const override { return false; }
    virtual bool canSetRallyPoint(bw::Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canSetRallyPoint(bw::Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canSetRallyPosition(bool checkCommandibility = true) const override { return false; }
    virtual bool canSetRallyUnit(bool checkCommandibility = true) const override { return false; }
    virtual bool canSetRallyUnit(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canMove(bool checkCommandibility = true) const override { return false; }
    virtual bool canMoveGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canPatrol(bool checkCommandibility = true) const override { return false; }
    virtual bool canPatrolGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canFollow(bool checkCommandibility = true) const override { return false; }
    virtual bool canFollow(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canGather(bool checkCommandibility = true) const override { return false; }
    virtual bool canGather(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canReturnCargo(bool checkCommandibility = true) const override { return false; }
    virtual bool canHoldPosition(bool checkCommandibility = true) const override { return false; }
    virtual bool canStop(bool checkCommandibility = true) const override { return false; }
    virtual bool canRepair(bool checkCommandibility = true) const override { return false; }
    virtual bool canRepair(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canBurrow(bool checkCommandibility = true) const override { return false; }
    virtual bool canUnburrow(bool checkCommandibility = true) const override { return false; }
    virtual bool canCloak(bool checkCommandibility = true) const override { return false; }
    virtual bool canDecloak(bool checkCommandibility = true) const override { return false; }
    virtual bool canSiege(bool checkCommandibility = true) const override { return false; }
    virtual bool canUnsiege(bool checkCommandibility = true) const override { return false; }
    virtual bool canLift(bool checkCommandibility = true) const override { return false; }
    virtual bool canLand(bool checkCommandibility = true) const override { return false; }
    virtual bool canLand(bw::TilePosition target, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canLoad(bool checkCommandibility = true) const override { return false; }
    virtual bool canLoad(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUnloadWithOrWithoutTarget(bool checkCommandibility = true) const override { return false; }
    virtual bool canUnloadAtPosition(bw::Position targDropPos, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUnload(bool checkCommandibility = true) const override { return false; }
    virtual bool canUnload(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkPosition = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUnloadAll(bool checkCommandibility = true) const override { return false; }
    virtual bool canUnloadAllPosition(bool checkCommandibility = true) const override { return false; }
    virtual bool canUnloadAllPosition(bw::Position targDropPos, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClick(bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClick(bw::Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClick(bw::Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickGrouped(bw::Position target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickGrouped(bw::Unit target, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickPosition(bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickPositionGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickUnit(bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickUnit(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickUnitGrouped(bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canRightClickUnitGrouped(bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkCanIssueCommandType = true, bool checkCommandibilityGrouped = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canHaltConstruction(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelConstruction(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelAddon(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelTrain(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelTrainSlot(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelTrainSlot(int slot, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelMorph(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelResearch(bool checkCommandibility = true) const override { return false; }
    virtual bool canCancelUpgrade(bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechWithOrWithoutTarget(bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechWithOrWithoutTarget(bw::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTech(bw::TechType tech, bw::Position target, bool checkCanTargetUnit = true, bool checkTargetsType = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTech(bw::TechType tech, bw::Unit target = nullptr, bool checkCanTargetUnit = true, bool checkTargetsType = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechWithoutTarget(bw::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechUnit(bw::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechUnit(bw::TechType tech, bw::Unit targetUnit, bool checkCanTargetUnit = true, bool checkTargetsUnits = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechPosition(bw::TechType tech, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canUseTechPosition(bw::TechType tech, bw::Position target, bool checkTargetsPositions = true, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }
    virtual bool canPlaceCOP(bool checkCommandibility = true) const override { return false; }
    virtual bool canPlaceCOP(bw::TilePosition target, bool checkCanIssueCommandType = true, bool checkCommandibility = true) const override { return false; }

private:
    // This method updates all the shadow unit's saved fields from the real unit,
    // regardless of whether the real unit is visible.
    void forceUpdate() {
        m_player                = m_real->getPlayer();
        m_type                  = m_real->getType();
        m_position              = m_real->getPosition();
        m_angle                 = m_real->getAngle();
        m_velocityX             = m_real->getVelocityX();
        m_velocityY             = m_real->getVelocityY();
        m_hitPoints             = m_real->getHitPoints();
        m_shields               = m_real->getShields();
        m_energy                = m_real->getEnergy();
        m_resources             = m_real->getResources();
        m_resourceGroup         = m_real->getResourceGroup();
        m_lastCommandFrame      = m_real->getLastCommandFrame();
        m_lastCommand           = m_real->getLastCommand();
        m_lastAttackingPlayer   = m_real->getLastAttackingPlayer();
        m_killCount             = m_real->getKillCount();
        m_acidSporeCount        = m_real->getAcidSporeCount();
        m_interceptorCount      = m_real->getInterceptorCount();
        m_scarabCount           = m_real->getScarabCount();
        m_spiderMineCount       = m_real->getSpiderMineCount();
        m_groundWeaponCooldown  = m_real->getGroundWeaponCooldown();
        m_airWeaponCooldown     = m_real->getAirWeaponCooldown();
        m_spellCooldown         = m_real->getSpellCooldown();
        m_defenseMatrixPoints   = m_real->getDefenseMatrixPoints();
        m_defenseMatrixTimer    = m_real->getDefenseMatrixTimer();
        m_ensnareTimer          = m_real->getEnsnareTimer();
        m_irradiateTimer        = m_real->getIrradiateTimer();
        m_lockdownTimer         = m_real->getLockdownTimer();
        m_maelstromTimer        = m_real->getMaelstromTimer();
        m_orderTimer            = m_real->getOrderTimer();
        m_plagueTimer           = m_real->getPlagueTimer();
        m_removeTimer           = m_real->getRemoveTimer();
        m_stasisTimer           = m_real->getStasisTimer();
        m_stimTimer             = m_real->getStimTimer();
        m_buildType             = m_real->getBuildType();
        m_trainingQueue         = m_real->getTrainingQueue();
        m_tech                  = m_real->getTech();
        m_upgrade               = m_real->getUpgrade();
        m_remainingBuildTime    = m_real->getRemainingBuildTime();
        m_remainingTrainTime    = m_real->getRemainingTrainTime();
        m_remainingResearchTime = m_real->getRemainingResearchTime();
        m_remainingUpgradeTime  = m_real->getRemainingUpgradeTime();
        m_buildUnit             = m_real->getBuildUnit();
        m_target                = m_real->getTarget();
        m_targetPosition        = m_real->getTargetPosition();
        m_order                 = m_real->getOrder();
        m_secondaryOrder        = m_real->getSecondaryOrder();
        m_orderTarget           = m_real->getOrderTarget();
        m_orderTargetPosition   = m_real->getOrderTargetPosition();
        m_rallyPosition         = m_real->getRallyPosition();
        m_rallyUnit             = m_real->getRallyUnit();
        m_addon                 = m_real->getAddon();
        m_nydusExit             = m_real->getNydusExit();
        m_powerUp               = m_real->getPowerUp();
        m_transport             = m_real->getTransport();
        m_loadedUnits           = m_real->getLoadedUnits();
        m_carrier               = m_real->getCarrier();
        m_interceptors          = m_real->getInterceptors();
        m_hatchery              = m_real->getHatchery();
        m_larva                 = m_real->getLarva();
        m_hasNuke               = m_real->hasNuke();
        m_isAccelerating        = m_real->isAccelerating();
        m_isAttacking           = m_real->isAttacking();
        m_isAttackFrame         = m_real->isAttackFrame();
        m_isBeingGathered       = m_real->isBeingGathered();
        m_isBeingHealed         = m_real->isBeingHealed();
        m_isBlind               = m_real->isBlind();
        m_isBraking             = m_real->isBraking();
        m_isBurrowed            = m_real->isBurrowed();
        m_isCarryingGas         = m_real->isCarryingGas();
        m_isCarryingMinerals    = m_real->isCarryingMinerals();
        m_isCloaked             = m_real->isCloaked();
        m_isCompleted           = m_real->isCompleted();
        m_isConstructing        = m_real->isConstructing();
        m_isDetected            = m_real->isDetected();
        m_isGatheringGas        = m_real->isGatheringGas();
        m_isGatheringMinerals   = m_real->isGatheringMinerals();
        m_isHallucination       = m_real->isHallucination();
        m_isIdle                = m_real->isIdle();
        m_isInterruptible       = m_real->isInterruptible();
        m_isInvincible          = m_real->isInvincible();
        m_isLifted              = m_real->isLifted();
        m_isMorphing            = m_real->isMorphing();
        m_isMoving              = m_real->isMoving();
        m_isParasited           = m_real->isParasited();
        m_isSelected            = m_real->isSelected();
        m_isStartingAttack      = m_real->isStartingAttack();
        m_isStuck               = m_real->isStuck();
        m_isTraining            = m_real->isTraining();
        m_isUnderAttack         = m_real->isUnderAttack();
        m_isUnderDarkSwarm      = m_real->isUnderDarkSwarm();
        m_isUnderDisruptionWeb  = m_real->isUnderDisruptionWeb();
        m_isUnderStorm          = m_real->isUnderStorm();
        m_isPowered             = m_real->isPowered();
        m_isTargetable          = m_real->isTargetable();
    }
};

using ShadowUnit = ShadowUnitImpl*;