#include "./arbol.h"
using namespace std;

// Estructura auxiliar para almacenar magos y relaciones padre-hijo
struct MageArray {
    Mage* data;
    int* ids;
    ChildList* children;
    int size;
    int capacity;
    MageArray() : data(nullptr), ids(nullptr), children(nullptr), size(0), capacity(0) {}
    ~MageArray() { delete[] data; delete[] ids; delete[] children; }
    void push_back(const Mage& m) {
        if (size == capacity) {
            int newcap = capacity == 0 ? 8 : capacity * 2;
            Mage* nd = new Mage[newcap];
            int* nids = new int[newcap];
            ChildList* nch = new ChildList[newcap];
            for (int i = 0; i < size; ++i) {
                nd[i] = data[i];
                nids[i] = ids[i];
                nch[i] = children[i];
            }
            delete[] data; delete[] ids; delete[] children;
            data = nd; ids = nids; children = nch;
            capacity = newcap;
        }
        data[size] = m;
        ids[size] = m.id;
        children[size] = ChildList();
        size++;
    }
    int findIndexById(int id) {
        for (int i = 0; i < size; ++i) if (ids[i] == id) return i;
        return -1;
    }
};

// Implementación para cargar desde CSV y construir el árbol
template<>
void Tree<Mage>::loadFromCSV(const char* filename) {
    ifstream file(filename);
    string line;
    MageArray mages;
    bool first = true;
    while (getline(file, line)) {
        if (first) { first = false; continue; }
        stringstream ss(line);
        string tokens[9];
        for (int i = 0; i < 9; ++i) {
            if (!getline(ss, tokens[i], ',')) tokens[i] = "";
        }
        int id = atoi(tokens[0].c_str());
        string name = tokens[1];
        string last_name = tokens[2];
        char gender = tokens[3].empty() ? 'H' : tokens[3][0];
        int age = atoi(tokens[4].c_str());
        int id_father = atoi(tokens[5].c_str());
        bool is_dead = tokens[6] == "1";
        string type_magic = tokens[7];
        bool is_owner = tokens[8] == "1";
        Mage mage(id, name, last_name, gender, age, id_father, is_dead, type_magic, is_owner);
        mages.push_back(mage);
    }
    // Construir relaciones padre-hijo
    for (int i = 0; i < mages.size; ++i) {
        int id_father = mages.data[i].id_father;
        if (id_father != 0) {
            int idx = mages.findIndexById(id_father);
            if (idx != -1) mages.children[idx].add(mages.data[i].id);
        }
    }
    // Encontrar raíz (el mago sin padre)
    int root_idx = -1;
    for (int i = 0; i < mages.size; ++i) {
        if (mages.data[i].id_father == 0) {
            root_idx = i;
            break;
        }
    }
    if (root_idx == -1) return;

    struct Builder {
        static Node<Mage>* build(int idx, MageArray& mages) {
            Node<Mage>* node = new Node<Mage>(mages.data[idx]);
            for (int c = 0; c < mages.children[idx].count; ++c) {
                int child_id = mages.children[idx].ids[c];
                int child_idx = mages.findIndexById(child_id);
                if (c == 0) node->setLeft(build(child_idx, mages));
                else if (c == 1) node->setRight(build(child_idx, mages));
            }
            return node;
        }
    };
    root = Builder::build(root_idx, mages);
}

template<>
Node<Mage>* Tree<Mage>::findById(int id) {
    struct Finder {
        static Node<Mage>* find(Node<Mage>* node, int id) {
            if (!node) return nullptr;
            if (node->data.id == id) return node;
            Node<Mage>* res = find(node->left, id);
            if (res) return res;
            return find(node->right, id);
        }
    };
    return Finder::find(root, id);
}

template<>
void Tree<Mage>::showSuccessionLine() {
    struct Printer {
        static void printAlive(Node<Mage>* node) {
            if (!node) return;
            if (!node->data.is_dead)
                cout << node->data.name << " " << node->data.last_name << endl;
            printAlive(node->left);
            printAlive(node->right);
        }
    };
    Printer::printAlive(root);
}

template<>
void Tree<Mage>::changeMageData(int id, const Mage& newData) {
    Node<Mage>* node = findById(id);
    if (!node) return;
    node->data.name = newData.name;
    node->data.last_name = newData.last_name;
    node->data.gender = newData.gender;
    node->data.age = newData.age;
    node->data.is_dead = newData.is_dead;
    node->data.type_magic = newData.type_magic;
    node->data.is_owner = newData.is_owner;
    node->data.spells = newData.spells;
}

template<>
void Tree<Mage>::showMageSpells(int id) {
    Node<Mage>* node = findById(id);
    if (!node) return;
    cout << "Hechizos de " << node->data.name << ":\n";
    for (int i = 0; i < node->data.spells.size; ++i)
        cout << "- " << node->data.spells[i] << endl;
}