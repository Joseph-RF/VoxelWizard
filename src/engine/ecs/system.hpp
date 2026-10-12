#pragma once

#include "engine/core/types.hpp"

#include <set>

class System {
public:
	std::set<Entity> m_entities;
};