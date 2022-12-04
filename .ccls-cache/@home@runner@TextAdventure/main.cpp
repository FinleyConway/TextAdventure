#include <iostream>
#include <string.h>
#include <vector>
#include <limits>
#include <chrono>
#include <thread>
#include <map>

struct Ground {
  const char ground = '.';
  const char river = '~';
  const char rock = 'R';
	const char wall = '#';
	const char wood = 'w';
	const char rope = 'r';
};

struct GameObject {
		const char* name;
   	char sprite;
   	int x = 0;
   	int y = 0;

   GameObject(const char* n, char s, int initX, int initY) {
		 name = n;
		 sprite = s;
		 x = initX;
		 y = initY;
	 }
};

enum Commands { 
	move = 1,
	grab,
	craft, 
	swim, 
	leave
};

enum Direction {
	north = 1,
	south, 
	east,
	west
};

// user input
std::pair<const char*, const char*> InputHandler() {
	int com = 0;
  int dir = 0;
	const char* command = "";
	const char* direction = "";

	// command input
	std::cout << "Move: 1" << std::endl;
	std::cout << "Grab: 2" << std::endl;
	std::cout << "Craft: 3" << std::endl;
	std::cout << "What action do you want to do?: " << std::endl;
	while (!(std::cin >> com)) {
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	// direction input
	if (com != 3) { 
		std::cout << "North: 1" << std::endl;
	  std::cout << "South: 2" << std::endl;
	  std::cout << "East:  3" << std::endl;
	  std::cout << "West:  4" << std::endl;
		std::cout << "Where from/to?: " << std::endl;
	  while (!(std::cin >> dir)) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	if (com == move) command = "move";
	if (com == grab) command = "grab";
	if (com == craft) command = "craft";
	
  if (dir == north) direction = "north";
  if (dir == south) direction = "south";
  if (dir == east) direction = "east";
  if (dir == west) direction = "west";
	
  return {command, direction};
}

// check element depending on the direction and returns the element
char CheckElementDirection(const char *dir, int x, int y, const std::vector<std::vector<char>> &map) {
	// return the element ahead of object
  if (strcmp(dir, "north") == 0) return map[y - 1][x];
  if (strcmp(dir, "east") == 0) return map[y][x + 1];
  if (strcmp(dir, "south") == 0) return map[y + 1][x];
  if (strcmp(dir, "west") == 0) return map[y][x - 1];
	
	// this will never happen unless the movement clamping is wrong
	return ' ';
}

std::vector<char> CheckElementDirection(int x, int y, const std::vector<std::vector<char>> &map) {
	return {map[y - 1][x], map[y][x + 1], map[y + 1][x], map[y][x - 1]};
}

// move to a x and y position
void Move(const char *dir, int &x, int &y, std::vector<std::vector<char>> &map, const std::vector<char> &collision) {
	// stores previous movement position
	int prevX = x;
	int prevY = y;
	
  // handler the position from a input
  if (strcmp(dir, "north") == 0) y--;
  if (strcmp(dir, "east") == 0) x++;
  if (strcmp(dir, "south") == 0) y++;
  if (strcmp(dir, "west") == 0) x--;
	
	// if the player is on collision element, push the player back
	for (const auto &c : collision) {
		if (map[y][x] == c) {
			x = prevX;
			y = prevY;
		}
	}
}

void Move(const char *dir, int &x, int &y, std::vector<std::vector<char>> &map) {
  // handler the position from a input
  if (strcmp(dir, "north") == 0) y--;
  if (strcmp(dir, "east") == 0) x++;
  if (strcmp(dir, "south") == 0) y++;
  if (strcmp(dir, "west") == 0) x--;
}

// draw sprite on grid
void DrawSprite(int x, int y, std::vector<std::vector<char>> &map, char sprite) {
  map[y][x] = sprite;
}

// construct map
void DrawMap(std::vector<std::vector<char>> &map, const Ground &sGround) {
  for (int x = 0; x < map.size(); x++) {
    for (int y = 0; y < map[x].size(); y++) {
      DrawSprite(y, x, map, sGround.ground);
      DrawSprite(y, 3, map, sGround.river);
      DrawSprite(y, 4, map, sGround.river);
      DrawSprite(7, 1, map, sGround.rock);
			DrawSprite(0, x, map, sGround.wall);
			DrawSprite(y, 0, map, sGround.wall);
			DrawSprite(y, map.size() - 1, map, sGround.wall);
			DrawSprite(map[0].size() - 1, x, map, sGround.wall);
    }
  }
}

// updates the games graphics
void UpdateMap(const std::vector<std::vector<char>> &map) {
  // print map
  for (int i = 0; i < map.size(); i++) {
    for (int j = 0; j < map[i].size(); j++) {
      std::cout << map[i][j];
    }
    // indent to next line
    std::cout << std::endl;
  }
	std::cout << std::endl;
}

// add to inv
void AddToInventory(std::map<const char*, int> &inv, const char* item, int amount) {
	// make sure there are no 0 items being added to inv
	if (amount <= 0) return;

	// if it doesnt exists
	if (inv.find(item) == inv.end()) {
		inv.insert({item, amount});
	}
	else {
		inv[item] += amount;
	}
}

// remove from inv
void RemoveFromInventory(std::map<const char*, int> &inv, const char* item, int amount) {
	if (inv.find(item) == inv.end()) {
		// throw an error
		std::cout << "Item is not in list" << std::endl;
	}
	else {
		// remove amount from map
		inv[item] -= amount;
		// remove item if its less then 0
		if (inv[item] <= 0) {
			inv.erase(item);
		}
	}
}

// print inv
void ShowInventory(const std::map<const char*, int> &inv) {
	std::cout << "Inventory" << std::endl;
	for (const auto& i : inv) {
		   std::cout << "item: "<< i.first << " " << "amount: " << i.second << std::endl;
	}
}

void DirectionInformation(GameObject &player, const std::vector<std::vector<char>> &map, const std::vector<GameObject> &interactables) {
	std::vector<const char*> importantElement;
	std::vector<char> element { CheckElementDirection(player.x, player.y, map) };

	// loop through surrounding elements and add key elements to list
	for (int i = 0; i < element.size(); i++) {
		if (element[i] == interactables[i].sprite) {
			importantElement.push_back(interactables[i].name);
		}
	}

	// if key list is not empty, alert player about its surrounds
	if (!importantElement.empty()) {
		std::cout << "You currently see ";
		for (const auto& i : importantElement) {
			std::cout << i << " ";
		}
		std::cout << "around you.\n" << std::endl;
	}
}

void Delay(int seconds) {
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

void Dialogue(const char* name, const char* dialogue) {
	std::cout << name << ": " << dialogue << std::endl;
}

int main() {
  // window
  bool isRunning = true;
  const unsigned int maxGridSizeX = 20;
  const unsigned int maxGridSizeY = 10;
	std::vector<std::vector<char>> map(maxGridSizeY, std::vector<char>(maxGridSizeX));

	// sprites / game objects
  Ground sGround;
	GameObject player("Patrick", 'P', 5, 7);
	GameObject buddy("Bob", 'F', 6, 7);
	
	// interactables
	std::vector<GameObject> interactables = {
		{"Wood", sGround.wood, 4, 5}, 
		{"Wood", sGround.wood, 10, 6}, 
		{"Rope", sGround.rope, 10, 8}, 
		{"Rope", sGround.rope, 15, 7}
	};
	
	// collision
	std::vector<char> collision = {sGround.river, sGround.rock, sGround.wood, sGround.wall, sGround.rope };

	// player storage
	std::map<const char*, int> inventory = {};

	// --------- intro ---------
	std::cout << "You walk up to a lake with your friend after a long day of school and saw a massive rock." << std::endl;
	Delay(3);
	
	// init graphics
  DrawMap(map, sGround); // background
  for (auto& g : interactables) {
			DrawSprite(g.x, g.y, map, g.sprite);
	}
	DrawSprite(player.x, player.y, map, player.sprite);
	DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
  UpdateMap(map); // update all objects

	// -----create intro--------
	Dialogue(buddy.name, "Oooo, a rock... A BIG ROCK!!!");
	Delay(2);
	Dialogue(player.name, "Please dont do want I think you are going to do");
	Delay(2);
	Dialogue(buddy.name, "Pffff, ill be 2 mins");
	Delay(2);	
	system("clear");
	
	for (int i = 0; i < 6; i++) {
		DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
		Move("north", buddy.x, buddy.y, map);
  	UpdateMap(map); // update all objects
		DrawMap(map, sGround); // background
  	for (auto& g : interactables) {
				DrawSprite(g.x, g.y, map, g.sprite);
		}
		DrawSprite(player.x, player.y, map, player.sprite);
		DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
		Delay(1);	
		system("clear");
	}

	Dialogue(buddy.name, "0_0, pointy!");
	Dialogue(buddy.name, "Come lo..., omg... how did I get here O_O");
	Delay(2);
	Dialogue(player.name, "(－‸ლ), ill come and help you. *again*");
	Delay(2);
	
	DrawMap(map, sGround); // background
  for (auto& g : interactables) {
			DrawSprite(g.x, g.y, map, g.sprite);
	}
	DrawSprite(player.x, player.y, map, player.sprite);
	DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
  UpdateMap(map); // update all objects
	// -------------------------
	
	// game loop
  while (isRunning) {
		// stop the game loop if the player is holding a boat
		if (inventory.count("Boat")) {
			isRunning = false;
			break;
		}
		
		// print surroundings
		DirectionInformation(player, map, interactables);
		
		// ---- input  ----
    std::pair<const char*, const char*> input = InputHandler(); // first {command} // second {direciton}
    system("clear");
		//-----------------
		
		// ---- update ----
		// move around the grid
		if (strcmp(input.first, "move") == 0) {
    	Move(input.second, player.x, player.y, map, collision);
			std::cout << player.x << " " << player.y << std::endl; 
		}
		// grab items on floor
		else if (strcmp(input.first, "grab") == 0) {
			const char* item = "";
			char itemElement = CheckElementDirection(input.second, player.x, player.y, map);
			
      for (int i = 0; i < interactables.size(); i++) {
				if (itemElement == interactables[i].sprite) {
					item = interactables[i].name;
					interactables.erase(interactables.begin() + i);
				}
			}
			
      if (item && !*item) { 
				std::cout << "I dont see anything to grab" << std::endl;
			}
			else {
        AddToInventory(inventory, item, 1);
			}
		}
		// craft boat
		else if (strcmp(input.first, "craft") == 0) {
			if (!map.empty()) {
				if (inventory.find("Wood")->second >= 2 && inventory.find("Rope")->second >= 2) {
					RemoveFromInventory(inventory, "Wood", 2);
					RemoveFromInventory(inventory, "Rope", 2);
					AddToInventory(inventory, "Boat", 1);
				}
				else{
					std::cout << "You dont have enough to make a boat!\n" << std::endl;
				}
			}
		}
		// ----------------
		
		// ---- render ----
		// draw all the sprites
    DrawMap(map, sGround); // background
		for (auto& g : interactables) {
			DrawSprite(g.x, g.y, map, g.sprite);
		}
		DrawSprite(player.x, player.y, map, player.sprite);
		DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
		if (!inventory.empty())
			ShowInventory(inventory);
    UpdateMap(map);
		// ----------------
	}

	// create ending
	system("clear");
	player.x = 6;
	player.y = 5;
	
  DrawMap(map, sGround); // background
  for (auto& g : interactables) {
			DrawSprite(g.x, g.y, map, g.sprite);
	}
	DrawSprite(player.x, player.y, map, player.sprite);
	DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
  UpdateMap(map); // update all objects
	system("clear");
	
	player.sprite = 'B';
	for (int i = 0; i < 3; i++) {
		Move("north", player.x, player.y, map);
  	UpdateMap(map); // update all objects
		DrawMap(map, sGround); // background
  	for (auto& g : interactables) {
				DrawSprite(g.x, g.y, map, g.sprite);
		}
		DrawSprite(player.x, player.y, map, player.sprite);
		DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
		Delay(1);	
		system("clear");
	}
	player.sprite = 'P';

  DrawMap(map, sGround); // background
  for (auto& g : interactables) {
			DrawSprite(g.x, g.y, map, g.sprite);
	}
	DrawSprite(player.x, player.y, map, player.sprite);
	DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
  UpdateMap(map); // update all objects
	system("clear");

	Dialogue(player.name, "You an idiot you know");
	Delay(2);
	Dialogue(player.name, "I've never met anyone who can just go through a river not realising it");
	Delay(2);
	Dialogue(buddy.name, "heh");
	Delay(1);
	Dialogue(player.name, "Lets go home");

	for (int i = 0; i < 7; i++) {
		Move("west", player.x, player.y, map);
		Move("west", buddy.x, buddy.y, map);
  	UpdateMap(map); // update all objects
		DrawMap(map, sGround); // background
  	for (auto& g : interactables) {
				DrawSprite(g.x, g.y, map, g.sprite);
		}
		DrawSprite(player.x, player.y, map, player.sprite);
		DrawSprite(buddy.x, buddy.y, map, buddy.sprite);
		Delay(1);	
		system("clear");
	}
	// ----------------
}