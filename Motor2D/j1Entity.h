#ifndef __j1ENTITY_H__
#define __j1ENTITY_H__

#include "j1Module.h"
#include "p2Defs.h"
#include "p2List.h"

enum entity_name
{
	player
};

class Entity;
class Player;
class j1Entity : public j1Module
{
public:

	j1Entity();

	// Destructor
	virtual ~j1Entity();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	// Called before all Updates
	bool PreUpdate();

	// Called each loop iteration
	bool Update(float dt);

	// Called before all Updates
	bool PostUpdate();

	// Called before quitting
	bool CleanUp();

	Entity* CreateEntity(entity_name entity);

private:

private:
	p2List<Entity*> entity_list;

};

#endif // __j1ENTITY_H__