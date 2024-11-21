#pragma once

#include "Tools.h"

// Takes a set of units and groups them into clusters of units that are close together,
// using a modified non-stochastic version of the k-means++ algorithm.
std::vector<bw::Unitset> findUnitClusters(int count, const bw::Unitset& units);

// Represents a list of attack commands, indicating who was attacked by whom.
using AttackList = std::vector<std::pair<bw::Unit, bw::Unit>>;

// Represents the basic state of a single unit. We need a way to represent the state
// of a unit in such a way that we aren't dependent on the unit being currently
// visible (since bw::Unit tells us nothing when the unit is invisible), as well as a
// data structure that we can copy and modify in a simulation.
struct UnitState {
	// A pointer to the unit in question. Even though we can't use the unit's
	// properties if it's not invisible, we need a unique handle to differentiate this
	// UnitState from any other.
	bw::Unit unit;

	// The type and last known position of this unit.
	bw::UnitType type;
	bw::Position position;

	// The current amount of health and shields that this unit has.
	int health;
	int shields;

	UnitState(bw::Unit unit) :
		unit(unit),
		type(unit->getType()),
		position(unit->getPosition()),
		health(unit->getHitPoints()),
		shields(unit->getShields()) {
	}
};

// Contains the game state for a single player, which starts out with the player's
// actual state and is copied and modified as the simulation progresses.
struct PlayerState {
	// A list of all the units that the player currently has.
	std::vector<UnitState> units;

	// Contains the list of attacks that the player made to get to this current state.
	AttackList attacks;

	// The "goodness" of this player's situation normalized to the initial goodness of
	// zero at the beginning of the simulation.
	double goodness = 0.0;

	PlayerState(const bw::Unitset& initial) {
		for (bw::Unit unit : initial) {
			units.push_back(UnitState(unit));
		}
	}
};

AttackList runCombatSimulation(const bw::Unitset& selfUnits, const bw::Unitset& enemyUnits);