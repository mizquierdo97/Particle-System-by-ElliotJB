#pragma once
#ifndef _P_FIREWORK_H_
#define _P_FIREWORK_H_

#include "ParticleManager.h"
#include "SceneElements.h"

#define MULITFIREWORK 4

class Particle;

class P_Firework
{
public:
	//	P_Follow(iPoint* element, iPoint area, iPoint timelife, SDL_Texture* texture, int size_particle, int num_textures, int num_particles = 20, bool active_ = false);
	P_Firework(SceneElement* element, iPoint* object_follow, iPoint position_static, SDL_Texture* texture, iPoint timelife = { 15,5 }, fPoint speed = { 0,0 }, P_Direction p_direction = P_NON, int num_particles = 20, int num_textures = 4, iPoint next_textures = { 0, 4 }, iPoint last_textures = { 0, 4 });
	~P_Firework();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	void render(fPoint pos);

	void MoveParticles();

public:
	SDL_Texture* texture;
	std::vector<Particle*> particle;
	SceneElement* element_to_follow;
	//Num of particles
	int number_particles;
	int number_multifirework;

	//test
	iPoint* object_follow;

	//timelife
	iPoint timelife;


	//textures
	iPoint next_textures;
	iPoint last_textures;

	//position
	fPoint pos;
	int size_rect;
	int n_textures;

	//velocity
	fPoint speed;

	bool godelete;

};


#endif /*_P_FIREWORK_H_*/