#include "stdafx.h"

using namespace std;

Graph_Node & Graph_Node::operator=(const Graph_Node & node){
	degree = node.degree;
	for (int i = 0; i < degree; i++){
		adjacent[i] = node.adjacent[i];
	}
	center[0] = node.center[0];
	center[1] = node.center[1];
	return * this;
}

Graph_Node::Graph_Node(const Graph_Node & node_from, bool copy_adjacent){
	center[0] = node_from.center[0];
	center[1] = node_from.center[1];
	if (copy_adjacent){
		degree = node_from.degree;
		for (int i = 0;i < degree;++ i)
			adjacent[i] = node_from.adjacent[i];
	}
	else
		degree = 0;
}

int Graph_Node::Get_Adjacent_Index(int adjacent_node) const{
	for (int i = 0;i < degree;++ i)
		if (adjacent[i] == adjacent_node)
			return i;
	return -1;
}

void Graph::persistence(void * obj_ptr, string out_file) {
	ofstream file(out_file.c_str(), ios::binary);
	file.write(reinterpret_cast<char *>(obj_ptr), sizeof(Graph));
	file.close();
}

void Graph::anti_persistence(void* obj_ptr, std::string in_file) {
	ifstream file(in_file.c_str(), ios::binary);
	file.read(reinterpret_cast<char *>(obj_ptr), sizeof(Graph));
	file.close();
}