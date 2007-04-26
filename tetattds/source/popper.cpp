#include "tetattds.h"
#include "popper.h"
#include "playfield.h"
#include "effecthandler.h"
#include "block.h"
#include "effcombo.h"
#include "game.h"
#include "sound.h"

Popper::Popper(PlayField* newpf, EffectHandler* neweh)
{
	for(int i = 0;i < MAX_CHAINS;i++)
	{
		bUsedThisFrame[i] = false;
		bFree[i] = true;
		ClearChain(&chains[i]);
		chains[i].chainNum = i;
		chains[i].activeBlocks = 0;
		chains[i].length = 0;
		chains[i].numBlocks = 0;
		chains[i].popCount = 0;
		chains[i].garbage.clear();
	}
	newChain = NULL;
	eh = neweh;
	pf = newpf;
}

Popper::~Popper()
{

}

void Popper::AddBlock(Block* block, int blocknum)
{
	Chain* addToChain = block->GetChain();
	
	if(addToChain == NULL)	// If the block isn't part of a chain
	{
		if(newChain == NULL)	// Make a new chain unless one was already created this frame
			newChain = GetFreeChain();	
		addToChain = newChain;
		block->SetChain(newChain);
	}

	addToChain->blocks[addToChain->numBlocks] = block;
	addToChain->blockNum[addToChain->numBlocks] = blocknum;
	addToChain->numBlocks++;
	if(!bUsedThisFrame[addToChain->chainNum]) // Increases chain length every frame it's involved in popping new blocks
	{
		addToChain->length++;
		addToChain->popCount = 0;
		bUsedThisFrame[addToChain->chainNum] = true;
	}
}

void Popper::Pop()
{
	int curChain;
	int curBlock;
	bool bBonusStop = false;
	int evil = 0;

	for(curChain = 0;curChain<MAX_CHAINS;curChain++)
	{
		if(bFree[curChain] || !bUsedThisFrame[curChain])
			continue;

		SortChain(&chains[curChain]);

		for(curBlock = 0;curBlock < chains[curChain].numBlocks;curBlock++)
		{
			chains[curChain].blocks[curBlock]->Pop(curBlock, chains[curChain].numBlocks);
			if(chains[curChain].blocks[curBlock]->GetType() == BLC_GRAY)
				evil++;
		}
	
		// Check add evil garbage
		if(evil > 0)
		{
			chains[curChain].garbage.push_back(evil-2 | GARBAGE_EVIL ); // -2 to specify number of garbage lines
			chains[curChain].bSentCombo = true;
		}
		
		bool offscreen = chains[curChain].blockNum[0]/PF_WIDTH < (PF_FIRST_VISIBLE_ROW-3); // If the block is too much offscreen
		
		if(chains[curChain].numBlocks > 3)
		{
			// A combo.
			// -1 to specify number of garbage blocks instead of combo size
			chains[curChain].garbage.push_back(chains[curChain].numBlocks-1 | GARBAGE_COMBO);
			chains[curChain].bSentCombo = true;
			pf->DelayScroll(chains[curChain].numBlocks*5);
			if(pf->GetHeight() > PF_STRESS_HEIGHT)
				bBonusStop = true;
			pf->AddScore((chains[curChain].numBlocks-1)*10);
			
			if(!offscreen) // Only add effect if it's onscreen
				eh->Add(
					new EffCombo(
						pf->GetFieldX(chains[curChain].blockNum[0]),
						pf->GetFieldY(chains[curChain].blockNum[0]),
						g_game->GetLevelData(),
						COMBO_4,
						chains[curChain].numBlocks));
			if(chains[curChain].length > 1)
			{
				// A chain involving the combo.
				pf->DelayScroll(chains[curChain].numBlocks*10);
				pf->AddScore(50+(chains[curChain].length-1)*20*chains[curChain].length);
				if(!offscreen)
					eh->Add(
						new EffCombo(
							pf->GetFieldX(chains[curChain].blockNum[0]-PF_WIDTH),
							pf->GetFieldY(chains[curChain].blockNum[0]-PF_WIDTH),
							g_game->GetLevelData(),
							COMBO_2X,
							chains[curChain].length));
			}
			Sound::PlayChainStepEffect(&chains[curChain]);
		}
		else if(chains[curChain].length > 1)
		{
			// Just a chain, without a combo
			pf->DelayScroll(chains[curChain].numBlocks*10);
			pf->AddScore(50+(chains[curChain].length-1)*20*chains[curChain].length);
			if(!offscreen)
				eh->Add(
					new EffCombo(
						pf->GetFieldX(chains[curChain].blockNum[0]),
						pf->GetFieldY(chains[curChain].blockNum[0]),
						g_game->GetLevelData(),
						COMBO_2X,
						chains[curChain].length));
			if(pf->GetHeight() > PF_STRESS_HEIGHT)
				bBonusStop = true;
			Sound::PlayChainStepEffect(&chains[curChain]);
		}

		if(bBonusStop)
		{
			// TODO: Add some nifty graphics, and perhaps not a static bonus?
			pf->DelayScroll(BONUS_STOP_TIME);
		}
		chains[curChain].numBlocks = 0;
		ClearChain(&chains[curChain]);
		bUsedThisFrame[curChain] = false;
	}
	newChain = NULL;
}

Chain* Popper::GetFreeChain()
{
	for(int i = 0;i<MAX_CHAINS;i++)
	{
		if(bFree[i])
		{
			bFree[i] = false;
			return &chains[i];
		}
	}
	return NULL;
}

void Popper::SortChain(Chain* chain)
{
	int tmp;
	Block* block;
	int i;
	int ii;
	bool bChanged = false;

	do
	{
		bChanged = false;
		for(i = 0;i < chain->numBlocks-1;i++)
		{
			if(chain->blockNum[i+1] < chain->blockNum[i])
			{
				tmp = chain->blockNum[i];
				chain->blockNum[i] = chain->blockNum[i+1];
				chain->blockNum[i+1] = tmp;
				block = chain->blocks[i];
				chain->blocks[i] = chain->blocks[i+1];
				chain->blocks[i+1] = block;
				bChanged = true;
			}
			if(chain->blockNum[i] == chain->blockNum[i+1])
			{
				for(ii = i;ii < chain->numBlocks;ii++)
				{
					chain->blockNum[ii] = chain->blockNum[ii+1];
					chain->blocks[ii] = chain->blocks[ii+1];
				}
				chain->numBlocks--;
				bChanged = true;
			}
		}
	}while(bChanged);
}

void Popper::ClearChain(Chain* chain)
{
	for(int i = 0;i<100;i++)
	{
		chain->blockNum[i] = -1;
		chain->blocks[i] = NULL;
	}
}

void Popper::Tick()
{
	for(int i = 0;i<MAX_CHAINS;i++)
	{
		ASSERT(chains[i].activeBlocks >= 0);
		
		if(!bFree[i] && chains[i].activeBlocks == 0)
		{
			// Chain done, so we send it, if there's a chain to send.
			if(chains[i].bSentCombo || chains[i].length > 1)
			{
				for(std::vector<unsigned int>::iterator j = chains[i].garbage.begin();j != chains[i].garbage.end();j++)
				{
					g_game->SendGarbage(*j&0xFFFF, (GarbageType)(*j&0xFFFF0000));
				}
				if(chains[i].length > 1)
					g_game->SendGarbage(chains[i].length-1, GARBAGE_CHAIN);
				Sound::PlayChainEndEffect(&chains[i]);
			}
			// Then clear the chain
			ClearChain(&chains[i]);
			chains[i].length = 0;
			chains[i].numBlocks = 0;
			chains[i].bSentCombo = false;
			chains[i].popCount = 0;
			chains[i].garbage.clear();
			bFree[i] = true;
		}
	}
}
