
#include <iostream>
#include "./src/arbol.h"
using namespace std;

int main() {
    Tree<Mage> tree;
    const char* csvfile = "bin/mago.csv"; 
    tree.loadFromCSV(csvfile);

    int opcion;
    do {
        cout << "\n--- Menu ---\n";
        cout << "1. Mostrar linea de sucesion actual\n";
        cout << "2. Mostrar discipulos y maestros de un mago\n";
        cout << "3. Mostrar rama principal\n";
        cout << "4. Cambiar datos de un mago\n";
        cout << "5. Mostrar lista de hechizos de un mago\n";
        cout << "6. Agregar hechizo a un mago\n";
        cout << "7. Asignar nuevo dueno automaticamente\n";
        cout << "0. Salir\n";
        cout << "Opcion: ";
        cin >> opcion;
        if (opcion == 1) {
            tree.showSuccessionLine();
        } else if (opcion == 2) {
            int id;
            cout << "ID del mago: "; cin >> id;
            tree.showMageRelations(id);
        } else if (opcion == 3) {
            tree.showMainBranch();
        } else if (opcion == 4) {
            int id;
            cout << "ID del mago: "; cin >> id;
            Node<Mage>* node = tree.findById(id);
            if (!node) { cout << "No encontrado.\n"; continue; }
            Mage m = node->data;
            cout << "Nuevo nombre: "; cin >> m.first_name;
            cout << "Nuevo apellido: "; cin >> m.last_name;
            cout << "Genero (H/M): "; cin >> m.gender;
            cout << "Edad: "; cin >> m.age;
            cout << "Esta muerto? (0/1): "; int dead; cin >> dead; m.is_dead = dead;
            cout << "Tipo de magia: "; cin >> m.type_magic;
            cout << "Es dueno? (0/1): "; int owner; cin >> owner; m.is_owner = owner;
            cout << "¿Confirmar cambio (1) o deshacer (0)?: ";
            int confirm;
            cin >> confirm;
            if (confirm == 1) {
                tree.changeMageData(id, m);
                // Guardar el árbol en el archivo CSV recorriendo en preorden
                ofstream out("bin/mago.csv");
                out << "id,first_name,last_name,gender,age,id_father,is_dead,type_magic,is_owner\n";
                auto saveNode = [&](auto&& self, Node<Mage>* node) -> void {
                    if (!node) return;
                    Mage& mg = node->data;
                    out << mg.id << "," << mg.first_name << "," << mg.last_name << "," << mg.gender << "," << mg.age << "," << mg.id_father << "," << (mg.is_dead ? 1 : 0) << "," << mg.type_magic << "," << (mg.is_owner ? 1 : 0) << "\n";
                    self(self, node->left);
                    self(self, node->right);
                };
                saveNode(saveNode, tree.findById(1));
                out.close();
                cout << "Datos cambiados y guardados en el archivo CSV.\n";
            } else {
                cout << "Cambios descartados.\n";
            }
        } else if (opcion == 5) {
            int id;
            cout << "ID del mago: "; cin >> id;
            tree.showMageSpells(id);
        } else if (opcion == 6) {
            int id;
            cout << "ID del mago: "; cin >> id;
            string spell;
            cout << "Nombre del hechizo: "; cin >> spell;
            tree.addSpellToMage(id, spell);
        } else if (opcion == 7) {
    tree.assignNewOwner();
    ofstream out("bin/mago.csv");
    if (!out.is_open()) {
        cout << "No se pudo abrir el archivo CSV.\n";
    } else {
        out << "id,first_name,last_name,gender,age,id_father,is_dead,type_magic,is_owner\n";

        auto saveNode = [&](auto&& self, Node<Mage>* node) -> void {
            if (!node) return;
            Mage& mg = node->data;
            out << mg.id << "," << mg.first_name << "," << mg.last_name << "," << mg.gender << ","
                << mg.age << "," << mg.id_father << "," << (mg.is_dead ? 1 : 0) << ","
                << mg.type_magic << "," << (mg.is_owner ? 1 : 0) << "\n";
            self(self, node->left);
            self(self, node->right);
        };

        saveNode(saveNode, tree.findById(1));
        out.close();
        cout << "Dueno reasignado y cambios guardados\n";
    }
}
    } while (opcion != 0);

    return 0;
}