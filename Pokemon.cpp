#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <string.h>
#include <random>
#include <time.h>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <windows.h>
#include <typeinfo>

// For SDL2
#undef main

using namespace std;

default_random_engine eng(time(0));


// Function to write strings to binary file
void serializeString(string words, fstream& f){
    int size = words.size();
    f.write((char*)&size, sizeof(size));

    for(int i = 0; i < size; i++)
        f.write((char*)&words[i], sizeof(words[i]));
}

// Function to read strings from binary file
void deserializeString(string& words, fstream& f){
    int size;
    f.read((char*)&size, sizeof(size));

    words = "";

    for(int i = 0; i < size; i++){
        char c;
        f.read((char*)&c, sizeof(c));
        words += c;
    }
}

// Function to compare two folder paths
bool CompareFilePaths(string path1, string path2)
{
    char c = '(';
    char c2 = ')';

    int number1 = 0;

    // Go until ( is encountered
    int i = 0;
    while(i < path1.size() && path1[i] != c)
        ++i;
    ++i;

    // The first parts of the paths are the same so ( is
    // on the same position
    int j = i;

    // Extract the number
    while(i < path1.size() && path1[i] != c2){
        number1 = number1 * 10 + (path1[i] - '0');
        ++i;
    }

    int number2 = 0;
    while(j < path2.size() && path2[j] != c2){
        number2 = number2 * 10 + (path2[j] - '0');
        ++j;
    }
    return number1 < number2;
}

class Item{
protected:
    int price;
    string name;

public:
    Item(string name = "", int price = 0): name(name), price(price) {}
    Item(const Item& obj) {this->price = obj.price; this->name = obj.name;}

    Item& operator =(const Item&);
    bool operator <(const Item& obj) const {return this->price < obj.price;}

    virtual void readFromFile(fstream&) = 0;
    virtual void writeToFile(fstream&) = 0;

    virtual void serialization(fstream&);
    virtual void deserialization(fstream&);

    virtual void show() const {cout << "Name: " << this->name <<  " Price: " << this->price;}
    virtual int use() const = 0;
    virtual void setAttributes() = 0;

    string getName() const {return this->name;}

    virtual ~Item() {}
};

class Potion: public Item{
    int hpRestored;

public:
    Potion(string name = "", int price = 0, int hpRestored = 0): Item(name, price), hpRestored(hpRestored) {}
    Potion(const Potion& obj): Item(obj) {this->hpRestored = obj.hpRestored;}

    Potion& operator =(const Potion&);

    void readFromFile(fstream&);
    void writeToFile(fstream&);

    void serialization(fstream&);
    void deserialization(fstream&);

    void show() const;
    void setAttributes();
    int use() const {return this->hpRestored;}

    ~Potion() {}
};

class Pokeball: public Item{
public:
    Pokeball(string name = "", int price = 0): Item(name, price) {}
    Pokeball(const Pokeball& obj): Item(obj) {}

    Pokeball& operator =(const Pokeball&);

    void readFromFile(fstream&);
    void writeToFile(fstream&);

    void serialization(fstream& f) {Item::serialization(f);}
    void deserialization(fstream& f) {Item::deserialization(f);}

    void show() const;
    int use() const;
    void setAttributes();

    ~Pokeball() {}
};

class Trainer;
class Pokemon{
    static const string allTypes[18];
    string species;
    int level;
    int maxHp;
    int hp;
    int attack;
    int speed;
    int exp;
    bool wild;
    string type[2];
    list<string> typeWeakness;
    list<string> typeResistance;
    map<string, pair<string, int>> moves;
    pair<string, int> evolutions[2];

public:
    Pokemon(): species(""), level(0), maxHp(0), hp(0), attack(0), speed(0), exp(0), wild(1), type{"", ""} {}
    Pokemon(string, int, int, int , int , int , int, bool, string[2], const list<string>&, const list<string>&,
             const map<string, pair<string, int>>&, pair<string, int>[2]);
    Pokemon(const Pokemon&);
    
    Pokemon& operator =(const Pokemon&);
    bool operator <(const Pokemon&) const;

    friend istream& operator >>(istream&, Pokemon&);
    friend ostream& operator <<(ostream&, const Pokemon&);

    void readFromFile(fstream&);
    void writeToFile(fstream&) const;

    void serialization(fstream&) const;
    void deserialization(fstream&);

    static void ShowAllTypes();

    int loss();
    bool caught(int);                                // See if you caught Pokemon
    bool fainted();                                  // End of battle
    bool battleReady() {return this->hp > 0;}

    void sendOutPokemon() {cout << "A wild " << this->species << " appeared! Level: " << this->level << "\n";}
    void receiveDamage(int);
    void win(int, bool caught = 0);                             // Gain exp after winning a battle
    void turn() {cout << this->species << "'s turn\n";}
    void winMessage(Pokemon*) {cout << this->species << " won\n";}
    void winMessage(Trainer*) {cout << this->species << " won\n";}                  

    map<int, pair<string, int>> calculateDamage(map<string, pair<string, int>>, const list<string>&, 
                                                const list<string>&);      // Calculates the damage for each move
    int useMove(const list<string>&, const list<string>&);   // A move is picked automatically
    int pickMove(const list<string>&, const list<string>&);  // Let's the user choose a move

    void evolve(const vector<Pokemon>&);
    void calculateStats(int);
    void levelUp(int caught = 0);
    void restoreHP(int value = -1);
    void learnMove(string, string, int);             // Learn a new move
    void checkNewMoves(const Pokemon&);
    void increaseMaxHP(int value);
    void increaseAttack(int value);
    void increaseSpeed(int value);

    int getHP() const {return this->hp;}
    int getMaxHP() const {return this->maxHp;}
    int getSpeed() const {return this->speed;}
    int getLevel() const {return this->level;}
    string getFirstType() const {return this->type[0];}
    string getSecondType() const {return this->type[1];}
    string getSpecies() const {return this->species;}
    void getTypeWeakness(list<string>& obj) const {obj = this->typeWeakness;}
    void getTypeResistance(list<string>& obj) const {obj = this->typeResistance;}
    void getMoves(map<string, pair<string, int>>& obj) const {obj = this->moves;}

    void resetHP() {this->hp = this->maxHp;}
    void setLevel(int level) {this->level = level;}
    void setWild(bool wild) {this->wild = wild;}
    void setMoves();

    ~Pokemon() {}
};

const string Pokemon::allTypes[] = {"grass", "fire", "water", "flying", "ground", "rock", "steel", "bug", 
"poison", "fighting", "dark", "ghost", "psychic", "fairy", "ice", "electric", "normal", "dragon"};

class Trainer{
protected:
    string name;
    int teamSize;                // Non-fainted pokemon
    vector<Pokemon> team;

public:
    Trainer(): name(""), teamSize(0) {}
    Trainer(string, const vector<Pokemon>&);
    Trainer(const Trainer&);

    Trainer& operator =(const Trainer&);

    friend istream& operator >>(istream& in, Trainer& obj) {return obj.read(in);}
    friend ostream& operator <<(ostream& os, const Trainer& obj) {return obj.show(os);}

    virtual istream& read(istream&);
    virtual ostream& show(ostream&) const;

    virtual void readFromFile(fstream&);
    virtual void writeToFile(fstream&);

    virtual void serialization(fstream&);
    virtual void deserialization(fstream&);

    virtual int loss() = 0;                     // Returns the level of the defeated Pokemon
    virtual void win(int) = 0;
    virtual void winMessage(Pokemon*) = 0;
    virtual void winMessage(Trainer*) = 0;
    virtual bool restoreHP(int, string = "") = 0;
    virtual int useMove(const list<string>&, const list<string>&) = 0;

    bool fainted();                             // End of battle, no Pokemon in team is able to battle
    bool battleReady() {return this->teamSize > 0 && this->team[0].battleReady();}
    void turn() {cout << "\n" << this->name << "'s turn\n";}
    void seeTeam();
    void resetHP();
    void sendOutPokemon();                      // Send out the Pokemon on the first position in team
    void receiveDamage(int);                    // Pokemon at position 0 receives damage
    void calculateTeamSize();                   // How many Pokemon are able to battle
    void calculateTeamStats();                  // Calculate the stats of a Pokemon with a given level

    int getSpeed() const;
    int getLevel() const;
    int getTeamSize() const {return this->teamSize;}
    int getAllTeamSize() const {return this->team.size();}
    void getTypeWeakness(list<string>&) const;
    void getTypeResistance(list<string>&) const;
    string getSpecies(int) const;
    string getName() const {return this->name;}
    string getFirstType() const;

    void setTeam(const vector<Pokemon>&);
    void setName(string name) {this->name = name;}

    virtual ~Trainer() {}
};

class PlayableTrainer: virtual public Trainer{
protected:
    int dollars;
    multiset<Pokemon> box;                        // Pokemon storage
    vector<Item*> items;

public:
    PlayableTrainer(): Trainer(), dollars(0) {}
    PlayableTrainer(string, const vector<Pokemon>&, int, const multiset<Pokemon>&, const vector<Item*>&);
    PlayableTrainer(const PlayableTrainer&);

    PlayableTrainer& operator =(const PlayableTrainer&);

    istream& read(istream&);
    ostream& show(ostream&) const;

    void readFromFile(fstream&);
    void writeToFile(fstream&);

    void serialization(fstream&);
    void deserialization(fstream&);
    
    int loss();                                      
    void win(int);
    void winMessage(Pokemon*);                                             // Can catch a wild Pokemon 
    void winMessage(Trainer*);                                             // Receive dollars from loser
    int useMove(const list<string>&, const list<string>&);                 // Calls pick move in Pokemon

    void seeBox();
    void seeItems();
    void buyItems();
    void putInBox();
    void seePokemon();
    void takeFromBox();
    void giveDollars();
    void trainPokemon(int);
    void releasePokemon();
    void evolve(const vector<Pokemon>&, int);
    void learnMove(const Pokemon&, int);
    void switchPokemon();
    int pickPokemon() const;
    bool checkPotions();
    bool restoreHP(int, string = "");

    int getDollars() const {return this->dollars;}

    ~PlayableTrainer();
};

class UnplayableTrainer: virtual public Trainer{
public:
    UnplayableTrainer(): Trainer() {}
    UnplayableTrainer(string name, const vector<Pokemon>& team): Trainer(name, team) {}
    UnplayableTrainer(const UnplayableTrainer& obj): Trainer(obj) {}

    UnplayableTrainer& operator =(const UnplayableTrainer&);

    void readFromFile(fstream& f) {Trainer::readFromFile(f);}
    void writeToFile(fstream& f) {Trainer::writeToFile(f);}

    void serialization(fstream& f) {Trainer::serialization(f);}
    void deserialization(fstream& f) {Trainer::deserialization(f);}

    int loss();
    void win(int);
    void winMessage(Pokemon*) {cout << this->name << " won\n";}
    void winMessage(Trainer*) {cout << this->name << " won\n";}
    int useMove(const list<string>&, const list<string>&);                  // Calls use move in Pokemon

    bool restoreHP(int, string = "");

    ~UnplayableTrainer() {}
};

class HybridTrainer: public PlayableTrainer, public UnplayableTrainer{
    int gymBadges;
    bool autoPlay;

public:
    HybridTrainer(): Trainer(), PlayableTrainer(), UnplayableTrainer(), autoPlay(0), gymBadges(0) {}
    HybridTrainer(string, const vector<Pokemon>&, int, const multiset<Pokemon>&, const vector<Item*>&, bool, int);
    HybridTrainer(const HybridTrainer&);

    HybridTrainer& operator =(const HybridTrainer&);

    istream& read(istream&);
    ostream& show(ostream&) const;

    void readFromFile(fstream&);
    void writeToFile(fstream&);

    virtual void serialization(fstream&);
    virtual void deserialization(fstream&);

    int loss();
    void win(int);
    void winMessage(Pokemon*);
    void winMessage(Trainer* obj) {PlayableTrainer::winMessage(obj);}
    int useMove(const list<string>&, const list<string>&);             

    bool restoreHP(int, string = "");
    void toggleAutoPlay();
    void increaseGymBadges() {++this->gymBadges;}

    int getAutoPlay() {return this->autoPlay;}
    int getGymBadges() {return this->gymBadges;}

    // For start of the game
    void setDollars(int dollars) {this->dollars = dollars;}
    void setItems(vector<Item*> items) {this->items = items;}

    ~HybridTrainer() {}
};

// This class works with pointers of type Pokemon* and any Trainer type
// Any combination works
// It must modify the actual objects
template <class T, class U> 
class Battle{
    T first;
    U second;

public:
    Battle(): first(nullptr), second(nullptr) {}
    Battle(T first, U second): first(first), second(second) {}
    Battle(const Battle<T, U>&  obj): first(obj->first), second(obj->second) {}

    Battle<T, U>& operator =(const Battle<T, U>& obj);

    // Parameters have to be different
    template <class T1, class U1>
    friend ostream& operator <<(ostream&, const Battle<T1, U1>& obj);

    void doBattle();
    void showBattleInformation();

    ~Battle() {}
}; 

class Game{
    string region;  
    HybridTrainer player;                   
    UnplayableTrainer gymLeaders[8];
    UnplayableTrainer eliteFour[4];
    UnplayableTrainer champion;
    vector<string> trainerNames;
    vector<Pokemon> inGamePokemon;
    pair<pair<int, int>, pair<string, pair<vector<string>, vector<string>> >> currentLocation; 
    int trainersBattled;  
    static fstream trainersFile;                      // Contains the trainers battled in link battles in a session
    static fstream databaseFile;                      // Contains all the Pokemon species in the game  
    static string playerFile;                         // Contains details of the current player     
    const static map<pair<int, int>, pair<string, pair<vector<string>, vector<string>> >> locations;  

    Game(string filename = "");
    Game(const Game&) = delete;
    Game& operator =(const Game&) = delete;
    ~Game();

    Trainer* readLinkBattleTrainer();
    Trainer* generateRandomTrainer(const vector<Pokemon>&);
    Trainer* rebattleLinkBattleTrainer();

public:
    static Game& getInstance();
    void startGame();

    friend ostream& operator <<(ostream&, const Game&);

    void showGymLeaders();
    void showEliteChampion();
    void showInGamePokemon();
    void writeDatabasePokemon();

    static void linkBattleTrainers();
    static void seeLocationInformation();
    const vector<Pokemon>& getInGamePokemon() {return this->inGamePokemon;}

    void newGame();
    void buyItems() {this->player.buyItems();}     
    void goToLocation();
    void putInBox() {this->player.putInBox();}
    void takeFromBox() {this->player.takeFromBox();}
    void toggleAutoPlay() {this->player.toggleAutoPlay();}

    void linkBattle();
    void seeTrainersBattled();
    void battleWildPokemon();
    void battleTrainer();
    void challengeGym();
    void challengeElite();
    void challengeChampion();

    int pickPokemon() {return this->player.pickPokemon();}

    void heal();
    void evolve(); 
    void learnMove();
    void trainPokemon();
    void playerInformation();
    void seeBox() {this->player.seeBox();}
    void seeTeam() {this->player.seeTeam();}
    void seeItems() {this->player.seeItems();}
    void seePokemon() {this->player.seePokemon();}
    void switchPokemon() {this->player.switchPokemon();}
    void releasePokemon() {this->player.releasePokemon();}

    void getAllPossiblePokemon(vector<Pokemon>&);
    void generateRandomPokemon(const vector<Pokemon>&, Pokemon&);

    Pokemon searchDatabase(string);
}; 

string Game::playerFile = "";

fstream Game::trainersFile("trainers.txt", ios::binary | ios::in | ios::out | ios::trunc);
fstream Game::databaseFile("database.txt");

const map<pair<int, int>, pair<string, pair<vector<string>, vector<string>> >> Game::locations = {
    {{2, 5}, {"Route 200", {{"grass", "bug", "flying", "normal"}, {"ice", "psychic", "dark", "ghost", "dragon", "steel", "fighting"}}}},
    {{5, 9}, {"Route 201", {{"bug", "electric", "poison", "normal", "grass"}, {"dragon", "steel", "ice"}}}}, 
    {{8, 12}, {"Route 202", {{"water", "flying", "electric", "psychic"}, {"dragon", "dark", "ice"}}}},
    {{12, 17}, {"Chargestone Cave", {{"rock", "electric", "poison"}, {"water", "flying", "fire", "grass", "ice"}}}},
    {{18, 25}, {"Deep Chargestone Cave", {{"rock", "steel", "electric", "dark", "ghost"}, {"flying", "fire", "grass", "ice"}}}},
    {{19, 21}, {"Route 203", {{"fighting", "fire", "flying"}, {}}}},
    {{25, 29}, {"Withering Desert", {{"ground", "rock"}, {"grass", "fire", "water", "dragon", "ice"}}}},
    {{30, 36}, {"Underground Withering Desert", {{"ground", "rock", "dark", "psychic", "ghost"}, {"grass", "fire", "water", "dragon", "ice"}}}},
    {{27, 33}, {"Grove of Whispers", {{"psychic", "fairy", "grass", "poison"}, {"normal", "dragon", "steel", "ice", "fighting"}}}},
    {{35, 37}, {"Celestial Tower", {{"ghost"}, {}}}},
    {{36, 38}, {"Route 204", {{"flying", "fire", "grass", "bug", "electric"}, {}}}},
    {{37, 42}, {"Lake Afar", {{"water"}, {}}}},
    {{41, 45}, {"Glacial Cavern", {{"ice"}, {}}}},
    {{45, 49}, {"Deep Glacial Cavern", {{"ice", "dark", "psychic"}, {"flying", "bug", "grass", "fire", "normal", "dragon"}}}},
    {{49, 52}, {"Route 205", {{"normal", "fighting", "flying", "psychic"}, {}}}},
    {{51, 57}, {"Dusk Forest", {{"grass", "dark", "ghost", "bug", "poison"}, {"water", "dragon", "fire", "ice"}}}},
    {{56, 61}, {"Wayward Cave", {{"dragon", "ground", "rock", "steel", "poison", "fighting"}, {"flying", "grass", "ice"}}}},
    {{60, 63}, {"Route 206", {{"dragon", "flying", "fighting", "normal"}, {}}}},
    {{63, 67}, {"Mystic Lake", {{"water", "ice", "fairy"}, {"poison", "steel"}}}},
    {{66, 70}, {"Route 207", {{"electric", "fire", "grass", "bug", "normal", "flying"}, {}}}},
    {{70, 75}, {"Victory Road", {{"dragon", "fighting", "poison", "psychic"}, {}}}},
    {{74, 80}, {"Underground Victory Road", {{"dragon", "ground", "rock", "steel", "dark"}, {"flying"}}}},
    {{80, 85}, {"Elsewhere Forest", {{"grass", "psychic", "fairy", "ghost"}, {"steel"}}}},
    {{85, 90}, {"Fire Island", {{"fire", "ground", "rock", "dark"}, {"water", "grass", "bug", "ice"}}}},
    {{90, 95}, {"Lake Acuity", {{"water", "ice"}, {}}}},
    {{100, 100}, {"Battle Zone", {{"grass", "fire", "water", "flying", "ground", "rock", "steel", "bug", "poison",
                "fighting", "dark", "ghost", "psychic", "fairy", "ice", "electric", "normal", "dragon"}, {}}}}
};

// Item class implementations
Item& Item::operator =(const Item& obj){
    if(this != &obj){
        this->name = obj.name;
        this->price = obj.price;
    }
    return *this;
}

void Item::serialization(fstream& f){
    serializeString(this->name, f);
    f.write((char*)&(this->price), sizeof(this->price));
}

void Item::deserialization(fstream& f){
    deserializeString(this->name, f);
    f.read((char*)&(this->price), sizeof(this->price));
}

// Potion class implementations
Potion& Potion::operator =(const Potion& obj){
    if(this != &obj){
        Item::operator =(obj);
        this->hpRestored = obj.hpRestored;
    }
    return *this;
}

void Potion::writeToFile(fstream& f){
    f << this->name << '\n' << this->price << ' ' << this->hpRestored << "\n";
}

void Potion::readFromFile(fstream& f){
    f.get();                        // Get newline character
    getline(f, this->name);
    f >> this->price >> this->hpRestored;
}

void Potion::serialization(fstream& f){
    Item::serialization(f);
    f.write((char*)&(this->hpRestored), sizeof(this->hpRestored));
}

void Potion::deserialization(fstream& f){
    Item::deserialization(f);
    f.read((char*)&(this->hpRestored), sizeof(this->hpRestored));
}

void Potion::show() const{
    Item::show();
    cout << " HP restored: " << this->hpRestored << "\n";
}

void Potion::setAttributes(){
    cout << "Enter the name\n";
    bool ok = 0;
    while(!ok){
        try{
            cout << "1. Potion\n";
            cout << "2. Super Potion\n";
            cout << "3. Hyper Potion\n";
            cout << "4. Max Potion\n";
            int k;
            cin >> k;

            if(cin.fail())
                throw 1;
            
            switch(k){
                case 1:{
                    this->name = "Potion";
                    this->price = 20;
                    this->hpRestored = 20;
                    ok = 1;
                    break;
                }

                case 2:{
                    this->name = "Super Potion";
                    this->price = 50;
                    this->hpRestored = 60;
                    ok = 1;
                    break;
                }

                case 3:{
                    this->name = "Hyper Potion";
                    this->price = 70;
                    this->hpRestored = 120;
                    ok = 1;
                    break;
                }

                case 4:{
                    this->name = "Max Potion";
                    this->price = 100;
                    this->hpRestored = 1000;
                    ok = 1;
                    break;
                }

                default:{
                    throw 'c';
                }
            }
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
            ok = 0;
        }
        catch(char c){
            cout << "Invalid value\n";
            ok = 0;
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

// Pokeball class implementations
Pokeball& Pokeball::operator =(const Pokeball& obj){
    if(this != &obj){
        Item::operator =(obj);
    }
    return *this;
}

void Pokeball::writeToFile(fstream& f){
    f << this->name << '\n' << this->price << "\n";
}

void Pokeball::readFromFile(fstream& f){
    f.get();
    getline(f, this->name);
    f >> this->price;
}

int Pokeball::use() const{
    if(this->name == "Pokeball"){
        uniform_int_distribution<int> random(0, 90);
        return random(eng);
    }
    else if(this->name == "Great Ball\n"){
        uniform_int_distribution<int> random(0, 60);
        return random(eng);
    }
    else if(this->name == "Ultra Ball\n"){
        uniform_int_distribution<int> random(0, 45);
        return random(eng);
    }   
}

void Pokeball::show() const{
    Item::show();
    cout << "\n";
}

void Pokeball::setAttributes(){
    cout << "Enter the name\n";
    bool ok = 0;
    while(!ok){
        try{
            cout << "1. Pokeball\n";
            cout << "2. Great Ball\n";
            cout << "3. Ultra Ball\n";
            int k;
            cin >> k;

            if(cin.fail())
                throw 1;

            switch(k){
                case 1:{
                    this->name = "Pokeball";
                    this->price = 20;
                    ok = 1;
                    break;
                }

                case 2:{
                    this->name = "Great Ball";
                    this->price = 40;
                    ok = 1;
                    break;
                }

                case 3:{
                    this->name = "Ultra Ball";
                    this->price = 60;
                    ok = 1;
                    break;
                }

                default:{
                    throw 'c';
                }
            }
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
            ok = 0;
        }
        catch(char c){
            cout << "Invalid value\n";
            ok = 0;
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

// Pokemon class implementations
Pokemon::Pokemon(string species, int level, int maxHp, int hp, int attack, int speed, int exp, bool wild, 
    string type[2], const list<string>& typeWeakness, const list<string>& typeResistance, const map<string, 
    pair<string, int>>& moves, pair<string, int> evolutions[2]){
    this->species = species;
    this->level = level;
    this->maxHp = maxHp;
    this->hp = hp;
    this->attack = attack;
    this->speed = speed;
    this->exp = exp;
    this->wild = wild;
    this->type[0] = type[0];
    this->type[1] = type[1];
    this->typeWeakness = typeWeakness;
    this->typeResistance = typeResistance;
    this->moves = moves;
    this->evolutions[0] = evolutions[0];
    this->evolutions[1] = evolutions[1];
}

Pokemon::Pokemon(const Pokemon& obj){
    this->species = obj.species;
    this->level = obj.level;
    this->maxHp = obj.maxHp;
    this->hp = obj.hp;
    this->attack = obj.attack;
    this->speed = obj.speed;
    this->exp = obj.exp;
    this->wild = obj.wild;
    this->type[0] = obj.type[0];
    this->type[1] = obj.type[1];
    this->typeWeakness = obj.typeWeakness;
    this->typeResistance = obj.typeResistance;
    this->moves = obj.moves;
    this->evolutions[0] = obj.evolutions[0];
    this->evolutions[1] = obj.evolutions[1];
}

Pokemon& Pokemon::operator =(const Pokemon& obj){
    if(this != &obj){
        this->species = obj.species;
        this->level = obj.level;
        this->maxHp = obj.maxHp;
        this->hp = obj.hp;
        this->attack = obj.attack;
        this->speed = obj.speed;
        this->exp = obj.exp;
        this->wild = obj.wild;
        this->type[0] = obj.type[0];
        this->type[1] = obj.type[1];
        this->typeWeakness = obj.typeWeakness;
        this->typeResistance = obj.typeResistance;
        this->moves = obj.moves;
        this->evolutions[0] = obj.evolutions[0];
        this->evolutions[1] = obj.evolutions[1];
    }
    return *this;
}

istream& operator >>(istream& in, Pokemon& obj){
    cout << "Enter species\n";
    in >> obj.species;
    
    bool ok = 0;
    while(!ok){
        try{
            cout << "Enter level\n";
            in >> obj.level;
            if(in.fail())
                throw 1;
            else{
                if(obj.level > 100)
                    throw 3.4;
                else if(obj.level < 1)
                    throw 'c';
            }

            cout << "Enter maximum hp\n";
            in >> obj.maxHp;
            if(in.fail())
                throw 1;
            else{
                if(obj.maxHp < 1)
                    throw 'c';
            }

            cout << "Enter hp\n";
            in >> obj.hp;
            if(in.fail())
                throw 1;
            else{
                if(obj.hp < 1)
                    throw 'c';
                else if(obj.hp > obj.maxHp)
                    throw "cc";
            }
                
            cout << "Enter attack\n";
            in >> obj.attack;
            if(in.fail())
                throw 1;
            else{
                if(obj.attack < 1)
                    throw 'c';
            }
                
            cout << "Enter speed\n";
            in >> obj.speed;
            if(in.fail())
                throw 1;
            else{
                if(obj.speed < 1)
                    throw 'c';
            }   

            cout << "Enter remaining exp to next level\n";
            in >> obj.exp;
            if(in.fail())
                throw 1;
            else{
                if(obj.exp < 1)
                    throw 'c';
            }

            cout << "Enter whether the Pokemon is wild or not\n";
            in >> obj.wild;
            if(in.fail())
                throw 1;

            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            in.clear();
            in.ignore(100, '\n');
        }
        catch(char c){
            cout << "This attribute can't have a negative value\n";
        }
        catch(double d){
            cout << "Level can't have a value over 100\n";
        }
        catch(const char* cc){
            cout << "HP can't have a value that's greater than Max HP\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    ok = 0;
    while(!ok){
        obj.type[0] = obj.type[1] = "";
        cout << "1. Single type\n";
        cout << "2. Dual type\n";
        int k;

        try{
            cin >> k;
            if(cin.fail())
                throw 1;
            else 
                ok = 1;
            
            if(k == 1){
                cout << "Enter type\n";
                in >> obj.type[0];

                bool found = 0;
                for(int i = 0; i < 18; i++)
                if(obj.type[0] == Pokemon::allTypes[i]){
                    found = 1;
                    break;
                }
                if(!found)
                    throw "cc";
                else 
                    ok = 1;
            }
            else if(k == 2){
                cout << "Enter first type\n";
                in >> obj.type[0];

                bool found = 0;
                for(int i = 0; i < 18; i++)
                if(obj.type[0] == Pokemon::allTypes[i]){
                    found = 1;
                    break;
                }
                if(!found)
                    throw "cc";
                else 
                    ok = 1;

                cout << "Enter second type\n";
                in >> obj.type[1];

                found = 0;
                for(int i = 0; i < 18; i++)
                if(obj.type[1] == Pokemon::allTypes[i]){
                    found = 1;
                    break;
                }
                if(!found)
                    throw "cc";
                else 
                    ok = 1;
            }
                else
                    throw 'c';
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
            ok = 0;
        }
        catch(char c){
            cout << "Invalid value\n";
            ok = 0;
        }
        catch(const char*){
            cout << "Invalid type\n";
            ok = 0;
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    if(!obj.typeWeakness.empty())
        obj.typeWeakness.clear();

    ok = 0;
    while(!ok){
        try{
            while(true){
                cout << "1. Enter type weakness\n";
                cout << "2. Stop\n";
                int k;
                cin >> k;
                if(cin.fail())
                    throw 1;
                
                if(k == 1){
                    string s;
                    in >> s;
                    // Transform to lowercase to match allTypes elements
                    for(int i = 0; i < s.size(); i++)
                        s[i] = tolower(s[i]);

                    bool found = 0;
                    for(int i = 0; i < 18; i++)
                        if(s == Pokemon::allTypes[i]){
                            found = 1;
                            break;
                        }
                    if(!found)
                        throw "cc";
                    else
                        ok = 1;

                    obj.typeWeakness.push_back(s);
                }
                else if(k == 2){
                    ok = 1;
                    break;
                }
                    else
                        throw 'c';
            }
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(const char* cc){
            cout << "Invalid type\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    if(!obj.typeResistance.empty())
        obj.typeResistance.clear();

    ok = 0;
    while(!ok){
        try{
            while(true){
                cout << "1. Enter type resistance\n";
                cout << "2. Stop\n";
                int k;
                cin >> k;
                if(cin.fail())
                    throw 1;
                
                if(k == 1){
                    string s;
                    in >> s;
                    for(int i = 0; i < s.size(); i++)
                        s[i] = tolower(s[i]);

                    bool found = 0;
                    for(int i = 0; i < 18; i++)
                        if(s == Pokemon::allTypes[i]){
                            found = 1;
                            break;
                        }
                    if(!found)
                        throw "cc";
                    else
                        ok = 1;

                    obj.typeResistance.push_back(s);
                }
                else if(k == 2){
                    ok = 1;
                    break;
                }
                    else
                        throw 'c';
            }
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
            ok = 0;
        }
        catch(char c){
            cout << "Invalid value\n";
            ok = 0;
        }
        catch(const char* cc){
            cout << "Invalid type\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    if(!obj.moves.empty())
        obj.moves.clear();

    ok = 0;
    while(!ok){
        try{
            while(true){
                cout << "1. Enter move\n";
                cout << "2. Stop\n";

                int k;
                    cin >> k;
                    if(cin.fail())
                        throw 6.7;
                
                    if(k == 1){
                        string name, type;
                        int power;
                        cout << "Enter move name\n";
                        in.ignore();
                        getline(in, name);

                        cout << "Enter move type\n";
                        in >> type;

                        bool found = 0;
                        for(int i = 0; i < 18; i++)
                            if(type == Pokemon::allTypes[i]){
                                found = 1;
                                break;
                            }
                        if(!found)
                            throw "cc";
                        
                        cout << "Enter move power\n";
                        in >> power;

                        if(in.fail())
                            throw 1;
                        else if(power < 1)
                            throw false;
                        
                        ok = 1;
                        obj.moves.insert({name, {type, power}});
                    }
                    else if(k == 2){
                        if(obj.moves.size() == 0)
                            throw 'c';
                        ok = 1;
                        break;
                    }
                        else
                            throw 6.7;
            }
        }
        catch(int x){
            cout << "Invalid input\n";
            in.clear();
            in.ignore(100, '\n');
            ok = 0;
        }
        catch(char c){
            cout << "Must have at least one move\n";
            ok = 0;
        }
        catch(double d){
            cout << "Invalid value\n";
            cin.clear();
            cin.ignore(100, '\n');
            ok = 0;
        }
        catch(const char* cc){
            cout << "Invalid type\n";
            in.clear();
            in.ignore(100, '\n');
            ok = 0;
        }
        catch(bool b){
            cout << "Attack power needs to have a positive value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    ok = 0;
    while(!ok){
        obj.evolutions[0].first = obj.evolutions[1].first = "-";
        obj.evolutions[0].second = obj.evolutions[1].second = 0;
        cout << "1. One evolution\n2. Two evolution\n3. No evolutions\n";
        int k;
        cin >> k;

        try{
            if(cin.fail())
                throw 1;
            if(k < 1 || k > 3)
                throw 'c';
            
            if(k <= 2){
                cout << "Enter species:\n";
                in >> obj.evolutions[0].first;
                cout << "Enter level:\n";
                in >> obj.evolutions[0].second;

                if(in.fail())
                    throw 3.4;
                if(obj.evolutions[0].second < 1 || obj.evolutions[0].second > 100)
                    throw 'c';
            }
            if(k == 2){
                cout << "Enter species:\n";
                in >> obj.evolutions[1].first;
                cout << "Enter level:\n";
                in >> obj.evolutions[1].second;

                if(in.fail())
                    throw 3.4;
                if(obj.evolutions[1].second < 1 || obj.evolutions[1].second > 100)
                    throw 'c';
            }
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(double d){
            cout << "Invalid input\n";
            in.clear();
            in.ignore(100, '\n');
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    return in;
}

ostream& operator <<(ostream& os, const Pokemon& obj){
    os << "Species: " << obj.species << "\n";
    os << "Level: " << obj.level << "\n";
    os << "Max HP: " << obj.maxHp << "\n";
    os << "Current HP: " << obj.hp << "\n";
    os << "Attack: " << obj.attack << "\n";
    os << "Speed: " << obj.speed << "\n";
    os << "Remaining exp to next level: " << obj.exp << "\n";
    os << "Wild: " << obj.wild << "\n";

    os << "Type: " << obj.type[0];
    if(obj.type[1] != "")
        os << " " << obj.type[1];
    os << "\n";

    os << "Type weaknesses: ";
    for(auto it = obj.typeWeakness.begin(); it != obj.typeWeakness.end(); it++)
        os << *it << " ";
    os << "\n";

    os << "Type resistances: ";
    for(auto it = obj.typeResistance.begin(); it != obj.typeResistance.end(); it++)
        os << *it << " ";
    os << "\n";

    os << "Moves:\n";
    for(auto it = obj.moves.begin(); it != obj.moves.end(); it++){
        os << it->first << " " << it->second.first << " " << it->second.second << "\n";
    }

    os << "Evolutions:\n";
    if(obj.evolutions[0].first != "-")
        os << "Species: " << obj.evolutions[0].first << " Level: " << obj.evolutions[0].second << "\n";
    if(obj.evolutions[1].first != "-")
        os << "Species: " << obj.evolutions[1].first << " Level: " << obj.evolutions[1].second << "\n";

    return os;
}

bool Pokemon::operator <(const Pokemon& obj) const{
    if(this->species < obj.species)
        return 1;
    return 0;
}

void Pokemon::ShowAllTypes(){
    for(int i = 0; i < 18; i++)
        cout << allTypes[i] << "\n";
}

void Pokemon::readFromFile(fstream& f){
    f >> this->species;
    if(this->species == "")
        return;

    f >> this->level >> this->maxHp >> this->hp >> this->attack >> this->speed 
        >> this->exp >> this->wild;
    int number;

    // Read type
    f >> number;
    if(number == 1){
        f >> this->type[0];
        this->type[1] = "";
    }
    else if(number == 2){
        f >> this->type[0] >> this->type[1];
    }

    // Read evolutions
    f >> this->evolutions[0].first >> this->evolutions[0].second 
                >> this->evolutions[1].first >> this->evolutions[1].second;

    // Read typeWeakness
    if(!this->typeWeakness.empty())
        this->typeWeakness.clear();

    string words;
    f >> number;
    for(int i = 0; i < number; i++){
        f >> words;
        this->typeWeakness.push_back(words);
    }
    // Read typeResistance
    if(!this->typeResistance.empty())
        this->typeResistance.clear();

    f >> number;
    for(int i = 0; i < number; i++){
        f >> words;
        this->typeResistance.push_back(words);
    }
    // Read moves
    if(!this->moves.empty())
        this->moves.clear();

    f >> number;
    string type;
    int power;
    for(int i = 0; i < number; i++){
        f.get();                              // Get the \n character
        getline(f, words);
        f >> type >> power;
        this->moves[words] = {type, power};
    }
}

void Pokemon::writeToFile(fstream& g) const{
    g << this->species << "\n" << this-> level << ' ' << this->maxHp << ' ' << this->hp << ' ' << this->attack 
        << ' ' << this->speed << ' ' << this->exp << ' ' << this->wild << "\n";
    // Write type
    if(this->type[1] == ""){
        g << 1 << ' ' << this->type[0] << "\n";
    }
    else{
        g << 2 << ' ' << this->type[0] << ' ' << this->type[1] << "\n";
    }

    // Write evolutions
    g << this->evolutions[0].first << " " << this->evolutions[0].second << " "
        << this->evolutions[1].first << " " << this->evolutions[1].second << "\n";

    // Write typeWeakness
    g << this->typeWeakness.size() << ' ';
    for(auto it = this->typeWeakness.begin(); it != this->typeWeakness.end(); it++)
        g << *it << ' ';
    g << "\n";

    // Write typeResistance
    g << this->typeResistance.size() << ' ';
    for(auto it = this->typeResistance.begin(); it != this->typeResistance.end(); it++)
        g << *it << ' ';
    g << "\n";

    // Write moves
    g << this->moves.size() << "\n";
    for(auto it = this->moves.begin(); it != this->moves.end(); it++){
        g << it->first << "\n";
        g <<  it->second.first << ' ' << it->second.second << "\n";
    }
}

void Pokemon::serialization(fstream& f) const{
    serializeString(this->species, f);

    int size; 

    f.write((char*)&(this->level), sizeof(this->level));
    f.write((char*)&(this->maxHp), sizeof(this->maxHp));
    f.write((char*)&(this->hp), sizeof(this->hp));
    f.write((char*)&(this->attack), sizeof(this->attack));
    f.write((char*)&(this->speed), sizeof(this->speed));

    f.write((char*)&(this->exp), sizeof(this->exp));
    f.write((char*)&(this->wild), sizeof(this->wild));


    // Write types
    serializeString(this->type[0], f);
    serializeString(this->type[1], f);

    // Write complex member variables
    size = this->typeWeakness.size();
    f.write((char*)&size, sizeof(size));

    for(auto it = this->typeWeakness.begin(); it != this->typeWeakness.end(); it++)
        serializeString(*it, f);

    size = this->typeResistance.size();
    f.write((char*)&size, sizeof(size));

    for(auto it = this->typeResistance.begin(); it != this->typeResistance.end(); it++)
        serializeString(*it, f);

    size = this->moves.size();
    f.write((char*)&size, sizeof(size));

    for(auto it = this->moves.begin(); it != this->moves.end(); it++){
        // Write move name
        serializeString(it->first, f);

        // Write move type
        serializeString(it->second.first, f);
        
        // Write move power
        f.write((char*)&(it->second.second), sizeof(it->second.second));
    }

    //Write evolutions
    serializeString(this->evolutions[0].first, f);
    f.write((char*)&(this->evolutions[0].second), sizeof(this->evolutions[0].second));

    serializeString(this->evolutions[1].first, f);
    f.write((char*)&(this->evolutions[1].second), sizeof(this->evolutions[1].second));
}

void Pokemon::deserialization(fstream& f){
    deserializeString(this->species, f);

    f.read((char*)&(this->level), sizeof(this->level));
    f.read((char*)&(this->maxHp), sizeof(this->maxHp));
    f.read((char*)&(this->hp), sizeof(this->hp));
    f.read((char*)&(this->attack), sizeof(this->attack));
    f.read((char*)&(this->speed), sizeof(this->speed));

    f.read((char*)&(this->exp), sizeof(this->exp));
    f.read((char*)&(this->wild), sizeof(this->wild));

    deserializeString(this->type[0], f);
    deserializeString(this->type[1], f);

    // Read complex member variables
    // typeWeakness
    if(!this->typeWeakness.empty())
        this->typeWeakness.clear();

    size_t size;
    f.read((char*)&size, sizeof(size));

    for(int i = 0; i < size; i++){
        string s;
        deserializeString(s, f);
        this->typeWeakness.push_back(s);
    }

    // typeResistance
    if(!this->typeResistance.empty())
        this->typeResistance.clear();

    f.read((char*)&size, sizeof(size));

    for(int i = 0; i < size; i++){
        string s;
        deserializeString(s, f);
        this->typeResistance.push_back(s);
    }

    // Moves
    if(!this->moves.empty())
        this->moves.clear();

    f.read((char*)&size, sizeof(size));

    for(int i = 0; i < size; i++){
        string move;
        deserializeString(move, f);

        string type;
        deserializeString(type, f);

        int power;
        f.read((char*)&power, sizeof(power));

        this->moves[move] = {type, power};
    }
    // Evolutions
    deserializeString(this->evolutions[0].first, f);
    f.read((char*)&(this->evolutions[0].second), sizeof(this->evolutions[0].second));

    deserializeString(this->evolutions[1].first, f);
    f.read((char*)&(this->evolutions[1].second), sizeof(this->evolutions[1].second));
}

void Pokemon::levelUp(int caught){
    // Maximum level is 100
    if(this->level >= 100)
        return;

    // Only show message for Pokemon that belong to a Playable or Hybrid Trainer
    if(caught){
        cout << "\n" << this->species << " leveled up!\n";
        cout << "Level: " << this->level << " -> ";
        ++this->level;
        cout << this->level << "\n";

        // HP increases faster than the other stats
        uniform_int_distribution<int> hp(0, 7);
        
        cout << "HP: " << this->maxHp << " -> "; 
        this->maxHp += hp(eng);
        cout << this->maxHp << "\n";

        uniform_int_distribution<int> number(0, 4);

        cout << "Attack: " << this->attack << " -> "; 
        this->attack += number(eng);
        cout << this->attack << "\n";

        cout << "Speed: " << this->speed << " -> "; 
        this->speed += hp(eng);
        cout << this->speed << "\n";
    }
    else{
        ++this->level;
        uniform_int_distribution<int> hp(0, 7);      
        this->maxHp += hp(eng);
        uniform_int_distribution<int> number(0, 4);       
        this->attack += number(eng);
        this->speed += number(eng);
    }
    this->exp = this->level * 100;
}

void Pokemon::checkNewMoves(const Pokemon& inGamePokemon){
    map<string, pair<string, int>> newMoves, learnableMoves;
    inGamePokemon.getMoves(learnableMoves);
    
    // newMoves will contain only the moves the Pokemon doesn't know already
    if(1 <= this->level && this->level <= 13){
        for(auto it = learnableMoves.begin(); it != learnableMoves.end(); it++)
            if(it->second.second <= 50 && this->moves.find(it->first) == this->moves.end())
                newMoves.insert(*it);
    }
    else if(14 <= this->level && this->level <= 25){
        for(auto it = learnableMoves.begin(); it != learnableMoves.end(); it++)
            if(it->second.second <= 65 && this->moves.find(it->first) == this->moves.end())
                newMoves.insert(*it);
    }
    else if(26 <= this->level && this->level <= 40){
        for(auto it = learnableMoves.begin(); it != learnableMoves.end(); it++)
            if(it->second.second <= 80 && this->moves.find(it->first) == this->moves.end())
                newMoves.insert(*it);
    }
    else if(41 <= this->level && this->level <= 55){
        for(auto it = learnableMoves.begin(); it != learnableMoves.end(); it++)
            if(it->second.second <= 90 && this->moves.find(it->first) == this->moves.end())
                newMoves.insert(*it);
    }
    else{
        for(auto it = learnableMoves.begin(); it != learnableMoves.end(); it++)
            if(this->moves.find(it->first) == this->moves.end())
                newMoves.insert(*it);
    }

    if(!newMoves.size()){
        cout << this->species << " can't learn any new move\n";
        return;
    }

    cout << "Select which move " << this->species << " should learn\n";
    int k = 1;
    for(auto it = newMoves.begin(); it != newMoves.end(); it++){
        cout << k << ". " << it->first << " " << it->second.first << " " << it->second.second << "\n";
        k++;
    }
    cout << k << ". Exit\n";

    bool ok = 0;
    while(!ok){
        try{
            int k2;
            cin >> k2;
            if(cin.fail())
                throw 1;
            
            if(k2 == k)
                return ;
            
            if(k2 < 1 || k2 > k)
                throw 'c';
            
            auto it = newMoves.begin();
            for(int i = 1; i < k2; i++)
                ++it;
            
            this->learnMove(it->first, it->second.first, it->second.second);
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void Pokemon::evolve(const vector<Pokemon>& inGamePokemon){
    // Check if the Pokemon can evolve
    if(this->evolutions[0].first != "-" && this->level >= this->evolutions[0].second){
        cout << this->species << " can evolve into " << this->evolutions[0].first << "\n";
        cout << "1. Evolve\n2. Don't evolve\n";

        bool ok = 0;
        while(!ok){
            try{
                int k;
                cin >> k;
                if(cin.fail())
                    throw 1;
                if(k == 1){
                    cout << this->species << " evolved into " << this->evolutions[0].first << "!\n";
                    // Change species
                    this->species = this->evolutions[0].first;
                    
                    uniform_int_distribution<int> number(1, 9);
                    // Raise stats
                    cout << this->maxHp << " -> ";
                    this->maxHp += number(eng);
                    cout << this->maxHp << "\n";

                    cout << this->attack << " -> ";
                    this->attack += number(eng);
                    cout << this->attack << "\n";

                    cout << this->speed << " -> ";
                    this->speed += number(eng);
                    cout << this->speed << "\n";

                    // Repair evolutions array
                    this->evolutions[0].first = this->evolutions[1].first;
                    this->evolutions[0].second = this->evolutions[1].second;
                    this->evolutions[1].first = "-";
                    this->evolutions[1].second = 0;

                    // Pokemon type may change during evolution
                    for(int i = 0; i < inGamePokemon.size(); i++){
                        if(this->species == inGamePokemon[i].getSpecies()){

                            this->type[0] = inGamePokemon[i].getFirstType();
                            this->type[1] = inGamePokemon[i].getSecondType();

                            inGamePokemon[i].getTypeWeakness(this->typeWeakness);
                            inGamePokemon[i].getTypeResistance(this->typeResistance);
                            break;
                        }
                    }
                }
                else if(k != 2)
                    throw 'c';
                ok = 1;
            }
            catch(int x){
                cout << "Invalid input\n";
                cin.clear();
                cin.ignore(100, '\n');
            }
            catch(char c){
                cout << "Invalid value\n";
            }
            catch(...){
                cout << "Error\n";
                exit(1);
            }
        }
    }
    else
        cout << "This Pokemon can't evolve\n";
}

void Pokemon::calculateStats(int level){
    // Calculate stats for a Pokemon of a given level
    // Everything is set to 0 and recalculated
    this->level = this->maxHp = this->attack = this->speed = 0;

    for(int i = 0; i < level; i++){
        this->exp = 0;
        this->levelUp();
    }

    this->hp = this->maxHp;
    this->level = level;
}

void Pokemon::increaseAttack(int value){
    cout << this->species << "'s attack increased!\n";
    cout << this->attack << " -> ";
    this->attack += value;
    cout << this->attack << "\n";
}

void Pokemon::increaseSpeed(int value){
    cout << this->species << "'s speed increased!\n";
    cout << this->speed << " -> ";
    this->speed += value;
    cout << this->speed << "\n";
}

void Pokemon::increaseMaxHP(int value){
    cout << this->species << "'s HP increased!\n";
    cout << this->maxHp << " -> ";
    this->maxHp += value;
    cout << this->maxHp << "\n";
}

void Pokemon::restoreHP(int value){
    // Heal with Potion
    if(value != -1){
        cout << this->species << "'s HP was restored\n" << this->hp << " -> ";
        this->hp = min(this->maxHp, this->hp+value);
        cout << this->hp << "\n";
        return;
    }
    // Heal at Pokemon Center
    cout << this->species << "'s HP was restored\n" << this->hp << " -> ";
    this->hp = this->maxHp;
    cout << this->hp << "\n";
}

bool Pokemon::caught(int value){
    if(value <= 30){
        cout << this->species << " was caught!\n";
        this->wild = 0;
        return 1;
    }
    cout << "The pokemon broke free!\n";
    return 0;
}

bool Pokemon::fainted(){
    if(this->hp == 0)
        return 1;
    return 0;
}

void Pokemon::win(int level, bool caught){
    int expGained = level * 30; 
    this->exp -= expGained;
    // Only show message for Pokemon that belong to Playable and Hybrid Trainer
    if(caught){
        cout << this->species << " gained " << expGained << " exp\n";
    }
    // Check if the Pokemon leveled up
    if(this->exp <= 0)
        this->levelUp(caught);
}

int Pokemon::loss(){
    // If the Pokemon fainted return the level so that the opponent can receive exp
    if(this->hp == 0){
        cout << this->species << " fainted\n";
        return this->level;
    }
    return 0;
}

void Pokemon::learnMove(string move, string type, int power){
    // If the pokemon has less than 4 moves, we simply insert the new move
    if(this->moves.size() < 4){
        this->moves[move] = {type, power};
        cout << this->getSpecies() << " learned " << move << "\n";
        return;
    }

    // Else we choose a move to be replaced
    cout << "Which move should be forgotten?\n";
    int k = 0;
    for(auto it = this->moves.begin(); it != this->moves.end(); it++){
        ++k;
        cout << k << ". " << it->first << " " << it->second.first << " " << it->second.second << "\n";
    }

    bool ok = 0;
    while(!ok){
        try{
            int index;
            cin >> index;
            if(cin.fail()){
                ok = 0;
                throw 1;
            }

            if(index > k){
                ok = 0;
                throw 'c';
            }
            
            ok = 1;
            auto it = this->moves.begin();
            for(int i = 1; i < index; i++)
                ++it;

            cout << this->getSpecies() << " forgot " << (*it).first << "\n";
            cout << this->getSpecies() << " learned " << move << "\n";
            
            // Replace with new move
            it = this->moves.erase(it);
            this->moves[move] = {type, power};
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void Pokemon::setMoves(){
    map<string, pair<string, int>> newMoves;

    // The moves are chosen based on the Pokemon's level
    if(1 <= this->level && this->level <= 13){
        for(auto it = this->moves.begin(); it != this->moves.end(); it++)
            if(it->second.second <= 50 && newMoves.size() < 4)
                newMoves.insert(*it);
    }
    else if(14 <= this->level && this->level <= 25){
        for(auto it = this->moves.begin(); it != this->moves.end(); it++)
            if(it->second.second <= 65 && newMoves.size() < 4)
                newMoves.insert(*it);
    }
    else if(26 <= this->level && this->level <= 40){
        for(auto it = this->moves.begin(); it != this->moves.end(); it++)
            if(it->second.second <= 80 && newMoves.size() < 4)
                newMoves.insert(*it);
    }
    else if(41 <= this->level && this->level <= 55){
        for(auto it = this->moves.begin(); it != this->moves.end(); it++)
            if(it->second.second <= 90 && newMoves.size() < 4)
                newMoves.insert(*it);
    }
    else{
        for(auto it = this->moves.begin(); it != this->moves.end(); it++)
            if(newMoves.size() < 4)
                newMoves.insert(*it);
    }
    this->moves.clear();
    this->moves = newMoves;
}

void Pokemon::receiveDamage(int damage){
    cout << this->species << ": " << this->hp << " -> ";
    this->hp = max(0, this->hp - damage);
    cout << this->hp << "\n";
}

map<int, pair<string, int>> Pokemon::calculateDamage(map<string, pair<string, int>> moves, const list<string>& typeWeakness, 
    const list<string>& typeResistance){
    // Calculate the damage done by each move and remember if it's super effective/ not very effective/ regular
    map<int, pair<string, int>> moveMap;
    int damageDone;

    // Iterate through self's moves
    for(auto it = moves.begin(); it != moves.end(); it++){
        // Check if the move is super effective
        bool weak = 0;
        for(auto it2 = typeWeakness.begin(); it2 != typeWeakness.end(); it2++)
            // It's super effective
            if(it->second.first == *it2){
                // Calculate the damage done
                damageDone = (it->second.second * this->attack * 1.5) / 150;

                // Add STAB if the move type is the same as the users type
                if(it->second.first == this->type[0] || it->second.first == this->type[1])
                    damageDone *= 1.5;

                moveMap.insert({damageDone, {it->first, 2}});
                weak = 1;
            }
        
        if(!weak){
            bool strong = 0;
            // Check if the move is not very effective
            for(auto it2 = typeResistance.begin(); it2 != typeResistance.end(); it2++)
                // It's not very effective
                if(it->second.first == *it2){
                    damageDone = (it->second.second * this->attack * 0.5) / 150;
                    if(it->second.first == this->type[0] || it->second.first == this->type[1])
                        damageDone *= 1.5;

                    moveMap.insert({damageDone, {it->first, 0}});
                    strong = 1;
                }

            if(!strong){
                // This move is regular
                damageDone = (it->second.second * this->attack) / 150;
                if(it->second.first == this->type[0] || it->second.first == this->type[1])
                    damageDone *= 1.5;

                moveMap.insert({damageDone, {it->first, 1}});
            }   
        }
    }

    // If two or more moves have the same damage only the first will be available to get chosen
    // Because the map's key is damage
    return moveMap;
}

int Pokemon::useMove(const list<string>& typeWeakness, const list<string>& typeResistance){
    map<int, pair<string, int>> moveMap = this->calculateDamage(this->moves, typeWeakness, typeResistance);
    // Choose a move, randomness is involved
    
    uniform_int_distribution<int> random(1, 10);
    if(random(eng) <= 6){
        // We pick the highest damage dealing move, that's located at the back of the map
        cout << this->species << " used " << moveMap.rbegin()->second.first << "\n";
        if(moveMap.rbegin()->second.second == 2)
            cout << "It's super effective!\n";
        else if(moveMap.rbegin()->second.second == 0)
            cout << "It's not very effective\n";

        return moveMap.rbegin()->first;
    }
    else{
        // Otherwise a random move is picked
        uniform_int_distribution<int> move(0, moveMap.size()-1);
        int index = move(eng);
        auto it = moveMap.begin();
        while(index){
            ++it;
            --index;
        }
        cout << this->species << " used " << it->second.first << "\n";
        if(it->second.second == 2)
            cout << "It's super effective!\n";
        else if(it->second.second == 0)
            cout << "It's not very effective\n";

        return it->first;
    }
}

int Pokemon::pickMove(const list<string>& typeWeakness, const list<string>& typeResistance){
    cout << "Pick a move\n";
    int k = 0;
    for(auto it = this->moves.begin(); it != this->moves.end(); it++){
        ++k;
        cout << k << ". " << it->first << " " << it->second.first << " " << it->second.second << "\n";
    }

    bool ok = 0;
    while(!ok){
        try{
            int move;
            cin >> move;
            if(cin.fail()){
                ok = 0;
                throw 1;
            }

            if(move > k){
                ok = 0;
                throw 'c';
            }
            ok = 1;

            // Find the move
            auto it = this->moves.begin();
            for(int i = 1; i < move; i++)
                ++it;
            
            // Calculate the damage
            map<string, pair<string, int>> movePicked;
            movePicked[it->first] = {it->second.first, it->second.second};

            map<int, pair<string, int>> moveDamage = this->calculateDamage(movePicked, typeWeakness, typeResistance);

            pair<int, string> movePair = {moveDamage.begin()->first, moveDamage.begin()->second.first};
            cout << this->species << " used " << movePair.second << "!\n";
            if(moveDamage.begin()->second.second == 2)
                cout << "It's super effective!\n";
            else if(moveDamage.begin()->second.second == 0)
                cout << "It's not very effective\n";
            
            return movePair.first;
        }
        catch(int x){
            cout << "Invalid input\n";
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

// Trainer class implementations
Trainer::Trainer(string name, const vector<Pokemon>& team){
    this->name = name;
    this->team = team;
    this->teamSize = team.size();
}

Trainer::Trainer(const Trainer& obj){
    this->name = obj.name;
    this->team = obj.team;
    this->teamSize = obj.teamSize;
}

Trainer& Trainer::operator =(const Trainer& obj){
    if(this != &obj){
        this->name = obj.name;
        this->team = obj.team;
        this->teamSize = obj.teamSize;
    }
    return *this;
}

istream& Trainer::read(istream& in){
    cout << "Enter trainer name:\n";
    in >> this->name;

    if(!this->team.empty())
        this->team.clear();
    
    // Maximum 6 Pokemon in the current team
    int count = 6;
    while(count){
        try{
            cout << "1. Enter Pokemon\n";
            cout << "2. Stop\n";
            int k;
            cin >> k;
            if(cin.fail())
                throw 1;

            switch(k){
                case 1: {
                    Pokemon temp;
                    in >> temp;
                    temp.setWild(0);
                    this->team.push_back(temp);
                    --count;
                    break;
                }
                case 2:{    
                    count = 0;
                    break;
                }
                default:{
                    throw 'c';
                }
            }   
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
    }

    // Team size will be the number of non-fainted Pokemon in the current team
    this->teamSize = 0;
    for(int i = 0; i < this->team.size(); i++)
        if(this->team[0].getHP() > 0)
            ++this->teamSize;
    return in;
}

ostream& Trainer::show(ostream& os) const{
    os << "Trainer name: " << this->name << "\n";
    os << "Team:\n";
    for(int i = 0; i < this->team.size(); i++)
        os << this->team[i] << "\n";
    os << "Number of Pokemon ready to fight: " << this->teamSize << "\n";
    return os;
}

void Trainer::writeToFile(fstream& f){
    f << this->name << "\n";
    f << this->team.size() << "\n";
    for(int i = 0; i < this->team.size(); i++)
        this->team[i].writeToFile(f);
}

void Trainer::readFromFile(fstream& f){
    f.get();
    getline(f, this->name);

    // Read team
    int number;
    f >> number;
    if(!this->team.empty())
        this->team.clear();
    for(int i = 0; i < number; i++){
        Pokemon p;
        p.readFromFile(f);
        this->team.push_back(p);
    }

    // teamSize is not read from file in case it's not correct
    this->calculateTeamSize();
}

void Trainer::serialization(fstream& f){
    serializeString(this->name, f);
    
    int size = this->team.size();

    f.write((char*)&size, sizeof(size));
    for(int i = 0; i < size; i++)
        this->team[i].serialization(f);
}

void Trainer::deserialization(fstream& f){
    deserializeString(this->name, f);

    int size;
    f.read((char*)&size, sizeof(size));

    if(!this->team.empty())
        this->team.clear();

    for(int i = 0; i < size; i++){
        Pokemon p;
        p.deserialization(f);
        this->team.push_back(p);
    }

    this->calculateTeamSize();
}

int Trainer::getSpeed() const{
    try{
        if(!this->team.size())
            throw 1;
        return this->team[0].getSpeed();
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return -1;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

int Trainer::getLevel() const{
    try{
        if(!this->team.size())
            throw 1;
        return this->team[0].getLevel();
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return -1;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

string Trainer::getSpecies(int index) const{
    if(index >= 0 && index < this->team.size())
        return this->team[index].getSpecies();
    return "";
}

string Trainer::getFirstType() const{
    if(this->team.size())
        return this->team[0].getFirstType();
    return "";
}

void Trainer::seeTeam(){
    for(int i = 0; i < this->team.size(); i++)
        cout << this->team[i] << "\n";
}

void Trainer::calculateTeamSize(){
    this->teamSize = 0;
    for(int i = 0; i < this->team.size(); i++)
        if(this->team[i].getHP() > 0)
            ++this->teamSize;
}

void Trainer::calculateTeamStats(){
    // Calculate stats of the Trainer's Pokemon team
    for(int i = 0; i < this->team.size(); i++)
        this->team[i].calculateStats(this->team[i].getLevel());
}

void Trainer::resetHP(){
    for(int i = 0; i < this->team.size(); i++)
        this->team[i].resetHP();
    this->teamSize = this->team.size();
}

bool Trainer::fainted(){
    // Checks if the Trainer can still battle
    try{
        if(!this->team.size())
            throw 1;
        if(this->team[0].getHP() <= 0)
            return 1;
        return 0;
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return 0;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void Trainer::sendOutPokemon(){
    // Prints out battle message
    if(this->team.size()){
        cout << this->name << " sent out " << this->team[0].getSpecies() << 
            ". Level: " << this->getLevel() << " " <<
            " Pokemon: " << this->teamSize << "\n"; 
    }
    else
        cout << this->name << " doesn't have any Pokemon\n";
}

void Trainer::setTeam(const vector<Pokemon>& team){
    if(!this->team.empty())
        this->team.clear();
    
    for(int i = 0; i < team.size(); i++)
        this->team.push_back(team[i]);
    this->calculateTeamSize();
}

void Trainer::receiveDamage(int damage){
    try{
        if(!this->team.size())
            throw 1;
        this->team[0].receiveDamage(damage);
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void Trainer::getTypeWeakness(list<string>& obj) const{
    try{
        if(this->team.size())
            this->team[0].getTypeWeakness(obj);
        else
            throw 1;
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void Trainer::getTypeResistance(list<string>& obj) const{
    try{
        if(this->team.size())
           this->team[0].getTypeResistance(obj);
        else
            throw 1;
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

// PlayableTrainer class implementations
PlayableTrainer::PlayableTrainer(string name, const vector<Pokemon>& team, int dollars,
    const multiset<Pokemon>& box, const vector<Item*>& items): Trainer(name, team){
    this->dollars = dollars;
    this->box = box;

    for(int i = 0; i < items.size(); i++){
        Potion* temp = dynamic_cast<Potion*>(items[i]);
        if(temp){
            // It's a potion
            this->items.push_back(new Potion(*temp));
        }
        else{
            Pokeball* temp2 = dynamic_cast<Pokeball*>(items[i]);
            if(temp2){
                // It's a Pokeball
                this->items.push_back(new Pokeball(*temp2));
            }
        }  
    }
}

PlayableTrainer::PlayableTrainer(const PlayableTrainer& obj): Trainer(obj){
    this->dollars = obj.dollars;
    this->box = obj.box;
    
    for(int i = 0; i < obj.items.size(); i++){
        Potion* temp = dynamic_cast<Potion*>(obj.items[i]);
        if(temp){
            // It's a potion
            this->items.push_back(new Potion(*temp));
        }
        else{
            Pokeball* temp2 = dynamic_cast<Pokeball*>(items[i]);
            if(temp2){
                // It's a Pokeball
                this->items.push_back(new Pokeball(*temp2));
            }
        }  
    }
}

PlayableTrainer& PlayableTrainer::operator =(const PlayableTrainer& obj){
    if(this != &obj){
        Trainer::operator =(obj);
        this->dollars = obj.dollars;
        this->box = obj.box;
        
        for(int i = 0; i < obj.items.size(); i++){
            Potion* temp = dynamic_cast<Potion*>(obj.items[i]);
            if(temp){
                // It's a potion
                this->items.push_back(new Potion(*temp));
            }
            else{
                Pokeball* temp2 = dynamic_cast<Pokeball*>(items[i]);
                if(temp2){
                    // It's a Pokeball
                    this->items.push_back(new Pokeball(*temp2));
                }
            }  
        }   
    }
    return *this;
}

istream& PlayableTrainer::read(istream& in){
    Trainer::read(in);

    bool ok = 0;
    cout << "Enter dollars:\n";
    while(!ok){
        try{
            in >> this->dollars;
            if(in.fail()){
                throw 1;
            }

            if(this->dollars < 0){
                throw 'c';
            }

            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            in.clear();
            in.ignore(100, '\n');
        }
        catch(char c){
            cout << "Dollars can't have a negative value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    if(!this->box.empty())
        this->box.clear();

    cout << "Enter the Pokemon in the boxes\n";
    ok = 0;
    while(!ok){
        try{
            cout << "1. Enter Pokemon\n";
            cout << "2. Stop\n";
            int k;
            cin >> k;

            if(cin.fail())
                throw 1;
            
            if(k == 2){
                ok = 1;
                break;
            }

            if(k != 1)
                throw 'c';
            
            Pokemon temp;
            in >> temp;
            // If the wrong value for wild attribute is entered, then it will be corrected
            temp.setWild(0);
            this->box.insert(*(new Pokemon(temp)));
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    if(!this->items.empty()){
        for(auto it = this->items.begin(); it != this->items.end(); it++){
            delete *it;
        }
        this->items.clear();
    }

    cout << "Enter the items\n";
    ok = 0;
    while(!ok){
        try{
            cout << "1. Potion\n";
            cout << "2. Pokeball\n";
            cout << "3. Stop\n";

            int k;
            cin >> k;

            if(cin.fail())
                throw 1;
            
            if(k == 3){
                ok = 1;
                break;
            }

            if(k == 1){
                this->items.push_back(new Potion());
                this->items.back()->setAttributes();
            }
            else if(k == 2){
                this->items.push_back(new Pokeball());
                this->items.back()->setAttributes();
            }
            else
                throw 'c';
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
    return in;
}

ostream& PlayableTrainer::show(ostream& os) const{
    Trainer::show(os);
    os << "Dollars: " << this->dollars << "\n";
    os << "Items:\n";
    for(auto it = this->items.begin(); it != this->items.end(); it++)
        (*it)->show();
    os << "Pokemon in boxes:\n";
    for(auto it = this->box.begin(); it != this->box.end(); it++)
        os << *it << "\n";
    return os;
}

void PlayableTrainer::writeToFile(fstream& f){
    Trainer::writeToFile(f);
    f << this->dollars << "\n";

    f << this->box.size() << "\n";
    for(auto it = this->box.begin(); it != this->box.end(); it++)
        (*it).writeToFile(f);

    f << this->items.size() << "\n";
    for(int i = 0; i < this->items.size(); i++){
        if(this->items[i]->getName() == "Potion" || this->items[i]->getName() == "Super Potion" ||
            this->items[i]->getName() == "Hyper Potion")
            f << "potion\n";
        else
            f << "pokeball\n";
        this->items[i]->writeToFile(f);
    }
}

void PlayableTrainer::readFromFile(fstream& f){
    Trainer::readFromFile(f);
    f >> this->dollars;

    // Read box Pokemon
    int number;
    f >> number;
    if(!this->box.empty())
        this->box.clear();
    for(int i = 0; i < number; i++){
        Pokemon p;
        p.readFromFile(f);
        this->box.insert(p);
    }

    // Read items
    f >> number;
    string word;
    if(!this->items.empty())
        this->items.clear();
    for(int i = 0; i < number; i++){
        f >> word;
        if(word == "potion")
            this->items.push_back(new Potion());
        else
            this->items.push_back(new Pokeball());
        this->items.back()->readFromFile(f);
    }
}

void PlayableTrainer::serialization(fstream& f){
    Trainer::serialization(f);
    f.write((char*)&(this->dollars), sizeof(this->dollars));

    int size = this->box.size();
    f.write((char*)&size, sizeof(size));
    
    for(auto it = this->box.begin(); it != this->box.end(); it++)
        it->serialization(f);
    
    size = this->items.size();
    f.write((char*)&size, sizeof(size));
    
    for(auto it = this->items.begin(); it != this->items.end(); it++){
        Potion* temp = dynamic_cast<Potion*>(*it);

        // Write a value to know if it's a Potion or a Pokeball
        int x;
        if(temp)
            x = 1;
        else
            x = 2;
        
        f.write((char*)&x, sizeof(x));
        (*it)->serialization(f);
    }
}

void PlayableTrainer::deserialization(fstream& f){
    Trainer::deserialization(f);
    f.read((char*)&(this->dollars), sizeof(this->dollars));

    int size;
    f.read((char*)&size, sizeof(size));

    if(!this->box.empty())
        this->box.clear();

    for(int i = 0; i < size; i++){
        Pokemon p;
        p.deserialization(f);
        this->box.insert(p);
    }

    if(!this->items.empty()){
        for(auto it = this->items.begin(); it != this->items.end(); it++)
            if(*it != nullptr)
                delete *it;
        this->items.clear();
    }

    f.read((char*)&size, sizeof(size));
    for(int i = 0; i < size; i++){
        int x;
        f.read((char*)&x, sizeof(x));

        if(x == 1)
            this->items.push_back(new Potion());
        else
            this->items.push_back(new Pokeball());
        this->items.back()->deserialization(f);
    }
}

void PlayableTrainer::evolve(const vector<Pokemon>& inGamePokemon, int index){
    if(index >= 0 && index < this->team.size())
        this->team[index].evolve(inGamePokemon);
}

void PlayableTrainer::learnMove(const Pokemon& pokemon, int index){
    if(index >= 0 && index < this->team.size())
        this->team[index].checkNewMoves(pokemon);
}

void PlayableTrainer::giveDollars(){
    // Gives dollars to the winner
    try{
        if(!this->team.size())
            throw 1;
        this->dollars = max(0, this->dollars - (this->team[0].getLevel() * 200));
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

bool PlayableTrainer::checkPotions(){
    // Check if this has any potions
    for(int i = 0; i < this->items.size(); i++){
        Potion* temp = dynamic_cast<Potion*>(this->items[i]);
        if(temp)
            return 1;
    }
    return 0;
}

int PlayableTrainer::pickPokemon() const{
    if(!this->team.size()){
        cout << this->name << " doesn't have any Pokemon\n";
        return -1;
    }

    cout << "Select a Pokemon\n";
    for(int i = 0; i < this->team.size(); i++)
        cout << i+1 << ". " << this->team[i].getSpecies() << "\n";

    while(true){
        try{
            int k;
            cin >> k;
            if(cin.fail())
                throw 1;
            
            if(k-1 >= 0 && k-1 < this->team.size())
                return k-1;
            
            throw 'c';
            
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void PlayableTrainer::switchPokemon(){
    if(this->team.size() < 2){
        cout << "You don't have enough Pokemon\n";
        return;
    }

    cout << "Pick which Pokemon will be placed on the first position\n";
    int index = this->pickPokemon();
    swap(this->team[0], this->team[index]);
}

void PlayableTrainer::seePokemon(){
    int index = this->pickPokemon();
    if(index != -1){
        cout << this->team[index] << '\n';
        string species = this->team[index].getSpecies();

        try{
            // Files will be the sprites
            vector<string> files;
            // The name of the folder will be the species of the Pokemon
            string folder = species;

            // All the folders are located in the Sprites folder
            string path = "Sprites\\" + folder + "/*";
            WIN32_FIND_DATAA data;          // Information about the file/ directory found
            HANDLE handle = FindFirstFileA(path.c_str(), &data); 
            
            do{
                // If it finds a directory it ignores it
                if(data.dwFileAttributes ^ FILE_ATTRIBUTE_DIRECTORY){
                    string fileName = data.cFileName;
                    string fullPath = "Sprites\\" + folder + "/" + fileName;
                    
                    files.push_back(fullPath);
                }
            } while(FindNextFileA(handle, &data));

            if(files.size() == 1)
                throw 1;

            FindClose(handle);
            // The frames won't be in order so they are sorted
            sort(files.begin(), files.end(), CompareFilePaths);

            SDL_Window* window = SDL_CreateWindow(species.c_str(), SDL_WINDOWPOS_CENTERED, 
                        SDL_WINDOWPOS_CENTERED, 450, 400, 0);
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

            vector<SDL_Texture*> textures;
            for(int i = 0; i < files.size(); i++){
                SDL_Surface* surface = IMG_Load(files[i].c_str());
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
                SDL_FreeSurface(surface);
                textures.push_back(texture);
            }

            bool ok = 1;
            SDL_Event event;

            while(ok){
                for(int i = 0; i < textures.size() && ok; i++){

                    while(SDL_PollEvent(&event))
                        if(event.type == SDL_QUIT)
                            ok = 0;

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, textures[i], nullptr, nullptr);
                    SDL_Delay(120);
                    SDL_RenderPresent(renderer);
                }
            }
            SDL_DestroyWindow(window);
            SDL_DestroyRenderer(renderer);
        }
        catch(int x){
            cout << "Invalid species\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void PlayableTrainer::seeBox(){
    if(!this->box.size()){
        cout << "The box is empty\n";
        return;
    }
    for(auto it = this->box.begin(); it != this->box.end(); it++)
        cout << *it << "\n";
}

void PlayableTrainer::seeItems(){
    if(!this->items.size()){
        cout << "No items\n";
        return;
    }
    for(int i = 0; i < this->items.size(); i++)
        this->items[i]->show();
}

void PlayableTrainer::winMessage(Trainer* opponent){
    try{
        if(opponent == nullptr)
            throw 1;
        
        cout << this->name << " won\n";
        int level = opponent->getLevel();

        if(level == -1)
            throw 'c';

        cout << this->name << " received " << level * 160 <<  " dollars\n";
        this->dollars += level * 160;

        // If opponent is a PlayableTrainer it will give dollars to this
        PlayableTrainer* temp = dynamic_cast<PlayableTrainer*>(opponent);
        if(temp)
            temp->giveDollars();
        else{
            HybridTrainer* temp2 = dynamic_cast<HybridTrainer*>(opponent);
            if(temp2)
                temp->giveDollars();
        }
    }
    catch(int x){
        cout << "Opponent is a nullptr\n";
    }
    catch(char c){
        cout << "Opponent doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void PlayableTrainer::winMessage(Pokemon* pokemon){
    // If the Pokemon is wild it can be caught
    if(pokemon != nullptr){
        cout << "1. Catch Pokemon\n";
        cout << "2. Exit\n";
        int k;
        bool ok = 0;

        while(!ok){
            try{
                cin >> k;
                if(cin.fail())
                    throw 1;
                
                if(k == 2)
                    ok = 1;

                else if(k == 1){
                    bool check = 0;
                    auto it = this->items.begin();

                    for(auto it = this->items.begin(); it != this->items.end(); it++){
                        Pokeball* temp = dynamic_cast<Pokeball*>(*it);

                        if(temp){
                            cout << "Threw a pokeball\n";
                            int value = temp->use();
                            bool caught = pokemon->caught(value);
                            if(caught){
                                ok = 1;
                                // Create copy and add to team
                                this->team.push_back(*pokemon);
                                // If team size is greater than 6 one Pokemon will be sent to the box
                                while(this->team.size() > 6)
                                    this->putInBox();

                                check = 1;
                            }
                            else
                                ok = 0;

                            delete *it;
                            it = this->items.erase(it);
                            break;
                        }
                    }
                    if(it == this->items.end() && !check){
                        cout << "You don't have any Pokeballs\n";
                        ok = 1;
                    }
                }
                else 
                    throw 'c';
            }
            catch(int x){
                cout << "Invalid input\n";
                cin.clear();
                cin.ignore(100, '\n');
            }
            catch(char c){
                cout << "Invalid value\n";
            }
            catch(...){
                cout << "Error\n";
                exit(1);
            }
        }
    }
}

void PlayableTrainer::buyItems(){
    cout << "You have " << this->dollars << " dollars\n";
    int k;

    while(true){
        cout << "1. Potion\n";
        cout << "2. Pokeball\n";
        cout << "4. Exit\n";
        try{
            cin >> k;
            if(cin.fail())
                throw 1;
            
            if(k == 1){
                bool ok2 = 0;

                while(!ok2){
                    cout << "1. Potion 20$\n";
                    cout << "2. Super Potion 50$\n";
                    cout << "3. Hyper Potion 70$\n";
                    cout << "4. Max Potion 100$\n";
                    cout << "5. Exit\n";
                    int k ;
                    cin >> k;

                    if(cin.fail())
                        throw 1;
                    
                    if(k == 5)
                        break;

                    if(k < 1 || k > 4)
                        throw 'c';
                    
                    cout << "Enter how many:\n";
                    int count;
                    cin >> count;

                    if(cin.fail())
                        throw 1;
                    
                    if(count < 0)
                        throw 'c';
                    
                    switch(k){
                        case 1:{
                            if(this->dollars < 20 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 20 * count;
                                while(count){
                                    this->items.push_back(new Potion("Potion", 20, 20));
                                    --count;
                                }
                            }
                            break;
                        }

                        case 2:{
                            if(this->dollars < 50 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 50 * count;
                                while(count){
                                    this->items.push_back(new Potion("Super Potion", 50, 60));
                                    --count;
                                }
                            }
                            break;
                        }

                        case 3:{
                            if(this->dollars < 70 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 70 * count;
                                while(count){
                                    this->items.push_back(new Potion("Hyper Potion", 70, 120));
                                    --count;
                                }
                            }
                            break;
                        }
                        case 4:{
                            if(this->dollars < 100 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 100 * count;
                                while(count){
                                    this->items.push_back(new Potion("Max Potion", 100, 1000));
                                    --count;
                                }
                            }
                            break;
                        }
                    }

                }
            }
            else if(k == 2){
                bool ok2 = 0;

                while(!ok2){
                    cout << "1. Pokeball 20$\n";
                    cout << "2. Great Ball 40$\n";
                    cout << "3. Ultra Ball 60$\n";
                    cout << "4. Exit\n";
                    int k ;
                    cin >> k;

                    if(cin.fail())
                        throw 1;
                    
                    if(k == 4)
                        break;

                    if(k < 1 || k > 3)
                        throw 'c';
                    
                    cout << "Enter how many:\n";
                    int count;
                    cin >> count;

                    if(cin.fail())
                        throw 1;
                    
                    switch(k){
                        case 1:{
                            if(this->dollars < 20 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 20 * count;
                                while(count){
                                    this->items.push_back(new Pokeball("Pokeball", 20));
                                    --count;
                                }
                            }
                            break;
                        }

                        case 2:{
                            if(this->dollars < 40 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 40 * count;
                                while(count){
                                    this->items.push_back(new Pokeball("Great Ball", 40));
                                    --count;
                                }
                            }
                            break;
                        }

                        case 3:{
                            if(this->dollars < 60 * count){
                                cout << "You don't have enough dollars\n";
                            }
                            else{
                                this->dollars -= 60 * count;
                                while(count){
                                    this->items.push_back(new Pokeball("Ultra Ball", 60));
                                    --count;
                                }
                            }
                            break;
                        }
                    }
                }
            }
            else if(k == 4)
                return;
            else
                throw 'c';
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void PlayableTrainer::putInBox(){
    if(this->team.size() == 1){
        cout << "Must have at least one Pokemon in the team\n";
        return;
    }
    cout << "Select which Pokemon will be sent to the box\n";

    for(int i = 0; i < this->team.size(); i++)
        cout << i + 1 << ". " << this->team[i].getSpecies() << " Level: " << this->team[i].getLevel() << "\n";
    cout << this->team.size() + 1 << ". Exit\n";

    bool ok = 0;
    while(!ok){
        try{
            int index;
            cin >> index;
            if(cin.fail())
                throw 1;
            
            if(index == this->team.size() + 1)
                return;

            if(index <= 0 || index > this->team.size())
                throw 'c';
            
            cout << this->team[index-1].getSpecies() << " will be sent to the box\n";
            // Insert in box and remove from team
            this->box.insert(this->team[index-1]);

            swap(this->team.back(), this->team[index-1]);
            this->team.pop_back();
            // Recalculate teamSize
            this->calculateTeamSize();
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void PlayableTrainer::releasePokemon(){
    if(!this->box.size()){
        cout << "The box is empty\n";
        return;
    }
    cout << "Select which Pokemon will be released\n";

    int k = 1;
    for(auto it = this->box.begin(); it != this->box.end(); it++){
        cout << k << ". " << (*it).getSpecies() << " Level: " << (*it).getLevel() << "\n";
        ++k;
    }
    cout << k << ". Exit\n";

    bool ok = 0;
    while(!ok){
        try{
            int index;
            cin >> index;
            if(cin.fail())
                throw 1;
            
            if(index == k)
                return;
            
            if(index <= 0 || index > this->box.size())
                throw 'c';
            
            auto it = this->box.begin();
            for(int i = 1; i < index; i++)
                ++it;
            
            cout << "Are you sure you want to release " << it->getSpecies() << "? (yes/ no)\n";
            string answer;
            bool ok2 = 0;

            while(!ok2){
                cin >> answer;
                if(answer == "yes"){
                    cout << it->getSpecies() << " was released\n";
                    it = this->box.erase(it);
                    ok2 = 1;
                }
                else if(answer == "no")
                    ok2 = 1;
                else
                    cout << "Input invalid\n";
            }
           
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void PlayableTrainer::takeFromBox(){
    if(!this->box.size()){
        cout << "The box is empty\n";
        return;
    }
    cout << "Select which Pokemon will be taken from the box\n";

    int k = 1;
    for(auto it = this->box.begin(); it != this->box.end(); it++){
        cout << k << ". " << (*it).getSpecies() << " Level: " << (*it).getLevel() << "\n";
        ++k;
    }
    cout << k << ". Exit\n";
    
    bool ok = 0;
    while(!ok){
        try{
            int index;
            cin >> index;
            if(cin.fail())
                throw 1;
            
            if(index == k)
                return;
            
            if(index <= 0 || index > this->box.size())
                throw 'c';
            
            auto it = this->box.begin();
            for(int i = 1; i < index; i++)
                ++it;
            
            // Add to team and remove from box
            this->team.push_back(*it);
            it = this->box.erase(it);

            ok = 1;
            // Check if there are more than 6 Pokemon in the team
            while(this->team.size() > 6)
                // Send a Pokemon to the box
                this->putInBox();

            // Recalculate teamSize
            this->calculateTeamSize();
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

void PlayableTrainer::trainPokemon(int i){
    if(i < 0 || i > this->team.size())
        return;
    
    cout << "Cost: " << 700 << " dollars\n";
    if(this->dollars < 700){
        cout << "You don't have enough money\n";
        return;
    }

    cout << "Select stat\n";
    cout << "1. HP\n2. Attack\n3. Speed\n4. Exit\n";

    try{
        int k;
        cin >> k;
        if(cin.fail())
            throw 1;

        switch(k){
            case 1:{
                this->team[i].increaseMaxHP(5);
                this->dollars -= 700;
                break;
            }

            case 2:{
                this->team[i].increaseAttack(5);
                this->dollars -= 700;
                break;
            }

            case 3:{
                this->team[i].increaseSpeed(5);
                this->dollars -= 700;
                break;
            }

            case 4:{
                break;
            }

            default:{
                cout << "Invalid value\n";
                break;
            }
        }
    }
    catch(int x){
        cout << "Invalid input\n";
        cin.clear();
        cin.ignore(100, '\n');
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

bool PlayableTrainer::restoreHP(int i, string potionName){
    if(i >= 0 && i < this->team.size()){
        if(this->team[i].getHP() == this->team[i].getMaxHP()){
            cout << this->team[i].getSpecies() << " doesn't need healing\n";
            return false;
        }

        if(potionName == ""){
            // Heal at Pokemon center
            this->team[i].restoreHP();
            return true;
        }
        else{
            // Heal with potion
            bool found = 0; 
            for(auto it = this->items.begin(); it != this->items.end(); it++){
                if((*it)->getName() == potionName){
                    this->team[i].restoreHP((*it)->use());
                    found = 1;
                    Item* aux = *it;
                    it = this->items.erase(it);
                    delete aux;
                    return true;
                }
            }
            if(!found){
                cout << this->name << " doesn't have any " << potionName << "s\n";
                return false;
            }
        }
    }
    return false;
}

int PlayableTrainer::useMove(const list<string>& typeWeakness, const list<string>& typeResistance){
    try{
        if(!this->team.size())
            throw 1;

        cout << this->team[0].getSpecies() << ": " << this->team[0].getHP() << "\n";
    
        while(true){
            cout << "1. Attack\n2. Restore HP\n3. Switch\n4. Flee\n";
            try{
                int k;
                cin >> k;
                if(cin.fail())
                    throw 1;

                switch(k){
                    case 1:{
                        return this->team[0].pickMove(typeWeakness, typeResistance);
                    }

                    case 2:{
                        if(!this->checkPotions()){
                            cout << "You don't have any potions\n";
                            // Go to attack
                            return this->team[0].pickMove(typeWeakness, typeResistance);
                        }

                        cout << "Select potion\n";
                        cout << "1. Potion\n2. Super Potion\n3. Hyper Potion\n4. Max Potion\n";

                        int potion;
                        cin >> potion;
                        if(cin.fail())
                            throw 1;
                        
                        if(potion == 1){
                            if(this->restoreHP(0, "Potion"))
                                return 0;
                        }
                        else if(potion == 2){
                            if(this->restoreHP(0, "Super Potion"))
                                return 0;
                        }
                        else if(potion == 3){
                            if(this->restoreHP(0, "Hyper Potion"))
                                return 0;
                        }
                        else if(potion == 4){
                            if(this->restoreHP(0, "Max Potion"))
                                return 0;
                        }
                        else
                            throw 'c';
                        break;
                    }

                    case 3:{
                        while(true){
                            int index = this->pickPokemon();
                            if(index == -1)
                                break;

                            if(this->team[index].battleReady()){
                                swap(this->team[0], this->team[index]);
                                break;
                            }
                            else
                                cout << "This Pokemon can't battle right now\n";
                        }
                        return 0;
                    }

                    case 4:{
                        return -1;
                    }

                    default:{
                        throw 'c';
                    }
                }
            }
            catch(int x){
                cout << "Invalid input\n";
                cin.clear();
                cin.ignore(100, '\n');
            }
            catch(char c){
                cout << "Invalid value\n";
            }
            catch(...){
                cout << "Error\n";
                exit(1);
            }
        }
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return 0;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void PlayableTrainer::win(int exp){
    try{
        if(!this->team.size())
            throw 1;
        // Pokemon who won the battle receives exp
        this->team[0].win(exp, 1);
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

int PlayableTrainer::loss(){
    try{
        if(!this->team.size())
            throw 1;
        
        // The Pokemon hasn't fainted
        if(this->team[0].getHP() > 0)
            return 0;

    	// Reduce team size and show message
        --this->teamSize;

        // Save fainted Pokemon's level
        int faintedLevel = this->team[0].loss();

        // Pick a new Pokemon and place it on the first position
        if(this->teamSize > 0){
            int k = 1;
            bool found = 0;
            for(int i = 1; i < this->team.size(); i++)
                if(this->team[i].getHP() > 0){
                    cout << i << ": " << this->team[i].getSpecies() << "\n";
                    found = 1;
                }

            // If there is a Pokemon that can battle we swap 
            if(found){
                bool ok = 0;
                while(!ok){
                    try{
                        cin >> k;
                        if(cin.fail())
                            throw 'c';

                        else if(k >= this->team.size() || k < 0) 
                            throw 'c';

                        else if(this->team[k].getHP() <= 0)
                            throw k;
                        
                        else{
                            swap(this->team[0], this->team[k]);
                            // Show message
                            this->sendOutPokemon();
                            ok = 1;
                        }
                        
                    }
                    catch(char c){
                        cout << "Invalid input\n";
                        cin.clear();
                        cin.ignore(100, '\n');
                    }
                    catch(int x){
                        cout << this->team[k].getSpecies() << " can't battle anymore\n";
                    }
                }
            }
        }
        // Return fainted Pokemon's level so the opponent can receive exp
        return faintedLevel;
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return 0;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

PlayableTrainer::~PlayableTrainer(){
    for(auto it = this->items.begin(); it != this->items.end(); it++)
        if(*it != nullptr){
            delete *it;
            *it = nullptr;
        }
}

// UnplayableTrainer class implementations
UnplayableTrainer& UnplayableTrainer::operator =(const UnplayableTrainer& obj){
    if(this != &obj)
        Trainer::operator =(obj);
    return *this;
}

bool UnplayableTrainer::restoreHP(int i, string potionName){
    // UnplayableTrainers can also use potions during a battle
    if(potionName == "Potion"){
        cout << this->name << " used a Potion\n";
        this->team[0].restoreHP(20);
        return true;
    }
    else if(potionName == "Super Potion"){
        cout << this->name << " used a Super Potion\n";
        this->team[0].restoreHP(60);
        return true;
    }
    else if(potionName == "Hyper Potion"){
        cout << this->name << " used a Hyper Potion\n";
        this->team[0].restoreHP(120);
        return true;
    }
    else if(potionName == "Max Potion"){
        cout << this->name << " used a Max Potion\n";
        this->team[0].restoreHP(1000);
        return true;
    }
    return false;
}

int UnplayableTrainer::useMove(const list<string>& typeWeakness, const list<string>& typeResistance){
    try{
        if(!this->team.size())
            throw 1;

        if(this->team[0].getMaxHP() / this->team[0].getHP() >= 4){
            // If hp is <= 1/4 of maxHp then the trainer may use a potion
            uniform_int_distribution<int> number(1, 20);
            if(number(eng) <= 3){
                if(this->team[0].getMaxHP() <= 40){
                    this->restoreHP(0, "Potion");
                    return 0;
                }
                else if(this->team[0].getMaxHP() <= 120){
                    this->restoreHP(0, "Super Potion");
                    return 0;
                }
                else if(this->team[0].getMaxHP() <= 200){
                    this->restoreHP(0, "Hyper Potion");
                    return 0;
                }
                else{
                    this->restoreHP(0, "Max Potion");
                    return 0;
                }
            }
        }
        return this->team[0].useMove(typeWeakness, typeResistance);
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return 0;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
    
}

void UnplayableTrainer::win(int exp){
    // UnplayableTrainer's Pokemon receive exp but no message is shown
    try{
        if(!this->team.size())
            throw 1;
        this->team[0].win(exp);
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

int UnplayableTrainer::loss(){
    try{
        if(!this->team.size())
            throw 1;

        if(this->team[0].getHP() > 0)
            return 0;

        --this->teamSize;
        int faintedLevel = this->team[0].loss();

        // Pick the next Pokemon that can battle and place it on the first position
        if(this->teamSize > 0){
            for(int i = 1; i < this->team.size(); i++)
                if(this->team[i].getHP() > 0){
                    swap(this->team[0], this->team[i]);
                    this->sendOutPokemon();
                    break;
                }    
        }

        // Return fainted Pokemon's level so the opponent can receive exp
        return faintedLevel;
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return 0;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

// HybridTrainer class implementations
HybridTrainer::HybridTrainer(string name, const vector<Pokemon>& team, int dollars,
    const multiset<Pokemon>& box, const vector<Item*>& items, bool autoPlay, int gymBadges): Trainer(name, team),
    PlayableTrainer(name, team, dollars, box, items), UnplayableTrainer(){
    this->autoPlay = autoPlay;
    this->gymBadges = gymBadges;
}

HybridTrainer::HybridTrainer(const HybridTrainer& obj): Trainer(obj), PlayableTrainer(obj), UnplayableTrainer(obj){
    this->autoPlay = obj.autoPlay;
    this->gymBadges = obj.gymBadges;
}

HybridTrainer& HybridTrainer::operator =(const HybridTrainer& obj){
    if(this != &obj){
        PlayableTrainer::operator =(obj);
        UnplayableTrainer::operator =(obj);
        this->autoPlay = obj.autoPlay;
        this->gymBadges = obj.gymBadges;
    }
    return *this;
}

istream& HybridTrainer::read(istream& in){
    PlayableTrainer::read(in);
    cout << "Enter autoplay:\n";
    bool ok = 0;
    while(!ok){
        try{
            in >> this->autoPlay;
            if(in.fail())
                throw 1;
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
    cout << "Enter the number of gym badges:\n";
    ok = 0;
    while(!ok){
        try{
            in >> this->gymBadges;
            if(in.fail())
                throw 1;
            if(this->gymBadges < 0 || this->gymBadges > 8)
                throw 'c';
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            in.clear();
            in.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
    return in;
}

ostream& HybridTrainer::show(ostream& os) const{
    PlayableTrainer::show(os);
    os << "Autoplay: " << this->autoPlay << "\n";
    os << "Gym badges: " << this->gymBadges << "\n";
    return os;
}

void HybridTrainer::writeToFile(fstream& f){
    PlayableTrainer::writeToFile(f);
    f << this->autoPlay << "\n";
    f << this->gymBadges << "\n";
}

void HybridTrainer::readFromFile(fstream& f){
    PlayableTrainer::readFromFile(f);
    f >> this->autoPlay;
    f >> this->gymBadges;
}

void HybridTrainer::serialization(fstream& f){
    PlayableTrainer::serialization(f);

    f.write((char*)&(this->autoPlay), sizeof(this->autoPlay));
    f.write((char*)&(this->gymBadges), sizeof(this->gymBadges));
}

void HybridTrainer::deserialization(fstream& f){
    PlayableTrainer::deserialization(f);

    f.read((char*)&(this->autoPlay), sizeof(this->autoPlay));
    f.read((char*)&(this->gymBadges), sizeof(this->gymBadges));
}

void HybridTrainer::toggleAutoPlay(){
    if(this->autoPlay){
        this->autoPlay = 0;
        cout << "Turned off auto play\n";
    }
    else{
        this->autoPlay = 1;
        cout << "Turned on auto play\n";
    }
}

bool HybridTrainer::restoreHP(int i, string potionName){
    return PlayableTrainer::restoreHP(i, potionName);
}

int HybridTrainer::useMove(const list<string>& typeWeakness, const list<string>& typeResistance){
    try{
        if(!this->team.size())
            throw 1;
        if(!this->autoPlay)
            return PlayableTrainer::useMove(typeWeakness, typeResistance);
        // If autoPlay is turned on this won't use potions during battles
        return this->team[0].useMove(typeWeakness, typeResistance);
    }
    catch(int x){
        cout << this->name << " doesn't have any Pokemon\n";
        return 0;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

int HybridTrainer::loss(){
    if(this->autoPlay)
        return UnplayableTrainer::loss();
    return PlayableTrainer::loss();
}

void HybridTrainer::win(int exp){
    PlayableTrainer::win(exp);
}

void HybridTrainer::winMessage(Pokemon* opponent){
    // If autoPlay is turned on this won't catch wild Pokemon
    if(this->autoPlay)
        UnplayableTrainer::winMessage(opponent);
    else
        PlayableTrainer::winMessage(opponent);
    
}

// Battle class implementations
template <class T, class U>
Battle<T, U>& Battle<T, U>::operator =(const Battle<T, U>& obj){
    if(this != &obj){
        this->first = obj->first;
        this->second = obj->second;
    }
    return *this;
}

template <class T, class U>
ostream& operator <<(ostream& os, const Battle<T, U>& obj){
    os << *(obj.first) << '\n' << *(obj.second) << '\n';
    return os;
}

// Checks the parameters' types
template <class T, class U>
void Battle<T, U>::showBattleInformation(){
    cout << "Opponent 1: ";
    if(typeid(*first) == typeid(HybridTrainer))
        cout << "Hybrid Trainer\n";
    else if(typeid(*first) == typeid(PlayableTrainer))
        cout << "Playable Trainer\n";
    else if(typeid(*first) == typeid(UnplayableTrainer))
        cout << "Unplayable Trainer\n";
    else if(typeid(*first) == typeid(Pokemon))
        cout << "Pokemon\n";
    
    cout << "Opponent 2: ";
    if(typeid(*second) == typeid(HybridTrainer))
        cout << "Hybrid Trainer\n";
    else if(typeid(*second) == typeid(PlayableTrainer))
        cout << "Playable Trainer\n";
    else if(typeid(*second) == typeid(UnplayableTrainer))
        cout << "Unplayable Trainer\n";
    else if(typeid(*second) == typeid(Pokemon))
        cout << "Pokemon\n";
}

template <class T, class U>
void Battle<T, U>::doBattle(){
    if(!first->battleReady() || !second->battleReady()){
        cout << "Now is not the time for a battle\n";
        return;
    }

    // Show battle messages
    second->sendOutPokemon();
    first->sendOutPokemon();

    while(!first->fainted() && !second->fainted()){
        // The one with the higher speed stat begins
        if(first->getSpeed() > second->getSpeed()){
            first->turn();

            list<string> typeWeakness = {};
            list<string> typeResistance = {};
            second->getTypeWeakness(typeWeakness);
            second->getTypeResistance(typeResistance);

            int moveFirst = first->useMove(typeWeakness, typeResistance);
            if(moveFirst > 0)
                second->receiveDamage(moveFirst);
            else if(moveFirst == -1){
                cout << "Gave up the battle\n";
                return;
            }

            int value = second->loss();
            // Second hasn't lost
            if(value == 0){
                second->turn();

                list<string> typeWeakness = {};
                list<string> typeResistance = {};
                first->getTypeWeakness(typeWeakness);
                first->getTypeResistance(typeResistance);

                int moveSecond = second->useMove(typeWeakness, typeResistance);
                if(moveSecond > 0)
                    first->receiveDamage(moveSecond);
                else if(moveSecond == -1){
                    cout << "Gave up the battle\n";
                    return;
                }

                int value2 = first->loss();
                // First lost
                if(value2 != 0){
                    second->win(value2);
                }
            }
            // Second lost
            else{
                first->win(value);
            }
        }
        else{
            second->turn();

            list<string> typeWeakness = {};
            list<string> typeResistance = {};
            first->getTypeWeakness(typeWeakness);
            first->getTypeResistance(typeResistance);


            int moveFirst = second->useMove(typeWeakness, typeResistance);
            if(moveFirst > 0)
                first->receiveDamage(moveFirst);
            else if(moveFirst == -1){
                cout << "Gave up the battle\n";
                return;
            }

            int value = first->loss();
            if(value == 0){
                first->turn();

                list<string> typeWeakness = {};
                list<string> typeResistance = {};
                second->getTypeWeakness(typeWeakness);
                second->getTypeResistance(typeResistance);

                int moveSecond = first->useMove(typeWeakness, typeResistance);
                if(moveSecond > 0)
                    second->receiveDamage(moveSecond);
                else if(moveSecond == -1){
                    cout << "Gave up the battle\n";
                    return;
                }

                int value2 = second->loss();
                if(value2 != 0){
                    first->win(value2);
                }
            }
            else{
                second->win(value);
            }
        }
    }
    if(first->fainted())
        second->winMessage(first);
    else
        first->winMessage(second);
    
    cout << "See battle information? (y/n)\n";
    string answer;
    while(true){
        try{
            cin >> answer;
        
            if(answer == "y"){
                showBattleInformation();
                cout << '\n' << *this;
                return;
            }
            if(answer == "n")
                return;
            
            throw 'c';
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

// Game class implementations
Game& Game::getInstance(){
    if(playerFile == ""){
        cout << "Enter the name of the file you want to use\n";
        cin >> playerFile;
    }

    static Game game(playerFile);
    return game;
}

void Game::startGame(){
    bool ok = 1;
    while(ok){
        try{
            cout << "1. Play\n";
            cout << "2. Go to Database\n";
            cout << "3. Exit\n";
            int k;
            cin >> k;

            if(cin.fail())
                throw 1;
            
            switch(k){
                case 1:{
                    bool playing = 1;
                    while(playing){
                        cout << "1. Team\n";
                        cout << "2. Battle\n";
                        cout << "3. Items\n";
                        cout << "4. Locations\n";
                        cout << "5. Trainer\n";
                        cout << "6. Box\n";
                        cout << "7. Stop playing\n";

                        try{
                            int k2;
                            cin >> k2;
                            if(cin.fail())
                                throw 1;
                            
                            switch(k2){
                                case 1:{
                                    cout << "1. See team\n";
                                    cout << "2. See Pokemon\n";
                                    cout << "3. Switch order\n";
                                    cout << "4. Heal\n";
                                    cout << "5. Learn a new move\n";
                                    cout << "6. Evolve\n";
                                    cout << "7. Train\n";
                                    cout << "8. Exit\n";

                                    try{
                                        int k3;
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        switch(k3){
                                            case 1:{
                                                this->seeTeam();
                                                break;
                                            }

                                            case 2:{
                                                this->seePokemon();
                                                break;
                                            }

                                            case 3:{
                                                this->switchPokemon();
                                                break;
                                            }

                                            case 4:{
                                                this->heal();
                                                break;
                                            }

                                            case 5:{
                                                this->learnMove();
                                                break;
                                            }

                                            case 6:{
                                                this->evolve();
                                                break;
                                            }

                                            case 7:{
                                                this->trainPokemon();
                                                break;
                                            }

                                            case 8:{
                                                break;
                                            }

                                            default:{
                                                throw 'c';
                                            }
                                        }
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                        cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                    break;
                                }
                                
                                case 2:{
                                    cout << "1. Battle a wild Pokemon\n";
                                    cout << "2. Battle a trainer\n";
                                    cout << "3. Challenge gym leader\n";
                                    cout << "4. Challenge elite four\n";
                                    cout << "5. Challenge the champion\n";
                                    cout << "6. Do a link battle\n";
                                    cout << "7. See trainers battled in link battles\n";
                                    cout << "8. Exit\n";

                                    try{
                                        int k3;
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        switch(k3){
                                            case 1:{
                                                this->battleWildPokemon();
                                                break;
                                            }

                                            case 2:{
                                                this->battleTrainer();
                                                break;
                                            }

                                            case 3:{
                                                this->challengeGym();
                                                break;
                                            }

                                            case 4:{
                                                this->challengeElite();
                                                break;
                                            }

                                            case 5:{
                                                this->challengeChampion();
                                                break;
                                            }

                                            case 6:{
                                                this->linkBattle();
                                                break;
                                            }

                                            case 7:{
                                                this->seeTrainersBattled();
                                                break;
                                            }

                                            case 8:{
                                                break;
                                            }

                                            default:{
                                                throw 'c';
                                            }
                                        }
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                        cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                    break;
                                }

                                case 3:{
                                    cout << "1. See items\n";
                                    cout << "2. Buy Items\n";
                                    cout << "3. Exit\n";

                                    try{
                                        int k3;
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        switch(k3){
                                            case 1:{
                                                this->seeItems();
                                                break;
                                            }

                                            case 2:{
                                                this->buyItems();
                                                break;
                                            }

                                            case 3:{
                                                break;
                                            }

                                            default:{
                                                throw 'c';
                                            }
                                        }
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                        cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                    break;
                                }

                                case 4:{
                                    cout << "1. See location information\n";
                                    cout << "2. Go somewhere\n";
                                    cout << "3. Exit\n";

                                    try{
                                        int k3;
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        switch(k3){
                                            case 1:{
                                                this->seeLocationInformation();
                                                break;
                                            }

                                            case 2:{
                                                this->goToLocation();
                                                break;
                                            }

                                            case 3:{
                                                break;
                                            }

                                            default:{
                                                throw 'c';
                                            }
                                        }
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                        cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                    break;
                                }

                                case 5:{
                                    cout << "1. See player information\n";
                                    cout << "2. Toggle autoplay\n";
                                    cout << "3. Exit\n";

                                    try{
                                        int k3;
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        switch(k3){
                                            case 1:{
                                                this->playerInformation();
                                                break;
                                            }

                                            case 2:{
                                                this->toggleAutoPlay();
                                                break;
                                            }

                                            case 3:{
                                                break;
                                            }

                                            default:{
                                                throw 'c';
                                            }
                                        }
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                        cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                    break;
                                }

                                case 6:{
                                    cout << "1. See box Pokemon\n";
                                    cout << "2. Put Pokemon in box\n";
                                    cout << "3. Take Pokemon from Box\n";
                                    cout << "4. Release Pokemon\n";
                                    cout << "5. Exit\n";

                                    try{
                                        int k3;
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        switch(k3){
                                            case 1:{
                                                this->seeBox();
                                                break;
                                            }

                                            case 2:{
                                                this->putInBox();
                                                break;
                                            }

                                            case 3:{
                                                this->takeFromBox();
                                                break;
                                            }

                                            case 4:{
                                                this->releasePokemon();
                                                break;
                                            }

                                            case 5:{
                                                break;
                                            }

                                            default:{
                                                throw 'c';
                                            }
                                        }
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                        cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                    break;
                                }

                                case 7:{
                                    playing = 0;
                                    break;
                                }

                                default: {
                                    throw 'c';
                                }
                            }

                        }
                        catch(int x){
                            cout << "Invalid input\n";
                            cin.clear();
                            cin.ignore(100, '\n');
                        }
                        catch(char c){
                            cout << "Invalid value\n";
                        }
                        catch(...){
                            cout << "Error\n";
                            exit(1);
                        }
                    }
                    break;
                }
                
                case 2:{
                    cout << "1. See database\n";
                    cout << "2. Add Pokemon to database\n";
                    cout << "3. Exit\n";
                    int k2;

                    try{
                        cin >> k2;
                        if(cin.fail())
                            throw 1;
                        
                        switch(k2){
                            case 1:{
                                cout << "1. See all\n";
                                cout << "2. See gym leaders\n";
                                cout << "3. See elite four and champion\n";
                                cout << "4. See database Pokemon\n";
                                cout << "5. Exit\n";
                                int k3;

                                try{
                                    cin >> k3;
                                    if(cin.fail())
                                        throw 1;
                                    
                                    switch(k3){
                                        case 1:{
                                            cout << *this << '\n';
                                            break;
                                        }

                                        case 2:{
                                            this->showGymLeaders();
                                            break;
                                        }

                                        case 3:{
                                            this->showEliteChampion();
                                            break;
                                        }

                                        case 4:{
                                            this->showInGamePokemon();
                                            break;
                                        }

                                        case 5:{
                                            break;
                                        }

                                        default:{
                                            throw 'c';
                                        }
                                    }
                                }
                                catch(int x){
                                    cout << "Invalid input\n";
                                    cin.clear();
                                    cin.ignore(100, '\n');
                                }
                                catch(char c){
                                    cout << "Invalid value\n";
                                }
                                catch(...){
                                    cout << "Error\n";
                                    exit(1);
                                }
                                break;
                            }
                            case 2:{
                                bool ok2 = 1;
                                while(ok2){
                                    cout << "1. Add\n";
                                    cout << "2. Stop\n";
                                    int k3;

                                    try{
                                        cin >> k3;
                                        if(cin.fail())
                                            throw 1;
                                        
                                        if(k3 == 1)
                                            this->writeDatabasePokemon();
                                        else if(k3 == 2)
                                            ok2 = 0;
                                        else
                                            throw 'c';
                                        
                                        
                                    }
                                    catch(int x){
                                        cout << "Invalid input\n";
                                        cin.clear();
                                        cin.ignore(100, '\n');
                                    }
                                    catch(char c){
                                            cout << "Invalid value\n";
                                    }
                                    catch(...){
                                        cout << "Error\n";
                                        exit(1);
                                    }
                                }
                                break;
                            }

                            case 3:{
                                break;
                            }

                            default:{
                                throw 'c';
                            }
                        }
                        
                    }
                    catch(int x){
                        cout << "Invalid input\n";
                        cin.clear();
                        cin.ignore(100, '\n');
                    }
                    catch(char c){
                            cout << "Invalid value\n";
                    }
                    catch(...){
                        cout << "Error\n";
                        exit(1);
                    }
                    break;
                }

                case 3:{
                    ok = 0;
                    break;
                }

                default:{
                    throw 'c';
                }
            }
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

Game::Game(string filename): trainersBattled(0){
    try{
        SDL_Init(SDL_INIT_VIDEO);

        if(!this->databaseFile.is_open())
            throw 1;

        this->databaseFile >> region;

        // Read gym leaders from file and calculate the Pokemon stats
        for(int i = 0; i < 8; i++){
            this->gymLeaders[i].readFromFile(this->databaseFile);
            this->gymLeaders[i].calculateTeamStats();
        }
        
        // The same for elite four and champion
        for(int i = 0; i < 4; i++){
            this->eliteFour[i].readFromFile(this->databaseFile);
            this->eliteFour[i].calculateTeamStats();
        }
        
        this->champion.readFromFile(this->databaseFile);
        this->champion.calculateTeamStats();

        // Read trainer names from file
        int number;
        this->databaseFile >> number;
        string name;
        for(int i = 0; i < number; i++){
            this->databaseFile >> name;
            this->trainerNames.push_back(name);
        }

        // Read the Pokemon that can appear in the game
        while(!this->databaseFile.eof()){
            Pokemon temp;
            temp.readFromFile(this->databaseFile);
            // Check if there was a newline character
            if(temp.getSpecies() != "")
                this->inGamePokemon.push_back(temp);
        }
        // Clear eofbit error flag to be able to write to file again
        this->databaseFile.clear();
    }
    catch(int x){
        cout << "Problems with opening the database file\n";
        exit(1);
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }

    try{
        if(!this->trainersFile.is_open())
            throw 1;
    }
    catch(int x){
        cout << "Problems with opening the trainers file\n";
        exit(1);
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }

    // The player file
    playerFile = filename;
    fstream f(playerFile, ios::binary | ios::in | ios::out);

    try{
        if(!f.is_open())
            throw 1;

        cout << "The game has started\n";

        // Only read from player file if it's not empty
        f.seekg(0, ios::end);
        if(f.tellg()){
            // Reset the get pointer to the beggining of the file
            f.seekg(0);

            this->player.deserialization(f);

            // Player may not be able to play if they end up with no dollars
            if(this->player.getDollars() == 0)
                this->player.setDollars(100);
            
            this->player.calculateTeamSize();
        }
        else{
            // Start game
            this->newGame();
        }
    }
    catch(int x){
        cout << "Problems with opening the player file\n";
        exit(1);
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }

    // Set currentLocation to first route
    this->currentLocation = *this->locations.begin();
}

ostream& operator <<(ostream& os, const Game& obj){
    os << "Region: " << obj.region << "\n";
    os << "Champion:\n" << obj.champion << "\n";

    os << "Elite four:\n";
    for(int i = 0; i < 4; i++)
        os << obj.eliteFour[i] << "\n";

    os << "Gym Leaders:\n";
    for(int i = 0; i < 8;i ++)
        os << obj.gymLeaders[i] << "\n";

    os << "Trainer names:\n";
    for(int i = 0; i < obj.trainerNames.size(); i++)
        os << obj.trainerNames[i] << ' ';
    os << "\n\n";

    os << "Pokemon in the game:\n";
    for(int i = 0; i < obj.inGamePokemon.size(); i++)
        os << obj.inGamePokemon[i] << "\n";

    os << "Locations:\n";
    for(auto it = obj.locations.begin(); it != obj.locations.end(); it++){
        os << "Name: " << it->second.first << "\n";
        os << "Min level: " << it->first.first << " Max level: " << it->first.second << "\n";
        os << "Pokemon types found: ";
        for(int i = 0; i < it->second.second.first.size(); i++)
            os << it->second.second.first[i] << ' ';
        os << '\n';
        os << "Pokemon types not found: ";
        for(int i = 0; i < it->second.second.second.size(); i++)
            os << it->second.second.second[i] << ' ';
        os << "\n\n";
    }

    os << "Player details:\n";
    os << obj.player << "\n";
    return os;
}

void Game::showGymLeaders(){
    for(int i = 0; i < 8; i++)
        cout << this->gymLeaders[i] << '\n';
    cout << '\n';
}

void Game::showEliteChampion(){
    for(int i = 0; i < 4; i++)
        cout << this->eliteFour[i] << '\n';
    cout << '\n' << this->champion << '\n';
}

void Game::showInGamePokemon(){
    for(int i = 0; i < this->inGamePokemon.size(); i++)
        cout << this->inGamePokemon[i] << '\n';
    cout << '\n';
    cout << "Total number: " << this->inGamePokemon.size() << '\n';
}

void Game::writeDatabasePokemon(){
    Pokemon p;
    cin >> p;
    p.writeToFile(this->databaseFile);
}

void Game::newGame(){
    cout << "What is your name?\n";
    string name;
    cin >> name;
    this->player.setName(name);

    cout << "Pick a starter Pokemon\n";
    for(int i = 0; i < 3; i++)
        cout << i+1 << ". " << this->inGamePokemon[i].getSpecies() << " the " << 
            this->inGamePokemon[i].getFirstType() << " type Pokemon\n";
    
    bool ok = 0;
    while(!ok){
        try{
            int k;
            cin >> k;
            if(cin.fail())
                throw 1;
            if(k < 1 || k > 3)
                throw 'c';

            cout << "Picked " << this->inGamePokemon[k-1].getSpecies() << "\n";
            cout << this->inGamePokemon[k-1].getSpecies() << " will be added to the team\n";
            vector<Pokemon> starter = {this->inGamePokemon[k-1]};
            starter[0].setMoves();

            this->player.setTeam(starter);
            this->player.calculateTeamStats();
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
    cout << "Received 1000 dollars\n";
    this->player.setDollars(1000);

    cout << "Received 20 Pokeballs\nReceived 20 Potions\n";
    vector<Item*> items;

    for(int i = 0; i < 20; i++){
        items.push_back(new Pokeball("Pokeball", 20));
        items.push_back(new Potion("Potion", 20, 20));
    }
    this->player.setItems(items);
}

void Game::seeLocationInformation(){
    cout << "Locations:\n";
    for(auto it = locations.begin(); it != locations.end(); it++){
        cout << "Name: " << it->second.first << "\n";
        cout << "Min level: " << it->first.first << " Max level: " << it->first.second << "\n";
        cout << "Pokemon types found: ";
        for(int i = 0; i < it->second.second.first.size(); i++)
            cout << it->second.second.first[i] << ' ';

        cout << '\n';
        cout << "Pokemon types not found: ";
        for(int i = 0; i < it->second.second.second.size(); i++)
            cout << it->second.second.second[i] << ' ';
        cout << "\n\n";
    }
}

void Game::playerInformation(){
    cout << "Name: " << this->player.getName() << "\n";
    cout << "Balance: " << this->player.getDollars() << "\n"; 
    cout << "Gym Badges: " << this->player.getGymBadges() << "\n";
    cout << "Autoplay: " << this->player.getAutoPlay() << "\n";
}

void Game::heal(){
    try{
        cout << "1. Go to Pokemon Center\n";
        cout << "2. Heal with potions\n";
        cout << "3. Exit\n";

        int k;
        cin >> k;
        if(cin.fail())
            throw 1;
        
        switch(k){
            case 1:{
                if(this->player.getDollars() < 50 * player.getAllTeamSize()){
                    cout << "You don't have enough dollars\n";
                    break;
                }

                int healed = 0;

                for(int i = 0; i < this->player.getAllTeamSize(); i++)
                    if(this->player.restoreHP(i))
                        ++healed;
                
                int currentDollars = this->player.getDollars() - (50 * healed);
                this->player.setDollars(currentDollars);
                this->player.calculateTeamSize();
                break;
            }

            case 2:{
                bool stop = 0;
                while(!stop){
                    if(!this->player.checkPotions()){
                        cout << "You doesn't have any Potions\n";
                        this->player.calculateTeamSize();
                        return;
                    }

                    int index = this->pickPokemon();
                    if(index == -1)
                        return;
                    
                    bool ok = 0;
                    while(!ok){
                        cout << "Select potion\n";
                        cout << "1. Potion\n2. Super Potion\n3. Hyper Potion\n4. Max Potion\n5. Exit\n";
                        try{
                            int potion;
                            cin >> potion;
                            if(cin.fail())
                                throw 1;
                                        
                            if(potion == 1){
                                ok = this->player.restoreHP(index, "Potion");
                            }
                            else if(potion == 2){
                                ok = this->player.restoreHP(index, "Super Potion");
                            }
                            else if(potion == 3){
                                ok = this->player.restoreHP(index, "Hyper Potion");
                            }
                            else if(potion == 4){
                                ok = this->player.restoreHP(index, "Max Potion");
                            }
                            else if(potion == 5)
                                ok = 1;
                            else
                                throw 'c';
                        }
                        catch(int x){
                            cout << "Invalid input\n";
                            cin.clear();
                            cin.ignore(100, '\n');
                        }
                        catch(char c){
                            cout << "Invalid value\n";
                        }
                        catch(...){
                            cout << "Error\n";
                            exit(1);
                        }
                    }
                    cout << "Continue healing? (y/ n)\n";
                    string answer;

                    try{
                        cin >> answer;
                        if(answer == "n"){
                            this->player.calculateTeamSize();
                            stop = 1;
                        }
                        else if(answer != "y")
                            throw 1;
                    }
                    catch(int x){
                        cout << "Invalid answer\n";
                    }
                    catch(...){
                        cout << "Error\n";
                        exit(1);
                    }
                }
                break;
            }

            case 3:{
                this->player.calculateTeamSize();
                break;
            }

            default:{
                cout << "Invalid value\n";
                break;
            }
        }
    }
    catch(int x){
        cout << "Invalid input\n";
        cin.clear();
        cin.ignore(100, '\n');
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void Game::learnMove(){
    int index = this->pickPokemon();
    if(index == -1)
        return;
    
    for(int i = 0; i < inGamePokemon.size(); i++)
        if(this->player.getSpecies(index) == inGamePokemon[i].getSpecies()){
            this->player.learnMove(inGamePokemon[i], index);
            return;
        }
}

void Game::evolve(){
    int index = this->pickPokemon();
    if(index == -1)
        return;
    this->player.evolve(inGamePokemon, index);
}

void Game::trainPokemon(){
    int index = this->player.pickPokemon();
    if(index == -1)
        return;
    this->player.trainPokemon(index);
}

void Game::goToLocation(){
    cout << "You are currently at " << this->currentLocation.second.first << "\n";
    cout << "Select where you want to go\n";

    int k = 1;
    for(auto it = locations.begin(); it != locations.end(); it++){
        cout << k << ". " << it->second.first << "\n";
        ++k;
    }
    cout << k << ". Exit\n";

    bool ok = 0;
    while(!ok){
        try{
            int k2;
            cin >> k2;
            if(cin.fail())
                throw 1;
            
            if(k2 == k)
                return ;
            
            if(k2 < 1 || k2 > k)
                throw 'c';
            
            auto it = locations.begin();
            for(int i = 1; i < k2; i++)
                ++it;
            
            this->currentLocation = *it;
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }
}

Pokemon Game::searchDatabase(string species){
    Pokemon temp;
    for(int i = 0; i < inGamePokemon.size(); i++)
        if(species == inGamePokemon[i].getSpecies()){
            temp = inGamePokemon[i];
            break;
        }
    return temp;
}

void Game::getAllPossiblePokemon(vector<Pokemon>& possiblePokemon){
    if(!possiblePokemon.empty())
        possiblePokemon.clear();

    // Find all the possible Pokemon that can appear in the current location
    for(int i = 0; i < this->inGamePokemon.size(); i++){
        int ok = 0, ok2 = 1;

        for(int j = 0; j < this->currentLocation.second.second.first.size(); j++)
                if(this->inGamePokemon[i].getFirstType() == this->currentLocation.second.second.first[j] ||
                    this->inGamePokemon[i].getSecondType() == this->currentLocation.second.second.first[j]){
                        ok = 1;
                    }
                
            for(int j = 0; j < this->currentLocation.second.second.second.size(); j++)
                if(this->inGamePokemon[i].getFirstType() ==this->currentLocation.second.second.second[j] ||
                    this->inGamePokemon[i].getSecondType() ==this->currentLocation.second.second.second[j]){
                        ok2 = 0;
                    }
            if(ok == 1 && ok2 == 1 && this->inGamePokemon[i].getLevel() <= this->currentLocation.first.second)
                possiblePokemon.push_back(inGamePokemon[i]);
        }
}

void Game::generateRandomPokemon(const vector<Pokemon>& possiblePokemon, Pokemon& randomPokemon){
    // Pick a random Pokemon out of all the possible ones
    uniform_int_distribution<int> number(0, possiblePokemon.size()-1);
    int index = number(eng);

    // Customize the Pokemon randomly
    randomPokemon = possiblePokemon[index];
    // Choose level based on the min and max level of the current location
    uniform_int_distribution<int> number2(this->currentLocation.first.first, this->currentLocation.first.second);
    int level = number2(eng);
    randomPokemon.setLevel(level);

    // Set the stats of the Pokemon
    randomPokemon.calculateStats(level);

    // Set the moves of the Pokemon
    randomPokemon.setMoves();
}

Trainer* Game::generateRandomTrainer(const vector<Pokemon>& possiblePokemon){
    // Pick a random number from 1 to 6 which will be the Trainer's team size
    uniform_int_distribution<int> number(1, 6);
    int size = number(eng);

    // Generate trainer team
    vector<Pokemon> randomTeam;
    for(int i = 0; i < size; i++){
        Pokemon p;
        this->generateRandomPokemon(possiblePokemon, p);
        randomTeam.push_back(p);
    }

    uniform_int_distribution<int> number2(0, this->trainerNames.size() - 1);
    Trainer* randomTrainer = new UnplayableTrainer(this->trainerNames[number2(eng)], randomTeam);
    return randomTrainer;
}

void Game::battleWildPokemon(){
    vector<Pokemon> possiblePokemon;
    this->getAllPossiblePokemon(possiblePokemon);

    Pokemon randomPokemon;
    this->generateRandomPokemon(possiblePokemon, randomPokemon);

    Battle<Trainer*, Pokemon*> battle1(&(this->player), &randomPokemon);
    battle1.doBattle();
}

void Game::battleTrainer(){
    vector<Pokemon> possiblePokemon;
    this->getAllPossiblePokemon(possiblePokemon);

    Trainer* randomTrainer = this->generateRandomTrainer(possiblePokemon);
    
    Battle<Trainer*, Trainer*> battle1(&(this->player), randomTrainer);
    battle1.doBattle();
    delete randomTrainer;
}

void Game::challengeGym(){
    if(this->player.getGymBadges() < 8){
        int gymBadges = this->player.getGymBadges();
        string type = gymLeaders[gymBadges].getFirstType();
        cout << "Challenge " << gymLeaders[gymBadges].getName() << " the " << type << 
                " type gym leader? (yes/ no)\n";

        string answer;
        cin >> answer;
        if(answer == "no")
            return;
        else if(answer != "yes"){
            cout << "Invalid value\n";
            return;
        }

        Battle<HybridTrainer*, UnplayableTrainer*> gymBattle(&(this->player),
                 &(gymLeaders[gymBadges]));
        gymBattle.doBattle();

        if(!this->player.fainted() && gymLeaders[gymBadges].fainted()){
            cout << "You received the " << type << " badge\n";
            this->player.increaseGymBadges();
        }
        else{
            cout << "Better luck next time!\n";
        }

        gymLeaders[gymBadges].resetHP();
        this->player.calculateTeamSize();
    }
    else{
        cout << "You defeated all the gym leaders\n";
        // TODO: Option to rematch
    }
}

void Game::challengeElite(){
    if(this->player.getGymBadges() < 8){
        cout << "You have to defeat all the gym leaders before challenging the elite four\n";
        return;
    }
    if(this->player.getGymBadges() == 12){
        cout << "You defeated all members of the elite four\n";
        // TODO: Option to rematch
        return;
    }

    int gymBadges = this->player.getGymBadges();
    string type = eliteFour[gymBadges - 8].getFirstType();
    cout << "Challenge " << eliteFour[gymBadges - 8].getName() << " of the elite four, the " << type << 
            " type user? (yes/ no)\n";

    string answer;
    cin >> answer;
    if(answer == "no")
        return;
    else if(answer != "yes"){
        cout << "Invalid value\n";
        return;
    }

    Battle<HybridTrainer*, UnplayableTrainer*> gymBattle(&(this->player),
                &(eliteFour[gymBadges - 8]));
    gymBattle.doBattle();

    eliteFour[gymBadges - 8].resetHP();

    if(!this->player.fainted() && eliteFour[gymBadges - 8].fainted()){
        this->player.increaseGymBadges();
    }
    else{
        cout << "Better luck next time!\n";
    }

    eliteFour[gymBadges - 8].resetHP();
    this->player.calculateTeamSize();
}

void Game::challengeChampion(){
    if(this->player.getGymBadges() < 12){
        cout << "You have to defeat all the gym leaders and the elite four before challenging the champion\n";
        return;
    }
    else if(this->player.getGymBadges() == 13){
        cout << "You already defeated " << this->champion.getName() << "\n";
        // TODO Option to rematch
        return;
    }
    cout << "Challenge " << this->champion.getName() << " the " << region << " champion? (yes/ no)\n";
    string answer;
    cin >> answer;
    if(answer == "no")
        return;
    else if(answer != "yes"){
        cout << "Invalid value\n";
        return;
    }

    Battle<HybridTrainer*, UnplayableTrainer*> gymBattle(&(this->player), &this->champion);
    gymBattle.doBattle();

    if(!this->player.fainted() && this->champion.fainted()){
        cout << "Congratulations! You are the new " << region << " champion!\n";
        this->player.increaseGymBadges();
    }
    else{
        cout << "Better luck next time!\n";
    }

    this->champion.resetHP();
    this->player.calculateTeamSize();
}

Trainer* Game::readLinkBattleTrainer(){
    string name;
    cout << "Enter the opponent's name\n";
    cin.get();
    getline(cin, name);

    int teamSize;
    vector<Pokemon> team;

    bool ok = 0;
    while(!ok){
        try{
            if(!team.empty())
                team.clear();
            cout << "Enter the team size:\n";
            cin >> teamSize;
            if(cin.fail())
                throw 1;
            if(teamSize < 1 || teamSize > 6)
                throw 'c';

            for(int i = 0; i < teamSize; i++){
                string species;
                int level;
                cout << "Enter the species\n";
                cin >> species;

                // Species must be present in the database
                Pokemon temp = this->searchDatabase(species);
                if(!(species == temp.getSpecies()))
                    throw 3.4;
                
                cout << "Enter the level\n";
                cin >> level;

                if(cin.fail())
                    throw 1;
                if(level < 1 || level > 100)
                    throw 'c';
                
                // Moves and stats will be calculated randomly
                temp.calculateStats(level);
                temp.setMoves();
                team.push_back(temp);
            }
            ok = 1;
        }
        catch(int x){
            cout << "Invalid input\n";
            cin.clear();
            cin.ignore(100, '\n');
        }
        catch(char c){
            cout << "Invalid value\n";
        }
        catch(double d){
            cout << "This Pokemon isn't in the database\n";
        }
        catch(...){
            cout << "Error\n";
            exit(1);
        }
    }

    Item *i1 = new Potion("Potion", 20, 20);
    Item *i2 = new Potion("Super Potion", 50, 60);
    Item *i3 = new Potion("Hyper Potion", 70, 120);
    Item *i4 = new Potion("Hyper Potion", 70, 120);
    Item *i5 = new Potion("Max Potion", 100, 1000);
    Item *i6 = new Potion("Max Potion", 100, 1000);

    Trainer* opponent = new PlayableTrainer(name, team, 1000, {}, {i1, i2, i3, i4, i5, i6});

    delete i1;
    delete i2;
    delete i3;
    delete i4;
    delete i5;
    delete i6;

    return opponent;
}

Trainer* Game::rebattleLinkBattleTrainer(){
    if(this->trainersBattled == 0){
        cout << "No trainers battled yet\n";
        return nullptr;
    }
    this->trainersFile.seekg(0);
    vector<Trainer*> trainers;

    // Read the trainers from file
    cout << "Select trainer\n";
    for(int i = 0; i < this->trainersBattled; i++){
        Trainer* opponent = new PlayableTrainer();
        opponent->deserialization(this->trainersFile);

        cout << i+1 << ". "<< opponent->getName() << "\n";
        trainers.push_back(opponent);
    }

    try{
        int k;
        cin >> k;

        if(cin.fail())
            throw 1;

        if(k-1 < 0 || k-1 >= this->trainersBattled){
            cout << "Invalid value\n";

            for(int i = 0; i < trainers.size(); i++)
                delete trainers[i];

            return nullptr;
        }

        for(int i = 0; i < trainers.size(); i++)
            if(i != k-1)
                delete trainers[i];

        return trainers[k-1];
    }
    catch(int x){
        cout << "Invalid input\n";
        cin.clear();
        cin.ignore(100, '\n');
        return nullptr;
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void Game::linkBattle(){
    try{
        cout << "1. Enter trainer\n";
        cout << "2. Rebattle trainer\n";
        cout << "3. Exit\n";
        int k;
        cin >> k;

        if(cin.fail())
            throw 1;

        switch(k){
            case 1:{
                // Allocates memory (holds PlayableTrainer address) 
                Trainer* opponent = this->readLinkBattleTrainer();    

                Battle<HybridTrainer*, Trainer*> linkBattle(&this->player, opponent);
                linkBattle.doBattle();
                // Reset the HP for rebattling
                opponent->resetHP();
                // Increase trainers battled to read correctly from file 
                ++this->trainersBattled;

                // Write the trainer to the file 
                opponent->serialization(this->trainersFile);
                delete opponent;

                break;
            }

            case 2:{
                Trainer* opponent = this->rebattleLinkBattleTrainer();
                if(opponent == nullptr)
                    return;

                Battle<HybridTrainer*, Trainer*> linkBattle(&this->player, opponent);
                linkBattle.doBattle();

                opponent->resetHP();
                delete opponent;
                break;
            }

            case 3:{
                return;
            }

            default:{
                cout << "Invalid value\n";
                return;
            }
        }
    }
    catch(int x){
        cout << "Invalid input\n";
        cin.clear();
        cin.ignore(100, '\n');
    }
    catch(...){
        cout << "Error\n";
        exit(1);
    }
}

void Game::seeTrainersBattled(){
    // View all the trainers fought in link battles
    this->trainersFile.seekg(0);

    for(int i = 0; i < this->trainersBattled; i++){
        PlayableTrainer opponent;
        opponent.deserialization(this->trainersFile);
        cout << opponent << '\n';
    }
}

Game::~Game(){
    SDL_Quit();
    // Save game by replacing the data in the player file with new data
    fstream f(playerFile, ios::binary | ios::in | ios::out | ios::trunc);
    this->player.serialization(f);

    // Check the data
    fstream g("text.txt", ios::out | ios::trunc);
    g << '\n';
    this->player.writeToFile(g);
}

int main(){
    Game& game = Game::getInstance();
    game.startGame();
    return 0;
}