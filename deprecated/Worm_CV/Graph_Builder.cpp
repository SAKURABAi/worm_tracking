#include "stdafx.h"

using namespace std;

void Bifurcate_Stack::Push(const vector<int> & in_stack_points, int parent_index) {
	if (top >= SKELETONIZE::STORAGE_MAX)
		throw new Simple_Exception("Bifurcate_Stack:Stack Full Error!");
	parent_node[top] = parent_index;
	item[top] = in_stack_points;
	++top;
}

void Graph_Builder::Check_Connectivity(vector<int>& selected_points, int parent_node) {
	vector<int> temp;
	if (selected_points.size() == 2) {
		if (!candidate_points->Is_Point_Nearby(selected_points[0], selected_points[1])) {
			temp.clear();
			temp.push_back(selected_points[1]);
			stack.Push(temp, parent_node);
			selected_points.pop_back();
		}
		return;
	}
	std::stack<int> point_stack;
	vector<int> first_branch;
	int temp_index;
	bool* point_used = new bool[selected_points.size()];
	for (int i = 0; i < selected_points.size(); ++i)
		point_used[i] = false;
	for (int i = 0; i < selected_points.size(); ++i) {
		if (!point_used[i]) {
			point_stack.push(i);
			temp.clear();
		}
		while (!point_stack.empty()) {
			temp_index = point_stack.top();
			point_stack.pop();
			point_used[temp_index] = true;
			temp.push_back(selected_points[temp_index]);
			for (int j = i + 1; j < selected_points.size(); ++j)
				if (!point_used[j] && candidate_points->Is_Point_Nearby(selected_points[temp_index], selected_points[j]))
					point_stack.push(j);
		}
		if (first_branch.empty())
			first_branch = temp;
		else stack.Push(temp, parent_node);
	}
	selected_points = first_branch;
	delete[] point_used;
}

void Graph_Builder::Search_Unused_Nearby_Points(std::vector<int> & selected_points) const {
	static vector<int> nearby_points;
	candidate_points->Query_Points_Nearby(selected_points, nearby_points);
	selected_points.clear();
	for (vector<int>::iterator itor = nearby_points.begin(); itor != nearby_points.end(); ++itor)
		if (point_mark[*itor] < 0)
			selected_points.push_back(*itor);
}

int Graph_Builder::Search_Further_Point(vector<int> & points_in_current_node, int current_node_index) const {
	double direction_vec[2];
	const Graph_Node* current_node = graph->Get_Node(current_node_index);
	if (current_node->degree != 1)
		return -1;
	const Graph_Node* last_node = graph->Get_Node(current_node->adjacent[0]);
	for (int i = 0; i < 2; ++i)
		direction_vec[i] = current_node->center[i] - last_node->center[i];
	double direction_norm_square = direction_vec[0] * direction_vec[0] + direction_vec[1] * direction_vec[1];
	if (direction_norm_square == 0)
		return -1;
	// find base point by projection
	double projection_len;
	double base_point[2] = { current_node->center[0], current_node->center[1] };
	for (vector<int>::iterator itor = points_in_current_node.begin(); itor < points_in_current_node.end(); ++itor) {
		const int* temp_center = candidate_points->Get_Point(*itor);
		projection_len = direction_vec[0] * (temp_center[0] - base_point[0]) + direction_vec[1] * (temp_center[1] - base_point[1]);
		if (projection_len > 0) {
			base_point[0] += direction_vec[0] * projection_len / direction_norm_square;
			base_point[1] += direction_vec[1] * projection_len / direction_norm_square;
		}
	}
	base_point[0] += SKELETONIZE::ANGLE_ERROR*direction_vec[0];
	base_point[1] += SKELETONIZE::ANGLE_ERROR*direction_vec[1];
	return candidate_points->Query_Points_By_Pointer(base_point, direction_vec);
}

void Graph_Builder::Search_Next_Points() {
	int current_node = graph->Get_Node_Num() - 1;
	vector<int> points_in_current_node = selected_points;

	Search_Unused_Nearby_Points(selected_points);
	if (selected_points.size() > 1)
		Check_Connectivity(selected_points, current_node);
	// try to find furthur point
	if (selected_points.empty()) {
		int furthur_point = Search_Further_Point(points_in_current_node, current_node);
		if (furthur_point >= 0) {
			if (point_mark[furthur_point] >= 0)
				graph->Connect_Node(point_mark[furthur_point], current_node);
			else
				selected_points.push_back(furthur_point);
		}
	}
	// try to find unused points in stack
	if (selected_points.empty()) {
		while (stack.top > 0) {
			--stack.top;
			if (point_mark[stack.item[stack.top][0]] < 0) {
				selected_points = stack.item[stack.top];
				current_node = stack.parent_node[stack.top];
			}
		}
	}
	// try to find an unused point
	if (selected_points.empty()) {
		for (int i = 0; i < candidate_points->Get_Point_Num(); ++i)
			if (point_mark[i] < 0) {
				selected_points.push_back(i);
				break;
			}
		current_node = -1;
	}
	// try to extend single point 
	if (selected_points.size() == 1)
		Search_Unused_Nearby_Points(selected_points);
	// add selected points into graph
	if (!selected_points.empty()) {
		for (vector<int>::iterator itor = selected_points.begin(); itor < selected_points.end(); ++itor)
			point_mark[*itor] = graph->Get_Node_Num();
		graph->Add_Node(candidate_points->Get_Center(selected_points), current_node);
	}
}

void Graph_Builder::Connecting_End() const {
	vector<int> end_node_points;
	int point_index;
	for (int i = 0; i < graph->Get_Node_Num(); ++i) {
		int end_node_index = i;
		while (graph->Get_Node(end_node_index)->degree == 1) {
			end_node_points.clear();
			for (int j = 0; j < candidate_points->Get_Point_Num(); ++j)
				if (point_mark[j] == end_node_index)
					end_node_points.push_back(j);
			point_index = Search_Further_Point(end_node_points, end_node_index);
			if (point_index < 0 || point_mark[point_index] < 0)
				break;
			graph->Connect_Node(end_node_index, point_mark[point_index]);
			end_node_index = point_mark[point_index];
		}
	}
}

void Graph_Builder::Convert_To_Graph(const Candidate_Points * candidate_points, Graph * skeleton_graph, string pic_num_str) {
	this->candidate_points = candidate_points;
	this->graph = skeleton_graph;
	skeleton_graph->Reset();
	int point_num = candidate_points->Get_Point_Num();
	point_mark = new int[point_num];
	for (int i = 0; i < point_num; ++i)
		point_mark[i] = -1;
	stack.top = 0;
	selected_points.clear();
	selected_points.push_back(0);
	while (!selected_points.empty())
		Search_Next_Points();
	Connecting_End();
	delete[] point_mark;
}