#pragma once
#ifdef DEBUG_VERBOSE
#define DEBUGVERBOSE printf
#else
#define DEBUGVERBOSE(...)
#endif

#define VERSION 2000

#define MAX_PLAYERS 5
#define SEND_FIELDSTATE_INTERVAL 60
#define SEND_PING_INTERVAL 60

// max squared distance in pixels two consecutive touch reads
// can have to register
#define TOUCH_DELTA 100

// sprite tile numbers
#define TILE_BLANK 7
#define TILE_BLOCK_GREEN 0
#define TILE_BLOCK_YELLOW 8
#define TILE_BLOCK_RED 16
#define TILE_BLOCK_PINK 24
#define TILE_BLOCK_CYAN 32
#define TILE_BLOCK_BLUE 40
#define TILE_BLOCK_GRAY 48
#define TILE_BLOCK_NORMAL_OFFSET 0
#define TILE_BLOCK_BOUNCE_1_OFFSET 1
#define TILE_BLOCK_BOUNCE_2_OFFSET 2
#define TILE_BLOCK_BOUNCE_3_OFFSET 3
#define TILE_BLOCK_FLASH_OFFSET 4
#define TILE_BLOCK_EYES_OFFSET 5
#define TILE_COMBO 56
#define TILE_CHAIN 67
#define TILE_EGG_SHELL 76
#define TILE_EGG 84
#define TILE_SMALL_MARKER 85
#define TILE_LARGE_MARKER 86
#define TILE_GARBAGE_BLUE 88
#define TILE_GARBAGE_GRAY 104
#define TILE_GARBAGE_TOP_OFFSET 0
#define TILE_GARBAGE_MIDDLE_OFFSET 3
#define TILE_GARBAGE_FLASH_1_OFFSET 6
#define TILE_GARBAGE_FLASH_2_OFFSET 7
#define TILE_GARBAGE_BOTTOM_OFFSET 8
#define TILE_GARBAGE_SINGLE_OFFSET 11

// text tile number
#define TILE_LARGE_SPACE 96
#define TILE_LARGE_ZERO 97
#define TILE_LARGE_COLON 107
#define TILE_LARGE_TICK 108
#define TILE_LARGE_DOUBLE_TICK 109
#define TILE_LARGE_FULL_STOP 110
#define LARGE_DIGIT_TILE_STRIDE 16

// garbage defines
#define MAX_GARBAGE 50
#define MAX_GARBAGE_SIZE 20

// sprite/background priorities
#define BACKGROUND_PRIORITY 3
#define BLOCKS_PRIORITY 2
#define CONSOLE_PRIORITY 0
#define TEXT_PRIORITY 1
#define MARKER_PRIORITY 0
#define COMBO_COUNTER_PRIORITY 0
#define COMBO_EGG_PRIORITY 1

// map baseblocks, the same are used for both main and sub
// 32 blocks of 0x800 bytes each starting at 0x6000000
#define BACKGROUND_MAP_BASE  0  // start = 0x6000000, size = 32*32*2 = 0x00800
#define BLOCKS_MAP_BASE      1  // start = 0x6000800, size = 32*32*2 = 0x00800
#define CONSOLE_MAP_BASE     2  // start = 0x6001000, size = 32*32*2 = 0x00800
#define TEXT_MAP_BASE        3  // start = 0x6001800, size = 32*32*2 = 0x00800

// tile baseblocks, the same are used for both main and sub
// 8 blocks of 0x4000 bytes each starting at 0x6000000
#define BACKGROUND_TILE_BASE 1  // start = 0x6004000, size ~ 39 KB = 0xA000
#define BLOCKS_TILE_BASE     4  // start = 0x6010000, size = 32 KB = 0x8000
#define CONSOLE_TILE_BASE    6  // start = 0x6018000, size =  8 KB = 0x2000
#define TEXT_TILE_BASE       7  // start = 0x601C000, size = ?

// map offsets etc
#define BLOCKMAP_STRIDE 32
#define SMALL_FIELD_OFFSET (11*BLOCKMAP_STRIDE + 1)

#define TEXTMAP_STRIDE 32
#define SCORE_TEXT_OFFSET (3*TEXTMAP_STRIDE)
#define TIME_TEXT_OFFSET (7*TEXTMAP_STRIDE)
#define STOP_TIME_TEXT_OFFSET (11*TEXTMAP_STRIDE)
#define COUNTDOWN_TEXT_OFFSET (5*TEXTMAP_STRIDE + 16)
#define NAME_TEXT_OFFSET (8*TEXTMAP_STRIDE)
#define WINS_TEXT_OFFSET (9*TEXTMAP_STRIDE)
#define LEVEL_TEXT_OFFSET (10*TEXTMAP_STRIDE)
#define READY_TEXT_OFFSET (14*TEXTMAP_STRIDE + 1)
#define PLACE_TEXT_OFFSET (15*TEXTMAP_STRIDE + 2)

#define MAX_CHAT_LENGTH 128
#define MAX_CHAT_LINES 6

// block behaviour
#define BLOCK_DROP_TIMER 0
#define BLOCK_FLASH_TIMER 40

// combo effect parameters
//#define COMBO_COUNTER_DURATION 50
#define COMBO_EGG_RADIUS 70
#define COMBO_NUM_EGGS 6

// defines that stop time! w00t!
#define MAX_STOP_TIME 180
#define BONUS_STOP_TIME 80

// dying effect parameters
#define DEATH_DURATION 52

#define GAME_END_DELAY 180

#define BLOCKSIZE 16

#define MAX_CHAINS 20

// 2^(1/3600)
//#define SCROLL_SPEED_INCREASE 1.0001925594206744839772558492056
// 2^(1/7200) // Speed doubles in two minutes
#define SCROLL_SPEED_INCREASE 1.0000962750758921229716090257793
#define MAX_SCROLL_SPEED 2

#define GRAY_BLOCK_CHANCE 0.10 // chance per block on a row of six
#define GRAY_BLOCK_DELAY 5	// number of rows to wait before they can appear

#define KEY_HOLD_DELAY 8
#define KEY_REPEAT_DELAY 2

#define PF_WIDTH 6
#define PF_HEIGHT 48
#define PF_STARTHEIGHT 8
#define PF_VISIBLE_HEIGHT 12
#define PF_STRESS_HEIGHT 10
#define PF_FIRST_VISIBLE_ROW (PF_HEIGHT-PF_VISIBLE_HEIGHT-1)
#define PF_FIRST_FILLED_ROW (PF_HEIGHT-PF_STARTHEIGHT)
#define PF_FIRST_BLOCK_FIRST_VISIBLE_ROW (PF_FIRST_VISIBLE_ROW*PF_WIDTH)
#define PF_FIRST_BLOCK_SECOND_VISIBLE_ROW ((PF_FIRST_VISIBLE_ROW+1)*PF_WIDTH)
#define PF_FIRST_BLOCK_LAST_ROW ((PF_HEIGHT-1)*PF_WIDTH)
#define PF_FIRST_BLOCK_SECOND_LAST_ROW ((PF_HEIGHT-2)*PF_WIDTH)
#define PF_NUM_BLOCKS (PF_WIDTH*PF_HEIGHT)
#define PF_MARKER_START (PF_NUM_BLOCKS-PF_WIDTH*6-(PF_WIDTH/2)-1)
#define PF_GARBAGE_DROP_START (PF_FIRST_BLOCK_FIRST_VISIBLE_ROW-2*PF_WIDTH-1)

enum Direction
{
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
};

enum Input
{
	INPUT_UP,
	INPUT_LEFT,
	INPUT_RIGHT,
	INPUT_DOWN,
	INPUT_SWAP,
	INPUT_RAISE
};
