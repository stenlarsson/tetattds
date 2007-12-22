#include "tetattds.h"
#include "platformgraphics.h"
#include "baseblock.h"
#include "playfield.h"
#include "util.h"
#include "sprite.h"
#include "marker.h"

FieldGraphics::FieldGraphics()
:	effects(),
	marker(GetMarker(true)),
	touchMarker(GetMarker(false)),
	lastChatLine(-1)
{
	memset(chatBuffer,0,sizeof(chatBuffer));
}

FieldGraphics::~FieldGraphics()
{
	delete marker;
	delete touchMarker;
}

void FieldGraphics::DrawFields(PlayField *pf)
{
	scrollOffset = (int)pf->GetScrollOffset();
	
	for(int i = PF_FIRST_BLOCK_FIRST_VISIBLE_ROW; i < PF_NUM_BLOCKS; i++)
	{
		BaseBlock* block = pf->GetField(i);
		
		bool shaded = (i >= PF_FIRST_BLOCK_LAST_ROW) || (pf->GetState() == PFS_DEAD);
		
		int tile = (block != NULL && !block->IsState(BST_MOVING)) ?
			block->GetTile() : TILE_BLANK;
		DrawField(pf, GetFieldX(i), GetFieldY(i), tile, shaded);
	}
}

void FieldGraphics::DrawMarkers(PlayField *pf)
{

	PFState state = pf->GetState();
	if(state == PFS_PLAY || state == PFS_START)
	{
		switch(pf->GetControlMode())
		{
		case MM_NONE:
			marker->Disable();
			touchMarker->Disable();
			break;
		case MM_KEY:
			{
				int markerPos = pf->GetMarkerPos();
				marker->SetPos(GetFieldX(markerPos), GetFieldY(markerPos, true));
				marker->Draw();
				touchMarker->Disable();
			}
			break;
		case MM_TOUCH:
		  {
				marker->Disable();
				int touchPos = pf->GetTouchPos();
				touchMarker->SetPos(GetFieldX(touchPos), GetFieldY(touchPos, true));
				touchMarker->Draw();
			}
			break;
		}
	}
	else
	{
		marker->Disable();
		touchMarker->Disable();
	}
}

void FieldGraphics::ReallyPrintPlayerInfo(PlayerInfo* player)
{
	if(player == NULL)
		return;

	// name
	char name[8+1];
	snprintf(name, sizeof(name), "%-8.8s", player->name);
	PrintSmall(NAME_TEXT_OFFSET + player->fieldNum * 8, name);
	
	// wins
	char wins[8+1];
	snprintf(wins, sizeof(wins), "%02i wins", player->wins);
	PrintSmall(WINS_TEXT_OFFSET + player->fieldNum * 8, wins);

	// level
	char level[8+1];
	snprintf(level, sizeof(level), "lv %02i", player->level+1);
	PrintSmall(LEVEL_TEXT_OFFSET + player->fieldNum * 8, level);

	// ready/typing
	char ready[8+1];
	snprintf(ready, sizeof(ready), (player->ready ? "READY" : (player->typing ? "typing" : "      ")));
	PrintSmall(READY_TEXT_OFFSET + player->fieldNum * 8, ready);

	// place
	const char* digit[] = {" ", "1", "2", "3", "4", "5"};
	const char* suffix[] = {"  ", "ST", "ND", "RD", "TH", "TH"};
	PrintLarge(
		PLACE_TEXT_OFFSET + player->fieldNum * 8,
		digit[player->place]);
	PrintSmall(
		PLACE_TEXT_OFFSET + player->fieldNum * 8 + TEXTMAP_STRIDE + 1,
		suffix[player->place]);
}

void FieldGraphics::PrintScore(int score)
{
	char str[9+1];
	snprintf(str, sizeof(str), "%9i", score);
	PrintLarge(SCORE_TEXT_OFFSET, str);
}

void FieldGraphics::PrintTime(int ticks)
{
	int seconds = ticks/60;
	char str[9+1];
	snprintf(str, sizeof(str), "%6i:%02i", seconds / 60, seconds % 60);
	PrintLarge(TIME_TEXT_OFFSET, str);
}

void FieldGraphics::PrintCountdown(int count)
{
	char str[2];
	if(count == 0)
		snprintf(str, sizeof(str), " ");
	else
		snprintf(str, sizeof(str), "%i", count);
	PrintLarge(COUNTDOWN_TEXT_OFFSET, str);
}

void FieldGraphics::PrintStopTime(int ticks)
{
	char str[9+1];
	snprintf(str, sizeof(str), "%6i.%02i", ticks/60, ticks*100/60%100);
	PrintLarge(STOP_TIME_TEXT_OFFSET, str);
}

int FieldGraphics::LargeCharTile(char c)
{
	switch (c)
	{
	case ':':  return TILE_LARGE_COLON;
	case '\'': return TILE_LARGE_TICK;
	case '"':  return TILE_LARGE_DOUBLE_TICK;
	case '.':  return TILE_LARGE_FULL_STOP;
	default:
		if(c >= '0' && c <= '9')
			return c - '0' + TILE_LARGE_ZERO;
		else
			return TILE_LARGE_SPACE;
	}	
}

int FieldGraphics::SmallCharTile(char c)
{
	return (c >= ' ' && c <= '~') ? c - ' ' : 0;
}

void FieldGraphics::AddChat(char* text)
{
	ASSERT(text != NULL);
	
	size_t textLen = strlen(text);
	if(textLen > 32) // Too long, try to break it up
	{
		char* space = NULL;
		char* nextSpace = NULL;
		// Skip to the first space (if any)
		space = strchr(text, ' ');
		if(space != NULL && (textLen - strlen(space) <= 32) ) // If there is at least one space and the first word isn't horribly long
			nextSpace = strchr(space+1, ' ');	// Find the next space (if any)
		if(nextSpace != NULL)
			if(textLen - strlen(nextSpace) > 32)
				nextSpace = NULL;
		if(nextSpace == NULL)
		{
			// We end up here if theres 33+ characters of junk and no space. (probably shouldn't happen at all)
			// Or if the first two words are too long. (happens when someone types a long word in chat, the first word being the nickname)
			if(text[32] == ' ') // takes care of case with words exactly 32 chars long.
			{
				text[32] = '\0';
				AddChat(text);
				AddChat(text+33);
				return;
			}
	
			char buf[32+1];
			strncpy(buf, text, 32); // Get the first 32 characters
			buf[32] = '\0';
			AddChat(buf);
			AddChat(text+32);
			return;
		}

		while(true) // Search for where to break up the line.
		{
			if(nextSpace == NULL)
				break;
			if(textLen - strlen(nextSpace) > 32)
				break;

			space = nextSpace;
			nextSpace = strchr(space+1, ' ');
		}
		// Breakpoint found, do some magic!
		*space = '\0';
		AddChat(text);
		AddChat(space+1);
	}
	else // A nice short line, add it to the buffer
	{
		lastChatLine++;
		if(lastChatLine == MAX_CHAT_LINES)
			lastChatLine = 0;
		strcpy(chatBuffer[lastChatLine], text);
	}
}
