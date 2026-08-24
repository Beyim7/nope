#include <iostream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <fstream>

using namespace std;

// ---------- simple validation helpers ----------
bool validateEmail(const string &email)
{
    bool hasAt  = false;
    bool hasDot = false;
    for (int i = 0; i < (int)email.size(); i++)
    {
        if (email[i] == '@') hasAt = true;
        if (email[i] == '.' && hasAt) hasDot = true;
    }
    return (hasAt && hasDot);
}

bool validatePhone(const string &phone)
{
    int length = (int)phone.size();
    for (int i = 0; i < length; i++)
    {
        if (!isdigit(phone[i]) && phone[i] != '-' && phone[i] != '+')
            return false;
    }
    return (length >= 10 && length <= 15);
}

// ================= Person (Abstract Base Class) =================
class Person
{
protected:
    int    id;
    string name;
    string joinDate;

    static int totalPersons;   // Static member: shared count across all Person objects

public:
    Person()
    {
        id = 0;
        name = "";
        joinDate = "";
        totalPersons++;
    }

    virtual ~Person() { totalPersons--; }

    // Pure virtual functions => makes Person abstract
    virtual void display()                 const = 0;
    virtual void saveToFile(ofstream &ofs) const = 0;
    virtual void loadFromFile(ifstream &ifs)     = 0;

    int    getID()       const { return id; }
    string getName()     const { return name; }
    string getJoinDate() const { return joinDate; }

    void setID(int i)             { id = i; }
    void setName(string n)        { name = n; }
    void setJoinDate(string d)    { joinDate = d; }

    static int getTotalPersons() { return totalPersons; }

    // Friend function declaration
    friend void displayPersonSummary(const Person &p);
};

int Person::totalPersons = 0;

// Friend function definition
void displayPersonSummary(const Person &p)
{
    cout << "\t\t[Friend] ID: " << p.id
         << "  Name: "         << p.name
         << "  Joined: "       << p.joinDate << endl;
}

// ================= Member (Inherits from Person) =================
class Member : public Person
{
private:
    string email;
    string phoneNumber;
    string membershipType;
    float  monthlyFee;
    string status;
    int    attendanceCount;

    static int totalMembers;

public:
    Member()
    {
        email = "";
        phoneNumber = "";
        membershipType = "";
        monthlyFee = 0.0f;
        status = "Active";
        attendanceCount = 0;
        totalMembers++;
    }

    ~Member() { totalMembers--; }

    // --- Getters ---
    string getEmail()          const { return email; }
    string getPhone()          const { return phoneNumber; }
    string getMembershipType() const { return membershipType; }
    float  getMonthlyFee()     const { return monthlyFee; }
    string getStatus()         const { return status; }
    int    getAttendance()     const { return attendanceCount; }

    static int getTotalMembers() { return totalMembers; }

    // --- Setters ---
    void setEmail(string e)          { email = e; }
    void setPhone(string p)          { phoneNumber = p; }
    void setMembershipType(string t) { membershipType = t; }
    void setMonthlyFee(float f)      { monthlyFee = f; }
    void setStatus(string s)         { status = s; }
    void setAttendance(int a)        { attendanceCount = a; }

    // --- Polymorphic display (overrides pure virtual) ---
    void display() const override
    {
        cout << "\t\t| ID: "              << id                                          << endl;
        cout << "\t\t| Name: "            << name                                        << endl;
        cout << "\t\t| Email: "           << email                                       << endl;
        cout << "\t\t| Phone: "           << phoneNumber                                 << endl;
        cout << "\t\t| Membership: "      << membershipType                              << endl;
        cout << "\t\t| Monthly Fee: Rs. " << fixed << setprecision(2) << monthlyFee     << endl;
        cout << "\t\t| Join Date: "       << joinDate                                    << endl;
        cout << "\t\t| Status: "          << status                                      << endl;
        cout << "\t\t| Attendance Days: " << attendanceCount                             << endl;
    }

    // --- File Handling: one member = one line, fields separated by '|' ---
    void saveToFile(ofstream &ofs) const override
    {
        ofs << id << "|" << name << "|" << email << "|" << phoneNumber << "|"
            << membershipType << "|" << monthlyFee << "|" << joinDate << "|"
            << status << "|" << attendanceCount << "\n";
    }

    void loadFromFile(ifstream &ifs) override
    {
        string line;
        getline(ifs, line);

        string parts[9];
        int index = 0;
        string current = "";

        for (char c : line)
        {
            if (c == '|')
            {
                parts[index++] = current;
                current = "";
            }
            else
            {
                current += c;
            }
        }
        parts[index] = current;

        id              = stoi(parts[0]);
        name            = parts[1];
        email           = parts[2];
        phoneNumber     = parts[3];
        membershipType  = parts[4];
        monthlyFee      = stof(parts[5]);
        joinDate        = parts[6];
        status          = parts[7];
        attendanceCount = stoi(parts[8]);
    }
};

int Member::totalMembers = 0;

// ================= File Manager =================
class FileManager
{
private:
    string memberFile;

public:
    FileManager() { memberFile = "members.txt"; }

    void saveMembers(Member members[], int count) const
    {
        try
        {
            ofstream ofs(memberFile);
            if (!ofs.is_open())
                throw runtime_error("Cannot open members file for writing.");

            ofs << count << "\n";
            for (int i = 0; i < count; i++)
                members[i].saveToFile(ofs);

            ofs.close();
        }
        catch (const runtime_error &e)
        {
            cout << "\n\t\t~ File Error: " << e.what() << " ~\n";
        }
    }

    int loadMembers(Member members[]) const
    {
        int count = 0;
        try
        {
            ifstream ifs(memberFile);
            if (!ifs.is_open())
                throw runtime_error("Members file not found. Starting fresh.");

            ifs >> count;
            ifs.ignore();
            for (int i = 0; i < count; i++)
                members[i].loadFromFile(ifs);

            ifs.close();
        }
        catch (const runtime_error &e)
        {
            cout << "\n\t\t~ " << e.what() << " ~\n";
            count = 0;
        }
        return count;
    }
};

// ================= Gym System (main controller) =================
class GymSystem
{
private:
    Member      members[100];
    int         memberCount;
    FileManager fileManager;

    int findMemberByID(int id) const
    {
        for (int i = 0; i < memberCount; i++)
            if (members[i].getID() == id) return i;
        return -1;
    }

    void clearScreen() const { system("cls"); }
    void pauseScreen() const { system("pause"); }

    void displayHeader() const
    {
        cout << "\n";
        cout << "\t\t+-----------------------------------------------+\n";
        cout << "\t\t|       GYM MEMBERSHIP MANAGEMENT SYSTEM        |\n";
        cout << "\t\t|             Membership Platform               |\n";
        cout << "\t\t+-----------------------------------------------+\n\n";
    }

    void displaySeparator() const
    {
        cout << "\t\t+-----------------------------------------------+\n";
    }

    // ---------- Add Member ----------
    void addMember()
    {
        clearScreen();
        displayHeader();
        displaySeparator();

        if (memberCount >= 100)
        {
            cout << "\n\t\t~ Member list is full! Cannot add more members. ~\n";
            pauseScreen();
            return;
        }

        cout << "\t\t\t         ADD NEW MEMBER\n";
        displaySeparator();

        try
        {
            int    tempID;
            string tempName, tempEmail, tempPhone, tempDate, tempType;
            int    membershipChoice;
            float  tempFee;

            cout << "\n\t\tEnter Member ID: ";
            if (!(cin >> tempID)) throw invalid_argument("Invalid ID input.");
            cin.ignore();

            if (findMemberByID(tempID) != -1)
            {
                cout << "\n\t\t~ Member with this ID already exists! ~\n";
                pauseScreen();
                return;
            }

            cout << "\t\tEnter Full Name: ";
            getline(cin, tempName);

            cout << "\t\tEnter Email Address: ";
            getline(cin, tempEmail);
            if (!validateEmail(tempEmail))
            {
                cout << "\n\t\t~ Invalid email format! ~\n";
                pauseScreen();
                return;
            }

            cout << "\t\tEnter Phone Number: ";
            getline(cin, tempPhone);
            if (!validatePhone(tempPhone))
            {
                cout << "\n\t\t~ Invalid phone number format! ~\n";
                pauseScreen();
                return;
            }

            cout << "\t\tSelect Membership Type:\n";
            cout << "\t\t[1] Basic (Rs. 2000/month)\n";
            cout << "\t\t[2] Standard (Rs. 4000/month)\n";
            cout << "\t\t[3] Premium (Rs. 6000/month)\n";
            cout << "\t\tEnter choice (1-3): ";
            if (!(cin >> membershipChoice)) throw invalid_argument("Invalid membership choice.");
            cin.ignore();

            if      (membershipChoice == 1) { tempType = "Basic";    tempFee = 2000.0f; }
            else if (membershipChoice == 2) { tempType = "Standard"; tempFee = 4000.0f; }
            else if (membershipChoice == 3) { tempType = "Premium";  tempFee = 6000.0f; }
            else
            {
                cout << "\n\t\t~ Invalid membership type! ~\n";
                pauseScreen();
                return;
            }

            cout << "\t\tEnter Join Date (DD/MM/YYYY): ";
            getline(cin, tempDate);

            members[memberCount].setID(tempID);
            members[memberCount].setName(tempName);
            members[memberCount].setEmail(tempEmail);
            members[memberCount].setPhone(tempPhone);
            members[memberCount].setMembershipType(tempType);
            members[memberCount].setMonthlyFee(tempFee);
            members[memberCount].setJoinDate(tempDate);
            members[memberCount].setStatus("Active");
            members[memberCount].setAttendance(0);
            memberCount++;

            fileManager.saveMembers(members, memberCount);

            cout << "\n\t\t~ Member added successfully!\n";
            cout << "\t\tTotal Members: " << memberCount << endl;
        }
        catch (const invalid_argument &e)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\n\t\t~ Input Error: " << e.what() << " ~\n";
        }
        catch (...)
        {
            cout << "\n\t\t~ An unexpected error occurred! ~\n";
        }

        pauseScreen();
    }

    // ---------- Display All Members ----------
    void displayAllMembers() const
    {
        clearScreen();
        displayHeader();

        if (memberCount == 0)
        {
            displaySeparator();
            cout << "\t\t~ No members in the system! ~\n";
            displaySeparator();
            pauseScreen();
            return;
        }

        displaySeparator();
        cout << "\t\t\t     ALL MEMBERS LIST (" << memberCount << " members)\n";
        displaySeparator();

        for (int i = 0; i < memberCount; i++)
        {
            cout << "\n\t\t+- Member #" << (i + 1) << " -----------------------------------+\n";
            members[i].display();   // Polymorphic call
            cout << "\t\t+-----------------------------------------------+\n";
        }

        displaySeparator();
        pauseScreen();
    }

    // ---------- Search Member ----------
    void searchMemberByID() const
    {
        clearScreen();
        displayHeader();
        displaySeparator();

        cout << "\t\t\t         SEARCH MEMBER BY ID\n";
        displaySeparator();

        try
        {
            int searchID;
            cout << "\n\t\tEnter Member ID to search: ";
            if (!(cin >> searchID)) throw invalid_argument("Invalid ID input.");
            cin.ignore();

            int index = findMemberByID(searchID);
            if (index == -1)
            {
                cout << "\n\t\t~ Member not found! ~\n";
            }
            else
            {
                displaySeparator();
                cout << "\t\t\t         MEMBER DETAILS\n";
                displaySeparator();
                cout << "\n\t\t+- Member Found --------------------------------+\n";
                members[index].display();
                cout << "\t\t+-----------------------------------------------+\n";
            }
        }
        catch (const invalid_argument &e)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\n\t\t~ Input Error: " << e.what() << " ~\n";
        }

        displaySeparator();
        pauseScreen();
    }

    // ---------- Update Member ----------
    void updateMember()
    {
        clearScreen();
        displayHeader();
        displaySeparator();

        cout << "\t\t           UPDATE MEMBER INFORMATION\n";
        displaySeparator();

        try
        {
            int searchID;
            cout << "\n\t\tEnter Member ID to update: ";
            if (!(cin >> searchID)) throw invalid_argument("Invalid ID input.");
            cin.ignore();

            int index = findMemberByID(searchID);
            if (index == -1)
            {
                cout << "\n\t\t~ Member not found! ~\n";
                pauseScreen();
                return;
            }

            cout << "\n\t\tCurrent Information:\n";
            cout << "\t\t+-----------------------------------------------+\n";
            cout << "\t\t| Name: "   << members[index].getName()   << endl;
            cout << "\t\t| Email: "  << members[index].getEmail()  << endl;
            cout << "\t\t| Phone: "  << members[index].getPhone()  << endl;
            cout << "\t\t| Status: " << members[index].getStatus() << endl;
            cout << "\t\t+-----------------------------------------------+\n";

            cout << "\n\t\tWhat do you want to update?\n";
            cout << "\t\t[1] Name\n";
            cout << "\t\t[2] Email\n";
            cout << "\t\t[3] Phone Number\n";
            cout << "\t\t[4] Status (Active/Inactive/Suspended)\n";
            cout << "\t\t[5] Attendance Count\n";
            cout << "\t\t[6] Back\n";
            cout << "\t\tEnter your choice: ";

            int updateChoice;
            if (!(cin >> updateChoice)) throw invalid_argument("Invalid choice.");
            cin.ignore();

            if (updateChoice == 1)
            {
                string newName;
                cout << "\t\tEnter new name: ";
                getline(cin, newName);
                members[index].setName(newName);
                cout << "\n\t\t~ Name updated successfully!\n";
            }
            else if (updateChoice == 2)
            {
                string newEmail;
                cout << "\t\tEnter new email: ";
                getline(cin, newEmail);
                if (validateEmail(newEmail))
                {
                    members[index].setEmail(newEmail);
                    cout << "\n\t\t~ Email updated successfully!\n";
                }
                else cout << "\n\t\t~ Invalid email format! ~\n";
            }
            else if (updateChoice == 3)
            {
                string newPhone;
                cout << "\t\tEnter new phone number: ";
                getline(cin, newPhone);
                if (validatePhone(newPhone))
                {
                    members[index].setPhone(newPhone);
                    cout << "\n\t\t~ Phone number updated successfully!\n";
                }
                else cout << "\n\t\t~ Invalid phone number format! ~\n";
            }
            else if (updateChoice == 4)
            {
                int statusChoice;
                cout << "\t\t[1] Active\n";
                cout << "\t\t[2] Inactive\n";
                cout << "\t\t[3] Suspended\n";
                cout << "\t\tEnter choice: ";
                cin >> statusChoice;
                cin.ignore();

                if      (statusChoice == 1) members[index].setStatus("Active");
                else if (statusChoice == 2) members[index].setStatus("Inactive");
                else if (statusChoice == 3) members[index].setStatus("Suspended");
                else { cout << "\n\t\t~ Invalid choice! ~\n"; pauseScreen(); return; }

                cout << "\n\t\t~ Status updated successfully!\n";
            }
            else if (updateChoice == 5)
            {
                int newAtt;
                cout << "\t\tEnter new attendance count: ";
                cin >> newAtt;
                cin.ignore();
                members[index].setAttendance(newAtt);
                cout << "\n\t\t~ Attendance updated successfully!\n";
            }
            else if (updateChoice == 6)
            {
                // back to menu, nothing to do
            }
            else
            {
                cout << "\n\t\t~ Invalid choice! ~\n";
            }

            fileManager.saveMembers(members, memberCount);
        }
        catch (const invalid_argument &e)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\n\t\t~ Input Error: " << e.what() << " ~\n";
        }

        pauseScreen();
    }

    // ---------- Delete Member ----------
    void deleteMember()
    {
        clearScreen();
        displayHeader();
        displaySeparator();

        cout << "\t\t\t          DELETE MEMBER\n";
        displaySeparator();

        try
        {
            int searchID;
            cout << "\n\t\tEnter Member ID to delete: ";
            if (!(cin >> searchID)) throw invalid_argument("Invalid ID input.");
            cin.ignore();

            int index = findMemberByID(searchID);
            if (index == -1)
            {
                cout << "\n\t\t~ Member not found! ~\n";
                pauseScreen();
                return;
            }

            cout << "\n\t\tAre you sure you want to delete this member?\n";
            cout << "\t\tName: " << members[index].getName() << endl;
            cout << "\t\t(This action cannot be undone!)\n";
            cout << "\t\t[1] Yes, Delete\n";
            cout << "\t\t[2] No, Cancel\n";
            cout << "\t\tEnter your choice: ";

            int confirmChoice;
            cin >> confirmChoice;
            cin.ignore();

            if (confirmChoice == 1)
            {
                for (int i = index; i < memberCount - 1; i++)
                    members[i] = members[i + 1];
                memberCount--;
                fileManager.saveMembers(members, memberCount);
                cout << "\n\t\t~ Member deleted successfully!\n";
                cout << "\t\tRemaining Members: " << memberCount << endl;
            }
            else
            {
                cout << "\n\t\t~ Deletion cancelled.\n";
            }
        }
        catch (const invalid_argument &e)
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "\n\t\t~ Input Error: " << e.what() << " ~\n";
        }

        pauseScreen();
    }

    // ---------- Member Menu ----------
    void memberMenu()
    {
        int  choice;
        bool menuRunning = true;

        while (menuRunning)
        {
            clearScreen();
            displayHeader();
            displaySeparator();
            cout << "\t\t              MEMBER MEMBERSHIP MENU\n";
            displaySeparator();
            cout << "\t\t[1] Add New Member \n";
            cout << "\t\t[2] View All Members \n";
            cout << "\t\t[3] Search Member by ID \n";
            cout << "\t\t[4] Update Member Information \n";
            cout << "\t\t[5] Delete Member \n";
            cout << "\t\t[6] Back to Main Menu\n";
            displaySeparator();

            cout << "\t\tEnter your choice: ";
            cin >> choice;
            cin.ignore();

            if      (choice == 1) addMember();
            else if (choice == 2) displayAllMembers();
            else if (choice == 3) searchMemberByID();
            else if (choice == 4) updateMember();
            else if (choice == 5) deleteMember();
            else if (choice == 6) menuRunning = false;
            else
            {
                cout << "\n\t\t~ Invalid choice! Please try again. ~\n";
                pauseScreen();
            }
        }
    }

    void mainMenu() const
    {
        displaySeparator();
        cout << "\t\t\t            MAIN MENU\n";
        displaySeparator();
        cout << "\t\t[1] Member Membership \n";
        cout << "\t\t[2] Exit System\n";
        displaySeparator();
    }

public:
    // Constructor: load saved data on startup
    GymSystem() : memberCount(0)
    {
        memberCount = fileManager.loadMembers(members);
    }

    void run()
    {
        system("color 0B");
        int  choice;
        bool running = true;

        while (running)
        {
            clearScreen();
            displayHeader();
            mainMenu();

            cout << "\n\t\tEnter your choice: ";
            cin  >> choice;
            cin.ignore();

            if (choice == 1)
            {
                memberMenu();
            }
            else if (choice == 2)
            {
                cout << "\n\t\t || Thank you for using Gym Membership System || \n";
                cout << "\t\t                  || Goodbye! || \n";
                running = false;
            }
            else
            {
                cout << "\n\t\t~ Invalid choice! Please try again. ~\n";
                pauseScreen();
            }
        }
    }
};

// ================= Main =================
int main()
{
    GymSystem gym;
    gym.run();
    return 0;
}
