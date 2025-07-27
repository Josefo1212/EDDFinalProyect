#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

// Lista dinámica simple para strings (hechizos)
class StringList {
public:
    string* data;
    int size;
    int capacity;
    StringList() : data(nullptr), size(0), capacity(0) {}
    ~StringList() { delete[] data; }
    void push_back(const string& s) {
        if (size == capacity) {
            int newcap = capacity == 0 ? 2 : capacity * 2;
            string* nd = new string[newcap];
            for (int i = 0; i < size; ++i) nd[i] = data[i];
            delete[] data;
            data = nd;
            capacity = newcap;
        }
        data[size++] = s;
    }
    string& operator[](int i) { return data[i]; }
};

// Estructura para hijos (máximo 2 por nodo)
struct ChildList {
    int ids[2];
    int count;
    ChildList() : count(0) { ids[0] = ids[1] = 0; }
    void add(int id) { if (count < 2) ids[count++] = id; }
};

// Clase que representa a un mago
class Mage {
public:
    int id;
    string first_name;
    string last_name;
    char gender; // 'H' o 'M'
    int age;
    int id_father;
    bool is_dead;
    string type_magic; // elemental, unique, mixed, no_magic
    bool is_owner;
    StringList spells; // Lista de hechizos

    Mage() {}
    Mage(int id, string first_name, string last_name, char gender, int age, int id_father, bool is_dead, string type_magic, bool is_owner)
        : id(id), first_name(first_name), last_name(last_name), gender(gender), age(age), id_father(id_father), is_dead(is_dead), type_magic(type_magic), is_owner(is_owner) {}

    // Agregar método para añadir hechizo
    void addSpell(const string& spell) {
        spells.push_back(spell);
    }
};

// Declaración adelantada de Node para Tree
template<class T>
class Node;

// Nodo del árbol
template<class T>
class Node {
public:
    T data;
    Node<T>* left;
    Node<T>* right;
    Node(T data, Node<T>* left = NULL, Node<T>* right = NULL) : data(data), left(left), right(right) {}
    T& getData() { return data; }
    Node<T>* getChild(int dir) { return dir == 0 ? left : right; }
    void setLeft(Node<T>* l) { left = l; }
    void setRight(Node<T>* r) { right = r; }
    void setData(T d) { data = d; }
    void print() {
        cout << data.id << " " << data.first_name << " " << data.last_name << " " << data.age << " " << data.type_magic << (data.is_owner ? " (OWNER)" : "") << endl;
    }
};

template<class T>
class Tree {
private:
    Node<T>* root = NULL;
public:
    // Solo lo necesario para el programa de magos
    void loadFromCSV(const char* filename);
    Node<T>* findById(int id);
    void showSuccessionLine();
    void changeMageData(int id, const Mage& newData);
    void showMageSpells(int id);
    void assignNewOwner();
    // Nuevos métodos requeridos:
    void showMageRelations(int id); // Discípulos y maestros
    void showMainBranch(); // Rama principal
    void addSpellToMage(int id, const string& spell); // Agregar hechizo
    // Puedes agregar más métodos específicos si lo necesitas
};