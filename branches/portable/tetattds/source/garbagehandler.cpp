#include "tetattds.h"
#include "garbagehandler.h"
#include "playfield.h"
#include "garbageblock.h"
#include "util.h"
#include <algorithm>
#include <functional>

/** Helper struct maintaining per-block information */
struct GBInfo
{
	GarbageBlock* block;
	Chain* chain;
};

GarbageHandler::GarbageHandler()
	: activeBlocks(),
		normalDrops(), chainDrops(),
		popBlocks()
{
}

GarbageHandler::~GarbageHandler()
{
	delete_and_clear(normalDrops);
	delete_and_clear(chainDrops);
	delete_and_clear(activeBlocks);
}

void GarbageHandler::AddGarbage(int num, int player, GarbageType type)
{
	switch(type)
	{
	case GARBAGE_CHAIN:
		chainDrops.push_back(new GarbageBlock(num, GARBAGE_CHAIN));
		break;
	case GARBAGE_COMBO:
		{
			int rows = (num - PF_WIDTH + 1) / PF_WIDTH;
			int small = num - rows * PF_WIDTH;
			if(small >= 7) {
				int half = small / 2; // Divide into (3,4) (4,4) (4,5) (5,5)
				normalDrops.push_back(new GarbageBlock(half, GARBAGE_COMBO));
				normalDrops.push_back(new GarbageBlock(small-half, GARBAGE_COMBO));
			}
			else
				normalDrops.push_back(new GarbageBlock(small, GARBAGE_COMBO));
			while(rows-- > 0)
				normalDrops.push_back(new GarbageBlock(PF_WIDTH, GARBAGE_COMBO));
		}
		break;
	case GARBAGE_EVIL:
		while(num-- > 0)
			normalDrops.push_back(new GarbageBlock(0, GARBAGE_EVIL));
		break;
	}
}

void GarbageHandler::DropGarbage(PlayField * pf)
{
	if(normalDrops.empty() && chainDrops.empty())
		return;
	
	int startField = PF_GARBAGE_DROP_START;
	int fieldHeight = pf->GetHeight();
	if(fieldHeight > 14)
		startField = PF_NUM_BLOCKS - fieldHeight*PF_WIDTH - 1;

	// Get the position where we should start inserting
	int curField = startField;
	while (curField >=2*PF_WIDTH-1 && !pf->IsLineOfFieldEmpty(curField))
		curField -= PF_WIDTH;

	// Process all garbage blocks about to drop.
	bool bLeftAlign = true;
	std::list<GarbageBlock*> * queues[] = {&normalDrops, &chainDrops};
	for(int i = 0; i < 2; i++)
	{
		for(std::list<GarbageBlock*> & q = *(queues[i]) ; !q.empty(); q.pop_front())
		{
			// Abort if we cannot place all garbage.
			if (!pf->InsertGarbage(curField, q.front(), bLeftAlign))
				break;

			bLeftAlign = !bLeftAlign;
			curField -= PF_WIDTH;
			q.front()->SetGraphic();
			
			activeBlocks.push_back(q.front());
		}
	}
}

void GarbageHandler::Tick()
{
	for_each(activeBlocks, std::mem_fun(&GarbageBlock::Tick));

	delete_and_erase_if(activeBlocks, std::mem_fun(&GarbageBlock::IsEmpty));
}

void GarbageHandler::AddPop(GarbageBlock* newPop, Chain* chain, bool first)
{
	GBInfo info = {newPop, chain};
	if (first)
		popBlocks.push_front(info);
	else
		popBlocks.push_back(info);
}

void GarbageHandler::Pop()
{
	int numBlocks = 0;
	for(std::list<GBInfo>::iterator it = popBlocks.begin(); it != popBlocks.end(); ++it)
		numBlocks += it->block->GetNum();

	int delay = 0;
	for(std::list<GBInfo>::iterator it = popBlocks.begin(); it != popBlocks.end(); ++it)
	{
		it->block->Pop(delay, numBlocks, it->chain);
		delay += it->block->GetNum();
	}

	popBlocks.clear();
}
