#pragma once

#include "Graph.h"
#include <stack>

class Graph;
class Subgraph_Count;

struct Bifurcate_Stack{
	int top;
	int parent_node[SKELETONIZE::STORAGE_MAX];
	std::vector<int> item[SKELETONIZE::STORAGE_MAX];

	Bifurcate_Stack():top(0){};
	void Push(const std::vector<int> & in_stack_points, int parent_index);
};

class Graph_Builder{
private:
	std::vector<int> selected_points;
	int * point_mark;
	Bifurcate_Stack stack;
	Graph * graph;
	const Candidate_Points * candidate_points;

	void Search_Unused_Nearby_Points(std::vector<int> & unused_points) const;
	int Search_Further_Point(std::vector<int> & item, int current_node) const;
	void Check_Connectivity(std::vector<int>& selected_points, int parent_node);
	void Search_Next_Points();
	void Connecting_End() const;
public:
	void Convert_To_Graph(const Candidate_Points * candidate_center_points, Graph * skeleton_graph, std::string pic_num_str);
};
