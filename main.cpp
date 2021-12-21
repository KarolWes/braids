#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace std;

typedef vector < vector < pair <int, int> > > braid;

void print(braid *braid){
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

braid *read_data(string path)
{
    fstream f;
    f.open(path, ios::in);
    int n, h;
    f >> n >> h;
    auto *braid = new ::braid;
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

braid *generate(int n, int h){
    auto *braid = new ::braid;
    auto *level= new vector < pair <int, int> >;
    for(int i = 0; i < n; i++){
        level->push_back(make_pair(i, 0));
    }
    braid->push_back(*level);
    level->clear();
    for(int i = 1; i < h; i++){
        copy(braid->at(i-1).begin(),  braid->at(i-1).end(), back_inserter(*level));
        level->at(n-1).second = 0;
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
bool untangle(braid *braid){
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


// Checks whether the braid doesn't consist of distinct sub-braids
// Inconsistent braids have columns between threads without a knot
// The braid must be unentangled before this operation
bool is_consistent(braid *braid){
    for(int col = 0; col < braid->at(0).size() - 1; col++){
        bool knot_found = false;

        int prev1 = braid->at(0).at(col).first;
        int prev2 = braid->at(0).at(col+1).first;
        for(int layer = 1; layer < braid->size(); layer++){
            int curr1 = braid->at(layer).at(col).first;
            int curr2 = braid->at(layer).at(col+1).first;

            // Found a knot between col and col+1
            if(curr1 == prev2 && curr2 == prev1){
                knot_found = true;
                break;
            }

            prev1 = curr1;
            prev2 = curr2;
        }

        if(!knot_found) return false;
    }
    return true;
}

int visits_all(braid *b)
{
    int res = 0;
    vector <vector <bool>> visited;
    int n = b->at(0).size();
    int h = b->size();
    for(int i = 0; i < n; i ++){
        vector <bool> tmp;
        for(int j = 0; j < n; j++){
            if(i==j){
                tmp.push_back(true);
            }
            else {
                tmp.push_back(false);
            }
        }
        visited.push_back(tmp);
    }
    for(int i = 0; i < n; i++) {
        int test = 1;
        int id = 1;
        while (test < n && id < h) {
            int act = b->at(id).at(i).first;
            if (!visited[i][act]) {
                test++;
                visited[i][act] = true;
            }
            id++;
        }
        if (test != n) {
            res += n - test;
        }
    }
    return res;
}
int quantity(braid *b){
    int q = 0;
    int n = b->at(0).size();
    q+= n*n-visits_all(b);
    q+=2*n*n*is_consistent(b);
    return q;
}

// Swaps the knot (i.e. the upper thread will go at the bottom)
void swap_threads_in_knot(braid *b, int layer, int col){
    b->at(layer).at(col).second *= -1;
    b->at(layer).at(col+1).second *= -1;
}

// Moves the knot one layer up
void move_knot_up(braid *b, int layer, int col){
    if(layer <= 1) return;
    // The layer above the knot must have straight threads
    if(b->at(layer-1).at(col).second != 0) return;
    if(b->at(layer-1).at(col+1).second != 0) return;

    // Copy the knot
    b->at(layer-1).at(col).second = b->at(layer).at(col).second;
    b->at(layer-1).at(col+1).second = b->at(layer).at(col+1).second;

    // Delete the original knot
    b->at(layer).at(col).second = b->at(layer).at(col+1).second = 0;

    // Swap the threads
    swap(
            b->at(layer-1).at(col).first,
            b->at(layer-1).at(col+1).first);
}

// Moves the knot one layer down
void move_knot_down(braid *b, int layer, int col){
    if(layer >= b->size() - 2) return;
    // The layer below the knot must have straight threads
    if(b->at(layer+1).at(col).second != 0) return;
    if(b->at(layer+1).at(col+1).second != 0) return;

    // Copy the knot
    b->at(layer+1).at(col).second = b->at(layer).at(col).second;
    b->at(layer+1).at(col+1).second = b->at(layer).at(col+1).second;

    // Delete the original knot
    b->at(layer).at(col).second = b->at(layer).at(col+1).second = 0;

    // Swap the threads
    swap(
            b->at(layer).at(col).first,
            b->at(layer).at(col+1).first);
}

// Makes a knot in the specified layer and in the next one
void make_knots(braid *b, int layer, int col){
    if(layer >= b->size() - 2) return;
    // The threads must be straight for two layers
    if(b->at(layer).at(col).second != 0) return;
    if(b->at(layer).at(col+1).second != 0) return;
    if(b->at(layer+1).at(col).second != 0) return;
    if(b->at(layer+1).at(col+1).second != 0) return;

    // Mark the knots
    b->at(layer).at(col).second = 1;
    b->at(layer).at(col+1).second = -1;
    b->at(layer+1).at(col).second = 1;
    b->at(layer+1).at(col+1).second = -1;

    // Swap the threads
    swap(
            b->at(layer).at(col).first,
            b->at(layer).at(col+1).first);
}


// Removes the knot at the specified layer, between col and col+1
void remove_knot(braid *b, int layer, int col){
    if(layer <= 0) return;

    // The intersecting threads
    int thr1 = b->at(layer).at(col).first;
    int thr2 = b->at(layer).at(col+1).first;

    // Check if the knot exists
    if(thr1 != b->at(layer-1).at(col+1).first) return;
    if(thr2 != b->at(layer-1).at(col).first) return;

    // Unset the "knot" mark
    b->at(layer).at(col).second = 0;
    b->at(layer).at(col+1).second = 0;

    // Swap the threads 1 & 2 below the layer
    for(int l = layer; l < b->size(); l++){
        for(int c = 0; c < b->at(0).size(); c++){
            auto &spot = b->at(l).at(c);
            if(spot.first == thr1){
                spot.first = thr2;
            }else if(spot.first == thr2){
                spot.first = thr1;
            }
        }
    }
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
    cout << is_consistent(braid) << endl;
    cout << visits_all(braid) << endl;
    cout << quantity(braid) << endl;
    braid->clear();

    return 0;
}
