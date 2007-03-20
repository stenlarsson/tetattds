#include "tetattds.h"
#include "playfield.h"
#include "fieldgraphics.h"
#include "garbagehandler.h"
#include "effecthandler.h"
#include "effready.h"
#include "effmoveblock.h"
#include "effpop.h"
#include "block.h"
#include "popper.h"
#include "garbage.h"
#include "game.h"
#include "sound.h"

static inline int RightOf(int i, int amount = 1)
{
	return i + 1 * amount;
}
static inline int LeftOf(int i, int amount = 1)
{
	return i - 1 * amount;
}
static inline int Above(int i, int amount = 1)
{
	return i - PF_WIDTH * amount;
}
static inline int Below(int i, int amount = 1)
{
	return i + PF_WIDTH * amount;
}
static inline bool IsLeftmost(int i, int amount = 1)
{
	return i % PF_WIDTH < amount;
}
static inline bool IsRightmost(int i, int amount = 1)
{
	return i % PF_WIDTH >= PF_WIDTH - amount;
}
static inline bool IsTopmostVisible(int pos, int amount = 1)
{
	return Above(pos, amount) < PF_FIRST_BLOCK_FIRST_VISIBLE_ROW;
}
static inline bool IsBottommostVisible(int pos, int amount = 1)
{
	return Below(pos, amount) >= PF_FIRST_BLOCK_LAST_ROW;
}
static inline bool IsTopmost(int pos)
{
	return pos < PF_WIDTH;
}
static inline bool IsForthcoming(int pos)
{
	return IsBottommostVisible(pos, 0);
}
static inline bool IsOfType(BaseBlock *b, BlockType t)
{
	return b != NULL && b->GetType() == t;
}
static inline bool IsGarbage(BaseBlock *b)
{
	return b != NULL && (b->GetType() == BLC_GARBAGE || b->GetType() == BLC_EVILGARBAGE);
}
static inline bool IsOfState(BaseBlock *b, BlockState s)
{
	return b != NULL && b->IsState(s);
}
static inline bool IsHoverOrMove(BaseBlock *b)
{
	if (b == NULL)
		return false;
	switch (b->GetState())
	{
	case BST_HOVER:
	case BST_MOVING:
	case BST_POSTMOVE:
		return true;
	default:
		return false;
	}
}
static inline bool IsHoverOrIdle(BaseBlock *b)
{
	if (b == NULL)
		return false;
	switch (b->GetState())
	{
	case BST_HOVER:
	case BST_IDLE:
		return true;
	default:
		return false;
	}	
}

PlayField::PlayField(EffectHandler *effects)
:	effects(effects)
{
	gh = new GarbageHandler(this);
	popper = NULL;
	markerPos = PF_MARKER_START;
	scrollOffset = 0;
	iScrollPause = 0;
	iSwapTimer = 0;
	bFastScroll = false;
	state = PFS_INIT;
	stateDelay = -1;
	bTooHigh = false;
	score = 0;
	timeTicks = 0;
	scrolledRows = 0;
	iDieTimer = 0;
	controlMode = MM_KEY;
	bMusicNormal = false;
	bMusicDanger = false;
	normalMusicDelay = 0;
}

PlayField::~PlayField()
{
	for(int i = 0; i < PF_NUM_BLOCKS; i++)
		DEL(field[i]);

	DEL(gh);
	DEL(popper);
	Sound::StopMusic();
}

void PlayField::Init(int xOffset, int yOffset)
{
	int x = xOffset;
	int y = yOffset - PF_FIRST_VISIBLE_ROW*BLOCKSIZE;

	for(int i = 0; i < PF_NUM_BLOCKS; i++)
	{
		fieldX[i] = x;
		fieldY[i] = y;
		x += BLOCKSIZE;
		if(((i+1) % PF_WIDTH) == 0)
		{
			x = xOffset;
			y += BLOCKSIZE;
		}
	}

	for(int i = 0; i < PF_NUM_BLOCKS; i++)
		field[i] = NULL;

	popper = new Popper(this, effects);
}

void PlayField::RandomizeField()
{
	for(int i = PF_FIRST_FILLED_ROW; i < PF_HEIGHT; i++)
		RandomizeRow(i);
}

void PlayField::RandomizeRow(int row)
{
	bool greyBlock = false;
	for(int i = row*PF_WIDTH;i < (row+1)*PF_WIDTH;i++)
	{
		bool blockAccepted = false;
		do
		{
			BlockType type;
			
			if(greyBlock == false && scrolledRows > GRAY_BLOCK_DELAY)
				type = g_game->GetRandomBlockType(true);
			else
				type = g_game->GetRandomBlockType(false);
			
			// make sure block won't pop immediately
			
			if(!IsLeftmost(i,2))
				if(IsOfType(field[LeftOf(i)], type) && IsOfType(field[LeftOf(i,2)], type))
					continue;

			if(IsOfType(field[Above(i)], type) && IsOfType(field[Above(i,2)], type))
				continue;
			
			field[i] = new Block(type);
			blockAccepted = true;
			if(type == BLC_GRAY)
				greyBlock = true;
		}
		while(!blockAccepted);
	}
	
}

void PlayField::Start()
{
	state = PFS_START;
	stateDelay = 150;
	effects->Add(new EffReady());
	Sound::PlayMusic(false);
	bMusicNormal = true;
}


void PlayField::Tick()
{
	DEBUGVERBOSE("PlayField: StateCheck\n");
	StateCheck();

	if(state == PFS_PLAY)
	{
		if(iSwapTimer > 0)
		{
			DEBUGVERBOSE("PlayField: SwapBlocks\n");
			if(SwapBlocks(markerPos))
				iSwapTimer = 0;
			else
				iSwapTimer--;
		}
		DEBUGVERBOSE("PlayField: field[].tick\n");
		for(int i = 0;i < PF_NUM_BLOCKS;i++)
		{
			if(field[i] != NULL)
				field[i]->Tick();
		}
		DEBUGVERBOSE("PlayField: gh.tick\n");
		gh->Tick();
		DEBUGVERBOSE("PlayField: popper.tick\n");
		popper->Tick();
		DEBUGVERBOSE("PlayField: scroll\n");
		if(ShouldScroll())
			ScrollField();
		DEBUGVERBOSE("PlayField: ClearDeadBlocks\n");
		ClearDeadBlocks();
		DEBUGVERBOSE("PlayField: DropBlocks\n");
		DropBlocks();
		DEBUGVERBOSE("PlayField: CheckForPops\n");
		CheckForPops();
		DEBUGVERBOSE("PlayField: CheckHeight\n");
		CheckHeight();
		timeTicks++;
	}
	else if(state == PFS_DIE)
	{
		// Shake screen
		if(stateDelay > 0)
		{
			if(stateDelay & 1)
			{
				if(stateDelay & 2)
					scrollOffset += 5;
				else
					scrollOffset -= 5;
			}
		}
	}

	DEBUGVERBOSE("PlayField: effects.tick\n");
	effects->Tick();
}

void PlayField::KeyInput(Input input)
{

	switch(input)
	{
	case INPUT_UP:
		controlMode = MM_KEY;
		iSwapTimer = 0;
		if(!IsTopmostVisible(markerPos, GetHeight() >= PF_VISIBLE_HEIGHT ? 1 : 2))
			markerPos = Above(markerPos);
		break;

	case INPUT_LEFT:
		controlMode = MM_KEY;
		iSwapTimer = 0;	
		if(!IsLeftmost(markerPos))
			markerPos = LeftOf(markerPos);
		break;

	case INPUT_RIGHT:
		controlMode = MM_KEY;
		iSwapTimer = 0;	
		if(!IsRightmost(markerPos, 2))
			markerPos = RightOf(markerPos);
		break;

	case INPUT_DOWN:
		controlMode = MM_KEY;
		iSwapTimer = 0;
		if(!IsBottommostVisible(markerPos))
			markerPos = Below(markerPos);
		break;

	case INPUT_SWAP:
		controlMode = MM_KEY;
		if(state != PFS_PLAY)
			break;
		if(!SwapBlocks(markerPos))
			iSwapTimer = 20;
		else
			iSwapTimer = 0;
		break;

	case INPUT_RAISE:
		bFastScroll = true;
		if(iScrollPause > 2)
			iScrollPause = 0;
		break;
	}
}

void PlayField::TouchDown(int col, int row)
{
	controlMode = MM_TOUCH;

	touchCol = col;
	touchRow = row;
}

void PlayField::TouchHeld(int col, int row)
{
	if(state != PFS_PLAY)
		return;

	if(col != touchCol)
	{
		if(col > touchCol)
		{
			if(SwapBlocks(ColRowToPos(touchCol, touchRow)))
				touchCol++;
		}
		else if(col < touchCol)
		{
			if(SwapBlocks(ColRowToPos(touchCol-1, touchRow)))
				touchCol--;
		}
	}
}

void PlayField::TouchUp()
{
	controlMode = MM_NONE;
}

#ifndef _WIN32
template<typename T> T max(T a, T b) { return (a > b) ? a : b; }
template<typename T> T min(T a, T b) { return (a < b) ? a : b; }
#endif

void PlayField::PixelsToColRow(int x, int y, int& col, int& row)
{
	col = fieldX[PF_FIRST_BLOCK_FIRST_VISIBLE_ROW];
	row = fieldY[PF_FIRST_BLOCK_FIRST_VISIBLE_ROW] + (int)scrollOffset;

	col = x - col;
	row = y - row;

	col /= BLOCKSIZE;
	row /= BLOCKSIZE;

	col = min(col, PF_WIDTH - 1);
	col = max(col, 0);
}

int PlayField::ColRowToPos(int col, int row)
{
	int pos = col + row*PF_WIDTH;
	pos += PF_FIRST_BLOCK_FIRST_VISIBLE_ROW;
	return pos;
}

bool PlayField::SwapBlocks(int pos)
{
	if(IsForthcoming(pos))
		return false;
		
	BaseBlock *left = field[pos], *right = field[RightOf(pos)];

	if(left && (!IsOfState(left, BST_IDLE) || IsGarbage(left)))
		return false;
	if(right && (!IsOfState(right, BST_IDLE) || IsGarbage(right)))
		return false;

	if(left == NULL && right == NULL)
		return false;

	if(left == NULL)
	{
		if(IsHoverOrMove(field[Above(pos)]))
			return false;

		if(IsOfState(field[RightOf(Above(pos))], BST_IDLE))
			field[RightOf(Above(pos))]->Hover(9);
	}
	else if(right == NULL)
	{
		if(IsHoverOrMove(field[RightOf(Above(pos))]))
			return false;

		if(IsOfState(field[Above(pos)], BST_IDLE))
			field[Above(pos)]->Hover(9);
	}
	
	field[pos] = right;
	field[RightOf(pos)] = left;
	
	if (right != NULL)
	{
		right->Move();
		effects->Add(new EffMoveBlock(DIR_LEFT, right, fieldX[RightOf(pos)], fieldY[RightOf(pos)]+(int)scrollOffset));
	}
	if (left != NULL)
	{
		left->Move();
		effects->Add(new EffMoveBlock(DIR_RIGHT, left, fieldX[pos], fieldY[pos]+(int)scrollOffset));
	}
	return true;
}

void PlayField::ScrollField()
{
	if(!bTooHigh)
	{
		scrolledRows++;

		if(bFastScroll)
			scrollOffset -= 2;
		else
			scrollOffset -= g_game->GetScrollSpeed();

		if(scrollOffset <= -BLOCKSIZE)
		{
			for(int i=0; IsTopmost(i); i++)
				DEL(field[i]);

			for(int i = 0; !IsForthcoming(i); i++)
				field[i] = field[Below(i)];
			RandomizeRow(PF_HEIGHT-1);

			if((GetHeight() == PF_FIRST_VISIBLE_ROW) ||
			   (markerPos > PF_FIRST_BLOCK_SECOND_VISIBLE_ROW))
			{
				switch(controlMode)
				{
				case MM_NONE: break;
				case MM_KEY: markerPos -= PF_WIDTH; break;
				case MM_TOUCH: touchRow--; break;
				}
			}

			if(GetHeight() == PF_FIRST_VISIBLE_ROW)
			{
				scrollOffset = 0;
			}
			else
			{
				scrollOffset += BLOCKSIZE;
				if(bFastScroll)
				{
					score++;
					iScrollPause = 2;
					bFastScroll = false;
				}
			}
		}
	}
	else
	{
		//scrollOffset = 0;
		if(++iDieTimer > 50)
		{
			state = PFS_DIE;
			stateDelay = DEATH_DURATION;
			Sound::PlayDieEffect();
		}
	}
}

void PlayField::DropBlocks()
{
	//loop through field, starting at the bottom
	for(int i = PF_LAST_BLOCK_SECOND_LAST_ROW; !IsTopmost(i); i--)
	{
		if(field[i] == NULL)
			continue;

		if(field[i]->IsState(BST_POSTMOVE))
		{
			if(field[Below(i)] == NULL)
			{
				//hover a bit before dropping  and don't pop
				field[i]->Hover(4);
				field[i]->PopCheck();
				continue;
			}

			if(IsOfState(field[Above(i)], BST_HOVER))
			{
				//if there's a block above, and it's hovering
				//(which it will be if it dropped on this block while it was MOVING)

				field[i]->Land(); //land this block now instead of next tick

				for(int y = Above(i); !IsTopmost(y) && IsOfState(field[y], BST_HOVER); y = Above(y))
					field[y]->DropAndLand();
			}
			continue;
		}

		if(!IsBottommostVisible(i) && field[i]->IsState(BST_IDLE))
		{
			if(field[Below(i)] == NULL)
				field[i]->Drop();
			else if(field[Below(i)]->IsState(BST_FALLING))
			{
				field[i]->Drop();
				if(!IsGarbage(field[i]))
					field[i]->SetChain(field[Below(i)]->GetChain());
			}
		}

		if(field[i]->IsState(BST_FALLING))
		{
			if(IsHoverOrMove(field[Below(i)]))
			{
				field[i]->Hover(1500);//we hover this block a while
									//(it won't hover this long though since
									//it'll drop when the ones below drop
									// BUG, sometimes it keeps hovering, why??
				continue;
									
			}
			else if(field[Below(i)] != NULL && !field[Below(i)]->IsState(BST_FALLING))
			{
				field[i]->Land();
				continue;
			}
			if(!IsTopmost(i) && IsHoverOrIdle(field[Above(i)]))
			{
				// This happens when a stack of blocks 'landed'
				// on a block the player moved in below the stack,
				// and it's time to start falling again (BST_HOVER),
				// or when a block is pulled out of a row and the
				// block above where it was has just stopped hovering (BST_IDLE)
				field[Above(i)]->Drop();
			}
		}

		if(IsOfState(field[i], BST_HOVER))
			if(IsOfState(field[Below(i)], BST_IDLE) || IsOfState(field[Below(i)], BST_FLASH))
				field[i]->DropAndLand();

	}

	//loop through field, starting at the bottom
	for(int i = PF_LAST_BLOCK_SECOND_LAST_ROW; !IsTopmost(i); i--)
	{
		if(IsOfState(field[i], BST_FALLING))
		{
			if(field[i]->CheckDrop())	//if it's time to really drop the block
			{
				if(field[Below(i)] == NULL)
					std::swap(field[i], field[Below(i)]);
#ifdef DEBUG
				else
				{
					printf("Fan i satan, vi ramlar! %d, %d\n", i%PF_WIDTH, i/PF_WIDTH); // DEBUG
					while(!(keysDown() & KEY_SELECT))
					{
						Draw();
						scanKeys();
						swiWaitForVBlank();
					}
				}
#endif
			}
		}
	}
}

void PlayField::CheckForPops()
{
	bool bPop = false; //flag to see if any pops occured this tick
	bool bClearChain = true; //flag to clear a blocks chain
	Chain *tmpChain = NULL;

	//here's a good place to update the height
	
	for(int i = 0; i < PF_WIDTH; i++)
		fieldHeight[i] = -1;

	for(int i = PF_WIDTH; !IsForthcoming(i); i++)//loop, top to bottom
	{
		bClearChain = true;
		if(field[i] == NULL)//skip if there's no block
			continue;

		if(!field[i]->IsState(BST_FALLING))// no falling blocks please
			if(fieldHeight[i%PF_WIDTH] == -1)//if we haven't already found one in the same column
				fieldHeight[i%PF_WIDTH] = PF_HEIGHT - i / PF_WIDTH - 1;//set height

		if(IsGarbage(field[i]))//and we don't really want to pop garbage like this =)
		{
			if(field[i]->IsState(BST_POP2))//but we want to check if we need to 'pop' it
			{
				if(i/PF_WIDTH >= PF_FIRST_VISIBLE_ROW)// only if it's onscreen
					effects->Add(new EffPop(fieldX[i], fieldY[i]+(int)scrollOffset, 0));
				Sound::PlayPopEffect(field[i]->GetChain());
			}
			continue;
		}

		if(!field[i]->NeedPopCheck())//and if the block doesn't need a check
		{
			//maybe this part shouldn't be here?
			if(field[i]->IsState(BST_POP2))//check if it's popping and we should insert gfx
			{
				score += 10;  //add score
				if(i/PF_WIDTH >= PF_FIRST_VISIBLE_ROW)// only if it's onscreen
					effects->Add(new EffPop(fieldX[i], fieldY[i]+(int)scrollOffset, field[i]->GetChain()->length));//pop-effect
				Sound::PlayPopEffect(field[i]->GetChain());
			}
			continue;
		}

		int check = 0;
		for(check = i; !IsTopmost(check); check = Above(check))
		{
			if(field[Above(check)] == NULL)
				break;
			if(!field[Above(check)]->SameType(field[i]))
				break;
			if(!IsHoverOrIdle(field[Above(check)]))
				break;
		}

		int top = check;
		int verticalCount = 1;
		/* Chain * */ tmpChain = NULL;

		for(check = top; !IsBottommostVisible(check); check = Below(check))
		{
			// TODO: Move the chain thing after we know the block is used
			if(tmpChain == NULL)
				tmpChain = field[check]->GetChain(); //if one of the blocks is in a chain
													 //this'll store that
			if(field[Below(check)] == NULL)
				break;
			if(!field[Below(check)]->SameType(field[i]))
				break;
			if(!IsHoverOrIdle(field[Below(check)]))
				break;
			verticalCount++;
		}

		int bottom = check;

		if(verticalCount >= 3)
		{
			for(int check = top; check <= bottom; check = Below(check))
			{
				if(!field[check]->IsPopped())
				{
					field[check]->SetChain(tmpChain);
 					popper->AddBlock(field[check], check);
					field[check]->SetPop();
				}
			}
			bClearChain = false;//and make sure we don't clear the chain
			bPop = true;//and set popflag
		}

		for(check = i; !IsLeftmost(check); check = LeftOf(check))
		{
			if(field[LeftOf(check)] == NULL)
				break;
			if(!field[LeftOf(check)]->SameType(field[i]))
				break;
			if(!IsHoverOrIdle(field[LeftOf(check)]))
				break;
		}

		int left = check;
		int horizontalCount = 1;
		/* Chain * */ tmpChain = NULL;

		for(check = left; !IsRightmost(check); check = RightOf(check))
		{
			// TODO: Move until we know block should be in chain
			if(tmpChain == NULL)
				tmpChain = field[check]->GetChain();//store the chain
			if(field[RightOf(check)] == NULL)
				break;
			if(!field[RightOf(check)]->SameType(field[i]))
				break;
			if(!IsHoverOrIdle(field[RightOf(check)]))
				break;
			horizontalCount++;
		}

		int right = check;

		if(horizontalCount >= 3)
		{
			for(int check = left;check <= right; check = RightOf(check))
			{
				if(!field[check]->IsPopped())
				{
					field[check]->SetChain(tmpChain);
					popper->AddBlock(field[check], check);
					field[check]->SetPop();
				}
			}
			bClearChain = false;
			bPop = true;
		}

		field[i]->PopCheck();//flag current block as checked
		if(bClearChain && !field[i]->IsState(BST_HOVER))
			field[i]->SetChain(NULL);
	}
	
	// TODO: Should tmpChain not be reset after the loop above?

	if(bPop)//if something popped
	{
		popper->Pop();//initiate popping

		bool bDoOver = true;
		int order = 0;
		bool bReverse = false;
		while(bDoOver) // Check again to catch garbage popping garbage below itself
		{
			bDoOver=false;
			for(int i = PF_LAST_BLOCK_SECOND_LAST_ROW; !IsTopmost(i); i--)//loop, bottom to top
			{
				// This checks if there are GarbageBlocks that needs to pop
				if(field[i] == NULL)
					continue;
				if(!IsGarbage(field[i]))
					continue;
				if(!field[i]->IsState(BST_IDLE))
					continue;
				if(field[i]->IsPopped())
					continue;

				BlockType curType = field[i]->GetType();	// Stores type of current block for easy access
				BlockType notCurType = curType==BLC_GARBAGE?BLC_EVILGARBAGE:BLC_GARBAGE;	// Stores the other type of garbage block for easy syntax
				BlockType otherType;	// The type of the block we're checking against
				
				if(field[Below(i)] != NULL && field[Below(i)]->IsPopped())
				{
					otherType = field[Below(i)]->GetType();				// Get blocktype, and...
					if(otherType == curType || otherType != notCurType)	// If it's the same garbagetype, or, not the other kind of garbage (== any other block)
					{														// Popping is GO!
						if(tmpChain == NULL)
							tmpChain = field[Below(i)]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				if(field[Above(i)] != NULL && field[Above(i)]->IsPopped())
				{
					otherType = field[Above(i)]->GetType();
					if(otherType == curType || otherType != notCurType)
					{
						if(tmpChain == NULL)
							tmpChain = field[Above(i)]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				if(!IsRightmost(i) && field[RightOf(i)] != NULL && field[RightOf(i)]->IsPopped())
				{
					otherType = field[RightOf(i)]->GetType();
					if(otherType == curType || otherType != notCurType)
					{
						if(tmpChain == NULL)
							tmpChain = field[RightOf(i)]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				if(!IsLeftmost(i) && field[LeftOf(i)] != NULL && field[LeftOf(i)]->IsPopped())
				{
					otherType = field[LeftOf(i)]->GetType();
					if(otherType == curType || otherType != notCurType)
					{
						if(tmpChain == NULL)
							tmpChain = field[LeftOf(i)]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				
				if(!field[i]->IsPopped()) // If it didn't pop, next!
					continue;
	
				if(tmpChain != NULL)
				{
					gh->AddPop(((Garbage*)field[i])->GetGB(), tmpChain, order);
					bDoOver = true;
					tmpChain = NULL;
					if(bReverse)
						order--;
					else
						order++;
				}
			}
			if(bDoOver && !bReverse)
			{
				bReverse = true;
				order = -1;
			}
		}
		
		gh->Pop();
	}
}

void PlayField::ClearDeadBlocks()
{
	BaseBlock* tmpBlock = NULL;
	Chain* tChain = NULL;	//keeps track of deleted blocks' chain

	//loop, top to bottom
	for(int i = PF_WIDTH; !IsForthcoming(i); i++)
	{
		if(field[i] == NULL)
			continue;//break if empty

		if(field[i]->IsState(BST_DEAD))
		{//if block is dead
			if(!IsGarbage(field[i]))//if it's an ordinary block
			{
				tChain = field[i]->GetChain();//store chain
				delete field[i];//delete block
				field[i] = NULL;//clear field
				for(int y = Above(i); !IsTopmost(y); y = Above(y))
				{//loop through upwards
					if(field[y] != NULL)//if there's a block
					{
						if(!IsGarbage(field[y]) && (field[y]->IsState(BST_IDLE) || IsHoverOrMove(field[y])))
						{//and if it's idle, moving or hovering, and not a garbage block
							field[y]->SetChain(tChain);//set chain
							if(field[y]->IsState(BST_IDLE))
								field[y]->Drop();//and drop it if it's idle
						}
						else
						{
							break;
						}//if we encounter a popping/falling block or an empty field, break
					}
					else
					{
						break;
					}
				}
			}
			else//if it's a garbage block
			{
				tmpBlock = ((Garbage*)field[i])->CreateBlock();
				delete field[i];
				field[i] = tmpBlock;
			}
		}
	}
}

bool PlayField::ShouldScroll()
{
	for(int i = 0; !IsForthcoming(i); i++)
	{
		if(field[i] == NULL)
			continue;

		if(!field[i]->IsState(BST_IDLE))
		{
			bFastScroll = false;
			return false;
		}
	}

	if(iScrollPause != 0)
	{
		iScrollPause--;
		bFastScroll = false;
		return false;
	}

	return true;
}

void PlayField::StateCheck()
{
	if(stateDelay > 0)
	{
		stateDelay--;
		return;
	}

	if(state == PFS_START)
	{
		state = PFS_PLAY;
	}

	if(state == PFS_DIE)
	{
		state = PFS_DEAD;
		g_game->PlayerDied();
	}
}

void PlayField::CheckHeight()
{
	bool bDanger = false; // music stuff
	
	bTooHigh = false;

	for(int i = 0; i < PF_WIDTH; i++)
		if(fieldHeight[i] >= PF_VISIBLE_HEIGHT)
			bTooHigh = true;

	if(!bTooHigh)
		iDieTimer = 0;

	for(int i = 0; i < PF_WIDTH; i++)
	{
		if(fieldHeight[i] >= PF_STRESS_HEIGHT)
		{
			bDanger = true; // music stuff
			for(int o = i; !IsForthcoming(o); o = Below(o))
			{
				if(field[o] != NULL)
				{
					if(bTooHigh || iScrollPause > 0)
						field[o]->Stop(true);
					else
						field[o]->Stress(true);
				}
			}
		}
		else
		{
			for(int o = i; !IsForthcoming(o); o = Below(o))
			{
				if(field[o] != NULL)
				{
					field[o]->Stop(false);
					field[o]->Stress(false);
				}
			}
		}
	}
	
	// music stuff
	if(bDanger)
	{
		if(!bMusicDanger)
		{
			Sound::PlayMusic(true);
			bMusicDanger = true;
			bMusicNormal = false;
		}
		normalMusicDelay = 20;
	}
	else
	{
		if(!bMusicNormal && --normalMusicDelay <= 0)
		{
			Sound::PlayMusic(false);
			bMusicNormal = true;
			bMusicDanger = false;
		}
	}
}

void PlayField::GetFieldState(char* dest)
{
	for(int i = PF_FIRST_BLOCK_FIRST_VISIBLE_ROW; !IsForthcoming(i); i++)
		*dest++ = (field[i] != NULL) ? field[i]->GetTile() : TILE_BLANK;
}

bool PlayField::IsLineOfFieldEmpty(int x) {
	for(int i = 0;i<PF_WIDTH;i++)
	{
		if(field[LeftOf(x,i)] != NULL)
			return false;
	}
	return true;
}

bool PlayField::InsertGarbage(int x, GarbageBlock *b, bool leftAlign) {
	if( IsTopmost(x - b->GetNum()) )
	{
#ifdef DEBUG
		printf("Out of space to drop blocks.\n");
#endif
		return false;
	}
	
	if (leftAlign)
		x -= PF_WIDTH - b->GetNum();
	for(int i = b->GetNum()-1; i >= 0; i--,x = LeftOf(x))
	{
#ifdef DEBUG
		if( field[x] != NULL)
			printf("Fanfanfan!");
#endif
		field[x] = b->GetBlock(i);
	}
	
	return true;
}
