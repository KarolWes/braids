#include <iostream>
#include <random>
#include <vector>
#include <utility>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <string>
#include <chrono>

#define TABU_SIZE 20
#define GEN_COUNT 100000
#define MIN(x) x*60000


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

braid *generatePlain(int n, int h){
    auto *braid = new ::braid;
    auto *level= new vector < pair <int, int> >;
    for(int j = 0; j < h; j++){
        for(int i = 0; i < n; i++){
            level->push_back(make_pair(i, 0));
        }
        braid->push_back(*level);
        level->clear();
    }
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

int knotCounter(braid *b){
    int knots = 0;
    for(auto & i : *b){
        for(auto & j : i){
            if(j.second != 0){
                knots++;
            }
        }
    }
    return knots/2;
}
int quantity(braid *b){ //max quantity = 3n^2+n
    int q = 0;
    int n = b->at(0).size();
    q+= n*n-visits_all(b);
    q+=2*n*n*is_consistent(b);
    q+= knotCounter(b)/b->size();
    return q;
}
bool detectKnot(braid *b, int layer, int col){
    bool res = false;
    if(b->at(layer).at(col).second != 0 && b->at(layer).at(col+1).second != 0){
        if (b->at(layer-1).at(col).first == b->at(layer).at(col+1).first && b->at(layer).at(col+1).first == b->at(layer-1).at(col).first ) {
            res = true;
        }
    }
    return res;
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

    if(detectKnot(b,layer,col)){
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

}

// Moves the knot one layer down
void move_knot_down(braid *b, int layer, int col){
    if(layer >= b->size() - 2) return;
    // The layer below the knot must have straight threads
    if(b->at(layer+1).at(col).second != 0) return;
    if(b->at(layer+1).at(col+1).second != 0) return;
    //how to detect a knot???
    if (detectKnot(b, layer, col)){
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

}

// Makes a knot in the specified layer and in the next one
void make_knots(braid *b, int layer, int col){
    if(layer > b->size() - 2) return;
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
template <typename T>
T* removeFirst(T* list){
    auto newList = new T;
    for(int i = 1; i < list->size(); i++){
        newList->push_back(list->at(i));
    }
    return newList;
}
braid *deepCopy(braid *b){
    braid *cp = new braid;
    for(auto el: *b){
        cp->push_back(el);
    }
    return cp;
}

vector <pair < braid* , int > >* generateUnstableNeighbours(braid *b, int changes, int size_of_neigh){
    auto neighbourhood = new vector <pair < braid* , int > >;
    for(int i = 0; i < size_of_neigh; i++){
        braid *tmp = deepCopy(b);
        for(int j = 0; j < changes; j++){
            switch(rand()%5){
                case 0:
                    swap_threads_in_knot(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
                    break;
                case 1:
                    move_knot_up(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
                    break;
                case 2:
                    move_knot_down(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
                    break;
                case 3:case 4:
                    make_knots(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
                    break;
                default:
                    cout << "Something went wrong\n";
                    return nullptr;
            }
        }
        while(untangle(tmp));
        neighbourhood->push_back(make_pair(tmp, quantity(tmp)));
    }
    return neighbourhood;
}

vector <pair < braid* , int > >* generateNeighbours(braid *b){
    auto neighbourhood = new vector <pair < braid* , int > >;
    braid *tmp = deepCopy(b);
    swap_threads_in_knot(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
    while(untangle(tmp));
    neighbourhood->push_back(make_pair(tmp, quantity(tmp)));
    tmp = deepCopy(b);
    move_knot_up(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
    while(untangle(tmp));
    neighbourhood->push_back(make_pair(tmp, quantity(tmp)));
    tmp = deepCopy(b);
    move_knot_down(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
    while(untangle(tmp));
    neighbourhood->push_back(make_pair(tmp, quantity(tmp)));
    tmp = deepCopy(b);
    make_knots(tmp, rand()%(tmp->size()-1)+1, rand()%(tmp->at(0).size()-1));
    while(untangle(tmp));
    neighbourhood->push_back(make_pair(tmp, quantity(tmp)));
    return neighbourhood;
}

bool find(vector <pair < braid* , int > >* list, braid* target){
    for(auto el: *list){
        if(equal(el.first->begin(), el.first->end(), target->begin())){
            return true;
        }
    }
    return false;
}
vector <double>* braidLength(braid *b){
    int n = b->at(0).size();
    int h = b->size();
    auto len = new vector <double>(n);
    for(int i = 0; i < h; i++){
        for(int j = 0; j < n; j ++){
            int address = b->at(i).at(j).first;
            if(b->at(i).at(j).second == 0){
                len->at(address) +=1;
            }
            else{
                len->at(address)+=1.41;
            }
        }
    }
    return len;
}

template <
        class result_t   = std::chrono::milliseconds,
        class clock_t    = std::chrono::steady_clock,
        class duration_t = std::chrono::milliseconds
>
auto since(std::chrono::time_point<clock_t, duration_t> const& start){
    return std::chrono::duration_cast<result_t>(clock_t::now() - start);
} // from Stack Overflow https://stackoverflow.com/a/21995693


void tabuSearch(braid *b){
    int n = b->at(0).size();
    pair < braid* , int > candidate = make_pair(b, quantity(b));
    auto tabuList = new vector <pair < braid* , int > >;
    auto neighborhood = new vector <pair < braid* , int > >;
    tabuList->push_back(make_pair(deepCopy(candidate.first), candidate.second));
    int gen = 0;
    int max_quantity = 3*n*n+n;
    auto start = chrono::steady_clock::now();
    while(candidate.second < max_quantity  && since(start).count() < MIN(5)){//&& gen < GEN_COUNT
        gen++;
        neighborhood->clear();
        neighborhood = generateUnstableNeighbours(candidate.first, 3,6);
        shuffle(neighborhood->begin(),  neighborhood->end(), std::mt19937(std::random_device()()));
        for(auto neighbour: *neighborhood){
            if(!find(tabuList, neighbour.first)){
                if(neighbour.second >= candidate.second){
                    candidate = make_pair(deepCopy(neighbour.first), neighbour.second);
                }
            }
        }
        tabuList->push_back(make_pair(deepCopy(candidate.first), candidate.second));
        if(tabuList->size() > TABU_SIZE){
            tabuList = removeFirst(tabuList);
        }

        cout <<gen << ": " << candidate.second << "\t";

    }
    cout << endl;
    print(candidate.first);
    cout << endl << "Lengths of strings in braid: \n";
    auto len = braidLength(candidate.first);
    for(double el: *len){
        cout << el << "\t";
    }
    double diff = *max_element(len->begin(),  len->end()) - *min_element(len->begin(),  len->end());
    cout << endl << "Difference between loose strings: "<< diff << endl;
}



int main() {
    srand(time(NULL));
    cout << "Welcome to braid generator" << endl;
    int n = 10;
    int h = 30;
    //auto braid = generate(n, h);
    //auto braid = read_data("test.txt");
    auto braid = generatePlain(n, h);
    print(braid);
    cout << "________\n";
    while(untangle(braid));
    print(braid);
    cout << is_consistent(braid) << endl;
    cout << visits_all(braid) << endl;
    cout << quantity(braid) << endl;
    cout << "________\n";
    tabuSearch(braid);

    braid->clear();

    return 0;
}
