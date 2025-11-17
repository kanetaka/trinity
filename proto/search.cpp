#include "search.h"

bool Bfs(const Graph& graph, const GraphNode* start, const GraphNode* goal, NodeToParentMap& out_map)
{
    bool found_path = false;
    std::queue<const GraphNode*> q;
    q.emplace(start);

    while (!q.empty()) {
        auto current = q.front();
        q.pop();
        if (current == goal) {
            found_path = true;
            break;
        }

        // キューに入っていない隣接ノードをエンキュー
        for (const GraphNode* node : current->adjacent_) {
            const GraphNode* parent = out_map[node];
            if (parent == nullptr && node != start) {
                out_map[node] = current;
                q.emplace(node);
            }
        }
    }

    return found_path;
}

bool Gbfs(const WeightedGraph& g, const WeightedGraphNode* start, const WeightedGraphNode* goal, GbfsMap& out_map)
{
    std::vector<const WeightedGraphNode*> open_set;

    // カレントノードに始点をセットし、クローズセットに入れる
    const WeightedGraphNode* current = start;
    out_map[current].in_closedset_ = true;

    do {
        // 隣接ノードをオープンセットに追加する
        for (const auto* edge : current->edges_) {
            // ノードのスクラッチデータを取得する
            auto& data = out_map[edge->to_];

            // クローズセットにない場合は追加する
            if (!data.in_closedset_) {
                // 隣接ノードの親エッジを設定する
                data.parent_edge_ = edge;
                if (!data.in_openset_) {
                    // ヒューリスティックを計算してオープンセットに追加する
                    data.heuristic_ = ComputeHeuristic(edge->to_, goal);
                    data.in_openset_ = true;
                    open_set.emplace_back(edge->to_);
                }
            }
        }

        if (open_set.empty()) {
            break;
        }

        auto iter = std::min_element(open_set.begin(), open_set.end(),
            [&out_map](const WeightedGraphNode* a, const WeightedGraphNode* b) {
                return out_map[a].heuristic_ < out_map[b].heuristic_;
            });

        current = *iter;
        open_set.erase(iter);
        out_map[current].in_openset_ = false;
        out_map[current].in_closedset_ = true;
    } while (current != goal);

    return (current == goal) ? true : false;
}
