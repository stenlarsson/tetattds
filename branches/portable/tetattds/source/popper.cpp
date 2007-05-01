#include "tetattds.h"
#include "popper.h"
#include "playfield.h"
#include "effecthandler.h"
#include "block.h"
#include "effcombo.h"
#include "game.h"
#include "sound.h"
#include "chain.h"
#include "util.h"
#include <algorithm>
#include <functional>

Popper::Popper(PlayField* newpf, EffectHandler* neweh)
	: chains(),
	  newChain(NULL),
		pf(newpf),
		eh(neweh)
{
}

Popper::~Popper()
{
	delete_and_clear(chains);
}

void Popper::AddBlock(Block* block, int blocknum)
{
	Chain* addToChain = block->GetChain();
	
	if(addToChain == NULL)	// If the block isn't part of a chain
	{
		// Make a new chain unless one was already created this frame
		if(newChain == NULL) {
			chains.push_back(newChain = new Chain());
		}
		addToChain = newChain;
		block->SetChain(newChain);
	}

	addToChain->AddBlock(block, blocknum);
}

void Popper::Pop()
{
	bool bBonusStop = false;
	int evil = 0;

	for(std::vector<Chain*>::iterator it = chains.begin(); it != chains.end(); ++it)
	{
		if(!(*it)->bUsedThisFrame)
			continue;

		(*it)->Sort();

		for(int curBlock = 0; curBlock < (*it)->numBlocks; curBlock++)
		{
			(*it)->blocks[curBlock]->Pop(curBlock, (*it)->numBlocks);
			if((*it)->blocks[curBlock]->GetType() == BLC_GRAY)
				evil++;
		}
	
		// Check add evil garbage
		if(evil > 0)
		{
			GarbageInfo g = {evil-2, GARBAGE_EVIL}; // -2 to specify number of garbage lines
			(*it)->garbage.push_back(g); 
			(*it)->bSentCombo = true;
		}
		
		bool offscreen = (*it)->blockNum[0]/PF_WIDTH < (PF_FIRST_VISIBLE_ROW-3); // If the block is too much offscreen
		
		if((*it)->numBlocks > 3)
		{
			// A combo.
			// -1 to specify number of garbage blocks instead of combo size
			GarbageInfo g = {(*it)->numBlocks-1, GARBAGE_COMBO};
			(*it)->garbage.push_back(g);
			(*it)->bSentCombo = true;
			pf->DelayScroll((*it)->numBlocks*5);
			if(pf->GetHeight() > PF_STRESS_HEIGHT)
				bBonusStop = true;
			pf->AddScore(((*it)->numBlocks-1)*10);
			
			if(!offscreen) // Only add effect if it's onscreen
				eh->Add(
					new EffCombo(
						(*it)->blockNum[0],
						COMBO_4,
						(*it)->numBlocks));
			if((*it)->length > 1)
			{
				// A chain involving the combo.
				pf->DelayScroll((*it)->numBlocks*10);
				pf->AddScore(50+((*it)->length-1)*20*(*it)->length);
				if(!offscreen)
					eh->Add(
						new EffCombo(
							(*it)->blockNum[0]-PF_WIDTH,
							COMBO_2X,
							(*it)->length));
			}
			Sound::PlayChainStepEffect(*it);
		}
		else if((*it)->length > 1)
		{
			// Just a chain, without a combo
			pf->DelayScroll((*it)->numBlocks*10);
			pf->AddScore(50+((*it)->length-1)*20*(*it)->length);
			if(!offscreen)
				eh->Add(
					new EffCombo(
						(*it)->blockNum[0],
						COMBO_2X,
						(*it)->length));
			if(pf->GetHeight() > PF_STRESS_HEIGHT)
				bBonusStop = true;
			Sound::PlayChainStepEffect(*it);
		}

		if(bBonusStop)
		{
			// TODO: Add some nifty graphics, and perhaps not a static bonus?
			pf->DelayScroll(BONUS_STOP_TIME);
		}
		(*it)->ClearBlocks();
		(*it)->numBlocks = 0;
		(*it)->bUsedThisFrame = false;
	}
	newChain = NULL;
}

void Popper::Tick()
{
	std::vector<Chain*>::iterator keep =
		std::partition(chains.begin(), chains.end(), std::mem_fun(&Chain::IsActive));
	
	// Send completed chains, if there's something to send.
	for(std::vector<Chain*>::iterator it = keep; it != chains.end(); ++it)
	{
		if((*it)->bSentCombo || (*it)->length > 1)
		{
			for(std::vector<GarbageInfo>::iterator j = (*it)->garbage.begin(); j != (*it)->garbage.end(); j++)
				g_game->SendGarbage(j->size, j->type);
			if((*it)->length > 1)
				g_game->SendGarbage((*it)->length-1, GARBAGE_CHAIN);
			Sound::PlayChainEndEffect(*it);
		}
	}

	delete_each(keep, chains.end());
	chains.erase(keep, chains.end());
}
