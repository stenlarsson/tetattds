#include "tetattds.h"
#include "garbagehandler.h"
#include "playfield.h"
#include "garbageblock.h"

GarbageHandler::GarbageHandler(PlayField* newpf)
{
	pf = newpf;
	for(int i = 0;i<MAX_GARBAGE;i++)
	{
		bDropped[i] = true;
		Blocks[i] = NULL;
		BlockChain[i] = NULL;
		bPop[i] = false;
		PopOrder[i] = 0xFFFF;
		bDropBlock[i] = false;
	}

	numBlocks = 0;
	bDropGarbage = false;
}

GarbageHandler::~GarbageHandler()
{

}

int GarbageHandler::NextFree()
{
	int nextFree = 0;

	for(nextFree = 0;nextFree < MAX_GARBAGE;nextFree++)
	{
		if(Blocks[nextFree] == NULL)
			return nextFree;
	}

	return -1;
}

void GarbageHandler::AddGarbage(int num, int player, GarbageType type)
{
	int nextFree = NextFree();
	if(nextFree == -1) // No room for more garbage!
	{
#ifdef DEBUG
		printf("Too much garbage, skipping.\n"); // DEBUG
#endif
		return;
	}
	
	bDropGarbage = true;
	
	switch(type)
	{
	case GARBAGE_CHAIN:
		Blocks[nextFree] = new GarbageBlock(num, GARBAGE_CHAIN);
		bDropped[nextFree] = false;
		bDropBlock[nextFree] = true;
		numBlocks++;
		break;
	case GARBAGE_COMBO:
		if(num-6 >= 5)	// If there's enough blocks to be sure we need a size-6 block
		{				// Takes care of cases 11+
			AddGarbage(num-6, player, GARBAGE_COMBO);	// Add remaining garbage first
			AddGarbage(6, player, GARBAGE_COMBO);	// Add the size-6 block on top of the others
		}
		else if(num >= 7)	// For cases 7-10
		{
			int half = num / 2; // Divide into (3,4) (4,4) (4,5) (5,5)
			AddGarbage(half, player, GARBAGE_COMBO); // Add small block first
			AddGarbage(num-half, player, GARBAGE_COMBO);	// Add larger block
		}
		else	// Cases 3-6 (well, and 1-2, which shouldn't happen...)
		{
			Blocks[nextFree] = new GarbageBlock(num, GARBAGE_COMBO);
			bDropped[nextFree] = false;
			bDropBlock[nextFree] = true;
			numBlocks++;
		}
		break;
	case GARBAGE_EVIL:
		for(int i = 0;i<num;i++)
		{
			Blocks[nextFree] = new GarbageBlock(0, GARBAGE_EVIL);
			bDropped[nextFree] = false;
			bDropBlock[nextFree] = true;
			numBlocks++;
			nextFree = NextFree();
			if(nextFree == -1)
				break; // No room for more garbage!!@$!%
		}
		break;
	}
}

void GarbageHandler::DropGarbage()
{
	// TODO: Rewrite/cleanup this function
	int fieldHeight = pf->GetHeight();
	int startField = PF_GARBAGE_DROP_START;
	int curBlock = 0;
	int curField = 0;
	bool bLeftAlign = true;

	if(numBlocks == 0)
		return;

	if(fieldHeight > 14)
		startField = PF_NUM_BLOCKS - fieldHeight*PF_WIDTH - 1;

	// Get the position where we should start inserting
	curField = startField;
	while (curField >=2*PF_WIDTH-1 && !pf->IsLineOfFieldEmpty(curField))
		curField -= PF_WIDTH;

	// Process all chains except the garbage chains
	for(curBlock = 0;curBlock < MAX_GARBAGE;curBlock++)
	{
		if(bDropBlock[curBlock] == false)
			continue;
		if(Blocks[curBlock]->GetType() == GARBAGE_CHAIN)
			continue;
				
		if (!pf->InsertGarbage(curField, Blocks[curBlock], bLeftAlign))
			break;

		bLeftAlign = !bLeftAlign;
		curField -= PF_WIDTH;
		Blocks[curBlock]->SetGraphic();

		bDropped[curBlock] = true;
		bDropBlock[curBlock] = false;
		numBlocks--;
	}

	// Now process the delayed garbage chains
	for(curBlock = 0;curBlock < MAX_GARBAGE;curBlock++)
	{
		if(bDropBlock[curBlock] == false)
			continue;
		
		if (!pf->InsertGarbage(curField, Blocks[curBlock], false))
			break;

		Blocks[curBlock]->SetGraphic();
		bDropped[curBlock] = true;
		bDropBlock[curBlock] = false;
		numBlocks--;
	}
	
	bDropGarbage = false;
}

void GarbageHandler::Tick()
{
	if(bDropGarbage)
		DropGarbage();
	
	for(int i = 0;i < MAX_GARBAGE;i++)
	{
		if(Blocks[i] == NULL)
			continue;
	
		Blocks[i]->Tick();

		if(Blocks[i]->GetNum() > 0)
			continue;

		DEL(Blocks[i]);
	}
}

void GarbageHandler::AddPop(GarbageBlock* newPop, Chain* chain, int order)
{
	for(int i = 0;i < MAX_GARBAGE;i++)
	{
		if(Blocks[i] == newPop)
		{
			bPop[i] = true;
			BlockChain[i] = chain;
			PopOrder[i] = order;
			break;
		}
	}
}

void GarbageHandler::Pop()
{
	int numBlocks = 0;
	int delay = 0;
	int order[MAX_GARBAGE];
	int numGarbage = 0;
	
	for(int i = 0;i < MAX_GARBAGE;i++)
	{
		if(bPop[i])
		{
			numBlocks += Blocks[i]->GetNum();
			numGarbage++;
		}
	}

	if(numBlocks == 0)
		return;

	for(int i = 0;i<MAX_GARBAGE;i++)
		order[i] = i;

	// Bubblesortey!
	for (int i=0; i<MAX_GARBAGE-1; i++)
	{
		for (int j=0; j<MAX_GARBAGE-1-i; j++)
		{
			if (PopOrder[j+1] < PopOrder[j])
			{
				int tmp = PopOrder[j];
				PopOrder[j] = PopOrder[j+1];
				PopOrder[j+1] = tmp;
				tmp = order[j];
				order[j] = order[j+1];
				order[j+1] = tmp;
			}
		}
	}
	
	for(int i = 0;i < numGarbage;i++)
	{
		int block = order[i];
		Blocks[block]->Pop(delay, numBlocks, BlockChain[block]);
		delay += Blocks[block]->GetNum();
		bPop[block] = false;
	}

/*	for(int i = 0;i < MAX_GARBAGE;i++)
	{
		if(bPop[i])
		{
			Blocks[i]->Pop(delay, numBlocks, BlockChain[i]);
			delay += Blocks[i]->GetNum();
			bPop[i] = false;
		}
	}*/
	
	for(int i = 0;i < MAX_GARBAGE;i++)
		PopOrder[i] = 0xFFFF;
}
