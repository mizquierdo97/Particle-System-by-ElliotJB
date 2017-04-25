#include "Particle.h"

Particle::Particle(fPoint respawn, iPoint area, iPoint timelife, fPoint speed_particle, bool speed_static, int size, int num_tex_pixel, bool active)
{
	//with random change position
	if (area.x == 0)
		position.x = respawn.x;
	else
		position.x = respawn.x - (rand() % area.x) + (rand() % area.x);

	if (area.y == 0)
		position.y = respawn.y;
	else
		position.y = respawn.y - (rand() % area.y) + (rand() % area.y);

	//set area_reapwn
	area_respawn = { (int)respawn.x - area.x, (int)respawn.y - area.y, (int)respawn.x + area.x, (int)respawn.y + area.y };

	//TimeLife of Particle
	if (active)
		degrade.y = 255;
	else
		degrade.y = 0;
	degrade.x = (rand() % timelife.x + timelife.y);

	//Set Speed
	speed = speed_particle;
	if (speed_static == false)
	{
		speed.y -= (((float)rand() / (float)(RAND_MAX)) * 10);
		speed.x = (((float)rand() / (float)(RAND_MAX)) * 20) - (((float)rand() / (float)(RAND_MAX)) * 20);
	}

	//Set SDL_Rect
	switch (rand() % num_tex_pixel)
	{
	case 0: rect = { 0,0,size,size }; break;
	case 1: rect = { size,0,size,size }; break;
	case 2: rect = { size * 2,0,size,size }; break;
	case 3: rect = { size * 3,0,size,size }; break;
	case 4: rect = { size * 4,0,size,size }; break;
	case 5: rect = { size * 5,0,size,size }; break;
	case 6: rect = { size * 6,0,size,size }; break;
	case 7: rect = { size * 7,0,size,size }; break;
	}
	size_rect = size;
}

bool Particle::Modify(fPoint respawn, iPoint area, iPoint timelife, iPoint num_tex_p)
{
	//with random change position
	if (area.x == 0)
		position.x = respawn.x;
	else
		position.x = respawn.x - (rand() % area.x) + (rand() % area.x);

	if (area.y == 0)
		position.y = respawn.y;
	else
		position.y = respawn.y - (rand() % area.y) + (rand() % area.y);

	//TimeLife of Particle
	degrade.y = 255;
	degrade.x = (rand() % timelife.x + timelife.y);

	//Set SDL_Rect
	switch (rand() % num_tex_p.y + num_tex_p.x)
	{
	case 0: rect = { 0, 0, size_rect, size_rect }; break;
	case 1: rect = { size_rect, 0, size_rect, size_rect }; break;
	case 2: rect = { size_rect * 2, 0, size_rect, size_rect }; break;
	case 3: rect = { size_rect * 3, 0, size_rect, size_rect }; break;
	case 4: rect = { size_rect * 4, 0, size_rect, size_rect }; break;
	case 5: rect = { size_rect * 5, 0, size_rect, size_rect }; break;
	case 6: rect = { size_rect * 6, 0, size_rect, size_rect }; break;
	case 7: rect = { size_rect * 7, 0, size_rect, size_rect }; break;
	}
	return true;
}

bool Particle::isDead()
{
	//return is dead;
	return degrade.y == 0;
}

void Particle::render(SDL_Texture* texture)
{
	degrade.y -= MIN(degrade.y, degrade.x);
	if (degrade.y <= 0)
	{
		degrade.y = 0;
	}
	SDL_SetTextureAlphaMod(texture, degrade.y);
	App->render->Blit(texture, position.x, position.y, &rect);
}

void Particle::Move(fPoint speed, Wind dir, bool Move_alternative)
{
	if (Move_alternative == false)
	{
		position.x += speed.x;
		position.y += speed.y;
	}
	else
	{
		if (dir == W_UP)
		{
			position.x += speed.x;
			if (degrade.y < 160)
			{
				position.y += speed.y;
			}
		}
		else if (dir == W_DOWN)
		{
			position.x += speed.x;
			if (degrade.y < 160)
			{
				position.y -= speed.y;
			}
		}
		else if (dir == W_RIGHT)
		{
			position.y += speed.y;
			if (degrade.y < 160)
			{
				position.x += speed.x;
			}
		}
		else //dir == W_LEFT
		{
			position.y += speed.y;
			if (degrade.y < 160)
			{
				position.x -= speed.x;
			}
		}

	}
}

fPoint Particle::GetPosition()
{
	return position;
}