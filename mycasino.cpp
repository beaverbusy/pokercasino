#include<memory> // smart pointers
#include<utility> // pairs
#include<iostream>
#include<vector>
#include<iterator>
#include<algorithm> // sort
#include<array>
#include <cstdlib>
#include <ctime>
 
using namespace std;
// debug func:
void printarray(array<int, 52>& v) {
	for (auto s : v) {
		cout << s << " ";
	}
	cout << endl;
}
void sumarray(array<int, 52>& v) {
	int n = 0;
	for (auto s : v) {
		n += s;
	}
	cout << "sume array is: " << n << endl;
}



//
// a hand is coded into string. players are 0 to 5 (6 max)
// each hand is coded in chunks of 6 moves ie one per player.
// always starting with player in position 0
// hands start with hand number then "P" for preflop
// e.g. wBsbfr means 0: waits, 1 waits he is button, 2: sb, 3: bb, 4: folds, 5: raises. once a player folded he will appear with f for all his positions in the string
// L means flop starts, T is for turn, R for river. action: f: fold, c: check/call, r: raise.
// ends with "S" + all players winning hand + "Ca" 1st card of player 0 (99 if no show) + "Cb" + 2nd card 
//
//Magic numbers:
const int nBots = 6; // : number of bots

class Bot {
	public:
		Bot(string fileName); // communication with the bot through filename or socket
	       	void setHand(pair<int, int> hand); // tell the bot his holdem start hand
		void setSeat(int pos); // tell the bot his seat at table
		string getAction(string history); // ask bot his move at stage <history> in current hand
	private:
		string 	mfileName; // socket used to talk to casino
		pair<int, int> mstartingHand; // current start hand
		int mSeat; // current seat on table
		string mcurrentHand; // status of current hand
		int mStack; // current money
};

class Casino {
	public:
		void shuffleDeck(); // randomly shuffles mDeck
		void populateTable(); // creates vector of bots
		void dealCards(); // tell bots their starting hands 
		
	private:
		vector<Bot*> mPlayers; // a queue of all the bots
		array<int, 52> mDeck; // shuffled deck
		int mCounter; // current hand number

};

void Casino::dealCards() {
	for (int i = 0; i < nBots; i++) {
		mPlayers[i]->setHand(make_pair(mDeck[2 * i], mDeck[2 * i + 1]));
	}
}

void Casino::shuffleDeck() { // randomly shuffles input deck todo only go to 2* players + 5
	array<int, 52> orderedDeck; // contains all cards from 0 to 51
	for (int i = 0; i < 52; i++)
		orderedDeck[i] = i;
	for (int i = 0; i < 52; i++) { 
		int k = rand() % (52 - i); // next card drawn
		int counter = 0;
		for (int j = 0; j < 52; j++) { // we fetch k-th undrawn card, i.e. among those not = 99
			if (orderedDeck[j] != 99) // 99 codes for card already drawn
				counter++;
			if (counter == k + 1) {
			       orderedDeck[j] = 99;
			       mDeck[i] = j;
		       		break;
			}		
			
		}
	}
	
	
}

void Casino::populateTable(){
	for (int i = 0; i < nBots; i++) { // creates a vector of nBots smart pointers to Bot objects.
		Bot* abot = new Bot("socket" + to_string(i)); // smart pointer to a bot object
		abot->setSeat(i);
		mPlayers.push_back(abot); // push it on mPlayers
	}
}



int main()
{
	// initialise rand, todo use mersenne for better randomness
	std::srand(std::time(nullptr)); 

}
