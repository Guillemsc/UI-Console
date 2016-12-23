#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Textures.h"
#include "j1Fonts.h"
#include "j1Input.h"
#include "j1Gui.h"

// Class Gui -------------------------
// -----------------------------------

j1Gui::j1Gui() : j1Module()
{
	name.create("gui");
}

// Destructor
j1Gui::~j1Gui()
{}

// Called before render is available
bool j1Gui::Awake(pugi::xml_node& conf)
{
	LOG("Loading GUI atlas");
	bool ret = true;

	atlas_file_name = conf.child("atlas").attribute("file").as_string("");

	return ret;
}

// Called before the first frame
bool j1Gui::Start()
{
	// Load atlas
	if (atlas == nullptr)
		atlas = App->tex->Load(atlas_file_name.GetString());

	if (atlas != nullptr)
		return true;

	return true;
}

// ---------------------------------------------------------------------
// Update all UI_Elements
// ---------------------------------------------------------------------
bool j1Gui::Update(float dt)
{
	// Start -------------------------------------------------
	if (start)
	{
		// Put all always top elements to top ----
		p2List<UI_Element*> always_top;
		GetAlwaysTopElements(always_top);

		for (int i = 0; i < always_top.count(); i++)
			always_top[i]->layer = elements_list.Count() + i;
		
		ReorderElements();
		// ---------------------------------------

		start = false;
	}

	// -------------------------------------------------------

	// Debug
	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
	{
		debug = !debug;
	}
	
	// Update all elements in order
	p2List<UI_Element*> to_top;
	p2List<UI_Element*> to_update;

	for (p2PQueue_item<UI_Element*>* elements = App->gui->elements_list.start; elements != nullptr; elements = elements->next)
	{
		if (elements->data->enabled)
		{
			to_update.add(elements->data);

			// Debug lines --------
			if (debug)
			{
				for (int y = 0; y < elements->data->childs.count(); y++)
				{
					if (elements->data->childs[y]->enabled)
					{
						App->render->DrawLine(elements->data->rect.x + elements->data->rect.w * 0.5f,
							elements->data->rect.y + elements->data->rect.h * 0.5f,
							elements->data->childs[y]->rect.x + elements->data->childs[y]->rect.w * 0.5f,
							elements->data->childs[y]->rect.y + elements->data->childs[y]->rect.h * 0.5,
							255, 255, 255);
					}
				}
			}
			// ---------------------
		}

		//Take higher layer
		if (elements->next == nullptr)
			higher_layer = elements->priority;
	}

	// Update
	for (uint i = 0; i < to_update.count(); i++)
	{
		to_update[i]->update();
	}

	return true;
}

// Update all guis
bool j1Gui::PreUpdate()
{
	return true;
}

// Called after all Updates
bool j1Gui::PostUpdate()
{
	return true;
}

// Called before quitting
bool j1Gui::CleanUp()
{
	LOG("Freeing GUI");

	App->tex->UnLoad(atlas);

	return true;
}

const void j1Gui::GetAtlas() const
{
	App->gui->atlas = App->tex->Load(atlas_file_name.GetString());
}

// ---------------------------------------------------------------------
// Create a new Window
// ---------------------------------------------------------------------
UI_Element* j1Gui::UI_CreateWin(iPoint pos, int w, int h, bool _dinamic)
{
	UI_Window* ret = nullptr;
	ret = new UI_Window();

	if (ret != nullptr)
	{
		ret->Set(pos, w, h);
		ret->dinamic = _dinamic;

		// Layer

		ret->layer = elements_list.Count();

		// -----

		ret->type = ui_window;
		ret->parent = ret;

		elements_list.Push(ret, ret->layer);
		windows.add(ret);
	}

	return ret;
}

void j1Gui::UI_CreateWinManager(iPoint pos, int w, int h, bool _dinamic)
{
	if (window_manager != nullptr)
		return;

	UI_WindowManager* ret = nullptr;
	ret = new UI_WindowManager();

	if (ret != nullptr)
	{
		ret->Set(pos, w, h);
		ret->dinamic = _dinamic;

		// Layer

		ret->layer = elements_list.Count();

		// -----

		ret->parent = nullptr;

		elements_list.Push(ret, ret->layer);
	}
}

// ----------------------------------

void j1Gui::Tab()
{
}

// ---------------------------------------------------------------------
// Funcion to get all childs of a UI_Element
// ---------------------------------------------------------------------
void j1Gui::GetChilds(UI_Element * element, p2List<UI_Element*>& visited)
{
	p2List<UI_Element*> frontier;

	visited.add(element);

	// Add the current childs
	for (int i = 0; i < element->childs.count(); i++)
	{
		frontier.add(element->childs[i]);
	}

	// Navigate through all the childs and add them (works but needs inprovement)

	int end = 0;
	while (end < frontier.count())
	{
		for (int i = 0; i < frontier.count(); i++) // Navegate through frontier
		{
			if (visited.find(frontier[i]) == -1 && frontier[i] != element) // Check if its not visited or current
			{
				visited.add(frontier[i]); // Add it to visited
				for (int y = 0; y < frontier[i]->childs.count(); y++) // Add new childs to frontier
				{
					frontier.add(frontier[i]->childs[y]);
					end = 0;
				}
			}
			else
				end++;
		}
	}

	// ---------------------------------------
}

void j1Gui::GetAlwaysTopElements(p2List<UI_Element*>& always_top)
{
	for (p2PQueue_item<UI_Element*>* elements = App->gui->elements_list.start; elements != nullptr; elements = elements->next)
	{
		if (elements->data->always_top)
		{
			p2List<UI_Element*> childs;
			GetChilds(elements->data, childs);

			for (int i = 0; i < childs.count(); i++)
			{
				childs[i]->always_top = true;
			}
		}
	}

	for (p2PQueue_item<UI_Element*>* elements = App->gui->elements_list.start; elements != nullptr; elements = elements->next)
	{
		if (elements->data->always_top)
			always_top.add(elements->data);
	}
}

void j1Gui::ReorderElements()
{
	p2List<UI_Element*> copy;

	// Copy all elements of PQ and clean it
	while (App->gui->elements_list.Count() != 0)
	{
		UI_Element* tmp;
		App->gui->elements_list.Pop(tmp);
		copy.add(tmp);
	}

	App->gui->elements_list.Clear();

	// Place againt he elements on the PQ (now they are on the correct order)
	for (int i = 0; i < copy.count(); i++)
	{
		App->gui->elements_list.Push(copy[i], copy[i]->layer);
	}
}

// -----------------------------------
// ------------------------- Class Gui

// -----------------------------------
// Element ---------------------------

UI_Element::UI_Element()
{
}

UI_Element::~UI_Element()
{
}

bool UI_Element::update()
{
	return true;
}

// ---------------------------------------------------------------------
// Moves all the UI_Element childs with the mouse
// ---------------------------------------------------------------------
bool UI_Element::Move_Element()
{
	int ret = false;

	// Click
	if(MouseClickEnterLeftIntern() && !moving)
	{
		App->input->GetMousePosition(mouse_x, mouse_y);

		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

		//Check if there is another object on the same point
		if (CheckClickOverlap(mouse_x, mouse_y) == layer)
		{
			// Put window and childs to top
			PutWindowToTop();
			moving = true;
			ret = true;
		}	
	}

	// Move ---------------------
	if(moving)
	{
		// Get Mouse ------------

		int curr_x; int curr_y;
		App->input->GetMousePosition(curr_x, curr_y);

		curr_x -= App->render->camera.x;
		curr_y -= App->render->camera.y;

		// ----------------------

		// Get childs 
		p2List<UI_Element*> visited;
		App->gui->GetChilds(this, visited);

		// Move all childs ------
		for(uint i = 0; i < visited.count(); i++)
		{
			if (curr_x != mouse_x)
				visited[i]->rect.x -= mouse_x - curr_x;

			if (curr_y != mouse_y)
				visited[i]->rect.y -= mouse_y - curr_y;
		}

		// Update mouse stored
		for (uint i = 0; i < visited.count(); i++)
		{
			visited[i]->mouse_x = curr_x;
			visited[i]->mouse_y = curr_y;
		}

		// ----------------------
	}

	//LOG("START---------------------------");
	//LOG("Priority----------------");
	//for (p2PQueue_item<UI_Element*>* elements = App->gui->elements_list.start; elements != nullptr; elements = elements->next)
	//{
	//	LOG("%d", elements->priority);
	//}
	//LOG("Layer------------------");
	//for (p2PQueue_item<UI_Element*>* elements = App->gui->elements_list.start; elements != nullptr; elements = elements->next)
	//{
	//	LOG("%d", elements->data->layer);
	//}
	//LOG("----------------");
	//LOG("END---------------------------");

	// Release click
	if (MouseClickOutLeftIntern())
	{
		moving = false;
	}

	return ret;
}

void UI_Element::SetEnabled(bool set)
{
	enabled = set;
}

// ---------------------------------------------------------------------
// Enables or disables all the childs of an UI_Element
// ---------------------------------------------------------------------
void UI_Element::SetEnabledAndChilds(bool set)
{
	p2List<UI_Element*> visited;
	App->gui->GetChilds(this, visited);

	for (int i = 0; i < visited.count(); i++)
	{
			visited[i]->enabled = set;
	}
}


// ---------------------------------------------------------------------
// Put all elements of a window to the top of the PQ
// ---------------------------------------------------------------------
bool UI_Element::PutWindowToTop()
{
	bool ret = true;

	p2List<UI_Element*> visited;
	p2List<UI_Element*> copy;

	p2List<UI_Element*> always_top;

	// Get childs from the window parent
	App->gui->GetChilds(parent, visited);

	// Get always top elements
	App->gui->GetAlwaysTopElements(always_top);

	// Update layer
	int i = 0;
	for (; i<visited.count(); i++)
	{
		if(!visited[i]->always_top)
		visited[i]->layer = App->gui->higher_layer + i + 1;
	}

	// Update always top layer
	for (int y = 0; y < always_top.count(); y++)
	{
		always_top[y]->layer = App->gui->higher_layer + i + 1;
	}

	// Rorded the elements of the PQ
	App->gui->ReorderElements();

	return ret;
}

// ---------------------------------------------------------------------
// Detects the highest layer of a clicked point
// ---------------------------------------------------------------------
int UI_Element::CheckClickOverlap(int x, int y)
{
	p2List<int> contactors;

	// Check the UI_Elements that are in the point
	for (p2PQueue_item<UI_Element*>* elements = App->gui->elements_list.start; elements != nullptr; elements = elements->next)
	{
		if (x > elements->data->rect.x && x < elements->data->rect.x + elements->data->rect.w)
		{
			if (y > elements->data->rect.y && y < elements->data->rect.y + elements->data->rect.h)
			{
				// Check if is dinamic
				if (elements->data->dinamic && !elements->data->click_through)
				{
					// Add to check
					contactors.add(elements->data->layer);
				}
			}
		}
	}

	// Get the higher layer
	uint higher_layer;
	if (contactors.count() > 0)
	{
		higher_layer = 0;
		for (uint i = 0; i < contactors.count(); i++)
		{
			if (contactors[i] > higher_layer)
				higher_layer = contactors[i];
		}
	}

	// In case elements are not dinamics
	else
	{
		higher_layer = -1;
	}
	return higher_layer;
}

// ---------------------------------------------------------------------
// Adds a child to an UI_Element
// ---------------------------------------------------------------------
void UI_Element::AddChild(UI_Element * _child)
{
	childs.add(_child);
}

// ---------------------------------------------------------------------
// Adds both childs one to the other to avoid the overlaping check
// ---------------------------------------------------------------------
void UI_Element::AddChildBoth(UI_Element * _child)
{
	childs.add(_child);
	_child->childs.add(this);
}

// ---------------------------------------------------------------------
// Mouse check functions
// ---------------------------------------------------------------------
bool UI_Element::MouseClickEnterLeftIntern()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		int mouse_x, mouse_y;
		App->input->GetMousePosition(mouse_x, mouse_y);
		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

		if (mouse_x > rect.x && mouse_x < rect.x + rect.w)
		{
			if (mouse_y > rect.y && mouse_y < rect.y + rect.h)
			{
				clicked = true;
				return true;
			}
		}
	}
	return false;
}

bool UI_Element::MouseClickOutLeftIntern()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
	{
		if (clicked)
		{
			clicked = false;
			return true;
		}
	}
	return false;
}

void UI_Element::SetDebugColor(SDL_Color _color)
{
	color.r = _color.r; color.g = _color.g; color.b = _color.b; color.a = _color.a;
}

// -----------------------------------
// --------------------------- Element

// -----------------------------------
// Window ----------------------------

UI_Window::UI_Window()
{
}

UI_Window::~UI_Window()
{
}

bool UI_Window::update()
{
	if (App->gui->debug)
		App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, false);

	Move_Element();
	
	return true;
}

void UI_Window::Set(iPoint pos, int w, int h)
{
	rect.x = pos.x;
	rect.y = pos.y;
	rect.w = w;
	rect.h = h;

	color.r = color.g = color.b = color.a = 255;
}

// ---------------------------------------------------------------------
// Create a button linked to the current window
// ---------------------------------------------------------------------
UI_Element* UI_Window::CreateButton(iPoint pos, int w, int h, bool _dinamic)
{
	UI_Button* ret = nullptr;
	ret = new UI_Button();

	if (ret != nullptr)
	{
		ret->type = ui_button;
		ret->Set(pos, w, h);
		ret->parent = this;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = childs.count() + layer + 1;

		// ---------

		App->gui->elements_list.Push(ret, ret->layer);
		childs.add((UI_Element*)ret);
	}
	return ret;
}


// ---------------------------------------------------------------------
// Create text linked to the current window
// ---------------------------------------------------------------------
UI_Element* UI_Window::CreateText(iPoint pos, _TTF_Font * font, int spacing, bool _dinamic, uint r, uint g, uint b)
{
	UI_Text* ret = nullptr;
	ret = new UI_Text();

	if (ret != nullptr)
	{
		ret->type = ui_text;
		ret->Set(pos, font, spacing, r, g, b);
		ret->parent = this;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = childs.count() + layer + 1;

		// ---------

		App->gui->elements_list.Push(ret, ret->layer);
		childs.add((UI_Element*)ret);
	}
	return ret;
}

// ---------------------------------------------------------------------
// Create an image linked to the current window
// ---------------------------------------------------------------------
UI_Element* UI_Window::CreateImage(iPoint pos, SDL_Rect image, bool _dinamic)
{
	UI_Image* ret = nullptr;
	ret = new UI_Image();
	
	if (ret != nullptr)
	{
		ret->type = ui_image;
		ret->Set(pos, image);
		ret->parent = this;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = childs.count() + layer + 1;

		// ---------

		App->gui->elements_list.Push(ret, ret->layer);
		childs.add((UI_Element*)ret);
	}
	return ret;
}

// ---------------------------------------------------------------------
// Create a text input box to the current window
// ---------------------------------------------------------------------
UI_Element* UI_Window::CreateTextInput(iPoint pos, int w, _TTF_Font* font,bool _dinamic, uint r, uint g, uint b)
{
	UI_Text_Input* ret = nullptr;
	ret = new UI_Text_Input();

	if (ret != nullptr)
	{
		ret->type = ui_text_input;
		ret->Set(pos, w, font, r, g, b);
		ret->parent = this;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = childs.count() + layer + 1;

		// ---------

		App->gui->elements_list.Push(ret, ret->layer);
		childs.add((UI_Element*)ret);
	}
	return ret;
}

UI_Element * UI_Window::CreateScrollBar(iPoint pos, int view_w, int view_h, int scroll, int button_size, bool _dinamic)
{
	UI_Scroll_Bar* ret = nullptr;
	ret = new UI_Scroll_Bar();

	if(ret != nullptr)
	{
		ret->type = ui_scroll_bar;
		ret->Set(pos, view_w, view_h, scroll, button_size);
		ret->parent = this;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = childs.count() + layer + 1;

		// ---------

		App->gui->elements_list.Push(ret, ret->layer);
		childs.add((UI_Element*)ret);
	}

	return ret;
}

UI_Element * UI_Window::CreateColoredRect(iPoint pos, int w, int h, SDL_Color color, bool filled, bool dinamic)
{
	UI_ColoredRect* ret = nullptr;
	ret = new UI_ColoredRect();

	if (ret != nullptr)
	{
		ret->type = ui_colored_rect;
		ret->Set(pos, w, h, color, filled);
		ret->parent = this;
		ret->dinamic = dinamic;

		// Layers --

		ret->layer = childs.count() + layer + 1;

		// ---------

		App->gui->elements_list.Push(ret, ret->layer);
		childs.add((UI_Element*)ret);
	}

	return ret;
}

// -----------------------------------
// ---------------------------- Window

// -----------------------------------
// Button ----------------------------

UI_Button::UI_Button()
{
}

UI_Button::~UI_Button()
{
}

void UI_Button::Set(iPoint _pos, int w, int h)
{
	rect.x = _pos.x;
	rect.y = _pos.y;
	rect.w = w;
	rect.h = h;

	color.r = color.g = color.b = color.a = 255;
}

bool UI_Button::update()
{
	if (!enabled)
		return false;

	if (App->gui->debug)
		App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, false);

	if(print)
		App->render->Blit(App->gui->atlas, rect.x, rect.y, &curr);

	Move_Element();

	return true;
}

bool UI_Button::MouseEnter()
{
	if (!enabled)
		return false;

	int mouse_x, mouse_y;
	App->input->GetMousePosition(mouse_x, mouse_y);

	if (CheckClickOverlap(mouse_x, mouse_y) != layer)
		return false;

	mouse_x -= App->render->camera.x;
	mouse_y -= App->render->camera.y;

	if (mouse_x > rect.x && mouse_x < rect.x + rect.w)
	{
		if (mouse_y > rect.y && mouse_y < rect.y + rect.h)
		{
			if (!enter)
			{
				enter = true;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool UI_Button::MouseOut()
{
	if (!enabled)
		return false;	
	
	int mouse_x, mouse_y;
	App->input->GetMousePosition(mouse_x, mouse_y);

	if (CheckClickOverlap(mouse_x, mouse_y) != layer && !enter)
		return true;

	mouse_x -= App->render->camera.x;
	mouse_y -= App->render->camera.y;

	if (mouse_x > rect.x && mouse_x < rect.x + rect.w)
	{
		if (mouse_y > rect.y && mouse_y < rect.y + rect.h)
		{
			return false;
		}
	}

	if(enter)
	{
		enter = false;
		return true;
	}
	else
		return false;
}

bool UI_Button::MouseClickEnterLeft()
{
	if (!enabled)
		return false;

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		int mouse_x, mouse_y;
		App->input->GetMousePosition(mouse_x, mouse_y);

		if (CheckClickOverlap(mouse_x, mouse_y) != layer)
			return false;

		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

		if (mouse_x > rect.x && mouse_x < rect.x + rect.w)
		{
			if (mouse_y > rect.y && mouse_y < rect.y + rect.h)
			{
				clicked_left = true;
				return true;
			}
		}
	}
	return false;
}

bool UI_Button::MouseClickOutLeft()
{
	if (!enabled)
		return false;

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_UP)
	{
		if (clicked_left)
		{
			clicked_left = false;
			return true;
		}
	}
	return false;
}

bool UI_Button::MouseClickEnterRight()
{
	if (!enabled)
		return false;

	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN)
	{
		int mouse_x, mouse_y;
		App->input->GetMousePosition(mouse_x, mouse_y);

		if (CheckClickOverlap(mouse_x, mouse_y) != layer)
			return false;

		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

		if (mouse_x > rect.x && mouse_x < rect.x + rect.w)
		{
			if (mouse_y > rect.y && mouse_y < rect.y + rect.h)
			{
				clicked_right = true;
				return true;
			}
		}
	}
	return false;
}

bool UI_Button::MouseClickOutRight()
{
	if (!enabled)
		return false;

	if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP)
	{
		if (clicked_right)
		{
			clicked_right = false;
			return true;
		}

	}
	return false;
}

void UI_Button::AddImage(SDL_Rect rect)
{
	rect_list.add(rect);
}

void UI_Button::SetImage(int index)
{
	curr = rect_list[index];
}

// -----------------------------------
// ---------------------------- Button

// -----------------------------------
// Text ------------------------------

UI_Text::UI_Text()
{
}

UI_Text::~UI_Text()
{
}

void UI_Text::Set(iPoint _pos, _TTF_Font* _font, int _spacing, uint r, uint g, uint b)
{
	color.r = r;
	color.g = g;
	color.b = b;
	font = _font;

	rect.x = _pos.x;
	rect.y = _pos.y;
	rect.w = -1;
	rect.h = -1;

	spacing = _spacing;

	color.r = color.g = color.b = color.a = 255;
}

void UI_Text::SetText(p2SString _text)
{
	texts.clear();

	char* tmp = new char[_text.Length() + 1];
	strcpy_s(tmp, _text.Length() + 1, _text.GetString());

	char* tmp2 = tmp;

	while (*tmp2 != '\0')
	{
		p2SString string;

		for (int i = 0; *tmp2 != '\n' && *tmp2 != '\0'; tmp2++)
		{
			if (string.Length() == 0)
			{
				string.create("%c", *tmp2);
			}
			else
			{
				string.create("%s%c", string.GetString(), *tmp2);
			}
		}
		if(*tmp2 != '\0')
			tmp2++;

		string.create("%s\0", string.GetString());
		texts.add(string);

	}
}

bool UI_Text::update()
{
	if (!enabled)
		return false;
	
	int w = 0, h = 0;
	for(int i = 0; i < texts.count(); i++)
	{
		App->font->CalcSize(texts[i].GetString(), rect.w, rect.h, font);
		h += rect.h;
		if (rect.w > w)
			w = rect.w;
	}

	rect.w = w;
	rect.h = h;

	if (App->gui->debug)
		App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, false);
	
	if (print)
	{
		SDL_Texture* texture = nullptr;
		int space = 0;
		for (int i = 0; i < texts.count(); i++)
		{
			texture = App->font->Print(texts[i].GetString(), color, font);
			App->render->Blit(texture, rect.x, rect.y + space, NULL);
			space += spacing;
			App->tex->UnLoad(texture);
		}
	}

	Move_Element();

	return true;
}

// -----------------------------------
// ------------------------------ Text

// -----------------------------------
// Image -----------------------------

UI_Image::UI_Image()
{
}

UI_Image::~UI_Image()
{
}

void UI_Image::Set(iPoint _pos, SDL_Rect _image)
{
	rect.x = _pos.x;
	rect.y = _pos.y;

	image.x = _image.x;
	image.y = _image.y;
	image.w = _image.w;
	image.h = _image.h;
	rect.w = _image.w;
	rect.h = _image.h;

	color.r = color.g = color.b = color.a = 255;
}

void UI_Image::ChangeImage(SDL_Rect _rect)
{
	image = { _rect };
	rect.w = _rect.w;
	rect.h = _rect.h;
}


bool UI_Image::update()
{
	if (!enabled)
		return false;

	if (App->gui->debug)
	{
		App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, false);
	}
	
	if(print)
		App->render->Blit(App->gui->atlas, rect.x, rect.y, &image);

	

	Move_Element();

	return true;
}
// -----------------------------------
// ----------------------------- Image

UI_Text_Input::UI_Text_Input()
{
}

UI_Text_Input::~UI_Text_Input()
{
}

void UI_Text_Input::Set(iPoint pos, int w, _TTF_Font* font, uint r, uint g, uint b)
{
	rect.x = pos.x;
	rect.y = pos.y;
	rect.w = w;
	text = new UI_Text();
	text->Set(iPoint(0, 0), font, 0, r, g, b);

	// Set bar size
	App->font->CalcSize("@", bar.x, bar.h, font);
	bar.w = 1;
	rect.h = bar.h;

	color.r = color.g = color.b = color.a = 255;
}

bool UI_Text_Input::update()
{
	if (!enabled)
		return false;

	if (App->gui->debug)
		App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, false);

	p2SString test = intern_text;

	// Print
	if (print)
	{
		SetIsActive();

		// Text setup
		if (intern_text.Length() == 0 && !active)
			text->SetText("Insert Text");
		else if (intern_text.Length() == 0 && active)
			text->SetText("");

		if (test != intern_text && !pasword)
			text->SetText(intern_text);

		if (active)
		{
			// Take and print input
			if (TakeInput() || Delete())
			{
				// Update words position list
				if(!pasword)
					UpdateWordsLenght(intern_text);
				else
					SetPasword();
			}

			// Move cursor
			MoveCursor();

			DrawBar();
		}

		// Viewport -----------
		App->render->SetViewPort(rect);

			text->update();

		App->render->ResetViewPort();
		// --------------------

	}

	Move_Element();

	return true;
}


bool UI_Text_Input::TakeInput()
{
	bool ret = false;

	if (App->input->input_text.Length() > 0)
	{
		// If intern text is empty
		if (intern_text.Length() == 0)
			intern_text.create("%s", App->input->input_text.GetString());

		// Else, add word on the cursor position ---
		else
		{
			p2SString back;
			p2SString forward;

			// Prepare text --
			char* tmp = new char[intern_text.Length() + 1];
			strcpy_s(tmp, intern_text.Length() + 1, intern_text.GetString());

			char* word = tmp;
			// ---------------

			// Take backward
			for (uint i = 0; i < bar_pos; i++)
			{
				if(back.Length() > 0)
					back.create("%s%c", back.GetString(), *word);
				else
					back.create("%c", *word);

				if(i + 1 <= bar_pos)
					word++;
			}

			// Take forward
			for (uint i = bar_pos+1; i <= intern_text.Length(); i++)
			{
				if(forward.Length()>0)
					forward.create("%s%c", forward.GetString(), *word);
				else
					forward.create("%c", *word);

				word++;
			}

			// Backward + input + Forward
			intern_text.create("%s%s%s", back.GetString(), App->input->input_text.GetString(), forward.GetString());
		}
		// -----------------------------------

		text->SetText(intern_text);
		App->input->input_text.Clear(); // Clean input

		// Increase bar positon
		bar_pos++;

		ret = true;
	}

	return ret;
}

bool UI_Text_Input::Delete()
{
	bool ret = false;

	if (App->input->GetKey(SDL_SCANCODE_BACKSPACE) == KEY_DOWN)
	{
		if (intern_text.Length() > 0 && bar_pos > 0)
		{
			p2SString back;
			p2SString forward;

			char* tmp = new char[intern_text.Length() + 1];
			strcpy_s(tmp, intern_text.Length() + 1, intern_text.GetString());

			char* word = tmp;

			for (uint i = 0; i < bar_pos-1; i++)
			{
				if (back.Length() > 0)
					back.create("%s%c", back.GetString(), *word);
				else
					back.create("%c", *word);

				if (i + 1 <= bar_pos)
					word++;
			}
			word++;
			for (uint i = bar_pos + 1; i <= intern_text.Length(); i++)
			{
				if (forward.Length() > 0)
					forward.create("%s%c", forward.GetString(), *word);
				else
					forward.create("%c", *word);
				word++;
			}

			intern_text.create("%s%s", back.GetString(), forward.GetString());
			bar_pos--;

			text->SetText(intern_text);

			ret = true;
		}
	}

	return ret;
}

void UI_Text_Input::MoveCursor()
{
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
	{
		if (bar_pos > 0)
		{
			bar_pos--;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
	{
		if(bar_pos < intern_text.Length())
		{
			bar_pos++;
		}
	}

	if (bar_pos != 0)
		bar_x = words_lenght[bar_pos - 1];
	else
		bar_x = 0;
}

void UI_Text_Input::UpdateWordsLenght(p2SString l_text)
{
	words_lenght.clear();

	char* tmp = new char[l_text.Length() + 1];
	strcpy_s(tmp, l_text.Length() + 1, l_text.GetString());

	char* word = tmp;

	int acumulated = 0;
	for (uint i = 0; i < intern_text.Length(); i++)
	{
		p2SString tmp2; tmp2.create("%c", *word);

		int x, y;
		App->font->CalcSize(tmp2.GetString(), x, y, text->font);

		// Special cases (idk why this happens)
		if (*word == 'j' || *word == 'f')
			x-=2;
		acumulated += x;
		words_lenght.add(acumulated);

		word++;
	}
}

void UI_Text_Input::DrawBar()
{
	bar.x = rect.x + bar_x;
	bar.y = rect.y;

	App->render->DrawQuad(bar, color.r, color.g, color.b, color.a, true);
}

void UI_Text_Input::SetPasword()
{
	p2SString tmp;
	for (int i = 0; i < intern_text.Length(); i++)
	{
		if (tmp.Length() == 0)
			tmp.create("*");
		else
			tmp.create("%s*", tmp.GetString());
	}

	text->SetText(tmp);

	UpdateWordsLenght(tmp);
}

char* UI_Text_Input::Enter()
{
	char* ret = nullptr;

	if (App->input->GetKey(SDL_SCANCODE_KP_ENTER) == KEY_DOWN)
	{
		if (intern_text.Length() && active)
		{
			char* ret = new char[intern_text.Length() + 1];
			strcpy_s(ret, intern_text.Length() + 1, intern_text.GetString());
		}
	}

	return ret;
}


void UI_Text_Input::SetIsActive()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		int mouse_x, mouse_y;
		App->input->GetMousePosition(mouse_x, mouse_y);

		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

		//Check if there is another object on the same point
		if (CheckClickOverlap(mouse_x, mouse_y) == layer)
		{
			active = true;
		}
		else
		{
			active = false;
		}
	}
}

bool UI_Text_Input::MouseClick()
{
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		int mouse_x, mouse_y;
		App->input->GetMousePosition(mouse_x, mouse_y);

		if (CheckClickOverlap(mouse_x, mouse_y) != layer)
			return false;

		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

		if (mouse_x > rect.x && mouse_x < rect.x + rect.w)
		{
			if (mouse_y > rect.y && mouse_y < rect.y + rect.h)
			{
				return true;
			}
		}
	}
	return false;
}

UI_Scroll_Bar::UI_Scroll_Bar()
{
}

UI_Scroll_Bar::~UI_Scroll_Bar()
{
}

void UI_Scroll_Bar::Set(iPoint pos, int view_w, int view_h, int scroll, int button_size)
{
	rect.x = pos.x;
	rect.y = pos.y;
	rect.w = view_w;
	rect.h = view_h;

	viewport_rect.x = pos.x;
	viewport_rect.y = pos.y + scroll;
	viewport_rect.w = view_w;
	viewport_rect.h = view_h + scroll;

	// Button ---
	button = new UI_Button();
	button->Set(iPoint(0, 0), 17, 17);
	childs.add(button);
	SetMinMaxBar();
	button->rect.y = min_bar;
	// ----------

	color.r = color.g = color.b = color.a = 255;
}

bool UI_Scroll_Bar::update()
{
	if (!enabled)
		return false;

	Move_Element();

	SetMinMaxBar();

	bar_pos = button->rect.y;
	ScrollWindow();

	// Viewport -----------
	App->render->SetViewPort(rect);

	for (int i = 0; i < elements.count(); i++)
	{
		elements[i]->update();
		elements[i]->rect.y -= button->rect.y - bar_pos;
	}

	App->render->ResetViewPort();
	// --------------------

	// Update positions ---
	scroll -= (button->rect.y - bar_pos); //* GetSpeed(viewport_rect.h);

	viewport_rect.x = rect.x;
	viewport_rect.y = rect.y + scroll;
	button->rect.x = rect.x + rect.w + 2;
	// --------------------

	// Viewport debug -----
	if (App->gui->debug)
	{
		App->render->DrawQuad(viewport_rect, color.r, color.g, color.b, color.a, false);
		App->render->DrawQuad(rect, 255, 255, 0, 255, false);
		App->render->DrawLine(rect.x + rect.w + 10, rect.y, rect.x + rect.w + 10, rect.y + rect.h, 255, 255, 255, 255);
	}
	// --------------------

	button->update();
}

bool UI_Scroll_Bar::ScrollWindow()
{
	bool ret = false;

	if(button->MouseClickEnterLeftIntern() && !is_scrolling)
	{ 
		int mouse_x;
		App->input->GetMousePosition(mouse_x, mouse_y);

		mouse_x -= App->render->camera.x;
		mouse_y -= App->render->camera.y;

			//Check if there is another object on the same point
		if (CheckClickOverlap(mouse_x, mouse_y) == -1)
		{
				// Put window and childs to top
			PutWindowToTop();
			is_scrolling = true;
			ret = true;
		}
	}

	if(is_scrolling)
	{ 
		int curr_x; int curr_y;
		App->input->GetMousePosition(curr_x, curr_y);
		mouse_y -= App->render->camera.y;

		int a = button->rect.y;

		if (button->rect.y >= min_bar && button->rect.y <= max_bar)
		{
			if (curr_y != mouse_y)
			{
				button->rect.y -= mouse_y - curr_y;

				if (button->rect.y < min_bar)
				{
					button->rect.y = min_bar;
				}
				else if (button->rect.y > max_bar)
				{
					button->rect.y = max_bar;
				}
			}
		}

		mouse_y = curr_y;

	}

	if (button->MouseClickOutLeftIntern())
	{
		is_scrolling = false;
	}

	return ret;
}

void UI_Scroll_Bar::AddElement(UI_Element * element)
{
	elements.add(element);
}

void UI_Scroll_Bar::SetMinMaxBar()
{
	max_bar = rect.h + rect.y - button->rect.h;
	min_bar = rect.y;

	bar_distance = max_bar - min_bar;
}

float UI_Scroll_Bar::GetSpeed(int size)
{
	LOG("%f", ((float)size / (float)bar_distance));
	return ((float)size / (float)bar_distance);
}

UI_Element * UI_Scroll_Bar::AddButton(iPoint _pos, int w, int h, bool _dinamic)
{
	UI_Button* ret = nullptr;
	ret = new UI_Button();

	if (ret != nullptr)
	{
		ret->type = ui_button;
		ret->Set(_pos, w, h);
		ret->parent = nullptr;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = layer;

		// ---------

		elements.add(ret);
	}
	return ret;
}

UI_Element * UI_Scroll_Bar::AddText(iPoint pos, _TTF_Font * font, int spacing, uint r, uint g, uint b, bool _dinamic)
{
	UI_Text* ret = nullptr;
	ret = new UI_Text();

	if (ret != nullptr)
	{
		ret->type = ui_text;
		ret->Set(pos, font, spacing, r, g, b);
		ret->parent = nullptr;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = layer;

		// ---------

		elements.add(ret);
	}

	return ret;
}

UI_Element * UI_Scroll_Bar::AddImage(iPoint pos, SDL_Rect image, bool _dinamic)
{
	UI_Image* ret = nullptr;
	ret = new UI_Image();

	if (ret != nullptr)
	{
		ret->type = ui_image;
		ret->Set(pos, image);
		ret->parent = nullptr;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = layer;

		// ---------

		elements.add(ret);
	}

	return ret;
}

UI_Element * UI_Scroll_Bar::AddTextInput(iPoint pos, int w, _TTF_Font * font, uint r, uint g, uint b, bool _dinamic)
{
	UI_Text_Input* ret = nullptr;
	ret = new UI_Text_Input();

	if (ret != nullptr)
	{
		ret->type = ui_text_input;
		ret->Set(pos, w, font, r, g, b);
		ret->parent = nullptr;
		ret->dinamic = _dinamic;

		// Layers --

		ret->layer = layer;

		// ---------

		elements.add(ret);
	}

	return ret;
}

UI_WindowManager::UI_WindowManager()
{
}

UI_WindowManager::~UI_WindowManager()
{
}

void UI_WindowManager::Set(iPoint pos, int w, int h)
{
	rect.x = pos.x;
	rect.y = pos.y;
	rect.w = w;
	rect.h = h;

	max_w = rect.w / 4;

	color.r = 255; color.g = 255; color.b = 255; color.a = 255;
}

bool UI_WindowManager::update()
{
	if (!enabled)
		return false;

	// SetObjects
	if (!one_time)
	{
		SetObjects();
		one_time = true;
	}

	if (App->gui->debug)
		App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, false);
	
	if (print)
	{
		UpdateWindowObjects();
		App->render->DrawQuad(rect, 255, 255, 255, 255, false);
	}

	MinimizeOrMaximise();

	Move_Element();

	return true;
}

void UI_WindowManager::SetObjects()
{
	// Calculate spacing --
	int spacing = max_w;
	if ((max_w * App->gui->windows.count()) + (App->gui->windows.count() * 3) > rect.w)
		spacing = rect.w / App->gui->windows.count();
	// --------------------

	int acumulated_spacing = 3;
	for (int i = 0; i < App->gui->windows.count(); i++)
	{
		WindowManagerObject object({ rect.x + acumulated_spacing, rect.y + 3, spacing, rect.h - 6 }, App->gui->windows[i]);
		object.button->AddChild(this);
		object.button->layer = layer;

		windows_rects.add(object);
		acumulated_spacing += spacing + 3;
	}
}


void UI_WindowManager::UpdateWindowObjects()
{
	for (int i = 0; i < windows_rects.count(); i++)
	{
		windows_rects[i].button->update();
	}
}

void UI_WindowManager::MinimizeOrMaximise()
{
	for (int i = 0; i < windows_rects.count(); i++)
	{
		int mouse_x, mouse_y;
		App->input->GetMousePosition(mouse_x, mouse_y);
		if (windows_rects[i].button->MouseClickEnterLeftIntern() && CheckClickOverlap(mouse_x, mouse_y))
		{
			if (windows_rects[i].window->enabled)
			{
				if (windows_rects[i].window != nullptr)
					windows_rects[i].window->SetEnabledAndChilds(false);
			}
			else
			{
				if (windows_rects[i].window != nullptr)
					windows_rects[i].window->SetEnabledAndChilds(true);
			}
		}
	}
}

UI_ColoredRect::UI_ColoredRect()
{
}

UI_ColoredRect::~UI_ColoredRect()
{
}

void UI_ColoredRect::Set(iPoint pos, int w, int h, SDL_Color _color, bool filled)
{
	rect.x = pos.x;
	rect.y = pos.y;
	rect.w = w;
	rect.h = h;
	color = _color;
}

bool UI_ColoredRect::update()
{
	if (!enabled)
		return false;

	App->render->DrawQuad(rect, color.r, color.g, color.b, color.a, filled);

	Move_Element();

	return true;
}

void UI_ColoredRect::SetColor(SDL_Color _color)
{
	color = _color;
}
