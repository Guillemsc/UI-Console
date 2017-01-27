#include "Animation.h"

Animation::Animation(p2List<SDL_Rect>& rects, float speed, bool loop) : speed(speed), loop(loop)
{
	for (p2List_item<SDL_Rect>* rect = rects.start; rect != nullptr; rect = rect->next)
		frames.add(rect->data);
}

Animation::~Animation()
{
}

SDL_Rect& Animation::GetAnimationFrame()
{
	curr_frame += speed;

	if (curr_frame >= frames.count())
	{
		if (loop)
			curr_frame = frames.count() - 1;
		else
			curr_frame = 0.0f;

		loops++;
	}

	return frames[(int)curr_frame];
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