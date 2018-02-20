#include<chrono> 
#include<thread>
#include<fstream> // smart files
#include<iostream>
 
using namespace std;
int ecriture = 0;
int erreur = 0;
int bound = 1000;

void attend() { // time given to bots for each decision
	std::this_thread::sleep_for(100ms);
}

void ecrire () {
	ofstream fout("ecrit", ios_base::trunc);
	if (!fout.good()) {
		erreur++;
	}
	else {
		fout << ecriture;
		ecriture++;
	}
}
	


int main()
{

	std::this_thread::sleep_for(2s);
	cout << "starting" << endl;
	for (int i = 0; i < bound; i++) {
		ecrire();
		attend();
	}
	cout << "ecriture: " << ecriture << " erreur d'ouverture fichier: " << erreur << endl;
}

