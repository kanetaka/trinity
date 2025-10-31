#pragma once

#include <vector>
#include <unordered_map>
#include <queue>

struct GraphNode
{
    std::vector<GraphNode*> adjacent_;
};

struct Graph
{
    std::vector<GraphNode*> nodes_;
};

struct WeightedEdge
{
    struct WeightedGraphNode* from_;
    struct WeightedGraphNode* to_;
    float weight_;
};

struct WeightedGraphNode
{
    std::vector<WeightedEdge*> edges_;
};

struct WeightedGraph
{
    std::vector<WeightedGraphNode*> nodes_;
};

using NodeToParentMap =
std::unordered_map<const GraphNode*, const GraphNode*>;

bool Bfs(const Graph& graph, const GraphNode* start, const GraphNode* goal, NodeToParentMap& out_map);

struct GbfsScratch
{
    const WeightedEdge* parent_edge_ = nullptr;
    float heuristic_ = 0.0f;
    bool in_openset_ = false;
    bool in_closedset_ = false;
};

using GbfsMap =
std::unordered_map<const WeightedGraphNode*, GbfsScratch>;

bool Gbfs(const WeightedGraph& g, const WeightedGraphNode* start, const WeightedGraph* goal, GbfsMap& out_map);

float ComputeHeuristic(const WeightedGraphNode* a, const WeightedGraphNode* b)
{
    return 0.0f;
}
