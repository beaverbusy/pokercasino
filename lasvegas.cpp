// 			Limit holdem poker bot tester
// cards are 4 * rank + suit where rank is 0 .. 12 for deuce to ace, and suits is 0 .. 3
// players are 0 to nBots -1
// the blinds count as one raise
// simplification: two 1$ blinds are taken, on turn bets become 2$.
// if player raises more than nMaxraises, he is autofolded
// communication files: casinoToBot<number> and botToCasino<number>
// at start: casinoToBot is written: <hand number>D<button position>A<holecard1>B<holecard2>
// then move is read in botToCasino. moves are either r/f/c. If error then player is autofolded
// casinoToBot is then cleared and written the hand action in the format: (without the spaces)
// <hand number> D <dealer button position>  P <action by all players in order from first to act, e.g. fccrf...> F <flop card 1> F <flop 2> F <flop 3> F <flop action starting with first player to act>
// T <turn card> T <turn action> R <river card> R <river action>
// once hand is over file handSummaryEven or Odd gets cleared, written the action string to, where the string is followed by the showdown info:
// SA <card 1 of pl 1> B <card 2 of pl 1> A <c1 of p2> B <c2 p2> ....  W <seat of winner 1> W <winner 2> .... E
// where E codes end of string
// handSummaryOdd and Even : summary of last two hands
// results : every nLogFrequency hands, the stacks of each player are concatenated to this file, use to declare winner
#include "./omp/HandEvaluator.h"
#include<chrono> 
#include<thread>
#include<memory> // smart pointers
#include<fstream> // smart files
#include<tuple> 
#include<utility> // tuples
#include<iostream>
#include<vector>
#include<deque>
#include<array>
#include <cstdlib>
#include <ctime>
 
using namespace std;

//Magic numbers:
const int nBots = 4; // : number of bots
const int nMaxRaises = 3;
const int nRounds = 100; // number of rounds to test the bots
const int nLogFrequency = 10; // frequency of result logs
// time given to each bot is set in the function attend()
const array<string, 52> eRank = {
	"2h", "2c", "2s", "2d",
	"3h", "3c", "3s", "3d",
	"4h", "4c", "4s", "4d",
	"5h", "5c", "5s", "5d",
	"6h", "6c", "6s", "6d",
	"7h", "7c", "7s", "7d",
	"8h", "8c", "8s", "8d",
	"9h", "9c", "9s", "9d",
	"Th", "Tc", "Ts", "Td",
	"Jh", "Jc", "Js", "Jd",
	"Qh", "Qc", "Qs", "Qd",
	"Kh", "Kc", "Ks", "Kd",
	"Ah", "Ac", "As", "Ad",
};

void attend() { // time given to bots for each decision
	std::this_thread::sleep_for(10ms);
}

class Bot; // declare first for recursive def
class Casino {
	public:
		// game flow:
		void populateTable(); // creates vector of bots
		void shuffleDeck(); // randomly shuffles mDeck
		void dealCards(); // tell bots their starting hands 
		void getPreflopBets(); 
		void getFlopBets(); 
		void getTurnBets(); 
		void getRiverBets(); 
		void getWinners();
		void payoffs(); // pay winners
		void showdown(); // add showdown to mCurrentHand
		void tellHandSummary(); // tells hand summary to all in even or odd file.
		void fileHandSummary(); // tells hand summary to all in even or odd file.
		void printHandSummary(); // tells hand summary to all in even or odd file.
		void prepareNext(); // initialise stuff for next hand
		// misc
		bool tableEmpty(); // check if only one player is left, ie can go to showdown
		
	public: // todo make private after debug
		vector<Bot*> mPlayers; // a queue of all the bots
		deque<Bot*> table; // all players still in the hand
		array<int, 52> mDeck; // shuffled deck
		string mCurrentHand; // status of current hand
		vector<int> mWinners; // winners among players left in hand
		int mCounter = 0; // current hand number
		int mButton = 0; // current button
		int mPot = 0; // current pot

		
};

class Bot {
	public:
		friend void Casino::printHandSummary(); // debug
		friend void Casino::fileHandSummary(); // debug
		void setSeat(int pos); // tell the bot his seat at table
	       	void setHand(int handno, int button, tuple<int, int> hand); // tell the bot his holdem start hand
		tuple<int, int> getHand();
		void addStack(int ammount); // adds ammount to mStack
		int getSeat(); // returns seat number
		char getAction(); // read from file botToCasino<no>
		void tellAction(string handStatus); // tell the bot the action so far via their file
	private: 
//		string 	mfileName; // socket used to talk to casino
		tuple<int, int> mHand; // current start hand
		int mSeat; // current seat on table
		int mStack = 0; // current money
};


void Bot::setSeat(int pos){
	mSeat = pos;
}

void Bot::setHand(int handno, int button, tuple<int, int> hand) {
	mHand = hand;
	ofstream fout("./botfiles/casinoToBot" + to_string(getSeat()), ios_base::trunc);
	if (!fout.good()) {
		cerr << "Error while opening output file for bot " << getSeat() << endl;
	}
	fout << handno << "D" << button << "A" << get<0>(hand) << "B" << get<1>(hand); // coded: hand number A card1 B card2
}

tuple<int, int> Bot::getHand() {
	return mHand;
}



void Bot::addStack(int ammount) {
	mStack += ammount;
	}

	
int Bot::getSeat(){return mSeat;}

//
// casino methods: 
// 
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
		Bot* abot = new Bot(); //("socket" + to_string(i)); // smart pointer to a bot object
		abot->setSeat(i);
		mPlayers.push_back(abot); // push it on mPlayers
	}
}



void Casino::dealCards() {
	for (int i = 0; i < nBots; i++) {
		mPlayers[i]->setHand(mCounter, mButton, make_tuple(mDeck[2 * i], mDeck[2 * i + 1]));
	}
}


void Casino::tellHandSummary() {
	if (mCounter % 2 == 0) {
		ofstream fout("./botfiles/handSummaryEven", ios_base::trunc);
		if (!fout.good()) {
			cerr << "Error opening handSummaryEven file " << endl;
		}
		fout << mCurrentHand;
	}
	else {
		ofstream fout("./botfiles/handSummaryOdd", ios_base::trunc); 
		if (!fout.good()) {
			cerr << "Error opening handSummaryOdd file " << endl;
		}
		fout << mCurrentHand;

	}
}



void Casino::showdown(){
	mCurrentHand += "S";
	for (int i = 0; i < nBots; i++) {
		mCurrentHand += "A" + to_string(mDeck[2 * i]) + "B" + to_string(mDeck[2 * i + 1]) ;
	}
	for (auto v : mWinners) {
		mCurrentHand += "W" + to_string(v);
	}
	mCurrentHand += 'E'; // end of hand
}	



bool Casino::tableEmpty(){
	return table.size() == 1;
}

void Casino::prepareNext() {
	mCounter++;
	mButton = (mButton + 1) % nBots;
}

void Bot::tellAction(string handStatus) {
	ofstream fout("./botfiles/casinoToBot" + to_string(getSeat()), ios_base::trunc); 
	if (!fout.good()) {
		cerr << "Error while opening output file for bot " << getSeat() << endl;
	}
	fout << handStatus;
}
void Casino::getPreflopBets() {
	table.clear();
	vector<int> vpip(nBots, 0); // track record of how many bets each bot already called
	for (int i = 0;  i < nBots; i++){
		table.push_back(mPlayers[(mButton + 3 + i) % nBots]); // queue of players, first player is first to act after sb
	}
	mPlayers[(mButton + 1) % nBots]->addStack(-1); // collect small blind
	vpip[(mButton + 1) % nBots]++; // he paid 1 bet
	mPlayers[(mButton + 2) % nBots]->addStack(-1); // big blind simplification: two players post a 1$ blind (no small or big) todo change
	vpip[(mButton + 2) % nBots]++; // big blind simplification: two players post a 1$ blind (no small or big) todo change
	mPot = 2; // two blinds
	int calls = 0;
	int raises = 1; // count blinds as one raise
	mCurrentHand = to_string(mCounter) + "D" + to_string(mButton) + "P"; // current hand is coded hand number + D + button position + "P"
	while (calls < nBots && table.size() > 1){
		Bot* currentPlayer = table.front(); // get first element
		table.pop_front(); // remove him
		currentPlayer->tellAction(mCurrentHand); // give him hand status
		attend(); // let him process it
		switch(char action = currentPlayer->getAction()) {
			case 'e':
				goto foldCase;
				break;
			case 'c':
				calls++;
				table.push_back(currentPlayer); // player stays on
				currentPlayer->addStack(vpip[currentPlayer->getSeat()] - raises);
				mPot += raises - vpip[currentPlayer->getSeat()];
				vpip[currentPlayer->getSeat()] = raises; // paid all so far
				mCurrentHand += 'c'; // current player called
				break;
			case 'f':
				foldCase:
					calls++;
					mCurrentHand += 'f';
					break;
			case 'r':
				if (raises > nMaxRaises){
					goto foldCase; // the player is folded automatically
				}
				else { // valid raise
					raises++;
					currentPlayer->addStack(vpip[currentPlayer->getSeat()] - raises);
					mPot += raises - vpip[currentPlayer->getSeat()];
					vpip[currentPlayer->getSeat()] = raises;
					table.push_back(currentPlayer); // player stays on
					mCurrentHand += 'r';
					calls = nBots - table.size() + 1; // this bet round stops once all remaining players minus the raiser have called this latest raise (unless there's new raise)
					break;
				}
		}
	}
}

char Bot::getAction(){
	ifstream fin("./botfiles/botToCasino" + to_string(getSeat()));
	if (!fin) {
		cerr << "error opening file botToCasino" << getSeat()  << endl;	
		return 'e';
	}
	else {
		char action;
		fin >> action;
		if (action == 'c' || action == 'f' || action == 'r')
			return action;
		else return 'e';
	}
}





void Casino::getFlopBets() {
	for (int j = 0; j < nBots; j++){// get next to act front of queue
		for (int i = 0; i < table.size(); i++){ 
			Bot* frontPlayer = table.front();
			if (frontPlayer->getSeat() == ((mButton + j + 1) % nBots))
				goto tableOrdered;
			else {
				table.pop_front();
				table.push_back(frontPlayer);
			}
		}
	}
	tableOrdered:
	vector<int> vpip(nBots, 0); // track record of how many bets each bot already called
	int calls = 0;
	int raises = 0;
	mCurrentHand += "F"; // current hand is coded hand number + "F"
	for (int i = 0; i < 3; i++){
//		mCurrentHand += to_string(mDeck[2 * nBots + i]) + "F";
		mCurrentHand += to_string(mDeck[2 * nBots + i]) + "F";
	}
	while (calls < nBots && table.size() > 1){
		Bot* currentPlayer = table.front(); // get first element
		table.pop_front(); // remove him
		currentPlayer->tellAction(mCurrentHand); // give him hand status
		attend(); // let him process it
		switch(char action = currentPlayer->getAction()) {
			case 'c':
				calls++;
				table.push_back(currentPlayer); // player stays on
				currentPlayer->addStack(vpip[currentPlayer->getSeat()] - raises);
				mPot += raises - vpip[currentPlayer->getSeat()];
				vpip[currentPlayer->getSeat()] = raises; // paid all so far
				mCurrentHand += 'c'; // current player called
				break;
			case 'f':
				foldCase:
					calls++;
					mCurrentHand += 'f';
					break;
			case 'r':
				if (raises >  nMaxRaises){
					goto foldCase; // the player is folded automatically
				}
				else { // valid raise
					raises++;
					currentPlayer->addStack(vpip[currentPlayer->getSeat()] - raises);
					mPot += raises - vpip[currentPlayer->getSeat()];
					vpip[currentPlayer->getSeat()] = raises;
					table.push_back(currentPlayer); // player stays on
					mCurrentHand += 'r';
					calls = nBots - table.size() + 1; // this bet round stops once all remaining players minus the raiser have called this latest raise (unless there's new raise)
					break;
				}
			case 'e':
				goto foldCase;
				break;
		}
	}
}


void Casino::getTurnBets() {
	for (int j = 0; j < nBots; j++){
		for (int i = 0; i < table.size(); i++){ // get next to act front of queue
			Bot* frontPlayer = table.front();
			if (frontPlayer->getSeat() == ((mButton + j + 1) % nBots))
				goto tableOrdered;
			else {
				table.pop_front();
				table.push_back(frontPlayer);
			}
		}
	}
	tableOrdered:
	vector<int> vpip(nBots, 0); // track record of how many bets each bot already called
	int calls = 0;
	int raises = 0;
	mCurrentHand += "T"; // current hand is coded hand number + "T"
	//mCurrentHand += to_string(mDeck[2 * nBots + 3]) + "T";
	mCurrentHand += to_string(mDeck[2 * nBots + 3]) + "T";
	while (calls < nBots && table.size() > 1){
		Bot* currentPlayer = table.front(); // get first element
		table.pop_front(); // remove him
		currentPlayer->tellAction(mCurrentHand); // give him hand status
		attend(); // let him process it
		switch(char action = currentPlayer->getAction()) {
			case 'e':
				goto foldCase;
				break;
			case 'c':
				calls++;
				table.push_back(currentPlayer); // player stays on
				currentPlayer->addStack(2 * (vpip[currentPlayer->getSeat()] - raises));
				mPot += 2 * (raises - vpip[currentPlayer->getSeat()]); // bets double on turn
				vpip[currentPlayer->getSeat()] = raises; // paid all so far
				mCurrentHand += 'c'; // current player called
				break;
			case 'f':
				foldCase:
					calls++;
					mCurrentHand += 'f';
					break;
			case 'r':
				if (raises > nMaxRaises){
					goto foldCase; // the player is folded automatically
				}
				else { // valid raise
					raises++;
					currentPlayer->addStack(2 * (vpip[currentPlayer->getSeat()] - raises));
					mPot += 2 * (raises - vpip[currentPlayer->getSeat()]);
					vpip[currentPlayer->getSeat()] = raises;
					table.push_back(currentPlayer); // player stays on
					mCurrentHand += 'r';
					calls = nBots - table.size() + 1; // this bet round stops once all remaining players minus the raiser have called this latest raise (unless there's new raise)
					break;
				}
		}
	}
}


void Casino::getRiverBets() {
	for (int j = 0; j < nBots; j++){
		for (int i = 0; i < table.size(); i++){ // get next to act front of queue
			Bot* frontPlayer = table.front();
			if (frontPlayer->getSeat() == ((mButton + j + 1) % nBots))
				goto tableOrdered;
			else {
				table.pop_front();
				table.push_back(frontPlayer);
			}
		}
	}
	tableOrdered:
	vector<int> vpip(nBots, 0); // track record of how many bets each bot already called
	int calls = 0;
	int raises = 0;
	mCurrentHand += "R"; // current hand is coded hand number + "R"
	//mCurrentHand += to_string(mDeck[2 * nBots + 4]) + "R";
	mCurrentHand += to_string(mDeck[2 * nBots + 4]) + "R";
	while (calls < nBots && table.size() > 1){
		Bot* currentPlayer = table.front(); // get first element
		table.pop_front(); // remove him
		currentPlayer->tellAction(mCurrentHand); // give him hand status
		attend(); // let him process it
		switch(char action = currentPlayer->getAction()) {
			case 'e':
				goto foldCase;
				break;
			case 'c':
				calls++;
				table.push_back(currentPlayer); // player stays on
				currentPlayer->addStack(2 * (vpip[currentPlayer->getSeat()] - raises));
				mPot += 2 * (raises - vpip[currentPlayer->getSeat()]); // bets double on turn
				vpip[currentPlayer->getSeat()] = raises; // paid all so far
				mCurrentHand += 'c'; // current player called
				break;
			case 'f':
				foldCase:
					calls++;
					mCurrentHand += 'f';
					break;
			case 'r':
				if (raises > nMaxRaises){
					goto foldCase; // the player is folded automatically
				}
				else { // valid raise
					raises++;
					currentPlayer->addStack(2 * (vpip[currentPlayer->getSeat()] - raises));
					mPot += 2* (raises - vpip[currentPlayer->getSeat()]);
					vpip[currentPlayer->getSeat()] = raises;
					table.push_back(currentPlayer); // player stays on
					mCurrentHand += 'r';
					calls = nBots - table.size() + 1; // this bet round stops once all remaining players minus the raiser have called this latest raise (unless there's new raise)
					break;
				}
		}
	}
}

void Casino::getWinners() {
	mWinners.clear();
	if (table.size() == 1){
		mWinners.push_back(table.front()->getSeat());
	}
	else {
		using namespace omp;
		HandEvaluator eval;
		int bestScore = 0;
		for (auto v : table) {
			Hand h = Hand::empty();
			h += Hand(mDeck[2 * nBots]) + Hand(mDeck[2 * nBots + 1]) + Hand(mDeck[2 * nBots + 2]) + Hand(mDeck[2 * nBots + 3]) + Hand(mDeck[2 * nBots + 4]) + Hand(get<0>(v->getHand())) + Hand(get<1>(v->getHand()));
			int score = eval.evaluate(h);
			if (score == bestScore) { // tie
				mWinners.push_back(v->getSeat());
			}
			else if (score > bestScore) { // new winner
				bestScore = score; // new best hand
				mWinners.clear(); // get rid of all false previous winners
				mWinners.push_back(v->getSeat());
			}
		}
	}
}

void Casino::payoffs() { // pay the winners
	for (auto w : mWinners){
		mPlayers[w]->addStack(mPot / mWinners.size());
	}
}
void Casino::fileHandSummary() {
	if (mCounter % nLogFrequency == 0) {
		ofstream fout("./botfiles/results", ios_base::app);
		if (!fout.good()) {
			cerr << "Error while opening results file "  << endl;
		}
		fout << mCurrentHand << endl;
		for (auto v : mPlayers){
			fout << "seat: " << v->mSeat << "  Stack: " << v->mStack  << endl;
		}
	}
}
void Casino::printHandSummary() {
	cout << endl << "hand number: " << mCounter << " button: " << mButton << " final pot: " << mPot << " winner seats: ";
	       for (auto p : mWinners)
	       		cout << p << " ";
	cout << endl;
	for (auto v : mPlayers){
		cout << "seat: " << v->mSeat << "  Hand: " << eRank[get<0>(v->mHand)] << " " << eRank[get<1>(v->mHand)] << "  Stack: " << v->mStack  << endl;
	}
	cout << " board: ";
	for (int i = 0; i  < 5; i++) {
		cout << eRank[mDeck[2 * nBots + i]] << " ";
	}
	cout << endl << "hand history :" << endl;
	cout << mCurrentHand << endl;
}


int main()
{
	// initialise rand, todo use mersenne for better randomness
	std::srand(std::time(nullptr)); 
	Casino lasVegas;
	lasVegas.populateTable();
			
	for (int r = 0; r < nRounds; r++){
		lasVegas.shuffleDeck();
		lasVegas.dealCards();
		lasVegas.getPreflopBets();
		if (!lasVegas.tableEmpty())
			lasVegas.getFlopBets();
		if (!lasVegas.tableEmpty())
			lasVegas.getTurnBets();
		if (!lasVegas.tableEmpty())
			lasVegas.getRiverBets();
		lasVegas.getWinners(); // get all winners
		lasVegas.payoffs(); // pay winner
		lasVegas.showdown(); // publish hands from all players
		lasVegas.tellHandSummary();
		lasVegas.printHandSummary();
		lasVegas.fileHandSummary();
		lasVegas.prepareNext(); // increase button, counter etc.
	}

}
