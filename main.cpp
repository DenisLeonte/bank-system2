#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <ctime>
#include "encrypt.h"
#ifdef __APPLE__
#include <unistd.h>
#include <term.h>
#include <stdlib.h>
#elif defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#include <windows.h>
#endif
#pragma warning(disable : 4996)

//Asta din cauza ca mai scriu si pe un macbook

using namespace std;

int m_code = 0, sel_user = 0;
bool ok = false;

//Nu stiu dece am facut asta
struct cred
{
    string password = "";
    int code = 0;
};

//Functie generalizata pentru a sterge ecranul. Merge si pe windows/mac-os
void clear()
{
#ifdef __APPLE__
    if (!cur_term)
    {
        int results;
        setupterm(NULL, STDOUT_FILENO, &results);
        if (results <= 0) return;
    }
    putp(tigetstr("clear"));
#endif
#ifdef _WIN32 || _WIN64 || WIN32 || WIN64
    system("clear");
#endif
}

//Functie generalizata pentru pune in pauza programul. Merge si pe windows/mac-os
void pause()
{
#ifdef __APPLE__
    system("read -n 1 -s -p \"Press any key to continue\"");
#endif
#ifdef _WIN32 || _WIN64
    system("pause");
#endif
}

//Clasa principala a programului.
class account
{
public:
    //Valoriile folositoare contului
    float balance = 0.0f;
    std::string name = "", surname = "";
    int code = 0;
    //Afiseaza cantitatea de bani in cont
    void printBal() { clear(); cout << "Current balance : " << balance << endl; }
    //Adauga bani in cont
    void addBal();
    //Scoate bani din cont
    void remBal();
    //Afiseaza informatiile contului
    void printInfo();
    //Modifica informatiile din cont
    void modify();
    //Sterge contul
    void delete_();
    //Functie de transcriere pentru operatile de depozitare si retragere cash
    void private_transcribe(float x, bool in);
};

void account::private_transcribe(float x,bool in)
{
    string fileName = "transactions\\" + surname + "_" + name + ".txt";
    ofstream out(fileName.c_str(), ios::app);
    time_t now = time(0);
    tm* ltm = localtime(&now);
    out << 1 << " " << 1900 + ltm->tm_year << " " << 1 + ltm->tm_mon << " " << ltm->tm_mday << " " << ltm->tm_hour << " " << ltm->tm_min << " " << in << " " << x << endl;
    out.close();
}

void account::addBal()
{
    clear();
    printBal();
    float x;
    cout << "Deposit value : "; cin >> x;
    //Valoare trebuie sa fie strict pozitiva
    if (x > 0)
    {
        balance += x;
        private_transcribe(x, true);
    }
    else cout << "TRANSACTION DENIED\n";
    pause();
}

void account::remBal()
{
    clear();
    printBal();
    float x;
    cout << "Cash-out value : "; cin >> x;
    //Valoarea trebuie sa apartina intervalului (0, valoare in cont]
    if (x <= balance && x > 0)
    {
        balance -= x;
        private_transcribe(x, false);
    }
    else
        cout << "TRANSACTION DENIED\n";
    pause();
}

//Functie de "refresh" a contului. Apelata la fiecare operatie executata. Practic Se duce in fisier si selecteaza contul dupa cod-ul lui
account select_account(int x)
{
    account aux;
    ifstream in("database.txt");
    while (in >> aux.code >> aux.name >> aux.surname >> aux.balance)
    {
        if (aux.code == x) break;
    }
    in.close();
    return aux;
}

/*Functie vitala. Poate modifica informatiile contului din fisier.
  Este apelata de fiecare data cand se modifica valoriile din cont.
  Parametrul add este folosit atunci cand inregistram un cont nou
*/
void writeToFile(account x, bool add)
{
    /*Ceea ce face este ca ia toate conturile din fisier, le baga in vector, daca s-a ajuns la contul interesat
    il baga pe x in locul lui si baga vectorul modificat inapoi in fisier
    */
    ifstream in("database.txt");
    vector<account> all;
    account aux;
    while (in >> aux.code >> aux.name >> aux.surname >> aux.balance)
    {
        if (x.code == aux.code && !add)
            all.push_back(x);
        else all.push_back(aux);
    }
    if (add)
        all.push_back(x);
    in.close();
    //Sortarea vectorului
    for (vector<account>::size_type i = 0; i != all.size() - 1; i++)
    {
        for (vector<account>::size_type j = i + 1; j != all.size(); j++)
        {
            if (all[i].code > all[j].code)
                swap(all[i], all[j]);
        }
    }
    //Scrie in fisier
    ofstream out("database.txt");
    for (auto& it : all)
    {
        out << it.code << " " << it.name << " " << it.surname << " " << it.balance << endl;
    }
    out.close();
}

void account::modify()
{
    string pass_c = "";
    int c = 0;
    clear();
    //Se cere parola pentru a modifica contul.
    string password;
    cout << "Please enter your password : "; cin >> password;
    /*Parola este cryptata SHA256, ceea ce inseamna ca odata ce a fost cryptata nu mai poate fii decryptata.
    Totusi, daca luam parola inserata, o cryptam, si o comparam cu parola deja cryptata din fisier
    ne putem da seama daca is la fel, chiar daca nu stim parola originala
    */
    SHA256 sha;
    sha.update(password);
    uint8_t* digest = sha.digest();
    password = SHA256::toString(digest);
    delete[] digest;
    ifstream in("accounts.txt");
    //Mergem in fisier pana gasim codul contului interesat si comparam parola
    while (in >> c >> pass_c)
    {
        if (c == code)
            break;
    }
    in.close();
    if (pass_c == password)
    {
        //Parola este ok deci ne lasa sa modificam contul
        cout << "Access granted\n";
        pause();
        clear();
        cout << "Select the information to modify\n";
        cout << "1. Name\n";
        cout << "2. Surname\n";
        cout << "3. Nothing\n";
        int op = 0;
        cin >> op;
        switch (op)
        {
        case 1:
            cout << "Enter the new name : "; cin >> name;
            cout << "Account information modified successfully\n";
            break;
        case 2:
            cout << "Enter the new surname : "; cin >> surname;
            cout << "Account information modified successfully\n";
            break;
        case 3:
            break;
        default:
            break;
        }
    }
    else
    {
        //Parola nu este ok
        cout << "WRONG PASSWORD. OPERATION DENIED\n";
    }
    pause();
}

void account::delete_()
{
    clear();
    char op;
    string name_c, surname_c, password;
    cout << "ACCOUNT DELETION MENU\n";
    cout << "ARE YOU SURE YOU WANT TO DELETE THIS ACCOUNT? ALL DATA WILL BE LOST AND THIS ACTION IS IRREVERSIBLE(Y/N)"; cin >> op;
    if (op == 'y' || op == 'Y')
    {
        //User-ul a dat confirmarea lui
        //Verificarea umana
        cout << "Name : "; cin >> name_c;
        if (name == name_c)
        {
            //Numele corect
            cout << "Surname : "; cin >> surname_c;
            if (surname == surname_c)
            {
                //Prenumele corect
                cout << "Password : "; cin >> password;
                //Cryptarea parolei curente
                SHA256 sha;
                sha.update(password);
                uint8_t* digest = sha.digest();
                password = SHA256::toString(digest);
                //Citirea din fisier
                ifstream in("accounts.txt");
                string password_c;
                int aux;
                while (in >> aux >> password_c)
                {
                    if (aux == code)
                        break;
                }
                in.close();
                if (password_c == password)
                {
                    //Parola corecta. Verificare umana completa
                    cout << "ARE YOU SURE YOU WANT TO DELETE THE ACCOUNT?(Y/N) : "; cin >> op;
                    if (op == 'Y' || op == 'y')
                    {
                        ifstream in("accounts.txt");
                        cred aux;
                        vector<cred> all;
                        //Citeste fisierul inafara de cont
                        while (in >> aux.code >> aux.password)
                        {
                            if (aux.code != code)
                                all.push_back(aux);
                        }
                        in.close();
                        //Scrie din fisier informatile din vector
                        ofstream out("accounts.txt");
                        for (auto& it : all)
                        {
                            out << it.code << " " << it.password << endl;
                        }
                        out.close();
                        //Citeste inca o data din fisier
                        ifstream in1("database.txt");
                        vector<account> aaa;
                        account a;
                        while (in1 >> a.code >> a.name >> a.surname >> a.balance)
                        {
                            if (a.code != code)
                            {
                                aaa.push_back(a);
                            }
                        }
                        in.close();
                        //Scrie inca o data din fisier
                        ofstream out1("database.txt");
                        for (auto& it : aaa)
                        {
                            out1 << it.code << " " << it.name << " " << it.surname << " " << it.balance << endl;
                        }
                        cout << "USER DELETED\n";
                    }
                }
                else
                {
                    //Parola gresita
                    cout << "WRONG PASSWORD\n";
                }
            }
            else
            {
                //Prenume gresit
                cout << "WRONG DATA\n";
            }
        }
        else
        {
            //Nume gresit
            cout << "WRONG DATA\n";
        }

    }
    else
    {
        //User-ul nu a dat confirmarea lui
    }
    pause();
}

bool login()
{
    int bal;
    string name, surname, name_c, surname_c, password;
    cred a, c;
    ifstream in("database.txt");
    clear();
    cout << "LOGIN SCREEN\n";
    cout << "USER NAME : "; cin >> name;
    cout << "USER SURNAME : "; cin >> surname;
    cout << "PASSWORD : "; cin >> a.password;
    bool found = false;
    //Aici cautam codul de referinta dupa numele si prenumele
    while (in >> c.code >> name_c >> surname_c >> bal)
    {
        if (name_c == name && surname_c == surname) {
            found = true;
            break;
        }
    }
    in.close();
    //Daca codul este gasit, continuam
    if (found) {
        /*Parola este cryptata SHA256, ceea ce inseamna ca odata ce a fost cryptata nu mai poate fii decryptata.
        Totusi, daca luam parola inserata, o cryptam, si o comparam cu parola deja cryptata din fisier
        ne putem da seama daca is la fel, chiar daca nu stim parola originala
        */
        SHA256 sha;
        sha.update(a.password);
        uint8_t* digest = sha.digest();
        string aux = SHA256::toString(digest);
        delete[] digest;
        int code;
        ifstream in("accounts.txt");
        //Aici cautam parola originala
        while (in >> code >> password)
        {
            if (code == c.code)
                break;
        }
        if (aux == password) {
            //Parola este corecta, deci functia va returna true, asadar putem continua in program
            sel_user = c.code;
            cout << "LOGIN SUCCESSFULL\n";
            pause();
            return true;
        }
        //Parola nu este corecta
        cout << "FAILED ATTEMPT TO LOGIN\n";
        pause();
        return false;
    }
    else
    {
        //Codul nu a fost gasit
        cout << "USER NAME OR SURNAME INCORRECT\n";
        pause();
        return false;
    }
}

//Creearea unui cont
void create_user()
{
    clear();
    m_code = 0;
    ifstream in("accounts.txt");
    cred aux;
    account aux1,aux2;
    string password;
    bool ok = true;
    //Verificam codul maxim existent
    while (in >> aux.code >> aux.password)
    {
        if (aux.code > m_code)
            m_code = aux.code;
    }
    in.close();
    //Introducem datele
    cout << "REGISTER SCREEN\n";
    cout << "NAME : "; cin >> aux1.name;
    cout << "SURNAME :"; cin >> aux1.surname;
    ifstream in("database.txt");
    while (in >> aux2.code >> aux2.name >> aux2.surname >> aux2.balance)
    {
        if (aux2.name == aux1.name && aux2.surname == aux1.surname)
        {
            ok = false;
            break;
        }
    }
    if (ok)
    {
        cout << "PASSWORD : "; cin >> aux.password;
        cout << "REPEAT THE PASSWORD : "; cin >> password;
        aux1.balance = 0;
        if (password == aux.password)
        {
            //Creste valoarea codului cel mai mare cu 1
            m_code++;
            aux1.code = m_code;
            //Cryptarea parolei in SHA256
            SHA256 sha;
            sha.update(aux.password);
            uint8_t* digest = sha.digest();
            string aux2 = SHA256::toString(digest);
            delete[] digest;
            aux.password = aux2;
            aux.code = m_code;
            //Scriem in fisier codul si parola
            ofstream out("accounts.txt", ios::app);
            out << aux.code << " " << aux.password << endl;
            out.close();
            //Scriem in fisier datele contului, cu parametrul true din cauza ca adaugam un cont
            writeToFile(aux1, true);
            cout << "USER REGISTERED SUCCESFULLY. YOU CAN NOW LOGIN\n";
        }
        //Parola este gresita
        else cout << "PASSWORDS MUST BE THE SAME\n";
    }
    else
    {
        cout << "USER ALREADY REGISTERED\n";
    }
    pause();
}

//Functie care va tine minte toate tranzactiile
void transcribe(account send, account dest, float value)
{
    string file_sender = "transactions\\" + send.surname + "_" + send.name + ".txt";
    string file_receiver = "transactions\\" + dest.surname + "_" + dest.name + ".txt";
    time_t now = time(0);
    tm* ltm = localtime(&now);
    ofstream out(file_sender.c_str(), ios::app);
    out << 0 << " " << 1900 + ltm->tm_year << " " << 1 + ltm->tm_mon << " " << ltm->tm_mday << " " << ltm->tm_hour << " " << ltm->tm_min << " " << send.name << " " << send.surname << " " << dest.name << " " << dest.surname << " " << value << endl;
    out.close();
    ofstream out1(file_receiver.c_str(), ios::app);
    out1 << 0 << " " << 1900 + ltm->tm_year << " " << 1 + ltm->tm_mon << " " << ltm->tm_mday << " " << ltm->tm_hour << " " << ltm->tm_min << " " << send.name << " " << send.surname << " " << dest.name << " " << dest.surname << " " << value << endl;
    out1.close();
}

//Functia care transfera bani dintr-un cont in altul
void transaction(account selected)
{
    clear();
    //Contul destinatar este cautat dupa nume si prenume
    cout << "State the name and surname of the destinatary\n";
    string name, surname;
    cout << "Name : "; cin >> name;
    cout << "Surname : "; cin >> surname;
    account destinatary;
    ifstream in("database.txt");
    bool ok = false;
    //Cautam contul destinatar in fisier
    while (in >> destinatary.code >> destinatary.name >> destinatary.surname >> destinatary.balance)
    {
        if (destinatary.name == name && destinatary.surname == surname)
        {
            ok = true;
            break;
        }
    }
    in.close();
    if (ok)
    {
        //Cont destinatar gasit
        float x = 0.0f;
        cout << "Insert balance to transfer : "; cin >> x;
        if (x > 0 && x <= selected.balance)
        {
            selected.balance -= x;
            destinatary.balance += x;
            cout << "Transaction succeded\n";
            //Tranzactia este efectuata, se schimba valorile din fisier
            writeToFile(selected, false);
            writeToFile(destinatary, false);
            transcribe(selected, destinatary, x);
        }
        else
        {
            //Daca valoarea inserata este mai mica ca 0 sau mai mare ca valoarea din cont va fii o eroare
            cout << "TRANSACTION DENIED. INCORRECT VALUE\n";
        }
    }
    else
    {
        //Contul nu a fost gasit
        cout << "USER NOT FOUND\n";
    }
    pause();
}

void changePass(account x)
{
    clear();
    //Verificarea identitatii prin inserarea parolei
    string password_c, password;
    bool ok = false;
    cout << "Insert current password : "; cin >> password_c;
    vector<string> a;
    //Cryptarea parolei inserate in SHA256
    SHA256 sha;
    sha.update(password_c);
    uint8_t* digest = sha.digest();
    password_c = SHA256::toString(digest);
    delete[] digest;
    int code;
    //Cautarea parolei originale in fisier
    ifstream in("accounts.txt");
    while (in >> code >> password)
    {
        if (code == x.code)
            break;
    }
    in.close();
    //Compararea parolei cu parola originala
    if (password == password_c)
    {
        ok = true;
    }
    if (ok)
    {
        //Parola este corecta, se poate schimba parola
        cout << "Enter the new password : ";
        cin >> password_c;  
        //Cryptarea noii parole
        sha.update(password_c);
        uint8_t* digest = sha.digest();
        password_c = SHA256::toString(digest);
        delete[] digest;
        //Punerea parolei in vector
        ifstream in("accounts.txt");
        while (in >> code >> password)
        {
            if (x.code == code) a.push_back(password_c);
            else a.push_back(password);
        }
        in.close();
        //Scrierea parolei in fisier
        ofstream out("accounts.txt");
        int i = 1;
        for (auto& it : a)
        {
            out << i << " " << it << endl;
            i++;
        }
        out.close();
        cout << "PASSWORD CHANGED SUCCESSFULLY\n";
    }
    else
    {
        //Parola e gresita
        cout << "OPERATION DENIED\n";
    }
    pause();
}

void account::printInfo()
{
    clear();
    cout << "Account name : " << name << endl;
    cout << "Account surname : " << surname << endl;
    cout << "Account balance : " << balance << endl;
    cout << "Account code : " << code << endl;
    pause();
}

//Functie care ne va permite sa verificam tranzactiile care vin sau iese
void check_history(account x)
{
    clear();
    string fileName = "transactions\\" + x.surname + "_" + x.name + ".txt";
    ifstream in(fileName.c_str());
    int a,year,month,day,hour,minute;
    float val;
    while (in >> a)
    {
        if (a == 0)
        {
            string s_name, s_surname, d_name, d_surname;
            in >> year >> month >> day >> hour >> minute >> s_name >> s_surname >> d_name >> d_surname >> val;
            cout << year << "/" << month << "/" << day << " " << hour << ":" << minute << " "<<s_name << " " << s_surname << "->" << val << "->" << d_name << " " << d_surname << endl;
        }
        else if(a == 1)
        {
            int op;
            in >> year >> month >> day >> hour >> minute >> op >> val;
            cout << year << "/" << month << "/" << day << " " << hour << ":" << minute << " ";
            if (op == 1) cout << "deposit ";
            else if (op == 0) cout << "cash-out ";
            cout << val<<endl;
        }
    }
    in.close();
    pause();
}

//Subprogramul principal
void loop()
{
    account selected = select_account(sel_user);
    while (ok)
    {
        clear();
        //Afisarea meniului si selectarea optiunii prin switch
        selected = select_account(sel_user);
        cout << "USER MENU\n";
        cout << "1. View balance\n";
        cout << "2. Deposit balance\n";
        cout << "3. Cash out balance\n";
        cout << "4. Transfer to another account\n";
        cout << "5. Change password\n";
        cout << "6. View transaction history\n";
        cout << "7. View account informations\n";
        cout << "8. Modify account informations\n";
        cout << "9. Delete account\n";
        cout << "0. Logout\n";
        int op = 0;
        cin >> op;
        switch (op)
        {
        case 1:
            selected.printBal();
            pause();
            break;
        case 2:
            selected.addBal();
            writeToFile(selected, false);
            break;
        case 3:
            selected.remBal();
            writeToFile(selected, false);
            break;
        case 4:
            transaction(selected);
            break;
        case 5:
            changePass(selected);
            break;
        case 6:
            check_history(selected);
            break;
        case 7:
            selected.printInfo();
            break;
        case 8:
            selected.modify();
            writeToFile(selected, false);
            break;
        case 9:
            selected.delete_();
            ok = false;
            break;
        case 0:
            ok = false;
            cout << "See you again\n";
            pause();
            break;
        }
    }
}

//Ecran de logare
void login_screen()
{
    while (!ok)
    {
        clear();
        int op = 0;
        cout << "1. Login\n";
        cout << "2. Register\n";
        cout << "0. Exit\n";
        cin >> op;
        if (op == 1)
        {
            ok = login();
        }
        else if (op == 2)
        {
            create_user();
        }
        else if (op == 0)
        {
            cout << "See you again\n";
            break;
        }
        else
        {
            cout << "INVALID CHOICE\n";
            pause();
            login_screen();
        }
        if (ok)loop();
    }
}

//main
int main()
{
    
    login_screen();
    return 0;
}

//PROIECT DE ATESTAT
/*
Facut de : Leonte Denis
Clasa : 12A
Profesor coordonator : Narcisa Stefanescu
Tema : Sistem Bancar
*/