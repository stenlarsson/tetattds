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
	{
		DEL(field[i]);
	}

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
	{
		field[i] = NULL;
	}

	popper = new Popper(this, effects);
}

void PlayField::RandomizeField()
{
	for(int i = PF_FIRST_FILLED_ROW; i < PF_HEIGHT; i++)
	{
		RandomizeRow(i);
	}
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
			
			if(i%PF_WIDTH > 1)	// Don't do this check for the first two blocks
				if(field[i-1] != NULL && field[i-1]->GetType() == type &&
					field[i-2] != NULL && field[i-2]->GetType() == type)
				{
					continue;
				}

			if(field[i-PF_WIDTH] != NULL && field[i-PF_WIDTH]->GetType() == type &&
				field[i-2*PF_WIDTH] != NULL && field[i-2*PF_WIDTH]->GetType() == type)
			{
				continue;
			}
			
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
			if(stateDelay & BIT(0))
			{
				if(stateDelay & BIT(1))
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
		if(GetHeight() >= PF_VISIBLE_HEIGHT)
		{
			if((markerPos - PF_WIDTH) >= PF_FIRST_BLOCK_FIRST_VISIBLE_ROW)
				markerPos -= PF_WIDTH;
		}
		else
		{
			if((markerPos - PF_WIDTH) >= PF_FIRST_BLOCK_SECOND_VISIBLE_ROW)
				markerPos -= PF_WIDTH;
		}
		break;

	case INPUT_LEFT:
		controlMode = MM_KEY;
		iSwapTimer = 0;	
		if(markerPos % PF_WIDTH != 0)
			markerPos -= 1;
		break;

	case INPUT_RIGHT:
		controlMode = MM_KEY;
		iSwapTimer = 0;	
		if(markerPos % PF_WIDTH != PF_WIDTH-2)
			markerPos += 1;
		break;

	case INPUT_DOWN:
		controlMode = MM_KEY;
		iSwapTimer = 0;
		if((markerPos + PF_WIDTH) < PF_FIRST_BLOCK_LAST_ROW)
			markerPos += PF_WIDTH;
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
			{
				touchCol++;
			}
		}
		else if(col < touchCol)
		{
			if(SwapBlocks(ColRowToPos(touchCol-1, touchRow)))
			{
				touchCol--;
			}
		}
	}
}

void PlayField::TouchUp()
{
	controlMode = MM_NONE;
}

template<typename T> T max(T a, T b) { return (a > b) ? a : b; }
template<typename T> T min(T a, T b) { return (a < b) ? a : b; }

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
	BaseBlock* tmp;
	//Don't move blocks that we shouldn't
	if(pos >= PF_FIRST_BLOCK_LAST_ROW)
		return false;
	if(field[pos] != NULL && (field[pos]->GetState() != BST_IDLE || field[pos]->GetType() == BLC_GARBAGE || field[pos]->GetType() == BLC_EVILGARBAGE))
		return false;
	if(field[pos+1] != NULL && (field[pos+1]->GetState() != BST_IDLE || field[pos+1]->GetType() == BLC_GARBAGE || field[pos+1]->GetType() == BLC_EVILGARBAGE))
		return false;
	if(field[pos] == NULL && field[pos+1] == NULL)
		return false;

	if(field[pos] == NULL)
	{
		if(field[pos-PF_WIDTH] != NULL)
			if(field[pos-PF_WIDTH]->GetState() == BST_HOVER || field[pos-PF_WIDTH]->GetState() == BST_MOVING || field[pos-PF_WIDTH]->GetState() == BST_POSTMOVE)
				return false;

		if(field[pos+1-PF_WIDTH] != NULL && field[pos+1-PF_WIDTH]->GetState() == BST_IDLE)
			field[pos+1-PF_WIDTH]->Hover(9);

		field[pos] = field[pos+1];
		field[pos+1] = NULL;
		field[pos]->Move();
		effects->Add(new EffMoveBlock(DIR_LEFT, field[pos], fieldX[pos+1], fieldY[pos+1]+(int)scrollOffset));
		return true;
	}
	else if(field[pos+1] == NULL)
	{
		if(field[pos-PF_WIDTH+1] != NULL)
			if(field[pos-PF_WIDTH+1]->GetState() == BST_HOVER || field[pos-PF_WIDTH+1]->GetState() == BST_MOVING || field[pos-PF_WIDTH+1]->GetState() == BST_POSTMOVE)
				return false;

		if(field[pos-PF_WIDTH] != NULL && field[pos-PF_WIDTH]->GetState() == BST_IDLE)
			field[pos-PF_WIDTH]->Hover(9);

		field[pos+1] = field[pos];
		field[pos] = NULL;
		field[pos+1]->Move();
		effects->Add(new EffMoveBlock(DIR_RIGHT, field[pos+1], fieldX[pos], fieldY[pos]+(int)scrollOffset));
		return true;
	}

	tmp = field[pos];
	field[pos] = field[pos+1];
	field[pos+1] = tmp;
	field[pos]->Move();
	field[pos+1]->Move();
	effects->Add(new EffMoveBlock(DIR_LEFT,  field[pos], fieldX[pos+1], fieldY[pos+1]+(int)scrollOffset));
	effects->Add(new EffMoveBlock(DIR_RIGHT,  field[pos+1], fieldX[pos], fieldY[pos]+(int)scrollOffset));
	return true;
}

void PlayField::ScrollField()
{
	int i;
	
	if(!bTooHigh)
	{
		scrolledRows++;

		if(bFastScroll)
			scrollOffset -= 2;
		else
			scrollOffset -= g_game->GetScrollSpeed();

		if(scrollOffset <= -BLOCKSIZE)
		{
			if(GetHeight() == PF_FIRST_VISIBLE_ROW)
			{
				//DEBUG
				DEL(field[0]);
				DEL(field[1]);
				DEL(field[2]);
				DEL(field[3]);
				DEL(field[4]);
				DEL(field[5]);
				//debug

				for(i = 0; i < PF_FIRST_BLOCK_LAST_ROW; i++)
					field[i] = field[i+PF_WIDTH];
				RandomizeRow(PF_HEIGHT-1);
				switch(controlMode)
				{
				case MM_NONE: break;
				case MM_KEY: markerPos -= PF_WIDTH; break;
				case MM_TOUCH: touchRow--; break;
				}
				scrollOffset = 0;
			}
			else
			{
				//DEBUG
				DEL(field[0]);
				DEL(field[1]);
				DEL(field[2]);
				DEL(field[3]);
				DEL(field[4]);
				DEL(field[5]);
				//debug

				scrollOffset += BLOCKSIZE;
				for(i = 0;i < PF_FIRST_BLOCK_LAST_ROW;i++)
					field[i] = field[i+6];
				RandomizeRow(PF_HEIGHT-1);
				if(markerPos > PF_FIRST_BLOCK_SECOND_VISIBLE_ROW)
				{
					switch(controlMode)
					{
					case MM_NONE: break;
					case MM_KEY: markerPos -= PF_WIDTH; break;
					case MM_TOUCH: touchRow--; break;
					}
				}
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
			//Sound::StopMusic();
			Sound::PlayDieEffect();
		}
	}

	// TODO: do we need this?
	//effects->SetOffset(xOffset, yOffset + scrollOffset);
}

void PlayField::DropBlocks()
{
	//loop through field, starting at the bottom
	for(int i = PF_FIRST_BLOCK_LAST_ROW-1; i >= PF_WIDTH; i--)
	{
		if(field[i] == NULL)
			continue;//if there's no block, continue

		if(field[i]->GetState() == BST_POSTMOVE)
		{//if this block just has finished moving
			if(field[i+PF_WIDTH] == NULL)//if there's no block below
			{
				field[i]->Hover(4);//hover a bit before dropping
				field[i]->PopCheck();//and don't pop
				continue;
			}

			//if there's a block above, and it's hovering
			//(which it will be if it dropped on this block while it was MOVING)
			//and since we already know theres a block below we don't need to check that again
			if(field[i-PF_WIDTH] != NULL && field[i-PF_WIDTH]->GetState() == BST_HOVER)
			{
				field[i]->Land(); //land this block now instead of next tick
				for(int y = i-PF_WIDTH; y >= PF_WIDTH; y -= PF_WIDTH)
				{//loop through and land all hovering blocks above
					if(field[y] != NULL)
					{
						if(field[y]->GetState() == BST_HOVER)
						{
							field[y]->Drop();
							field[y]->Land();
						}
						else
						{//break if it's not hovering
							break;
						}
					}
					else
					{//and break if there's no block
						break;
					}
				}
			}
			continue;
		}//end of BST_POSTMOVE actions

		//if it's an idle block (that's not on the bottom row)
		if(i < PF_FIRST_BLOCK_SECOND_LAST_ROW && field[i]->GetState() == BST_IDLE)
		{
			if(field[i+PF_WIDTH] == NULL)
			{//and there's no block below
				field[i]->Drop();	//we drop the block
			}
			else if(field[i+PF_WIDTH]->GetState() == BST_FALLING)
			{//or if the block below is falling
				field[i]->Drop();	//drop this and
				//set chain to same as the falling block
				if(field[i]->GetType() != BLC_GARBAGE && field[i]->GetType() != BLC_EVILGARBAGE)
				{
					field[i]->SetChain(field[i+PF_WIDTH]->GetChain());
				}
			}
		}//end of BST_IDLE actions

		//if it's a falling block
		if(field[i]->GetState() == BST_FALLING)
		{
			if(field[i+PF_WIDTH] != NULL)  //if there's a block below this one
			{
				if(field[i+PF_WIDTH]->GetState() == BST_HOVER || field[i+PF_WIDTH]->GetState() == BST_MOVING || field[i+PF_WIDTH]->GetState() == BST_POSTMOVE)
				{//if the block below is hovering or moving
					field[i]->Hover(1500);//we hover this block a while
										//(it won't hover this long though since
										//it'll drop when the ones below drop
										// BUG, sometimes it keeps hovering, why??
					continue;
										
				}
				else if(field[i+PF_WIDTH]->GetState() != BST_FALLING)
				{//else, if it's not falling, we should land here
				 //(meaning, we land on idle and popping blocks)
					field[i]->Land();
					continue;
				}
			}
			if(i >= PF_WIDTH && field[i-PF_WIDTH] != NULL)//if there's a block above
			{
				if(field[i-PF_WIDTH]->GetState() == BST_HOVER || field[i-PF_WIDTH]->GetState() == BST_IDLE)
				{ //and it's hovering or idle
					field[i-PF_WIDTH]->Drop();	//we drop it
										// This happens when a stack of blocks 'landed'
										// on a block the player moved in below the stack,
										// and it's time to start falling again (BST_HOVER),
										// or when a block is pulled out of a row and the
										// block above where it was has just stopped hovering (BST_IDLE)
				}
			}
		}//end of BST_FALLING


		// There's no point in hovering above an idle or popping block, let's just land instead.
		if(field[i] != NULL && field[i]->GetState() == BST_HOVER)
		{
			if(field[i+PF_WIDTH] != NULL && (field[i+PF_WIDTH]->GetState() == BST_IDLE || field[i+PF_WIDTH]->GetState() == BST_FLASH))
			{
				field[i]->Drop();
				field[i]->Land();
			}
		}

	}//end of main loop

	//loop through field, starting at the bottom
	for(int i = PF_FIRST_BLOCK_LAST_ROW;i >= PF_WIDTH;i--)
	{
		if(field[i] == NULL)
			continue;//if there's no block, continue

		//if it's a falling block
		if(field[i]->GetState() == BST_FALLING)
		{
			if(field[i]->CheckDrop())	//if it's time to really drop the block
			{
				if(field[i+PF_WIDTH] == NULL)
				{
					field[i+PF_WIDTH] = field[i];	//move it down
					field[i] = NULL;
				}
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
	int top = 0;	//topmost block of same type as current
	int bottom = 0;	//bottommost
	int left = 0;	//guess..
	int right = 0;	//well...
	int num = 1;	//number of blocks of same type
	bool bPop = false; //flag to see if any pops occured this tick
	bool bClearChain = true; //flag to clear a blocks chain
	Chain* tmpChain = NULL; //temp pointer used to keep track of the active chain, if there is one

	//here's a good place to update the height
	
	for(int i = 0; i < PF_WIDTH; i++)
		fieldHeight[i] = -1;

	for(int i = PF_WIDTH; i < PF_FIRST_BLOCK_LAST_ROW; i++)//loop, top to bottom
	{
		bClearChain = true;
		if(field[i] == NULL)//skip if there's no block
			continue;

		if(field[i]->GetState() != BST_FALLING)// no falling blocks please
			if(fieldHeight[i%PF_WIDTH] == -1)//if we haven't already found one in the same column
				fieldHeight[i%PF_WIDTH] = PF_HEIGHT - i / PF_WIDTH - 1;//set height

		if(field[i]->GetType() == BLC_GARBAGE  || field[i]->GetType() == BLC_EVILGARBAGE)//and we don't really want to pop garbage like this =)
		{
			if(field[i]->GetState() == BST_POP2)//but we want to check if we need to 'pop' it
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
			if(field[i]->GetState() == BST_POP2)//check if it's popping and we should insert gfx
			{
				score += 10;  //add score
				if(i/PF_WIDTH >= PF_FIRST_VISIBLE_ROW)// only if it's onscreen
					effects->Add(new EffPop(fieldX[i], fieldY[i]+(int)scrollOffset, field[i]->GetChain()->length));//pop-effect
				Sound::PlayPopEffect(field[i]->GetChain());
			}
			continue;
		}

		int check = 0;
		//scroll upwards from current block
		for(check = i; check >= PF_WIDTH; check -= PF_WIDTH)
		{
			if(field[check-PF_WIDTH] == NULL)
				break;//break if there's no block
			if(field[check-PF_WIDTH]->GetType() != field[i]->GetType())
				break;//or if it's not of the same type
			if(field[check-PF_WIDTH]->GetState() != BST_IDLE && field[check-PF_WIDTH]->GetState() != BST_HOVER)
				break;//or if it's not idle or hovering
		}

		top = check;//topmost block found
		num = 1;
		tmpChain = NULL;

		//scroll downwards from top and count
		for(check = top; check < PF_FIRST_BLOCK_SECOND_LAST_ROW; check += PF_WIDTH)
		{
			if(tmpChain == NULL)
				tmpChain = field[check]->GetChain(); //if one of the blocks is in a chain
													 //this'll store that
			if(field[check+PF_WIDTH] == NULL)
				break;//break if there's no block
			if(field[check+PF_WIDTH]->GetType() != field[i]->GetType())
				break;//or if it's not the same type
			if(field[check+PF_WIDTH]->GetState() != BST_IDLE && field[check+PF_WIDTH]->GetState() != BST_HOVER)
				break;//or if it's not idle or hovering
			num++;//increase number of blocks
		}

		bottom = check;//bottom found

		if(num >= 3)//if there's three or more blocks
		{
			for(check = top; check <= bottom; check += PF_WIDTH)
			{//loop from top to bottom
				if(!field[check]->IsPopped()) //so we don't mess with blocks already added to the popper
				{
					field[check]->SetChain(tmpChain);//set chain of all blocks
 					popper->AddBlock(field[check], check);//add blocks to popper
					field[check]->SetPop(); //flag block as popped, so we don't add it twice
				}
			}
			bClearChain = false;//and make sure we don't clear the chain
			bPop = true;//and set popflag
		}

		//scroll left from current block
		for(check = i; 1; check--)
		{
			if((check-1)%PF_WIDTH == PF_WIDTH-1)
				break;//break if we hit the left edge
			if(field[check-1] == NULL)
				break;//break if there's no block
			if(field[check-1]->GetType() != field[i]->GetType())
				break;//break if it's not the same
			if(field[check-1]->GetState() != BST_IDLE && field[check-1]->GetState() != BST_HOVER)
				break;//break if it's not idle or hovering
		}

		left = check;//leftmost block found
		num = 1;
		tmpChain = NULL;

		//scroll right and count
		for(check = left; 1; check++)
		{
			if(tmpChain == NULL)
				tmpChain = field[check]->GetChain();//store the chain
			if((check+1)%PF_WIDTH == 0)
				break;//break if we hit the right edge
			if(field[check+1] == NULL)
				break;//break if there's no block
			if(field[check+1]->GetType() != field[i]->GetType())
				break;//break if it's not the same
			if(field[check+1]->GetState() != BST_IDLE && field[check+1]->GetState() != BST_HOVER)
				break;//break if it's not idle or hovering
			num++;//and increase number of blocks
		}

		right = check;//rightmost block found

		//if there's three or more
		if(num >= 3)
		{
			for(check = left;check <= right;check++)
			{//loop left to right
				if(!field[check]->IsPopped()) //no adding blocks twice
				{
					field[check]->SetChain(tmpChain);//set chain
					popper->AddBlock(field[check], check);//att to popper
					field[check]->SetPop(); //flag as popped
				}
			}
			bClearChain = false;
			bPop = true;
		}

		field[i]->PopCheck();//flag current block as checked
		if(bClearChain && field[i]->GetState() != BST_HOVER)//if nothing happened to the block and it's not hovering
			field[i]->SetChain(NULL);//clear the chain
	}

	if(bPop)//if something popped
	{
		popper->Pop();//initiate popping

		bool bDoOver = true;
		int order = 0;
		bool bReverse = false;
		while(bDoOver) // Check again to catch garbage popping garbage below itself
		{
			bDoOver=false;
			for(int i = PF_FIRST_BLOCK_LAST_ROW-1; i > PF_WIDTH; i--)//loop, bottom to top
			{ // This checks if there are GarbageBlocks that needs to pop
				if(field[i] == NULL)
					continue;
				if(field[i]->GetType() != BLC_GARBAGE && field[i]->GetType() != BLC_EVILGARBAGE)
					continue;
				if(field[i]->GetState() != BST_IDLE)
					continue;
				if(field[i]->IsPopped())
					continue;

				BlockType curType = field[i]->GetType();	// Stores type of current block for easy access
				BlockType notCurType = curType==BLC_GARBAGE?BLC_EVILGARBAGE:BLC_GARBAGE;	// Stores the other type of garbage block for easy syntax
				BlockType otherType;	// The type of the block we're checking against
				
				if(field[i+PF_WIDTH] != NULL && field[i+PF_WIDTH]->IsPopped())
				{
					otherType = field[i+PF_WIDTH]->GetType();				// Get blocktype, and...
					if(otherType == curType || otherType != notCurType)	// If it's the same garbagetype, or, not the other kind of garbage (== any other block)
					{														// Popping is GO!
						if(tmpChain == NULL)
							tmpChain = field[i+PF_WIDTH]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				if(field[i-PF_WIDTH] != NULL && field[i-PF_WIDTH]->IsPopped())
				{
					otherType = field[i-PF_WIDTH]->GetType();
					if(otherType == curType || otherType != notCurType)
					{
						if(tmpChain == NULL)
							tmpChain = field[i-PF_WIDTH]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				if(i%PF_WIDTH!=PF_WIDTH-1 && field[i+1] != NULL && field[i+1]->IsPopped())
				{
					otherType = field[i+1]->GetType();
					if(otherType == curType || otherType != notCurType)
					{
						if(tmpChain == NULL)
							tmpChain = field[i+1]->GetChain();
						((Garbage*)field[i])->GetGB()->InitPop(tmpChain);
					}
				}
				if(i%PF_WIDTH!=0 && field[i-1] != NULL && field[i-1]->IsPopped())
				{
					otherType = field[i-1]->GetType();
					if(otherType == curType || otherType != notCurType)
					{
						if(tmpChain == NULL)
							tmpChain = field[i-1]->GetChain();
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
	for(int i = PF_WIDTH;i < PF_FIRST_BLOCK_LAST_ROW; i++)
	{
		if(field[i] == NULL)
			continue;//break if empty

		if(field[i]->GetState() == BST_DEAD)
		{//if block is dead
			if(field[i]->GetType() != BLC_GARBAGE && field[i]->GetType() != BLC_EVILGARBAGE)//if it's an ordinary block
			{
				tChain = field[i]->GetChain();//store chain
				delete field[i];//delete block
				field[i] = NULL;//clear field
				for(int y = i-PF_WIDTH;y >= PF_WIDTH;y -= PF_WIDTH)
				{//loop through upwards
					if(field[y] != NULL)//if there's a block
					{
						if(field[y]->GetType() != BLC_GARBAGE && field[y]->GetType() != BLC_EVILGARBAGE && (field[y]->GetState() == BST_IDLE || field[y]->GetState() == BST_HOVER || field[y]->GetState() == BST_MOVING || field[y]->GetState() == BST_POSTMOVE))
						{//and if it's idle, moving or hovering, and not a garbage block
							field[y]->SetChain(tChain);//set chain
							if(field[y]->GetState() == BST_IDLE)
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
	for(int i = 0;i < PF_FIRST_BLOCK_LAST_ROW;i++)
	{
		if(field[i] == NULL)
			continue;

		if(field[i]->GetState() != BST_IDLE)
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
			for(int o = i; o < PF_FIRST_BLOCK_LAST_ROW; o += PF_WIDTH)
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
			for(int o = i; o < PF_FIRST_BLOCK_LAST_ROW; o += 6)
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
	for(int i = PF_FIRST_BLOCK_FIRST_VISIBLE_ROW; i < PF_FIRST_BLOCK_LAST_ROW; i++)
	{
		if(field[i] != NULL)
		{
			*dest = field[i]->GetTile();
		}
		else
		{
			*dest = TILE_BLANK;
		}

		dest++;
	}
}
