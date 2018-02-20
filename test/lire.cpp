#include<chrono> 
#include<thread>
#include<fstream> // smart files
#include<iostream>
 
using namespace std;
int lastRead = 0;
int lecture = 0;
int erreur = 0;
int bound = 11000;

void attend() { // time given to bots for each decision
	std::this_thread::sleep_for(10ms);
}

void lire () {
	ifstream fin("ecrit");
	if (!fin) {
		erreur++;
	}
	else {
		int temp;
		fin >> temp;
		if (temp != lastRead){
			lastRead = temp;
			lecture++;
		}
	}
}
	


int main()
{
	for (int i = 0; i < bound; i++) {
		lire();
		attend();
	}
	cout << "lecture: " << lecture << " erreur d'ouverture fichier: " << erreur << endl;
}

