#pragma once
#include "CONST_PARA.h"
#include<vector>

struct Graph_Structure_Node {
	int degree;
	std::vector<int> edges[SKELETONIZE::DEGREE_MAX];
};

class Graph_Structure {
private:
	Graph_Structure_Node *nodes;
	int *node_hash;
	int node_num;
	void Delete_Edge_Oneway(int edge_start, int midway1);
public:
	Graph_Structure(int real_node_num, int structure_node_max);
	~Graph_Structure();
	void Check_Structure();
	void Add_Edge(const std::vector<int> & edge);
	void Delete_Edge(const std::vector<int> & edge);
	const Graph_Structure_Node * Get_Node_List() { return nodes; }
	int Get_Node_Num() { return node_num; }
	bool Move_To_Other_End(int & last_node, int & current_node);
};