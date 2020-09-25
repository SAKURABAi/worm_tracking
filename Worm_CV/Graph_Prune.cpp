#include "stdafx.h"
using namespace std;

enum BACKBONE_STATE{REVERSE, ABANDON, FORWARD};
#define LARGE_SCALE_PARTITION_NUM 10

double Graph_Prune::Calculate_Turning_Angle(double current_cood[2], double last_cood[2]){
	double angle = abs(atan2(current_cood[0], current_cood[1]) - atan2(last_cood[0], last_cood[1]));
	if (angle > WORM::PI)
		angle = 2 * WORM::PI - angle;
	return angle;
}

bool Graph_Prune::Same_Direction(const std::vector<int> & cline, const Backbone& last_backbone) {
	const double* p0 = graph->Get_Node(cline.at(0))->center;
	const double* p2 = graph->Get_Node(cline.at(cline.size() - 1))->center;
	double* last_start = last_backbone.cood[0];
	double* last_end = last_backbone.cood[last_backbone.length - 1];
	return (p0[0] - p2[0])*(last_start[0] - last_end[0])
		+ (p0[1] - p2[1])*(last_start[1] - last_end[1]) > 0;
}

bool Graph_Prune::Same_Clockwise(const std::vector<int> & cline, const Backbone& last_backbone, int start_2, int end_2) {
	const double* p0 = graph->Get_Node(cline.at(0))->center;
	const double* p1 = graph->Get_Node(cline.at(cline.size() / 3))->center;
	const double* p2 = graph->Get_Node(cline.at(cline.size() * 2/3))->center;
	double* l0 = last_backbone.cood[start_2];
	double* l1 = last_backbone.cood[(start_2*2 + end_2) / 3];
	double* l2 = last_backbone.cood[(start_2 + end_2*2) / 3];
	double angle_p = Calc_Clockwise_Angle(p0, p1, p2);
	double angle_l = Calc_Clockwise_Angle(l0, l1, l2);
	return (angle_p < WORM::PI == angle_l < WORM::PI);
}

void Graph_Prune::Get_Largest_Subgraph() {
	int subgraph_num = 0;
	int stack_top = 0;
	int* subgraph_mark = new int[node_num];
	int* node_stack = new int[node_num];
	for (int i = 0; i < node_num; ++i)
		subgraph_mark[i] = -1;
	//深搜每个分支
	int current_index;
	const Graph_Node * current_node;
	for (int i = 0; i < node_num; ++i) {
		if (subgraph_mark[i] < 0) {
			++subgraph_num;
			node_stack[stack_top++] = i;
		}
		while (stack_top > 0) {
			current_index = node_stack[--stack_top];
			subgraph_mark[current_index] = subgraph_num-1;
			current_node = graph->Get_Node(current_index);
			for (int j = 0; j < current_node->degree; ++j)
				if (subgraph_mark[current_node->adjacent[j]] < 0)
					node_stack[stack_top++] = current_node->adjacent[j];
		}
	}
	// 计算分支大小
	int* subgraph_count = new int[subgraph_num];
	for (int i = 0; i < subgraph_num; ++i)
		subgraph_count[i] = 0;
	for (int i = 0; i < node_num; ++i)
		++subgraph_count[subgraph_mark[i]];
	// 选择最大分支
	Select_Minimum find_largest_subgraph(-subgraph_count[0], 0);
	for (int i = 1; i < subgraph_num; ++i)
		find_largest_subgraph.Renew(-subgraph_count[i], i);
	int largest_subgraph = find_largest_subgraph.Get_Min_Index();
	for (int i = 0; i < node_num; ++i)
		node_available[i] = (subgraph_mark[i] == largest_subgraph);
	delete[] subgraph_count;
	delete[] subgraph_mark;
	delete[] node_stack;
}

void Graph_Prune::Rotate_To_Next(int & last_node, int & current_node) const {
	const Graph_Node* current_graph_node = graph->Get_Node(current_node);
	const Graph_Node* last_graph_node = graph->Get_Node(last_node);
	double direction = atan2(last_graph_node->center[0] - current_graph_node->center[0],
		last_graph_node->center[1] - current_graph_node->center[1]) + SKELETONIZE::ANGLE_ERROR;
	Select_Minimum adjacent_select(WORM::INF, -1);
	double angle_temp;
	int node_to;
	const double * temp_center;
	for (int i = 0; i < current_graph_node->degree; ++i) {
		node_to = current_graph_node->adjacent[i];
		temp_center = graph->Get_Node(node_to)->center;
		// angle_temp 表示从direction到base_node指向node_to的方向的顺时针转角，范围在0-2pi
		angle_temp = atan2(temp_center[0] - current_graph_node->center[0],
			temp_center[1] - current_graph_node->center[1]) - direction;
		while (angle_temp < 0)
			angle_temp += 2 * WORM::PI;
		adjacent_select.Renew(angle_temp, i);
	}
	last_node = current_node;
	current_node = current_graph_node->adjacent[adjacent_select.Get_Min_Index()];
}

void Graph_Prune::Start_Node_Locate(int & first_node, int & second_node) const {
	Select_Minimum find_leftmost_point(WORM::INF, -1);
	Select_Minimum second_select(WORM::INF, -1);
	for (int i = 0; i < node_num; ++i) {
		if (node_available[i])
			find_leftmost_point.Renew(graph->Get_Node(i)->center[0], i);
	}
	first_node = find_leftmost_point.Get_Min_Index();
	const Graph_Node* leftmost_node = graph->Get_Node(first_node);
	int node_to;
	for (int i = 0; i < leftmost_node->degree; ++i) {
		node_to = leftmost_node->adjacent[i];
		if (!node_available[node_to]) continue;
		double temp_metric = atan2(graph->Get_Node(node_to)->center[0] - leftmost_node->center[0],
			graph->Get_Node(node_to)->center[1] - leftmost_node->center[1]) + WORM::PI / 2;
		if (temp_metric < 0) temp_metric += WORM::PI * 2;
		second_select.Renew(temp_metric, i);
	}
	second_node = leftmost_node->adjacent[second_select.Get_Min_Index()];
	int edge_num = 1;
	while (graph->Get_Node(second_node)->Select_Next(first_node, second_node)) {
		if (++edge_num > node_num)
			throw new Simple_Exception("Circle Error!");
	}
	Rotate_To_Next(first_node, second_node);
}

void Graph_Prune::Graph_Structure_Analyze(int & first_node, int & second_node) {
	int last_node, current_node;
	last_node = first_node;
	current_node = second_node;
	vector<int> edge;
	while (true) {
		edge.clear();
		edge.push_back(last_node);
		edge.push_back(current_node);
		while (graph->Get_Node(current_node)->Select_Next(last_node, current_node))
			edge.push_back(current_node);
		graph_structure->Add_Edge(edge);
		do {
			Rotate_To_Next(last_node, current_node);
			if (last_node == first_node && current_node == second_node)
				return;
		} while (graph_structure->Move_To_Other_End(last_node, current_node));
	}
}

void Graph_Prune::Delete_Short_Route() {
	for (int i = 0; i < structure_node_num; ++i) {
		const Graph_Structure_Node & structure_node = structure_node_list[i];
		for (int j = 0; j < structure_node.degree - 1; ++j) {
			if (structure_node.edges[0].at(0) == structure_node.edges[j].at(structure_node.edges[j].size() - 1) && structure_node.edges[j].size() <= 4)
				graph_structure->Delete_Edge(structure_node.edges[j]);
		}
		if (structure_node.degree == 1 && structure_node.edges[0].size() <= 2)
			graph_structure->Delete_Edge(structure_node.edges[0]);
	}
	graph_structure->Check_Structure();
}

bool Graph_Prune::Delete_Shorter_Routes_With_Same_End() {
	bool changed = false;
	int delete_index;
	for (int i = 0; i < structure_node_num; ++i) {
		const Graph_Structure_Node & structure_node = structure_node_list[i];
		for (int j = 0; j < structure_node.degree - 1; ++j) {
			if (structure_node.edges[j].at(structure_node.edges[j].size() - 1) == structure_node.edges[0].at(0))
				continue;
			for (int k = j + 1; k < structure_node.degree; ++k)
				if (structure_node.edges[j].at(structure_node.edges[j].size() - 1) == structure_node.edges[k].at(structure_node.edges[k].size() - 1)) {
					changed = true;
					delete_index = structure_node.edges[j].size() > structure_node.edges[k].size() ? k : j;
					graph_structure->Delete_Edge(structure_node.edges[delete_index]);
				}
		}
	}
	if (changed)
		graph_structure->Check_Structure();
	return changed;
}

bool Graph_Prune::Delete_Branch_And_Loopback_Except_For_Two_Longest() {
	bool changed = false;
	int* branch_index = new int[structure_node_num*SKELETONIZE::DEGREE_MAX];
	int* branch_len = new int[structure_node_num*SKELETONIZE::DEGREE_MAX];
	int branch_num = 0;
	// 寻找所有的分支与自环
	for (int i = 0; i < structure_node_num; ++i) {
		const Graph_Structure_Node & structure_node = structure_node_list[i];
		if (structure_node.degree == 1) {
			branch_index[branch_num] = i*SKELETONIZE::DEGREE_MAX;
			branch_len[branch_num] = structure_node.edges[0].size();
			++branch_num;
		}
		else for (int j = 0; j < structure_node.degree; ++j) {
			if (structure_node.edges[j].at(structure_node.edges[j].size() - 1) == structure_node.edges[0].at(0) &&
				structure_node.edges[j].at(1) < structure_node.edges[j].at(structure_node.edges[j].size() - 2)) {
				branch_index[branch_num] = i*SKELETONIZE::DEGREE_MAX + j;
				branch_len[branch_num] = structure_node.edges[j].size() >> 1;
				++branch_num;
			}
		}
	}
	if (branch_num > 2) {
		changed = true;
		// 找到最长两个分支
		Select_Minimum longest_branch(WORM::INF, -1);
		Select_Minimum second_longest_branch(WORM::INF, -1);
		for (int i = 0; i < branch_num; ++i)
			longest_branch.Renew(-branch_len[i], i);
		for (int i = 0; i < branch_num; ++i)
			if (i != longest_branch.Get_Min_Index())
				second_longest_branch.Renew(-branch_len[i], i);
		// 删除最长两个分支之外的分支
		for (int i = 0; i < branch_num; ++i)
			if (!(i == longest_branch.Get_Min_Index() || i == second_longest_branch.Get_Min_Index())) {
				const Graph_Structure_Node & structure_node_delete = structure_node_list[branch_index[i] / SKELETONIZE::DEGREE_MAX];
				int edge_delete = branch_index[i] % 5;
				graph_structure->Delete_Edge(structure_node_delete.edges[edge_delete]);
			}
		graph_structure->Check_Structure();
	}
	delete[] branch_index;
	delete[] branch_len;
	return changed;
}

void Graph_Prune::Structure_Node_Statistic(int& special_node_num, int& loopback_count, const Graph_Structure_Node* special_node[2]) {
	special_node_num = 0;
	loopback_count = 0;
	for (int i = 0; i < structure_node_num; ++i) {
		if (structure_node_list[i].degree > 0) {
			if (special_node_num == 0)
				special_node[0] = structure_node_list + i;
			else
				special_node[1] = structure_node_list + i;
			++special_node_num;
		}
		if (structure_node_list[i].degree > 1)
			++loopback_count;
	}
}

void Graph_Prune::Delete_Smaller_Loopback(int& bifurcate_node_num, const Graph_Structure_Node* special_node[2]) {
	int edge_index[2], edge_len[2];
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < special_node[i]->degree - 1; ++j)
			if (special_node[i]->edges[j].at(0) == special_node[i]->edges[j].at(special_node[i]->edges[j].size() - 1)) {
				edge_index[i] = j;
				edge_len[i] = special_node[i]->edges[j].size();
			}
	if (edge_len[0]>edge_len[1])
		graph_structure->Delete_Edge(special_node[1]->edges[edge_index[1]]);
	else
		graph_structure->Delete_Edge(special_node[0]->edges[edge_index[0]]);
	--bifurcate_node_num;
}

void Graph_Prune::Connect_Correct_Loopback_To_Route(Backbone& last_backbone,
	double worm_full_width, const Graph_Structure_Node* special_node[2], vector<int> & route) {

	vector<int> loopback[2];
	int loop_count = 0;

	// initialize, direction calc
	for (int j = 0; j < special_node[1]->degree; ++j){
		if (special_node[1]->edges[j].at(0) == special_node[1]->edges[j].at(special_node[1]->edges[j].size() - 1))
			loopback[loop_count++] = special_node[1]->edges[j];
	}
	route.pop_back();

	vector<int> forward_route = route;
	forward_route.insert(forward_route.end(), loopback[0].begin(), loopback[0].end());
	vector<int> backward_route = route;
	backward_route.insert(backward_route.end(), loopback[1].begin(), loopback[1].end());

	Backbone forward_backbone(forward_route.size()), backward_backbone(forward_route.size());
	for (int i = 0; i < forward_route.size(); ++i){
		forward_backbone.cood[i][0] = graph->Get_Node(forward_route.at(i))->center[0];
		forward_backbone.cood[i][1] = graph->Get_Node(forward_route.at(i))->center[1];
		backward_backbone.cood[i][0] = graph->Get_Node(backward_route.at(i))->center[0];
		backward_backbone.cood[i][1] = graph->Get_Node(backward_route.at(i))->center[1];
	}
	forward_backbone.length = forward_route.size();
	backward_backbone.length = forward_route.size();

	Backbone reverse_forward_backbone = forward_backbone;
	reverse_forward_backbone.reverse();
	Backbone reverse_backward_backbone = backward_backbone;
	reverse_backward_backbone.reverse();

	double forward_es = Calc_AngleCurve_ES(forward_backbone, last_backbone);
	double reverse_forward_es = Calc_AngleCurve_ES(reverse_forward_backbone, last_backbone);
	double backward_es = Calc_AngleCurve_ES(backward_backbone, last_backbone);
	double reverse_backward_es = Calc_AngleCurve_ES(reverse_backward_backbone, last_backbone);

	double min_forward = (forward_es > reverse_forward_es ? reverse_forward_es : forward_es);
	double min_backward = (backward_es > reverse_backward_es ? reverse_backward_es : backward_es);
	cout << "Forward: " << min_forward << ", Backward: " << min_backward <<endl;

	if (min_forward > min_backward){ route = backward_route; }
	else{ route = forward_route; }
	
	const double* bifurcate_cood = graph->Get_Node(loopback[0].at(0))->center;
	while (Point_Dist_Square(bifurcate_cood, graph->Get_Node(route.at(route.size() - 1))->center) < worm_full_width * worm_full_width / 10)
		route.pop_back();
}

void Graph_Prune::Save_GraphStructureNodes(const Graph_Structure_Node* special_node[2])
{
	ofstream file(".\\cache_data\\structre_node", ios::binary);
	int edge_num = 0;
	for (int n = 0; n < 2; ++n){
		for (int i = 0; i < 4; ++i){
			if (special_node[n]->edges[i].size()>0)
				++edge_num;
		}
		file.write(reinterpret_cast<const char *>(&edge_num), sizeof(int));
		for (int i = 0; i < edge_num; ++i){
			int edge_size = special_node[n]->edges[i].size();
			file.write(reinterpret_cast<const char*>(&edge_size), sizeof(int));
			for (int j = 0; j < edge_size; ++j){
				file.write(reinterpret_cast<const char*>(&(special_node[n]->edges[i])[j]), sizeof(int));
			}
		}
		edge_num = 0;
	}
}

void Graph_Prune::Prune(const Graph* graph_before_prune, Backbone& last_backbone, double worm_full_width, bool is_first_pic) {
	graph = graph_before_prune;
	node_num = graph->Get_Node_Num();
	node_available = new bool[node_num];
	for (int i = 0; i < node_num; ++i)
		node_available[i] = true;

	Get_Largest_Subgraph();
	// graph structure init
	structure_node_num = 0;
	for (int i = 0; i < node_num; ++i)
		if (node_available[i] && graph->Get_Node(i)->degree != 2)
			++structure_node_num;

	graph_structure = new Graph_Structure(node_num, structure_node_num);
	structure_node_list = graph_structure->Get_Node_List();

	// analyze structure
	int first_node;
	int second_node;
	Start_Node_Locate(first_node, second_node);
	Graph_Structure_Analyze(first_node, second_node);
	graph_structure->Check_Structure();

	// prune
	Delete_Short_Route();
	while (Delete_Shorter_Routes_With_Same_End() || Delete_Branch_And_Loopback_Except_For_Two_Longest());

	// search backbone
	int special_node_num;
	int loopback_count;
	const Graph_Structure_Node * special_node[2];
	Structure_Node_Statistic(special_node_num, loopback_count, special_node);

	if (special_node_num != 2)
		throw new Simple_Exception("Prune Error!!! Special Node Num Must Be 2!!!");
	if (is_first_pic && loopback_count > 0)
		throw new Simple_Exception("First Pic Cannot Have Loopback!");

	if (loopback_count == 2)
		Delete_Smaller_Loopback(loopback_count, special_node);
	if (loopback_count == 1 && special_node[0]->degree > 1)
		swap(special_node[0], special_node[1]);

	vector<int> route = special_node[0]->edges[0];
	if (loopback_count > 0)
		Connect_Correct_Loopback_To_Route(last_backbone, worm_full_width, special_node, route);

	BACKBONE_STATE backbone_state = FORWARD;
	if (!is_first_pic) {
		if (Point_Dist_Square(graph->Get_Node(route.at(0))->center, graph->Get_Node(route.at(route.size() - 1))->center)
			< SKELETONIZE::WORM_SPEED * SKELETONIZE::WORM_SPEED * worm_full_width*worm_full_width) {
			backbone_state = (BACKBONE_STATE)Head_Tail_Recognize(route, last_backbone);
		}
		else{ 
			backbone_state = (BACKBONE_STATE)Long_Distance_Condition(route, last_backbone);
		}
	}

	if (backbone_state == ABANDON){ throw new Simple_Exception("Backbone is abandoned"); }
	else if (backbone_state == REVERSE){ reverse(route.begin(), route.end()); }

	double(*new_cood)[2] = new double[route.size()][2];
	const double *temp_cood;
	for (int i = 0; i < route.size(); ++i) {
		temp_cood = graph->Get_Node(route.at(i))->center;
		new_cood[i][0] = temp_cood[0];
		new_cood[i][1] = temp_cood[1];
	}
	delete[] last_backbone.cood;
	last_backbone.cood = new_cood;
	last_backbone.length = route.size();
	last_backbone.size = route.size();
	last_backbone.updateWormLength(); // @cosmo-20170403

	delete[] node_available;
	delete graph_structure;
}

int Graph_Prune::Long_Distance_Condition(const std::vector<int> & current_route, Backbone &last_backbone){
	BACKBONE_STATE backbone_state = FORWARD;
	const double* p0 = graph->Get_Node(current_route.at(0))->center;
	const double* p1 = graph->Get_Node(current_route.at(current_route.size() - 1))->center;
	double* l0 = last_backbone.cood[0];
	double* l1 = last_backbone.cood[last_backbone.length - 1];

	double current_worm_dir[2], last_worm_dir[2];
	current_worm_dir[0] = p0[0] - p1[0];
	current_worm_dir[1] = p0[1] - p1[1];
	last_worm_dir[0] = l0[0] - l1[0];
	last_worm_dir[1] = l0[1] - l1[1];

	double turning_angle = Calculate_Turning_Angle(current_worm_dir, last_worm_dir);

	// Analyze the turning angle
	if (turning_angle < SKELETONIZE::WORM_TURNING_ANGLE )
		backbone_state = FORWARD;
	else if (turning_angle >(WORM::PI - SKELETONIZE::WORM_TURNING_ANGLE) )
		backbone_state = REVERSE;
	else{
		// Analyze the distance ratio
		double Start_Head_Dist = sqrt(Point_Dist_Square(p0, last_backbone.cood[0]));
		double End_Tail_Dist = sqrt(Point_Dist_Square(p1, last_backbone.cood[last_backbone.length-1]));
		double Start_Tail_Dist = sqrt(Point_Dist_Square(p0, last_backbone.cood[last_backbone.length-1]));
		double End_Head_Dist = sqrt(Point_Dist_Square(p1, last_backbone.cood[0]));

		int condition_flag = 0;
		if (Start_Tail_Dist / Start_Head_Dist > SKELETONIZE::FORWARD_DIST_PORTION && End_Head_Dist / End_Tail_Dist > SKELETONIZE::FORWARD_DIST_PORTION){
			++condition_flag;
			backbone_state = FORWARD;
		}
		if (Start_Head_Dist / Start_Tail_Dist > SKELETONIZE::FORWARD_DIST_PORTION && End_Tail_Dist / End_Head_Dist > SKELETONIZE::FORWARD_DIST_PORTION){
			++condition_flag;
			backbone_state = REVERSE;
		}
		if (condition_flag == 2){
			backbone_state = ABANDON;
		}
	}
	return int(backbone_state);
}

int Graph_Prune::Head_Tail_Recognize(const std::vector<int> & current_route, Backbone &last_backbone)
{
	Backbone current_reverse_backbone(current_route.size());
	Backbone current_backbone(current_route.size());
	current_reverse_backbone.length = current_route.size();
	current_backbone.length = current_route.size();

	int route_len = current_route.size();
	for (int i = 0; i < current_route.size(); ++i) {
		current_reverse_backbone.cood[i][0] = graph->Get_Node(current_route.at(route_len - 1 - i))->center[0];
		current_reverse_backbone.cood[i][1] = graph->Get_Node(current_route.at(route_len - 1 - i))->center[1];
		current_backbone.cood[i][0] = graph->Get_Node(current_route.at(i))->center[0];
		current_backbone.cood[i][1] = graph->Get_Node(current_route.at(i))->center[1];
	}
	double es = Calc_AngleCurve_ES(current_backbone, last_backbone);
	double reverse_es = Calc_AngleCurve_ES(current_reverse_backbone, last_backbone);

	if (es > reverse_es) { return (int)REVERSE; }
	else { return (int)FORWARD; }
}

double Graph_Prune::Calc_AngleCurve_ES(Backbone &current_backbone, Backbone &last_backbone)
{
	Backbone last_backbone_smooth = last_backbone;
	Backbone current_backbone_smooth = current_backbone;
	root_smooth.Interpolate_And_Equal_Divide(last_backbone_smooth, LARGE_SCALE_PARTITION_NUM);
	root_smooth.Interpolate_And_Equal_Divide(current_backbone_smooth, LARGE_SCALE_PARTITION_NUM);

	double *last_angle_curve = new double[LARGE_SCALE_PARTITION_NUM];
	double *current_angle_curve = new double[LARGE_SCALE_PARTITION_NUM];
	for (int i = 0; i < LARGE_SCALE_PARTITION_NUM; ++i){
		last_angle_curve[i] = atan2(last_backbone_smooth.cood[i + 1][1] - last_backbone_smooth.cood[i][1],
			last_backbone_smooth.cood[i + 1][0] - last_backbone_smooth.cood[i][0]);
		current_angle_curve[i] = atan2(current_backbone_smooth.cood[i + 1][1] - current_backbone_smooth.cood[i][1],
			current_backbone_smooth.cood[i + 1][0] - current_backbone_smooth.cood[i][0]);
	}

	// angle curve unwarp (bug!!!)
	unwrap(last_angle_curve, LARGE_SCALE_PARTITION_NUM);
	unwrap(current_angle_curve, LARGE_SCALE_PARTITION_NUM);

	double es = 0, _es = 0, es_ = 0; // error square
	for (int i = 0; i < LARGE_SCALE_PARTITION_NUM; ++i){
		_es += (current_angle_curve[i] - last_angle_curve[i] - 2 * WORM::PI)*(current_angle_curve[i] - last_angle_curve[i] - 2 * WORM::PI);
		es += (current_angle_curve[i] - last_angle_curve[i])*(current_angle_curve[i] - last_angle_curve[i]);
		es_ += (current_angle_curve[i] - last_angle_curve[i] + 2 * WORM::PI)*(current_angle_curve[i] - last_angle_curve[i] + 2 * WORM::PI);
	}
	delete last_angle_curve;
	delete current_angle_curve;

	es = min(es,min( _es, es_));
	return es;
}

void Graph_Prune::unwrap(double *v, int num)
{
	double *diff_wrap = new double[num];
	for (int i = 1; i < num; ++i){
		diff_wrap[i] = (v[i] - v[i - 1]);
		diff_wrap[i] = atan2(sin(diff_wrap[i]), cos(diff_wrap[i]));
	}
	for (int i = 1; i < num; ++i){
		v[i] = v[i - 1] + diff_wrap[i];
	}
	delete diff_wrap;
}