#include "stdafx.h"
#include<algorithm>

using namespace std;

Graph_Structure::Graph_Structure(int real_node_num, int structure_node_max) :node_num(0) {
	nodes = new Graph_Structure_Node[structure_node_max];
	node_hash = new int[real_node_num];
	for (int i = 0; i < real_node_num; ++i)
		node_hash[i] = -1;
	for (int i = 0; i < structure_node_max; ++i)
		nodes[i].degree = 0;
}

Graph_Structure::~Graph_Structure() {
	if (nodes != NULL)
		delete[] nodes;
	if (node_hash != NULL)
		delete[] node_hash;
}

void Graph_Structure::Check_Structure() {
	for (int node_index = 0; node_index < node_num; ++node_index) {
		Graph_Structure_Node & structure_node_0 = nodes[node_index];
		// 度为2的点可删除
		if (structure_node_0.degree != 2)
			continue;
		// 连接当前点相连的两条边
		Graph_Structure_Node * adjacents[2];
		int edge_index[2];
		for (int i = 0; i < 2; ++i) {
			adjacents[i] = nodes + node_hash[structure_node_0.edges[i].at(structure_node_0.edges[i].size() - 1)];
			for (int j = 0; j < adjacents[i]->degree; ++j) {
				if (adjacents[i]->edges[j].at(1) == structure_node_0.edges[i].at(structure_node_0.edges[i].size() - 2))
					edge_index[i] = j;
			}
		}
		adjacents[0]->edges[edge_index[0]].pop_back();
		adjacents[0]->edges[edge_index[0]].insert(adjacents[0]->edges[edge_index[0]].end(),
			structure_node_0.edges[1].begin(), structure_node_0.edges[1].end());
		adjacents[1]->edges[edge_index[1]].clear();
		reverse_copy(adjacents[0]->edges[edge_index[0]].begin(), adjacents[0]->edges[edge_index[0]].end(), 
			back_inserter(adjacents[1]->edges[edge_index[1]]));
		structure_node_0.degree = 0;
		node_hash[structure_node_0.edges[0].at(0)] = -1;
	}
}

void Graph_Structure::Delete_Edge_Oneway(int edge_start, int midway1) {
	if (node_hash[edge_start] < 0) return;
	Graph_Structure_Node & deleted_node = nodes[node_hash[edge_start]];
	--deleted_node.degree;
	for (int i = 0; i < deleted_node.degree; ++i) {
		if (deleted_node.edges[i].at(1) == midway1) {
			deleted_node.edges[i] = deleted_node.edges[deleted_node.degree];
			break;
		}
	}
}

void Graph_Structure::Add_Edge(const vector<int> & edge) {
	if (node_hash[edge.at(0)] < 0)
		node_hash[edge.at(0)] = node_num++;
	if (node_hash[edge.at(edge.size() - 1)] < 0)
		node_hash[edge.at(edge.size() - 1)] = node_num++;
	Graph_Structure_Node & start_node = nodes[node_hash[edge.at(0)]];
	Graph_Structure_Node & end_node = nodes[node_hash[edge.at(edge.size() - 1)]];
	copy(edge.begin(), edge.end(), inserter(start_node.edges[start_node.degree], start_node.edges[start_node.degree].begin()));
	++start_node.degree;
	reverse_copy(edge.begin(), edge.end(), back_inserter(end_node.edges[end_node.degree]));
	++end_node.degree;
}

void Graph_Structure::Delete_Edge(const vector<int> & edge) {
	int end_node = edge.at(edge.size() - 1);
	int end_second = edge.at(edge.size() - 2);
	Delete_Edge_Oneway(edge.at(0), edge.at(1));
	Delete_Edge_Oneway(end_node, end_second);
}

bool Graph_Structure::Move_To_Other_End(int& last_node, int& current_node) {
	int start_hash = node_hash[last_node];
	if (start_hash < 0)
		return false;
	Graph_Structure_Node & start_node = nodes[start_hash];
	for (int i = 0; i < start_node.degree; ++i) {
		if (start_node.edges[i].at(1) == current_node) {
			last_node = start_node.edges[i].at(start_node.edges[i].size()-2);
			current_node = start_node.edges[i].at(start_node.edges[i].size()-1);
			return true;
		}
	}
	return false;
}