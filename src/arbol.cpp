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
    if (!file.is_open()) {
        cout << "No se pudo abrir el archivo: " << filename << endl;
        return;
    }
    string line;
    MageArray mages;
    bool first = true;
    int rowCount = 0;
    while (getline(file, line)) {
        if (first) { first = false; continue; }
        stringstream ss(line);
        string tokens[9];
        for (int i = 0; i < 9; ++i) {
            if (!getline(ss, tokens[i], ',')) tokens[i] = "";
        }
        int id = atoi(tokens[0].c_str());
        string first_name = tokens[1];
        string last_name = tokens[2];
        char gender = tokens[3].empty() ? 'H' : tokens[3][0];
        int age = atoi(tokens[4].c_str());
        int id_father = atoi(tokens[5].c_str());
        bool is_dead = tokens[6] == "1";
        string type_magic = tokens[7];
        bool is_owner = tokens[8] == "1";
        Mage mage(id, first_name, last_name, gender, age, id_father, is_dead, type_magic, is_owner);
        mages.push_back(mage);
        rowCount++;
    }
    cout << "Magos cargados desde CSV: " << rowCount << endl;
    // Construir relaciones padre-hijo
    for (int i = 0; i < mages.size; ++i) {
        int id_father = mages.data[i].id_father;
        if (id_father != 0) {
            int idx = mages.findIndexById(id_father);
            if (idx != -1) mages.children[idx].add(mages.data[i].id);
        }
    }
    // Encontrar raiz (el mago sin padre)
    int root_idx = -1;
    for (int i = 0; i < mages.size; ++i) {
        if (mages.data[i].id_father == 0) {
            root_idx = i;
            break;
        }
    }
    if (root_idx == -1) {
        cout << "No se encontro la raiz (mago sin padre) en el archivo CSV.\n";
        return;
    }
    cout << "Raiz encontrada: " << mages.data[root_idx].first_name << " " << mages.data[root_idx].last_name << endl;

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
                cout << node->data.first_name << " " << node->data.last_name << endl;
            printAlive(node->left);
            printAlive(node->right);
        }
    };
    Printer::printAlive(root);
}

template<>
void Tree<Mage>::showMageRelations(int id) {
    Node<Mage>* node = findById(id);
    if (!node) {
        cout << "Mago no encontrado.\n";
        return;
    }
    // Mostrar maestros (ascendencia)
    cout << "Maestros de " << node->data.first_name << " " << node->data.last_name << ":\n";
    Node<Mage>* curr = node;
    while (curr && curr->data.id_father != 0) {
        Node<Mage>* father = findById(curr->data.id_father);
        if (father) {
            cout << "- " << father->data.first_name << " " << father->data.last_name << endl;
            curr = father;
        } else break;
    }
    // Mostrar discipulos (descendencia directa)
    cout << "Discipulos de " << node->data.first_name << " " << node->data.last_name << ":\n";
    if (node->left)
        cout << "- " << node->left->data.first_name << " " << node->left->data.last_name << endl;
    if (node->right)
        cout << "- " << node->right->data.first_name << " " << node->right->data.last_name << endl;
}

template<>
void Tree<Mage>::showMainBranch() {
    // Encuentra la rama con más descendientes desde la raíz
    struct Counter {
        static int count(Node<Mage>* node) {
            if (!node) return 0;
            return 1 + count(node->left) + count(node->right);
        }
        static void printBranch(Node<Mage>* node) {
            if (!node) return;
            cout << node->data.first_name << " " << node->data.last_name << endl;
            if (count(node->left) >= count(node->right))
                printBranch(node->left);
            else
                printBranch(node->right);
        }
    };
    cout << "Rama principal:\n";
    Counter::printBranch(root);
}

template<>
void Tree<Mage>::addSpellToMage(int id, const string& spell) {
    Node<Mage>* node = findById(id);
    if (!node) {
        cout << "Mago no encontrado.\n";
        return;
    }
    node->data.addSpell(spell);
    cout << "Hechizo agregado a " << node->data.first_name << ".\n";
}

template<>
void Tree<Mage>::changeMageData(int id, const Mage& newData) {
    Node<Mage>* node = findById(id);
    if (!node) return;
    // Solo cambia datos permitidos (no id ni id_father)
    node->data.first_name = newData.first_name;
    node->data.last_name = newData.last_name;
    node->data.gender = newData.gender;
    node->data.age = newData.age;
    node->data.is_dead = newData.is_dead;
    node->data.type_magic = newData.type_magic;
    node->data.is_owner = newData.is_owner;
    node->data.spells = newData.spells;
}

template<>
void Tree<Mage>::assignNewOwner() {
    // Buscar el nodo que actualmente posee el hechizo
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
    if (!owner) {
        cout << "No se encontro dueno actual del hechizo.\n";
        return;
    }

    // Verificar si el dueno actual sigue vivo y tiene 70 años o menos
    if (!owner->data.is_dead && owner->data.age <= 70) {
        cout << "No se requiere reasignacion. El dueno actual (" << owner->data.first_name << " " << owner->data.last_name << ") sigue siendo el dueno.\n";
        return;
    }

    Node<Mage>* candidate = nullptr;

    // Si el dueño tiene más de 70 años y tiene discípulos, buscar primero discípulo vivo con magia igual
    if (owner->data.age > 70 && (owner->left || owner->right)) {
        if (owner->left && !owner->left->data.is_dead && owner->left->data.type_magic == owner->data.type_magic)
            candidate = owner->left;
        else if (owner->right && !owner->right->data.is_dead && owner->right->data.type_magic == owner->data.type_magic)
            candidate = owner->right;
        else {
            // Si no hay magia igual, elegir al discípulo vivo de mayor edad
            if (owner->left && !owner->left->data.is_dead)
                candidate = owner->left;
            if (owner->right && !owner->right->data.is_dead && (!candidate || owner->right->data.age > candidate->data.age))
                candidate = owner->right;
        }
    }

    // Si el dueño ha muerto y tiene discípulos, buscar por tipo de magia priorizando elemental y unique
    if (!candidate && (owner->left || owner->right)) {
        // Buscar discípulo vivo con magia elemental o unique
        if (owner->left && !owner->left->data.is_dead &&
            (owner->left->data.type_magic == "elemental" || owner->left->data.type_magic == "unique"))
            candidate = owner->left;
        else if (owner->right && !owner->right->data.is_dead &&
                 (owner->right->data.type_magic == "elemental" || owner->right->data.type_magic == "unique"))
            candidate = owner->right;

        // Si no hay elemental o unique, buscar magia mixta
        if (!candidate) {
            if (owner->left && !owner->left->data.is_dead && owner->left->data.type_magic == "mixed")
                candidate = owner->left;
            else if (owner->right && !owner->right->data.is_dead && owner->right->data.type_magic == "mixed")
                candidate = owner->right;
        }

        // Si no hay magia válida, buscar primer discípulo hombre vivo
        if (!candidate) {
            if (owner->left && !owner->left->data.is_dead && owner->left->data.gender == 'H')
                candidate = owner->left;
            else if (owner->right && !owner->right->data.is_dead && owner->right->data.gender == 'H')
                candidate = owner->right;
        }
    }

    // Si el dueño no tiene hijos, buscar compañero discípulo con magia igual
    if (!candidate && !owner->left && !owner->right) {
        Node<Mage>* father = findById(owner->data.id_father);
        if (father) {
            // Buscar compañero vivo con magia igual
            if (father->left && father->left != owner && !father->left->data.is_dead &&
                father->left->data.type_magic == owner->data.type_magic)
                candidate = father->left;
            else if (father->right && father->right != owner && !father->right->data.is_dead &&
                     father->right->data.type_magic == owner->data.type_magic)
                candidate = father->right;

            // Si no hay coincidencia directa, buscar hijos del compañero
            if (!candidate) {
                Node<Mage>* companion = (father->left == owner) ? father->right : father->left;
                if (companion && !companion->data.is_dead) {
                    // Buscar primer hijo vivo del compañero con magia elemental o unique
                    if (companion->left && !companion->left->data.is_dead &&
                        (companion->left->data.type_magic == "elemental" || companion->left->data.type_magic == "unique"))
                        candidate = companion->left;
                    else if (companion->right && !companion->right->data.is_dead &&
                             (companion->right->data.type_magic == "elemental" || companion->right->data.type_magic == "unique"))
                        candidate = companion->right;
                }

                // Si el compañero está muerto y no tiene hijos, buscar compañero del maestro
                if (!candidate && (!companion || companion->data.is_dead)) {
                    Node<Mage>* grandfather = findById(father->data.id_father);
                    if (grandfather) {
                        if (grandfather->left && grandfather->left != father && !grandfather->left->data.is_dead)
                            candidate = grandfather->left;
                        else if (grandfather->right && grandfather->right != father && !grandfather->right->data.is_dead)
                            candidate = grandfather->right;
                    }
                }
            }
        }
    }

    // Si ningún caso se cumple, buscar mujer viva más joven con discípulos cuyo maestro haya sido dueño y con magia mixta
    if (!candidate) {
        Node<Mage>* best = nullptr;
        struct {
            static void search(Node<Mage>* node, Node<Mage>*& best) {
                if (!node) return;
                bool hasChildren = node->left || node->right;
                if (!node->data.is_dead && node->data.gender == 'M' &&
                    hasChildren && node->data.type_magic == "mixed") {
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

    // Si todo lo anterior falla, buscar mujer viva más joven en todo el árbol
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
    }

    // Asignar nuevo dueño si se encontró candidato válido
    if (candidate) {
        string prev_owner = owner->data.first_name + " " + owner->data.last_name;
        string new_owner = candidate->data.first_name + " " + candidate->data.last_name;
        owner->data.is_owner = false;
        candidate->data.is_owner = true;
        cout << "Cambio realizado: " << prev_owner << " deja de ser dueno.\n";
        cout << "Nuevo dueno del hechizo: " << new_owner << endl;
    } else {
        cout << "No se encontro un nuevo dueno para el hechizo. No se realizo ningun cambio.\n";
    }
}


template<>
void Tree<Mage>::showMageSpells(int id) {
    Node<Mage>* node = findById(id);
    if (!node) {
        cout << "Mago no encontrado.\n";
        return;
    }
    cout << "Hechizos de " << node->data.first_name << " " << node->data.last_name << ":\n";
    for (int i = 0; i < node->data.spells.size; ++i)
        cout << "- " << node->data.spells[i] << endl;
}