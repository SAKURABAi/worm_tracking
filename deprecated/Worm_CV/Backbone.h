#pragma once

#include "CONST_PARA.h"

struct Backbone{
	int length;
	int size;
	double (* cood)[2];
	double wormLength; // @cosmo-20170403

	explicit Backbone(int root_max_length):length(0), size(root_max_length){
		cood = new double[size][2];
		updateWormLength(); // @cosmo-20170404
	}
	Backbone(const Backbone & root);
	Backbone & operator=(const Backbone & root);
	~Backbone(){
		delete[] cood;
	}
	static void persistence(void *obj_ptr, std::string out_file);
	static void anti_persistence(void *obj_ptr, std::string in_file);

	// @cosmo-20170403
	static double calculate_worm_length(void* obj_ptr);
	void updateWormLength();
};

