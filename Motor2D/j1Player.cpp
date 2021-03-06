#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Scene.h"
#include "j1Player.h"
#include "j1FileSystem.h"
#include "j1AnimationManager.h"
#include "j1Collision.h"
#include "j1InputManager.h"
#include "Soldier.h"
#include "j1Item.h"
#include "j1DynamicObjects.h"
#include "j1Creature.h"
#include "j1Weapon.h"
#include "Animation.h"
#include "ParticleManager.h"
#include "P_Follow.h"

//Constructor
Player::Player() : Creature()
{
	type = CREATURE;
	name = "Link";
}

// Destructor
Player::~Player()
{}

// Called before render is available
bool Player::Awake(pugi::xml_node& conf)
{
	LOG("Loading Texture Player");
	bool ret = true;
	hp = conf.child("stats").attribute("hp").as_int(0);
	attack = conf.child("stats").attribute("attack").as_int(0);
	speed = conf.child("stats").attribute("speed").as_int(0);
	position.x = conf.child("stats").attribute("pos_x").as_int(0);
	position.y = conf.child("stats").attribute("pos_y").as_int(0);
	hp_hearts = { 6,6 };

	return ret;
}

// Called before the first frame
bool Player::Start()
{
	bool ret = true;
	//changeResolution = false;
	attacker = false;

	//ANIMATION ---------------
	direction = DOWN;
	state = L_IDLE;
	anim_state = L_IDLE;
	scale = App->win->GetScale();
	offset_x = 7;
	offset_y = 10;
	//--------------------------
	test_bole = App->tex->Load("Particles/test_bole.png");
	pos_bole = position;
	/*timetoplay.Start();*/
	canSwitchMap = true;
	collision_feet = App->collision->AddCollider({ position.x - offset_x, position.y - offset_y, 14, 14 }, COLLIDER_PLAYER, this);
	App->input_manager->AddListener(this);
	game_timer.Start();
	return ret;
}

bool Player::PreUpdate()
{
	BROFILER_CATEGORY("PreUpdate_Player", Profiler::Color::RosyBrown)
	bool ret = true;

	return ret;
}

bool Player::Update(float dt)
{
	/*sprintf_s(buffer, 30, "%.f seconds of playtime", game_timer.ReadSec());
	std::string var = std::to_string(game_timer.ReadSec()) + "seconds of playtime";
	time->Write(var.c_str());
	var.clear();*/
	BROFILER_CATEGORY("DoUpdate_Player", Profiler::Color::Red);
	//pos_bole.x += 1; 
	bool ret = true;
	//if you dead, you appear on the Link House
	if (hp_hearts.y == 0)
	{
		score = 0;
		hp_hearts = { hp_hearts.x, hp_hearts.x };

		gems = 0;
		bombs = 0;
		arrows = 0;
		
		// SWITCH MAPS ------------------
		if (App->scene->IdMap() == 2)
		{
			App->scene->switch_map = App->scene->id_map;
		}
		else if (App->scene->IdMap() == 4)
		{
			App->scene->switch_map = App->scene->id_map;
		}
		else if (App->scene->IdMap() == 5)
		{
			App->scene->switch_map = App->scene->id_map;
		}
	}

	// STATE MACHINE ------------------
	if (App->scene->gamestate == INGAME)
	{
		//CHARGE BAR --------------
		if (equiped_item != nullptr && equiped_item == hook && hook->in_use == false)
		{
			if ((App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::BUTTON_B) == EVENTSTATE::E_REPEAT) && charge <= 34)
			{
				charge++;
			}
			else if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP)
			{
				state = L_HOOKTHROWN;
				anim_state = L_IDLE;
				ThrowHookshot(charge);
			}
			else if (charge > 0)
			{
				charge--;
			}
		}
		if (equiped_item != nullptr && equiped_item == bombmanager && App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP && bombs>0)
		{
			bombmanager->Drop(position);
			bombs--;
			App->audio->PlayFx(6);
		}

		switch (state)
		{
		case L_IDLE:
		{
			Idle();
			break;
		}
		case L_WALKING:
		{
			Walking();
			break;
		}
		case L_ATTACKING:
		{
			Attack();
			break;
		}
		case L_HIT:
		{
			Hit();
			break;
		}
		case L_INTERACTING:
		{
			Interact();
			break;
		}
		case L_HOOKTHROWN:
		{
			Hooking();
			break;
		}
		default:
		{
			break;
		}

		}
	}

	/*else if (gamestate == TIMETOPLAY)
	{

		if (timetoplay.ReadSec() > 1 || dialog == nullptr)
		{
			gamestate = INGAME;
		}
		else
		{
			if (dialog->end && timetoplay.ReadSec() > 0.2f)
			{
				gamestate = INGAME;
				delete dialog;
				dialog = nullptr;
			}
		}
	}*/
	else if (App->scene->gamestate == GAMEOVER)
	{
		if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_DOWN || App->input_manager->EventPressed(INPUTEVENT::BUTTON_START) == EVENTSTATE::E_DOWN ||
			App->input_manager->EventPressed(INPUTEVENT::BUTTON_A) == EVENTSTATE::E_DOWN)
		{
			App->scene->gamestate = INGAME;
		}
	}

	//Collision follow the player
	collision_feet->SetPos(position.x - offset_x, position.y - offset_y);

	return ret;
}

void Player::Draw()
{

	//Draw player
	App->anim_manager->Drawing_Manager(anim_state, direction, position, LINK); 
	//App->render->Blit(test_bole, pos_bole.x, pos_bole.y);


}

bool Player::CleanUp()
{
	bool ret = true;

	return ret;
}

bool Player::Save()
{
	App->entity_elements->XML.child("config").child("Link").child("stats").attribute("hp").set_value(hp);
	App->entity_elements->XML.save_file("config.xml");
	return true;
}

void Player::OnCollision(Collider* c1, Collider* c2)
{
	if (App->scene->combat == false)
	{
		if (c1 != nullptr && c2 != nullptr)
		{
			if (c2->callback != nullptr)
			{
				if (c1 == collision_attack && c2->type == COLLIDER_DYNOBJECT && c2->callback->name != "chest" && c2->callback->name != "bigchest")
				{
					iPoint pos_dyn = App->map->WorldToMap(c2->callback->position.x, c2->callback->position.y);
					//srand(time(NULL)); 		int canDrop = rand() % 5 + 1;
					int canDrop = 1;
					if (canDrop == 1)
					{
						iPoint position;
						position.x = c2->callback->position.x + 4;
						position.y = c2->callback->position.y;
						DynamicObjects* temp = (DynamicObjects*)c2->callback;
						App->scene->items.push_back(App->entity_elements->CreateItem(temp->item_id, position));

					}

					App->map->EditCost(pos_dyn.x, pos_dyn.y, 0);
					App->map->EditCost(pos_dyn.x + 1, pos_dyn.y, 0);
					App->map->EditCost(pos_dyn.x, pos_dyn.y + 1, 0);
					App->map->EditCost(pos_dyn.x + 1, pos_dyn.y + 1, 0);


					App->entity_elements->DeleteDynObject((DynamicObjects*)c2->callback);
				}
			}

			if (c1 == collision_interact && c2->type == COLLIDER_DYNOBJECT && c2->callback != nullptr)
			{
				if (c2->callback->name == "chest" || c2->callback->name == "bigchest")
				{
					iPoint pos_dyn = App->map->WorldToMap(c2->callback->position.x, c2->callback->position.y);
					App->audio->PlayFx(15);
					//create item
					iPoint position;
					position.x = c2->callback->position.x + c2->rect.w*0.5;
					position.y = c2->callback->position.y + c2->rect.h;
					DynamicObjects* temp = (DynamicObjects*)c2->callback;
					App->scene->items.push_back(App->entity_elements->CreateItem(temp->item_id, position));
					score += 75;
					App->entity_elements->DeleteDynObject((DynamicObjects*)c2->callback);
				}
			}


			if (c1 == collision_feet && c2->type == COLLIDER_ITEM && c2->callback != nullptr)
			{
				Item* temp = (Item*)c2->callback;
				if (temp->pickable == true)
				{
					if (c2->callback->name == "rupee")
					{
						App->audio->PlayFx(4);
						gems++;
					}
					if (c2->callback->name == "bomb")
					{
						//First time picking a bomb
						if (bombmanager == nullptr)
						{
						}
						bombs++;
						score += 5;
					}
					if (c2->callback->name == "hookshot")
					{
						//First time picking a hookshot
						if (hook == nullptr)
						{
						}
					}
					if (c2->callback->name == "heart")
					{
					}
					//Delete item when picked
					App->entity_elements->DeleteItem((Item*)c2->callback);
				}
			}
			
			if (c1 == collision_feet && c2->type == COLLIDER_ENEMY) //if green soldier attack you
			{
				if (state != L_HIT && invincible_timer.ReadSec() >= 1)
				{
					App->audio->PlayFx(13);
					state = L_HIT;
					anim_state = L_IDLE;
					hurt_timer.Start();
					invincible_timer.Start();
					hp_hearts.y--;
					dir_hit = c2->callback->direction;
					prev_position = position;
				}
			}

			if (c1 == collision_feet && c2->type == COLLIDER_SWITCH_MAP)
			{
				if (canSwitchMap == false)
				{
					canSwitchMap = true;
				}
				else
				{
					iPoint temp_meta = App->map->WorldToMap(position.x, position.y);
					MapLayer* meta_ = App->map->data.layers[1];
					int id_meta = meta_->Get(temp_meta.x, temp_meta.y);
					for (int i = 0; i < App->map->directMap.size(); i++)
					{
						if (App->map->directMap[i].id_tile == id_meta)
						{
							canSwitchMap = false;
							App->scene->switch_map = App->map->directMap[i].id_map;
							App->scene->newPosition = App->map->directMap[i].position;
						}
					}
				}
			}

			if (c1 == collision_feet && c2->type == COLLIDER_BOMB)
			{
				if (hp_hearts.y > 0)
				{
					GetDamage();
				}
			}
		}
	}
}

bool Player::Camera_inside() 
{
	//256x224
	if (camera_follow == true)
	{
		iPoint camera_pos(-App->render->camera.x / 2, -App->render->camera.y / 2);
		iPoint size_map = App->map->MapToWorld(App->map->data.width, App->map->data.height);
		if (direction == UP)
		{
			if (camera_pos.y == 0)
			{
				return false;
			}
			else
			{
				if (position.y > size_map.y - (App->win->GetHeight() / scale) / 2)
				{
					return false;
				}
			}
		}
		if (direction == DOWN)
		{
			if (camera_pos.y + (App->win->GetHeight() / scale) >= size_map.y)
			{
				return false;
			}
			else
			{
				if (position.y < (App->win->GetHeight() / scale) / 2)
				{
					return false;
				}
			}
		}
		if (direction == LEFT)
		{
			if (camera_pos.x <= 0)
			{
				camera_pos.x = 0;
				return false;
			}
			else
			{
				if (position.x > size_map.x - (App->win->GetWidth() / scale) / 2)
				{
					return false;
				}
			}
		}
		if (direction == RIGHT)
		{
			if (camera_pos.x + (App->win->GetWidth() / scale) >= size_map.x)
			{
				return false;
			}
			else
			{
				if (position.x < (App->win->GetWidth() / scale) / 2)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool Player::Camera_inside(iPoint pos) 
{
	//256x224
	if (camera_follow == true)
	{
		iPoint camera_pos(-App->render->camera.x / 2, -App->render->camera.y / 2);
		iPoint size_map = App->map->MapToWorld(App->map->data.width, App->map->data.height);
		if (direction == UP)
		{
			if (camera_pos.y - pos.y <= 0)
			{
				return false;
			}
			else
			{
				if (position.y > size_map.y - (App->win->GetHeight() / scale) / 2)
				{
					return false;
				}
			}
		}
		if (direction == DOWN)
		{
			if (camera_pos.y + pos.y + (App->win->GetHeight() / scale) >= size_map.y)
			{
				return false;
			}
			else
			{
				if (position.y < (App->win->GetHeight() / scale) / 2)
				{
					return false;
				}
			}
		}
		if (direction == LEFT)
		{
			if (camera_pos.x - pos.x <= 0)
			{
				return false;
			}
			else
			{
				if (position.x > size_map.x - (App->win->GetWidth() / scale) / 2)
				{
					return false;
				}
			}
		}
		if (direction == RIGHT)
		{
			if (camera_pos.x + pos.x + (App->win->GetWidth() / scale) >= size_map.x)
			{
				return false;
			}
			else
			{
				if (position.x < (App->win->GetWidth() / scale) / 2)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}
	return true;
}

//STATE MACHINE ---------------------------------------------------------------------
bool Player::Idle()
{
	if (App->input->GetKey(SDL_SCANCODE_B) == KEY_DOWN)
	{
		pos_bole = position;
		//App->particlemanaher->CreateFollow_P(nullptr, nullptr App->particlemanaher->texture, &pos_bole, 20, true);
	}
	if (App->input->GetKey(SDL_SCANCODE_V) == KEY_DOWN)
	{
		pos_bole = position;
	}
	//TEST MOVE LINK
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MLEFT) == EVENTSTATE::E_REPEAT ||
		App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MDOWN) == EVENTSTATE::E_REPEAT ||
		App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MRIGHT) == EVENTSTATE::E_REPEAT ||
		App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MUP) == EVENTSTATE::E_REPEAT)
	{
		state = L_WALKING;
		anim_state = L_WALKING;
	}

	else if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
	{
		state = L_ATTACKING;
		anim_state = L_ATTACKING;
		current_animation = App->anim_manager->GetAnimation(anim_state, direction, LINK);
		current_animation->Reset();
	}

	else if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
	{
		state = L_INTERACTING;
		anim_state = L_IDLE;
		//current_animation = App->anim_manager->GetAnimation(state, direction, 0);
		//current_animation->Reset();

	}

	else
	{
		state = L_IDLE;
		anim_state = L_IDLE;
	}

	return true;
}

bool Player::Walking()
{
	walking = false;
	Move();

	if (walking == false)
	{
		state = L_IDLE;
		anim_state = L_IDLE;
	}

	else if (App->input->GetKey(SDL_SCANCODE_E) == KEY_DOWN)
	{
		state = L_ATTACKING;
		anim_state = L_ATTACKING;
		current_animation = App->anim_manager->GetAnimation(anim_state, direction, LINK);
		current_animation->Reset();
	}

	else if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_DOWN)
	{
		state = L_INTERACTING;
		anim_state = L_IDLE;
		//current_animation = App->anim_manager->GetAnimation(state, direction, 0);
		//current_animation->Reset();

	}

	else
	{
		state = L_WALKING;
		anim_state = L_WALKING;
	}
	return false;
}

bool Player::Move()
{
	int keysuse = GetnuminputUse();
	if (keysuse > 0)
	{
		App->particlemanaher->Group_Follow.begin()._Ptr->_Myval->active = true;
	}
	else
	{
		App->particlemanaher->Group_Follow.begin()._Ptr->_Myval->active = false;
	}
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MLEFT) == EVENTSTATE::E_REPEAT)
	{
		direction = LEFT;
		//int temp = App->map->MovementCost(position.x - speed, position.y, LEFT
		int temp = App->map->MovementCost(collision_feet->rect.x - speed, collision_feet->rect.y, collision_feet->rect.w, collision_feet->rect.h, LEFT);
		if (temp == T_CONTINUE)
		{
			if (Camera_inside())
				App->render->camera.x += speed * scale;
			position.x -= speed;
		}
		if (keysuse == 1) //if you pres a key left and up this if will do that dont move more fast
		{
			if (temp == T_UP)//up
			{
				direction = UP;
				if (Camera_inside())
					App->render->camera.y += speed * scale;
				position.y -= speed;
				direction = LEFT;
			}
			if (temp == T_DOWN)//down
			{
				direction = DOWN;
				if (Camera_inside())
					App->render->camera.y -= speed * scale;
				position.y += speed;
				direction = LEFT;
			}
		}

		walking = true;
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MRIGHT) == EVENTSTATE::E_REPEAT)
	{
		direction = RIGHT;
		//int temp = App->map->MovementCost(position.x + (speed + width), position.y, RIGHT);
		int temp = App->map->MovementCost(collision_feet->rect.x + collision_feet->rect.w + speed, collision_feet->rect.y, collision_feet->rect.w, collision_feet->rect.h, RIGHT);
		if (temp == T_CONTINUE)
		{
			if (Camera_inside())
				App->render->camera.x -= speed * scale;
			position.x += speed;
		}
		if (keysuse == 1)
		{
			if (temp == T_UP)//up
			{
				direction = UP;
				if (Camera_inside())
					App->render->camera.y += speed * scale;
				position.y -= speed;
				direction = RIGHT;
			}
			if (temp == T_DOWN)//down
			{
				direction = DOWN;
				if (Camera_inside())
					App->render->camera.y -= speed * scale;
				position.y += speed;
				direction = RIGHT;
			}
		}
		walking = true;
	}

	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MUP) == EVENTSTATE::E_REPEAT)
	{
		direction = UP;
		//int temp = App->map->MovementCost(position.x, position.y - speed, UP);
		int temp = App->map->MovementCost(collision_feet->rect.x, collision_feet->rect.y - speed, collision_feet->rect.w, collision_feet->rect.h, UP);
		if (temp == T_CONTINUE)
		{
			if (Camera_inside())
				App->render->camera.y += speed * scale;
			position.y -= speed;
		}
		if (keysuse == 1)
		{
			if (temp == T_LEFT)//left
			{
				direction = LEFT;
				if (Camera_inside())
					App->render->camera.x += speed * scale;
				position.x -= speed;
				direction = UP;
			}
			if (temp == T_RIGHT)//right
			{
				direction = RIGHT;
				if (Camera_inside())
					App->render->camera.x -= speed * scale;
				position.x += speed;
				direction = UP;
			}
		}
		walking = true;
	}

	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MDOWN) == EVENTSTATE::E_REPEAT)
	{
		direction = DOWN;
		//int temp = App->map->MovementCost(position.x, position.y + (speed + height), DOWN);
		int temp = App->map->MovementCost(collision_feet->rect.x, collision_feet->rect.y + collision_feet->rect.h + speed, collision_feet->rect.w, collision_feet->rect.h, DOWN);
		if (temp == T_CONTINUE)
		{
			if (Camera_inside())
				App->render->camera.y -= speed * scale;
			position.y += speed;
		}
		if (keysuse == 1)
		{
			if (temp == T_LEFT)//left
			{
				direction = LEFT;
				if (Camera_inside())
					App->render->camera.x += speed * scale;
				position.x -= speed;
				direction = DOWN;
			}
			if (temp == T_RIGHT)//right
			{
				direction = RIGHT;
				if (Camera_inside())
					App->render->camera.x -= speed * scale;
				position.x += speed;
				direction = DOWN;
			}
		}
		walking = true;
	}

	//Set the actual floor of the player
	if (App->map->data.layers.size() >= 3) // Only Victory Road has 3 layers
	{
		GetfloorLvl(position);
	}

	return walking;
}

bool Player::Attack()
{
	if (attacker)
	{
		if (current_animation->Finished())
		{
			App->collision->EraseCollider(collision_attack);
			attacker = false;
			current_animation->Reset();
			current_animation = nullptr;
			state = L_IDLE;
			anim_state = L_IDLE;
		}
	}
	else
	{
		attacker = true;
		if (direction == UP)
		{
			App->audio->PlayFx(5);
			collision_attack = App->collision->AddCollider({ position.x - 4, position.y - offset_y - 16, 8, 20 }, COLLIDER_SWORD, this);
		}
		else if (direction == RIGHT)
		{
			App->audio->PlayFx(5);
			collision_attack = App->collision->AddCollider({ position.x + 3, position.y - 8, 20, 8 }, COLLIDER_SWORD, this);
		}
		else if (direction == DOWN)
		{
			App->audio->PlayFx(5);
			collision_attack = App->collision->AddCollider({ position.x - 4, position.y - 2, 8, 20}, COLLIDER_SWORD, this);
		}
		else if (direction == LEFT)
		{
			App->audio->PlayFx(5);
			collision_attack = App->collision->AddCollider({ position.x - 22, position.y - 7, 20, 8 }, COLLIDER_SWORD, this);
		}
	}
	return true;
}

bool Player::Interact()
{
	if (interaction)
	{
		if (timer.ReadSec() >= 0.3) // change to animation.finished
		{
			//if (current_animation->Finished())
			//{
			App->collision->EraseCollider(collision_interact);
			interaction = false;
			//current_animation->Reset();
			//current_animation = nullptr;
			state = L_IDLE;
			anim_state = L_IDLE;
			//}
		}
	}
	else
	{
		timer.Start();
		interaction = true;
		if (direction == UP)
		{
			collision_interact = App->collision->AddCollider({ position.x - 8, position.y - 14, 16, 5 }, COLLIDER_PLAYER, this);
		}
		else if (direction == RIGHT)
		{
			collision_interact = App->collision->AddCollider({ position.x + offset_x - 1, position.y - offset_y - 1, 5, 16 }, COLLIDER_PLAYER, this);
		}
		else if (direction == DOWN)
		{
			collision_interact = App->collision->AddCollider({ position.x - 8, position.y + 3, 16, 5 }, COLLIDER_PLAYER, this);
		}
		else if (direction == LEFT)
		{
			collision_interact = App->collision->AddCollider({ position.x - offset_x - 4, position.y - offset_y - 1, 5, 16 }, COLLIDER_PLAYER, this);
		}
	}
	return true;
}

bool Player::Hooking()
{
	if (hook != nullptr)
	{
		//collider follows the hookshot
		hook->collision->SetPos(hook->position.x - hook->offset_x, hook->position.y - hook->offset_y);
		HookState stat = hook->GetState();

		if (hook->actual_range_pos < hook->range)
		{
			if (stat == MISS)
			{
				HookState stat = hook->ReachObjective(actual_floor);
				KeepGoing();
				hook->actual_range_pos++;
			}
			else if (hook->GetState() == TARGET)
			{
				MoveTo(hook->position);
			}
			else if (hook->GetState() == OBSTACLE)
			{
				PickUpHook();
			}
		}

		else
		{
			PickUpHook();
		}
	}

	return true;
}

bool Player::Hit()
{
	if (collision_attack != nullptr)
	{
		collision_attack->to_delete = true;
	}
	if (hurt_timer.ReadSec() >= 0.2)
	{
		state = L_IDLE;
		anim_state = L_IDLE;
		return true;
	}
	if (hp <= 0)
	{
		state = L_DYING;
		anim_state = L_IDLE;
		return true;
	}

	if (dir_hit == UP)
	{
		if (App->map->MovementCost(collision_feet->rect.x, collision_feet->rect.y - 3, collision_feet->rect.w, collision_feet->rect.h, DOWN) == 0)
		{
			if (Camera_inside())
				App->render->camera.y += 3 * scale;

			position.y -= 3;
		}
	}
	else if (dir_hit == DOWN)
	{
		if (App->map->MovementCost(collision_feet->rect.x, collision_feet->rect.y + collision_feet->rect.h + 2.5, collision_feet->rect.w, collision_feet->rect.h, UP) == 0)
		{
			if (Camera_inside())
				App->render->camera.y -= 3 * scale;

			position.y += 3;
		}
	}
	else if (dir_hit == LEFT)
	{
		if (App->map->MovementCost(collision_feet->rect.x - 3, collision_feet->rect.y, collision_feet->rect.w, collision_feet->rect.h, RIGHT) == 0)
		{
			if (Camera_inside())
				App->render->camera.x += 3 * scale;

			position.x -= 3;
		}
	}
	else if (dir_hit == RIGHT)
	{
		if (App->map->MovementCost(collision_feet->rect.x + collision_feet->rect.w + 3, collision_feet->rect.y, collision_feet->rect.w, collision_feet->rect.h, LEFT) == 0)
		{
			if (Camera_inside())
				App->render->camera.x -= 3 * scale;

			position.x += 3;
		}
	}


	return true;
}

//----------------------------------------------------------------------------------

//EQUIP, UNEQUIP AND USE ITEMS
bool Player::Equip(Weapon* to_equip)
{
	if (to_equip != nullptr)
	{
		if (equiped_item == nullptr && to_equip->equipable == true)
		{
			equiped_item = to_equip;
			equiped_item->equiped = true;
			LOG("Equiped %s", equiped_item->name.c_str());
			return true;
		}
	}

	LOG("Can't equip item");
	return false;
}

bool Player::Unequip()
{
	bool ret = false;
	if (equiped_item == nullptr)
	{
		LOG("Nothing equiped");
	}
	else
	{
		if (equiped_item->in_use == false)
		{
			LOG("Unequiped %s", equiped_item->name.c_str());
			equiped_item->equiped = false;
			equiped_item = nullptr;
			ret = true;
		}
	}
	return ret;
}

//HOOKSHOT
void Player::KeepGoing()
{
	if (hook != nullptr)
	{
		switch (direction)
		{
		case UP:
			hook->position.y -= hook->speed;
			break;
		case DOWN:
			hook->position.y += hook->speed;
			break;
		case LEFT:
			hook->position.x -= hook->speed;
			break;
		case RIGHT:
			hook->position.x += hook->speed;
			break;
		default:
			break;
		}
	}
}

void Player::PickUpHook()
{
	if (hook != nullptr)
	{
		switch (direction)
		{
		case UP:
			hook->position.y += hook->speed;
			if (hook->position.y + hook->offset_y >= collision_feet->rect.y)
			{
				hook->Reset();
				state = L_IDLE;
				anim_state = L_IDLE;
			}
			break;
		case DOWN:
			hook->position.y -= hook->speed;
			if (hook->position.y <= collision_feet->rect.y + collision_feet->rect.h)
			{
				hook->Reset();
				state = L_IDLE;
				anim_state = L_IDLE;
			}
			break;
		case LEFT:
			hook->position.x += hook->speed;
			if (hook->position.x + hook->offset_x >= collision_feet->rect.x)
			{
				hook->Reset();
				state = L_IDLE;
				anim_state = L_IDLE;
			}
			break;
		case RIGHT:
			hook->position.x -= hook->speed;
			if (hook->position.x <= collision_feet->rect.x + collision_feet->rect.w)
			{
				hook->Reset();
				state = L_IDLE;
				anim_state = L_IDLE;
			}
			break;
		default:
			break;
		}
	}

}

void Player::MoveTo(const iPoint& pos)
{
	switch (direction)
	{
	case UP:
	{
		//int temp = App->map->MovementCost(position.x, position.y - hook->speed, UP);
		if (Camera_inside())
			App->render->camera.y += hook->speed * scale;

		position.y -= hook->speed;

		if (hook->position.y >= position.y)
		{
			hook->Reset();
			state = L_IDLE;
			anim_state = L_IDLE;
		}
		break;
	}

	case DOWN:
	{
		//int temp = App->map->MovementCost(position.x, position.y + (hook->speed + height), DOWN
		if (Camera_inside())
			App->render->camera.y -= hook->speed * scale;
		position.y += hook->speed;

		if (hook->position.y <= position.y)
		{
			hook->Reset();
			state = L_IDLE;
			anim_state = L_IDLE;
		}
		break;
	}

	case LEFT:
	{
		//int temp = App->map->MovementCost(position.x - hook->speed, position.y, LEFT);

		if (Camera_inside())
			App->render->camera.x += hook->speed * scale;
		position.x -= hook->speed;

		if (hook->position.x >= position.x)
		{
			hook->Reset();
			state = L_IDLE;
			anim_state = L_IDLE;
		}
		break;
	}

	case RIGHT:
	{
		//int temp = App->map->MovementCost(position.x + (hook->speed + width), position.y, RIGHT

		if (Camera_inside())
			App->render->camera.x -= hook->speed * scale;
		position.x += hook->speed;

		if (hook->position.x <= position.x)
		{
			hook->Reset();
			state = L_IDLE;
			anim_state = L_IDLE;
		}
		break;
	}

	default:
		break;
	}
}

void Player::ThrowHookshot(uint charge)
{
	if (hook != nullptr)
	{
		hook->in_use = true;
		//CHECK DIRECTION
		if (direction == UP)
		{
			iPoint pos(position.x, position.y - 3);
			hook->SetPos(pos);
			hook->offset_x = 6;
			hook->offset_y = 4;
			hook->collision = App->collision->AddCollider({ pos.x - hook->offset_x, pos.y - hook->offset_y, 12, 8 }, COLLIDER_HOOKSHOT, hook);
			hook->direction = UP;

		}
		else if (direction == RIGHT)
		{
			iPoint pos(position.x, position.y - 3);
			hook->SetPos(pos);
			hook->offset_x = 4;
			hook->offset_y = 6;
			hook->collision = App->collision->AddCollider({ pos.x + offset_x, pos.y - hook->offset_y, 8, 12 }, COLLIDER_HOOKSHOT, hook);
			hook->direction = RIGHT;
		}
		else if (direction == DOWN)
		{
			iPoint pos(position.x, position.y);
			hook->SetPos(pos);
			hook->offset_x = 6;
			hook->offset_y = 4;
			hook->collision = App->collision->AddCollider({ pos.x - hook->offset_x, pos.y + hook->offset_y, 12, 8 }, COLLIDER_HOOKSHOT, hook);
			hook->direction = DOWN;
		}
		else if (direction == LEFT)
		{
			iPoint pos(position.x, position.y - 3);
			hook->SetPos(pos);
			hook->offset_x = 4;
			hook->offset_y = 6;
			hook->collision = App->collision->AddCollider({ pos.x - hook->offset_x, pos.y - hook->offset_y, 8, 12 }, COLLIDER_HOOKSHOT, hook);
			hook->direction = LEFT;
		}

		//SET MAX RANGE
		hook->SetRange((float)charge);
	}

}

//-----------------

//UTILITY METHODS ----------------------------------------------------------------
void Player::OnInputCallback(INPUTEVENT action, EVENTSTATE e_state)
{

	switch (action)
	{
		if (App->scene->gamestate == INGAME)
		{
	case BUTTON_X:
	{
		if (e_state == E_DOWN && state != L_HOOKTHROWN)
		{
			state = L_ATTACKING;
			anim_state = L_ATTACKING;
			current_animation = App->anim_manager->GetAnimation(anim_state, direction, LINK);
			current_animation->Reset();
		}
		break;
	}
	case BUTTON_A:
	{
		if (e_state == E_DOWN && state != L_HOOKTHROWN)
		{
		}
		break;
	}

	case BUTTON_B:
		if (equiped_item != nullptr && equiped_item == hook && hook->in_use == false)
		{
			if (e_state == E_UP)
			{
				state = L_HOOKTHROWN;
				anim_state = L_IDLE;
				ThrowHookshot(charge);
			}
		}
		else if (bombmanager != nullptr && equiped_item == bombmanager && bombs > 0)
		{
			if (e_state == E_UP)
			{
				bombmanager->Drop(position);
				bombs--;
			}
		}
		break;
		}

	case BUTTON_START:
	{
		if (e_state == E_DOWN)
		{
			if (App->scene->combat == false)
			{
				if (App->scene->inventory)
				{
					App->audio->PlayFx(2);
				}
				else
				{
					App->audio->PlayFx(3);
				}

				App->scene->switch_menu = true;
				App->scene->gamestate = INMENU;
			}
			if (App->scene->gamestate == INMENU)
			{

				App->scene->gamestate = INGAME;
			}
			break;
		}
	}

	}
}

int Player::GetnuminputUse()
{
	int ret = 0;
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MLEFT) == EVENTSTATE::E_REPEAT)
	{
		ret++;
	}
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MDOWN) == EVENTSTATE::E_REPEAT)
	{
		ret++;
	}
	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MRIGHT) == EVENTSTATE::E_REPEAT)
	{
		ret++;
	}
	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT || App->input_manager->EventPressed(INPUTEVENT::MUP) == EVENTSTATE::E_REPEAT)
	{
		ret++;
	}
	return ret;
}

void Player::GetfloorLvl(iPoint pos)
{
	const MapLayer* meta_layer = App->map->data.layers[2];
	iPoint map_pos = App->map->WorldToMap(pos.x, pos.y);
	int player_lvl = meta_layer->Get(map_pos.x, map_pos.y);

	const TileSet* tileset = App->map->data.tilesets[1];
	int first_floor = tileset->firstgid + 1; // RED TILE
	int second_floor = tileset->firstgid + 2; // YELLOW TILE
	int third_floor = tileset->firstgid ; // GREEN TILE

	if (first_floor == player_lvl)
	{
		actual_floor = 0;
	}
	else if (second_floor == player_lvl)
	{
		actual_floor = 1;
	}
	else if (third_floor == player_lvl)
	{
		actual_floor = 2;
	}
}

void Player::GetDamage()
{
	if (hp_hearts.y>0)
		hp_hearts.y--;
}

void Player::SetState(LinkState set)
{
	if (set >= L_IDLE && set <= L_INTERACTING)
	{
		state = set;
	}
}

LinkState Player::GetState() const
{
	return state;
}

void Player::SetAnimState(LinkState anim)
{
	if (anim >= L_IDLE && anim <= L_INTERACTING)
	{
		anim_state = anim;
	}
}

bool Player::CameraisIn() //Comprovate if the camera is inside the Map
{
	bool ret = false;
	iPoint camera_pos(-App->render->camera.x / 2, -App->render->camera.y / 2);
	iPoint size_map = App->map->MapToWorld(App->map->data.width, App->map->data.height);
	if (camera_pos.x < 0)
	{
		camera_pos.x = 0;
		ret = true;
	}
	if (camera_pos.x > size_map.x - (App->win->GetWidth() / scale) / 2)
	{
		camera_pos.x = size_map.x - (App->win->GetWidth() / scale) / 2;
		ret = true;
	}
	if (camera_pos.y < 0)
	{
		camera_pos.y = 0;
		ret = true;
	}
	if (camera_pos.y > size_map.y - (App->win->GetHeight() / scale) / 2)
	{
		camera_pos.y = size_map.y - (App->win->GetHeight() / scale) / 2;
		ret = true;
	}
	return ret;
}
