#include "stdafx.h"

using namespace std;

Backbone::Backbone(const Backbone & root):length(root.length), size(root.size){
	cood = new double[size][2];
	for (int i= 0;i < length;++ i){
		cood[i][0] = root.cood[i][0]; 
		cood[i][1] = root.cood[i][1];
	}
	wormLength = calculate_worm_length(this); // @cosmo-20170403
}
Backbone & Backbone::operator = (const Backbone & root){
	length = (root.length);
	if (size < length){
		size = length;
		delete[] cood;
		cood = new double[length][2];
	}
	for (int i= 0; i < length; i++){
		cood[i][0] = root.cood[i][0]; 
		cood[i][1] = root.cood[i][1];
	}
	wormLength = root.wormLength; // @cosmo-20170403
	return * this;
}

void Backbone::persistence(void * obj_ptr, string out_file){
	Backbone* typed_ptr = reinterpret_cast<Backbone *>(obj_ptr);
	ofstream file(out_file.c_str(), ios::binary);
	file.write(reinterpret_cast<char *>(&typed_ptr->length), sizeof(int));
	file.write(reinterpret_cast<char *>(typed_ptr->cood), 2 * typed_ptr->length * sizeof(double));
	file.close();
}

void Backbone::anti_persistence(void* obj_ptr, std::string in_file) {
	Backbone* typed_ptr = reinterpret_cast<Backbone *>(obj_ptr);
	ifstream file(in_file.c_str(), ios::binary);
	file.read(reinterpret_cast<char *>(&typed_ptr->length), sizeof(int));
	typed_ptr->size = typed_ptr->length;
	if (typed_ptr->cood != NULL)
		delete[] typed_ptr->cood;
	typed_ptr->cood = new double[typed_ptr->length][2];
	file.read(reinterpret_cast<char *>(typed_ptr->cood), 2 * typed_ptr->length * sizeof(double));
	file.close();
	typed_ptr->wormLength = calculate_worm_length(typed_ptr); // @cosmo-20170403
}

// @cosmo-20170403
double Backbone::calculate_worm_length(void* obj_ptr) {
	Backbone* typed_ptr = reinterpret_cast<Backbone *>(obj_ptr);
	double x1 = typed_ptr->cood[0][0];
	double y1 = typed_ptr->cood[0][1];
	double len = 0;
	for (int i = 1; i < typed_ptr->length; ++i){
		double x2 = typed_ptr->cood[i][0];
		double y2 = typed_ptr->cood[i][1];
		len = len + sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
		x1 = x2;
		y1 = y2;
	}
	return len;
}

// @kevin
void Backbone::reverse()
{
	double temp[2];
	for (int i = 0; i < length / 2; ++i){
		temp[0] = cood[length - i - 1][0];
		temp[1] = cood[length - i - 1][1];
		cood[length - i - 1][0] = cood[i][0];
		cood[length - i - 1][1] = cood[i][1];
		cood[i][0] = temp[0];
		cood[i][1] = temp[1];
	}
}

// @cosmo-20170403
void Backbone::updateWormLength() {
	wormLength = calculate_worm_length(this);
}