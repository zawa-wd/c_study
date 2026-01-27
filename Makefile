fleet_manager: main.c fleet_logic.c
	gcc main.c fleet_logic.c -o fleet_manager

clean:
	rm -f fleet_manager
