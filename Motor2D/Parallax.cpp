#include "Parallax.h"
#include "p2Log.h"

#define CHUNKS 4

Parallax::Parallax(int _layer, fPoint _pos, SDL_Texture* _texture, SDL_Rect _rect, float _speed) 
	: layer(_layer), target(_pos), texture(_texture), speed(_speed)
{
	rect = { _rect.x, _rect.y, _rect.w, _rect.h };
}

Parallax::~Parallax()
{
}

void Parallax::Update(float dt, fPoint target_pos)
{
	// Start
	if (start)
	{
		chunks.add({ target.x, target.y });

		float to_print = target.x + rect.w;
		for (int i = 0; i < CHUNKS / 2; i++)
		{
			chunks.add({ to_print, target.y });
			to_print += rect.w;
		}

		to_print = target.x - rect.w;

		for (int i = 0; i < CHUNKS / 2; i++)
		{
			chunks.add({ to_print, target.y });
			to_print -= rect.w;
		}

		distance = rect.w*CHUNKS;
		target = target_pos;

		start = false;
	}

	// Move
	if (target_pos.x != target.x)
	{
		for (int i = 0; i < chunks.count(); i++)
		{
			if(target_pos.x - target.x > 0)
				chunks[i].x += ((target_pos.x - target.x + speed) * dt);
			else
				chunks[i].x += ((target_pos.x - target.x - speed) * dt);
		}
	}

	// Create or delte
	for (int i = 0; i < chunks.count();)
	{
		// Delete left
		if (chunks[i].x + rect.w < target_pos.x - (distance*0.5f))
		{
			chunks.add({ chunks[GetRightChunk()].x + rect.w, chunks[i].y });
			chunks.del(chunks.At(i));
		}

		// Delete right
		if (chunks[i].x + rect.w > target_pos.x + (distance*0.5f))
		{
			chunks.add({ chunks[GetLeftChunk()].x - rect.w, chunks[i].y });
			chunks.del(chunks.At(i));
		}
		i++;
	}

	// Print 
	for (int i = 0; i < chunks.count(); i++)
		App->scene->LayerBlit(layer, texture, { (int)floor(chunks[i].x), (int)floor(chunks[i].y) }, rect);

	target = target_pos;
}

int Parallax::GetLeftChunk()
{
	int ret = 0;
	float smallest = chunks.start->data.x;

	for (int i = 0; i < chunks.count(); i++)
	{
		if (chunks[i].x < smallest)
		{
			smallest = chunks[i].x;
			ret = i;
		}
	}

	return ret;
}

int Parallax::GetRightChunk()
{
	int ret = 0;
	float highest = chunks.start->data.x;

	for (int i = 0; i < chunks.count(); i++)
	{
		if (chunks[i].x > highest)
		{
			highest = chunks[i].x;
			ret = i;
		}
	}

	return ret;
}
