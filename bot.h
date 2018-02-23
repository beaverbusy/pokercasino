#pragma once
#include "includes.h"
#include "casino.h"
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
