#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace std;


void print(vector < vector < pair <int, int> > > *braid){
    for(int i = 0; i < braid->size(); i++){
        auto &layer = braid->at(i);
        for(int j = 0; j < layer.size(); j++){
            auto el = layer.at(j);
            if(el.second == 0){
                cout << "| ";
                continue;
            }
            if(j == layer.size() - 1) continue;
            if(el.first != braid->at(i-1).at(j+1).first) continue;

            cout << (el.second == 1 ? " /  " : " \\  ");
        }
        cout << "\n";
        for(int j = 0; j < layer.size(); j++) cout << "| ";
        cout << "\n";
    }
}

vector < vector < pair <int, int> > > *read_data(string path)
{
    fstream f;
    f.open(path, ios::in);
    int n, h;
    f >> n >> h;
    auto *braid = new vector < vector < pair <int, int> > >;
    auto *level= new vector < pair <int, int> >;
    for(int i = 0; i < h; i++){
        for(int j = 0; j < n; j++){
            int pos, top;
            f >> pos >> top;
            level->push_back(make_pair(pos, top));
        }
        braid->push_back(*level);
        level->clear();
    }

    f.close();
    return braid;
}

vector < vector < pair <int, int> > > *generate(int n, int h){
    auto *braid = new vector < vector < pair <int, int> > >;
    auto *level= new vector < pair <int, int> >;
    for(int i = 0; i < n; i++){
        level->push_back(make_pair(i, 0));
    }
    braid->push_back(*level);
    level->clear();
    for(int i = 1; i < h; i++){
        copy(braid->at(i-1).begin(),  braid->at(i-1).end(), back_inserter(*level));
        for(int j = 0; j < n-1; j++){
            int a = rand()%100;
            if(a < 50){
                swap(level->at(j), level->at(j+1));
                if(a < 25){
                    level->at(j).second = 1;
                    level->at(j+1).second = -1;
                }
                else{
                    level->at(j).second = -1;
                    level->at(j+1).second = 1;
                }
                j+=1;
            }
            else{
                level->at(j).second = 0;
            }
        }
        braid->push_back(*level);
        level->clear();
    }
    return braid;
}

// Tries to unentangle the braid; returns false if nothing was changed
bool untangle(vector < vector < pair <int, int> > > *braid){
    // Will remain false if there are no untanglements possible
    bool changed_anything = false;

    // col - number of a column that's being checked
    for(int col = 0; col < braid->at(0).size() - 1; col++){
        int last1, last2, last_knot, last_knot_layer;
        last1 = braid->at(0).at(col).first;
        last2 = braid->at(0).at(col+1).first;
        last_knot = 0;
        last_knot_layer = 0;

        // Scan through the layers of the braid
        for(int layer = 0; layer < braid->size(); layer++){
            int curr1, curr2;
            curr1 = braid->at(layer).at(col).first;
            curr2 = braid->at(layer).at(col+1).first;

            // Nothing has changed so go to the next layer
            if(last1 == curr1 && last2 == curr2) continue;

            // There was a different change than exchange
            // between threads 1 and 2
            if(last1 != curr2 || last2 != curr1) {
                last1 = curr1;
                last2 = curr2;
                last_knot = 0;
                last_knot_layer = 0;
                continue;
            }

            // Here is the exchange between threads 1 and 2
            int knot = braid->at(layer).at(col).second;
            if(knot * last_knot == -1){
                // Current and last knots are opposites
                // Therefore we can swap the threads
                for(int i = last_knot_layer; i < layer; i++){
                    braid->at(i).at(col).first = curr1;
                    braid->at(i).at(col+1).first = curr2;
                }

                // Mark the former knots as straight lines
                braid->at(last_knot_layer).at(col).second = 0;
                braid->at(last_knot_layer).at(col+1).second = 0;
                braid->at(layer).at(col).second = 0;
                braid->at(layer).at(col+1).second = 0;

                last1 = last2 = -1;
                last_knot = 0;
                last_knot_layer = 0;

                changed_anything = true;
            }else{
                // Alas, the last two knots don't cancel each other
                last1 = curr1;
                last2 = curr2;
                last_knot = knot;
                last_knot_layer = layer;
            }
        }
    }

    return changed_anything;
}


int main() {
    srand(time(NULL));
    cout << "Welcome to braid generator" << endl;
    int n = 3;
    int h = 10;
    //auto braid = generate(n, h);
    auto braid = read_data("test.txt");
    print(braid);
    cout << "________\n";
    while(untangle(braid));
    print(braid);

    return 0;
}
