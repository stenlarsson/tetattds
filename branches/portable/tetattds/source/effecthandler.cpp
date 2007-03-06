#include "tetattds.h"
#include "effecthandler.h"
#include "effect.h"


EffectHandler::EffectHandler()
{
	first = NULL;
	last = NULL;
	current = NULL;
	bGet = false;
}

EffectHandler::~EffectHandler()
{
	if(first != NULL)
	{
		current = first;

		while(current->next != NULL)
		{
			last = current;
			current = current->next;
			delete last->effect;
			delete last;
		}
		delete current->effect;
		delete current;
	}
}

void EffectHandler::Add(Effect* newEff)
{

	EffList* newList;

	newList = new EffList;
	newList->effect = newEff;
	newList->next = NULL;

	//check if it's the first layer created
	if(last == NULL)
	{
		first = last = newList;
		return;
	}

	//else stick it at the end
	last->next = newList;
	last = newList;
}

void EffectHandler::Remove(Effect* remEff)
{
	EffList* temp;
	EffList* temp2;

	//check if it's the first or 'last'
	if(first->effect == remEff)
	{
		temp2 = first;
		first = first->next;
		if(first == NULL)
			last = NULL;
		delete temp2->effect;
		delete temp2;
		current = first;
		return;
	}

	temp = first;
	//else go look it up
	while(temp != NULL && temp->effect != remEff)
	{
		temp2 = temp;
		temp = temp->next;
	}
	//and get rid of it =)
	if(temp != NULL)
	{
		temp2->next = temp->next;
		if(temp2->next == NULL)
			last = temp2;
		delete temp->effect;
		delete temp;
	}
	current = first;
}

Effect* EffectHandler::GetEffects()
{
	if(first == NULL)
		return NULL;

	if(!bGet)
	{
		current = first;
		bGet = true;
		return current->effect;
	}

	if(current == NULL)
	{
		bGet = false;
		return NULL;
	}

	current = current->next;

	if(current == NULL)
	{
		bGet = false;
		return NULL;
	}

	return current->effect;
}

void EffectHandler::Tick()
{
	Effect* eff;
	while((eff = GetEffects()) != NULL)
	{
		eff->Tick();
	}

	while((eff = GetEffects()) != NULL)
	{
		if(eff->IsDone())
			Remove(eff);
	}
}

void EffectHandler::Draw()
{
	Effect* eff;
	while((eff = GetEffects()) != NULL)
		eff->Draw();
}
