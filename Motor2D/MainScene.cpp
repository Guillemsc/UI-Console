#include "MainScene.h"
#include "j1Scene.h"
#include "p2Log.h"


MainScene::MainScene()
{
}

MainScene::~MainScene()
{
}

bool MainScene::Start()
{
	bool ret = true;

	LOG("Start MainScene");

	//pugi::xml_document doc;
	//App->LoadXML("test.xml", doc);
	//pugi::xml_node node = doc.child("test");
	//node.append_child("child").append_attribute("fuck");
	//App->SaveLoadedXML(doc, "test.xml");

	return ret;
}

bool MainScene::PreUpdate()
{
	bool ret = true;


	return ret;
}

bool MainScene::Update(float dt)
{
	bool ret = true;


	return ret;
}

bool MainScene::PostUpdate()
{
	bool ret = true;


	return ret;
}

bool MainScene::CleanUp()
{
	bool ret = true;


	return ret;
}