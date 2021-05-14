#include "pathfinding.hpp"

#include <queue>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>

/**
 * Checks whether the given coordinates are within bounds for the given world
 */
bool coord_in_bounds(const World& world, int x, int y) {
	return y >= 0 && y < world.height;
}

/**
 * Calculates the neighbors for a given Tile. The neighbors are the 8 tiles around
 * it, while taking into account the map bounds.
 */
std::vector<Tile *> generate_neighbors(const World& world, Tile * tile) {
	std::vector<Tile *> result;
	
	for(size_t i = -1; i <= 1; i++) {
		for(size_t j = -1; j <= 1; j++) {
			// Skip middle
			if(i == 0 && j == 0)
				continue;
			
			if(coord_in_bounds(world, tile->x + i, tile->y + j)) {
				// Wrap x east to west
				size_t x = (tile->x + i);
				
				// Adjust x, allow wrapping
				if(x < 0) {
					x = world.width + x;
				} else if(x >= world.width) {
					x %= world.width;
				}
				
				size_t index = x + (tile->y + j) * world.width;
				Tile * neighbour = &world.tiles[index];
				if(neighbour->elevation > world.sea_level) {
					result.push_back(neighbour);
				}
			}
		}
	}
	return result;
}

/**
 * Calculates the euclidean distance between the two tiles
 * considering only x and y (ignoring elevation)
 */
float euclidean_distance(Tile * t1, Tile * t2) {
	return std::hypot(t1->x - t2->x, t1->y - t2->y);
}

/**
 * Calculates the cost accrued by moving from one tile to another, taking into
 * account elevation and infrastructure.
 */
float tile_cost(Tile * t1, Tile * t2) {
	int x_diff = t1->x - t2->x;
	int y_diff = t1->y - t2->y;
	
	// Maximum elevation difference accounts to same cost as one jump in x or y direction (1.0)
	float elev_diff = (int)t1->elevation - (int)t2->elevation / 128.f;
	
	// Base distance is euclidean distance in x, y and elevation
	float distance = std::sqrt(x_diff * x_diff + y_diff * y_diff + elev_diff * elev_diff);
	
	// Calculate average infrastructure level between the two tiles
	float avg_infra = ((t1->infra_level + t2->infra_level) / 2.f) + 1.f;
	
	// Cost modifier from infrastructure scales linearly with infrastructure
	// with 1.0 cost modifier at max infra and 5.0 modifier at 0 infra
	// NOTE: Make sure that the infrastructure modifier is always larger than 1 for bad infra
	// (rather than <1 for good infra), or the heuristic will no longer be admissible
	// and A* will no longer be optimal
	float infra_modifier = 1.f + 4.f * (1 - avg_infra / 8.f); 
	
	// Rivers double the cost
	float river_modifier = (t1->elevation == 127 || t2->elevation == 127) ? 2.f : 1.f;
	
	return river_modifier * infra_modifier * distance;
}

std::vector<Tile *> find_path(const World& world, Tile * start, Tile * end) {
	if (start->elevation <= world.sea_level && end->elevation <= world.sea_level) {
		return std::vector<Tile *>();
	}
	
	// Keeps track of the costs so far
	std::unordered_map<Tile *, float> cost_map;
	
	// Keeps track of the previous tile for each tile
	std::unordered_map<Tile *, Tile *> prev_map;
	
	// Keeps track of which tiles have been visited
	std::unordered_set<Tile *> visited;
	
	// Priority queue based on cost
	std::priority_queue<std::pair<float, Tile *>> queue;
	
	cost_map[start] = 0.f;
	queue.push({0.f, start});
	
	while(!queue.empty()) {
		Tile * current = queue.top().second;
		queue.pop();
		
		// If the current node has been previously visited, 
		// it's optimal cost has already been calculated, and we can skip it  
		if(visited.count(current))
			continue;
		
		visited.insert(current);
		
		// We are done
		if(current == end)
			break;
		
		for(auto neighbor: generate_neighbors(world, current)) {
			// If the neighbor is visited, we already have the optimal path to it
			if(visited.count(neighbor))
				continue;
			
			float cost = cost_map[current] + euclidean_distance(current, neighbor);
			// If we found a new tile or a shorter path to a previously found tile
			if(!cost_map.count(neighbor) || cost < cost_map[neighbor]) {
				cost_map[neighbor] = cost;
				float priority = cost + euclidean_distance(neighbor, end);
				queue.push({priority, neighbor});
				prev_map[neighbor] = current;
			}
		}
	}
	
	std::vector<Tile *> path;
	
	// Unwind path and reverse
	Tile * current = end;
	while(current != start) {
		path.push_back(current);
		current = prev_map[current];
	}
	
	path.push_back(start);
	std::reverse(path.begin(), path.end());
	return path;
}
