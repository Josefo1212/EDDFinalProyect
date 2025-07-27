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

template<>
void Tree<Mage>::assignNewOwner() {
    // 1. Buscar el nodo actual dueño
    Node<Mage>* owner = nullptr;
    struct Finder {
        static Node<Mage>* findOwner(Node<Mage>* node) {
            if (!node) return nullptr;
            if (node->data.is_owner) return node;
            Node<Mage>* res = findOwner(node->left);
            if (res) return res;
            return findOwner(node->right);
        }
    };
    owner = Finder::findOwner(root);
    if (!owner) return;

    // 2. Verificar si está muerto o >70 años
    if (!owner->data.is_dead && owner->data.age <= 70) return;

    // 3. Buscar nuevo dueño según reglas
    Node<Mage>* candidate = nullptr;

    // Primer hijo vivo con magia elemental o unique
    if (owner->left && !owner->left->data.is_dead &&
        (owner->left->data.type_magic == "elemental" || owner->left->data.type_magic == "unique")) {
        candidate = owner->left;
    } else if (owner->right && !owner->right->data.is_dead &&
        (owner->right->data.type_magic == "elemental" || owner->right->data.type_magic == "unique")) {
        candidate = owner->right;
    }

    // Si no, primer hijo vivo con magia mixed
    if (!candidate) {
        if (owner->left && !owner->left->data.is_dead && owner->left->data.type_magic == "mixed")
            candidate = owner->left;
        else if (owner->right && !owner->right->data.is_dead && owner->right->data.type_magic == "mixed")
            candidate = owner->right;
    }

    // Si no, primer hijo hombre vivo
    if (!candidate) {
        if (owner->left && !owner->left->data.is_dead && owner->left->data.gender == 'H')
            candidate = owner->left;
        else if (owner->right && !owner->right->data.is_dead && owner->right->data.gender == 'H')
            candidate = owner->right;
    }

    // Si no tiene hijos vivos, buscar compañero discípulo (hermano) vivo y que comparta magia
    if (!candidate) {
        // Buscar el padre del dueño actual
        Node<Mage>* father = findById(owner->data.id_father);
        if (father) {
            // Buscar hermano (compañero discípulo)
            if (father->left && father->left != owner && !father->left->data.is_dead &&
                father->left->data.type_magic == owner->data.type_magic)
                candidate = father->left;
            else if (father->right && father->right != owner && !father->right->data.is_dead &&
                father->right->data.type_magic == owner->data.type_magic)
                candidate = father->right;
        }
    }


    // Si no buscar compañero de su maestro (tío) independientemente de la magia
    if (!candidate) {
        Node<Mage>* father = findById(owner->data.id_father);
        if (father) {
            Node<Mage>* grandfather = findById(father->data.id_father);
            if (grandfather) {
                if (grandfather->left && grandfather->left != father && !grandfather->left->data.is_dead)
                    candidate = grandfather->left;
                else if (grandfather->right && grandfather->right != father && !grandfather->right->data.is_dead)
                    candidate = grandfather->right;
            }
        }
    }

    // Si no buscar la mujer más joven con discípulos viva y magia mixed
    if (!candidate) {
    Node<Mage>* best = nullptr;
    struct {
        static void search(Node<Mage>* node, Node<Mage>*& best) {
            if (!node) return;
            bool hasChildren = node->left || node->right;
            if (!node->data.is_dead && node->data.gender == 'M' && hasChildren && node->data.type_magic == "mixed") {
                if (!best || node->data.age < best->data.age)
                    best = node;
            }
            search(node->left, best);
            search(node->right, best);
        }
    } searcher;
    searcher.search(root, best);
    if (best) candidate = best;
}


    // si no buscar a la mujer más joven viva
if (!candidate) {
    Node<Mage>* best = nullptr;
    struct {
        static void search(Node<Mage>* node, Node<Mage>*& best) {
            if (!node) return;
            if (!node->data.is_dead && node->data.gender == 'M') {
                if (!best || node->data.age < best->data.age)
                    best = node;
            }
            search(node->left, best);
            search(node->right, best);
        }
    } searcher;
    searcher.search(root, best);
    if (best) candidate = best;

   
   
    // Asignar nuevo dueño 
    if (candidate) {
        owner->data.is_owner = false;
        candidate->data.is_owner = true;
        cout << "Nuevo dueño del hechizo: " << candidate->data.name << " " << candidate->data.last_name << endl;
    } else {
        cout << "No se encontró un nuevo dueño para el hechizo.\n";
    }
}