#include "Animation.h"
#include "Functions.h"
#include "p2Log.h"

Animation::Animation(const char* _name, p2List<SDL_Rect>& rects, float speed, bool loop) : speed(speed), loop(loop)
{
	for (p2List_item<SDL_Rect>* rect = rects.start; rect != nullptr; rect = rect->next)
		frames.add(rect->data);

	name.create("%s", _name);
}

Animation::~Animation()
{
}

SDL_Rect& Animation::GetAnimationFrame(float dt)
{
	curr_frame += (speed * dt);

	if (curr_frame >= frames.count())
	{
		if (!loop)
			curr_frame = frames.count() - 1;
		else
			curr_frame = 0.0f;

		loops++;
	}
	return frames[(int)floor(curr_frame)];
}

SDL_Rect& Animation::GetActualFrame()
{
	return frames[(int)curr_frame];
}

float Animation::GetFrameIndex() const
{
	return (int)curr_frame;
}

SDL_Rect & Animation::GetFrame(int frame)
{
	return frames[frame];
}

void Animation::SetCurrFrame(int frame)
{
	curr_frame = frame;
}

void Animation::SetSpeed(float _speed)
{
	speed = _speed;
}

void Animation::SetLoop(bool _loop)
{
	loop = _loop;
}

bool Animation::IsCurrFrame(int frame)
{
	return (frame == curr_frame);
}

bool Animation::Finished() const
{
	return loops > 0;
}

void Animation::Reset()
{
	curr_frame = 0.0f;
	loops = 0;
}

const char* Animation::GetName()
{
	if (name.Length() > 0)
		return name.GetString();
	else
		return "";
}

Animator::Animator()
{
	for (p2List_item<Animation*>* current = animations.start; current != nullptr; current = current->next)
		delete current->data;
}

Animator::~Animator()
{
}

void Animator::AddAnimation(Animation* animation)
{
	animations.add(animation);
}

void Animator::SetAnimation(const char* name)
{
	if (current_animation != nullptr && TextCmp(current_animation->GetName(), name))
		return;

	for (p2List_item<Animation*>* current = animations.start; current!= nullptr; current = current->next)
	{
		if (TextCmp(name, current->data->GetName()))
		{
			current_animation = current->data;
			break;
		}
	}
}

Animation* Animator::GetAnimation(const char * name)
{
	Animation* ret = nullptr;

	for (p2List_item<Animation*>* current = animations.start; current != nullptr; current = current->next)
	{
		if (TextCmp(name, current->data->GetName()))
		{
			ret = current->data;
			break;
		}
	}

	return ret;
}

Animation * Animator::GetCurrentAnimation()
{
	return current_animation;
}
