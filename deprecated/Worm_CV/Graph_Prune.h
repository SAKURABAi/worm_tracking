#pragma once
#include"Graph.h"
#include"Graph_Structure.h"

class Graph_Prune {
private:
	const Graph * graph;
	Graph_Structure * graph_structure;
	bool * node_available;
	int node_num;
	int structure_node_num;
	const Graph_Structure_Node * structure_node_list;

	bool Same_Direction(const std::vector<int> & cline, const Backbone & last_backbone);
	bool Same_Clockwise(const std::vector<int> & cline,	const Backbone& last_backbone, int start_2, int end_2);

	void Get_Largest_Subgraph();
	void Rotate_To_Next(int & last_node, int & current_node) const;
	void Start_Node_Locate(int & first_node, int & second_node) const;
	void Graph_Structure_Analyze(int & first_node, int & second_node);

	void Delete_Short_Route();
	bool Delete_Shorter_Routes_With_Same_End();
	bool Delete_Branch_And_Loopback_Except_For_Two_Longest();
	void Structure_Node_Statistic(int& special_node_num, int& loopback_count, const Graph_Structure_Node* special_node[2]);
	void Delete_Smaller_Loopback(int& bifurcate_node_num, const Graph_Structure_Node* special_node[2]);
	void Connect_Correct_Loopback_To_Route(Backbone& last_backbone, double worm_full_width,
		const Graph_Structure_Node* special_node[2], std::vector<int> & route);

public:
	void Prune(const Graph * graph_before_prune, Backbone & last_backbone, double worm_full_width, bool is_first_pic);
};

