
all: tetatt-ds tetatt-sdl

tetatt-ds: fwgui tetatt-tools
	@make -C tetattds
	
tetatt-sdl: fwgui-sdl tetatt-tools
	@make -C tetattds/sdl

fwgui: fwgui-tools
	@make -C fwgui/arm9
	
fwgui-sdl: fwgui-tools
	@make -C fwgui/sdl

fwgui-tools:
	@make -C fwgui/tools

tetatt-tools:
	@make -C tetattds/tools

run-sdl: tetatt-sdl
	@(cd tetattds; sdl/tetattsdl)

run-ds: tetatt-ds
	@desmume-cli tetattds/tetattds.ds.gba

debug-sdl: tetatt-sdl
	@(cd tetattds; gdb sdl/tetattsdl)

run: run-sdl

debug: debug-sdl

emulate: run-ds