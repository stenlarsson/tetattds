#include "tetattds.h"
#include "garbagehandler.h"
#include "playfield.h"
#include "garbageblock.h"

GarbageHandler::GarbageHandler(PlayField* pf)
	: pf(pf),
		numPopBlocks(0),
		numNormalBlocks(0),
		numDropBlocks(0)
{
}

GarbageHandler::~GarbageHandler()
{
}

int GarbageHandler::NextFree()
{
	int nextFree = numPopBlocks + numNormalBlocks + numDropBlocks;
	return nextFree == MAX_GARBAGE ? -1 : nextFree;
}

void GarbageHandler::AllocGarbage(int num, GarbageType type)
{
	int nextFree = NextFree();
	if(nextFree == -1) // No room for more garbage!
	{
#ifdef DEBUG
		printf("Too much garbage, skipping.\n"); // DEBUG
#endif
		return;
	}

	Blocks[nextFree].block = new GarbageBlock(num, type);
	
	numDropBlocks++;
}

void GarbageHandler::AddGarbage(int num, int player, GarbageType type)
{
	switch(type)
	{
	case GARBAGE_CHAIN:
		AllocGarbage(num, GARBAGE_CHAIN);
		break;
	case GARBAGE_COMBO:
		{
			int rows = (num - PF_WIDTH + 1) / PF_WIDTH;
			int small = num - rows * PF_WIDTH;
			if(small >= 7) {
				int half = small / 2; // Divide into (3,4) (4,4) (4,5) (5,5)
				AllocGarbage(half, GARBAGE_COMBO);
				AllocGarbage(small-half, GARBAGE_COMBO);
			}
			else
				AllocGarbage(small, GARBAGE_COMBO);
			while(rows-- > 0)
				AllocGarbage(PF_WIDTH, GARBAGE_COMBO);
		}
		break;
	case GARBAGE_EVIL:
		while(num-- > 0)
			AllocGarbage(0, GARBAGE_EVIL);
		break;
	}
}

void GarbageHandler::DropGarbage()
{
	int startField = PF_GARBAGE_DROP_START;
	int fieldHeight = pf->GetHeight();
	if(fieldHeight > 14)
		startField = PF_NUM_BLOCKS - fieldHeight*PF_WIDTH - 1;

	// Get the position where we should start inserting
	int curField = startField;
	while (curField >=2*PF_WIDTH-1 && !pf->IsLineOfFieldEmpty(curField))
		curField -= PF_WIDTH;

	// Process all chains except the garbage chains
	bool bLeftAlign = true;
	for(int i = 0; i < numDropBlocks; i++)
	{
		GBInfo & info = Blocks[i + numPopBlocks + numNormalBlocks];
		if(info.block->GetType() == GARBAGE_CHAIN)
			continue;
				
		if (!pf->InsertGarbage(curField, info.block, bLeftAlign))
			return;

		bLeftAlign = !bLeftAlign;
		curField -= PF_WIDTH;
		info.block->SetGraphic();
	}

	// Now process the delayed garbage chains
	for(int i = 0; i < numDropBlocks; i++)
	{
		GBInfo & info = Blocks[i + numPopBlocks + numNormalBlocks];
		if(info.block->GetType() != GARBAGE_CHAIN)
			continue;
		
		if (!pf->InsertGarbage(curField, info.block, false))
			return;

		info.block->SetGraphic();
	}

	numNormalBlocks += numDropBlocks;
	numDropBlocks = 0;
}

void GarbageHandler::Tick()
{
	if(numDropBlocks > 0)
		DropGarbage();
	
	ASSERT(numPopBlocks == 0);
	for(int i = 0; i < numNormalBlocks; )
	{
		Blocks[i].block->Tick();

		if(Blocks[i].block->GetNum() > 0)
			i++;
		else
		{
			DEL(Blocks[i].block);
			std::swap(Blocks[i], Blocks[--numNormalBlocks]);
		}	
	}
}

void GarbageHandler::AddPop(GarbageBlock* newPop, Chain* chain, int order)
{
	for(int i = numPopBlocks; i < numPopBlocks + numNormalBlocks; i++)
	{
		GBInfo & info = Blocks[i];
		if(info.block == newPop)
		{
			info.chain = chain;
			info.PopOrder = order;
			std::swap(Blocks[i], Blocks[numPopBlocks]);
			numPopBlocks++;
			numNormalBlocks--;
			break;
		}
	}
}

void GarbageHandler::Pop()
{
	// Bubblesortey!
	for(int i=0; i<numPopBlocks-1; i++)
		for(int j=0; j<numPopBlocks-1-i; j++)
			if (Blocks[j+1].PopOrder < Blocks[j].PopOrder)
				std::swap(Blocks[j], Blocks[j+1]);
	
	int numBlocks = 0;
	for(int i = 0; i < numPopBlocks; i++)
		numBlocks += Blocks[i].block->GetNum();

	int delay = 0;
	for(int i = 0; i < numPopBlocks; i++)
	{
		Blocks[i].block->Pop(delay, numBlocks, Blocks[i].chain);
		delay += Blocks[i].block->GetNum();
	}

	for(int i = 0;i < MAX_GARBAGE;i++)
		Blocks[i].PopOrder = 0xFFFF;
		
	numNormalBlocks += numPopBlocks;
	numPopBlocks = 0;
}
