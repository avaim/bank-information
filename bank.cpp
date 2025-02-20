// Project identifier: 292F24D17A4455C1B5133EDD8C7CEAA0C9570A98

#include <getopt.h>
#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <deque>
#include <queue>
#include <cmath>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

using namespace std;

struct Transaction {
    uint64_t place_timestamp;
    string ip;
    string sender;
    string recipient;
    uint64_t amount;
    uint64_t exec_date;
    string fee_cover;
    uint32_t transaction_id;
    uint64_t fee_amount = 0;

    Transaction() = default;

    Transaction(uint64_t pt, const string& tip, const string& sid, const string& rid, uint64_t tamt, 
    uint64_t edate, string fc, uint32_t tid) 
    : place_timestamp(pt), ip(tip), sender(sid), recipient(rid), amount(tamt), exec_date(edate), 
    fee_cover(fc), transaction_id(tid) {}
};

struct TransactionComparator {
    bool operator()(const Transaction& t1, const Transaction& t2) {
        if (t1.exec_date != t2.exec_date) {
            return t1.exec_date > t2.exec_date;
        }
        return t1.transaction_id > t2.transaction_id;
    }
};

struct User {
    string reg_timestamp;
    uint64_t reg_time_num;
    string user_id;
    string pin;
    string balance;
    uint64_t bal_num;
    unordered_set<string> valid_ips;

    User() = default;

    User(uint64_t reg, string id, string pin_num, uint64_t bal) 
    : reg_time_num(reg), user_id(id), pin(pin_num), bal_num(bal) {}
};

class Bank {

    public:

    void get_mode (int argc, char *argv[]) {
        int option_index = 0;
        int option = 0;
        opterr = false;
        struct option longOpts[] = {
            { "help", no_argument, nullptr, 'h' },
            { "file", required_argument, nullptr, 'f' },
            { "verbose", no_argument, nullptr, 'v' },
            { nullptr, 0, nullptr, '\0' }
        };

        
        while ((option = getopt_long(argc, argv, "hf:v", longOpts, &option_index)) != -1) {
            switch (option) {
                case 'h':
                    cout << "This is 281Bank, where you can input transactions\n";
                    exit(0);
                case 'f':
                    filename = optarg;
                    break;
                case 'v':
                    verbose = true;
                    break;
                default:
                    cerr << "Error: Unknown command line option\n";
                    exit(1);
            }
    }
} // get_mode()

    void read_registration() {
        ifstream reg_file(filename, ifstream::in);
        
        if (filename.empty()) {
        cerr << "Registration file failed to open.";
        exit(1);
        }

        else {
            if (!reg_file) {
                cerr << "Registration file failed to open.";
                exit(1);
            }

            else {
                while (reg_file) {
                    // reading each line from the file
                    getline(reg_file, user_info.reg_timestamp, '|');

                    if (user_info.reg_timestamp.empty()) {
                        break;
                    }

                    // formatting registration timestamp
                    user_info.reg_timestamp.erase(remove(user_info.reg_timestamp.begin(), 
                    user_info.reg_timestamp.end(), ':'), user_info.reg_timestamp.end());

                    rtime = stoull(user_info.reg_timestamp);

                    getline(reg_file, user_info.user_id, '|');
                    getline(reg_file, user_info.pin, '|');
                    getline(reg_file, user_info.balance);
                    sbalance = stoull(user_info.balance);

                    // adds to hash map
                    user_info.reg_time_num = rtime;
                    user_info.bal_num = sbalance;
                    users.emplace(user_info.user_id, user_info);
                }
            }
        }
        reg_file.close();
    } // read_registration

    void read_input() {
        string current_time;
        string junk;
        string temp;

        if (cin.fail()) {
            cerr << "Error: Reading from cin has failed";
            exit(1);
        }

        else {
            while (temp != "$$$") {

                cin >> temp;

                if (temp.empty()) {
                    continue;
                }

                if (temp.at(0) == '#') {
                    getline(cin, junk);
                }

                if (temp.at(0) == 'l') {
                    process_login();
                }

                if (temp.at(0) == 'o') {
                    process_logout();
                }

                if (temp.at(0) == 'b') {
                    process_balance();
                }

                if (temp.at(0) == 'p') {
                    process_place();
                }
            }

            process_transaction(true);

            while (cin >> temp) {

                if (temp.empty()) {
                    continue;
                }

                if (temp.at(0) == 'l') {
                    process_list();
                }

                if (temp.at(0) == 'r') {
                    process_revenue();
                }

                if (temp.at(0) == 'h') {
                    process_history();
                }

                if (temp.at(0) == 's') {
                    process_summary();
                }
            }
        }
    } // read_input

    void process_login() {
        string luid;
        string lpin;
        string lip;

        cin >> luid >> lpin >> lip;

        auto login_iterator = users.find(luid);

        // if user exists
        if (login_iterator != users.end()) {
            
            if (login_iterator->second.pin == lpin) {

                login_iterator->second.valid_ips.insert(lip); // add IP to valid IPs set

                if (verbose) {
                    cout << "User " << luid << " logged in.\n";
                }
            }

            else {
                if (verbose) {
                    cout << "Login failed for " << luid << ".\n";
                }
                return;
            }
        }

        // user doesn't exist
        else {
            if (verbose) {
                cout << "Login failed for " << luid << ".\n";
            }
            return;
        }
    } // process_login

    void process_logout() {
        string ouid;
        string oip;

        cin >> ouid >> oip;

        auto logout_iterator = users.find(ouid);

        // if user exists and is logged in
        if (logout_iterator != users.end()) {

            auto ip_iterator = logout_iterator->second.valid_ips.find(oip);
            
            // if in valid IP list, if not then logout failed
            if (!logout_iterator->second.valid_ips.empty() && ip_iterator != logout_iterator->second.valid_ips.end()) {

                logout_iterator->second.valid_ips.erase(oip);

                if (verbose) {
                    cout << "User " << ouid << " logged out.\n";
                }
            }

            else {
                if (verbose) {
                    cout << "Logout failed for " << ouid << ".\n";
                }
                return;
            }
        }

        // if either user doesn't exist or isn't logged in
        else {
            if (verbose) {
                cout << "Logout failed for " << ouid << ".\n";
            }
            return;
        }
    } // process_logout

    void process_balance() {
        string buid;
        string bip;

        cin >> buid >> bip;

        auto balance_iterator = users.find(buid);

        if (balance_iterator == users.end()) {
            if (verbose) {
                cout << "Account " << buid << " does not exist.\n";
            }
            return;
        }

        if (balance_iterator->second.valid_ips.empty()) {
            if (verbose) {
                cout << "Account " << buid << " is not logged in.\n";
            }
            return;
        }

        if (balance_iterator->second.valid_ips.count(bip) == 0) {

            if (verbose) {
                cout << "Fraudulent transaction detected, aborting request.\n";
            }
            return;
        }

        else if (current_timestamp == 0) {
            cout << "As of " << balance_iterator->second.reg_time_num << ", " << buid 
            << " has a balance of $" << balance_iterator->second.bal_num << ".\n";

        }

        else {
            cout << "As of " << current_timestamp << ", " << buid << " has a balance of $" 
            << balance_iterator->second.bal_num << ".\n";
        }
    } // process_balance

    void process_place() {
        string ptimestamp;
        string pip;
        string psender;
        string precip;
        uint64_t pamount;
        string pexec_date;
        string pfee;

        cin >> ptimestamp >> pip >> psender >> precip >> pamount >> pexec_date >> pfee;

        // formatting place timestamp
        ptimestamp.erase(remove(ptimestamp.begin(), ptimestamp.end(), ':'), ptimestamp.end());

        ptime = stoull(ptimestamp);

        // formatting exec_date timestamp
        pexec_date.erase(remove(pexec_date.begin(), pexec_date.end(), ':'), pexec_date.end());

        pexec_num = stoull(pexec_date);

        if (pexec_num < ptime) {
            cerr << "You cannot have an execution date before the current timestamp.";
            exit(1);
        }

        current_timestamp = ptime;

        if (ptime < prev_ptime) {
            cerr << "Invalid decreasing timestamp in 'place' command.";
            exit(1);
        }

        if (psender == precip) {
            if (verbose) {
                cout << "Self transactions are not allowed.\n";
            }
            return;
        }

        // if exec_date is more than 3 days past place_timestamp
        if (pexec_num - ptime > 3000000) {
            if (verbose) {
                cout << "Select a time up to three days in the future.\n";
            }
            return;
        }

        auto sender_iterator = users.find(psender);

        if (sender_iterator == users.end()) {
            if (verbose) {
                cout << "Sender " << psender << " does not exist.\n";
            }
            return;
        }

        auto recip_iterator = users.find(precip);

        if (recip_iterator == users.end()) {
            if (verbose) {
                cout << "Recipient " << precip << " does not exist.\n";
            }
            return;
        }

        if ((pexec_num < sender_iterator->second.reg_time_num) || (pexec_num < recip_iterator->second.reg_time_num)) {
            if (verbose) {
                cout << "At the time of execution, sender and/or recipient have not registered.\n";
            }
            return;
        }

        if (sender_iterator->second.valid_ips.empty()) {
            if (verbose) {
                cout << "Sender " << psender << " is not logged in.\n";
            }
            return;
        }

        if (sender_iterator->second.valid_ips.count(pip) == 0) {
            if (verbose) {
                cout << "Fraudulent transaction detected, aborting request.\n";
            }
            return;
        }

        else {
            while (!trans_pq.empty() && trans_pq.top().exec_date <= ptime) {
                process_transaction(false);
            }

            if (verbose) {
                cout << "Transaction " << trans_id << " placed at " << ptime << ": $" << pamount 
                << " from " << psender << " to " << precip << " at " << pexec_num << ".\n";
            }

            Transaction new_trans(ptime, pip, psender, precip, pamount, pexec_num, pfee, trans_id);
            trans_pq.push(new_trans);

            process_transaction(false);

            trans_id++;
            prev_ptime = ptime;
        }
    } // process_place

    void process_transaction(bool is_final) {
        while (!trans_pq.empty()) {

            Transaction top_trans = trans_pq.top();

            if (!is_final) {
                if (top_trans.exec_date > current_timestamp) {
                    return;
                }
            }

            trans_pq.pop();

            auto find_sender = users.find(top_trans.sender);
            auto find_recip = users.find(top_trans.recipient);

            User& tsender = find_sender->second;
            User& trecip = find_recip->second;

            uint64_t fee = (top_trans.amount * 1) / 100;

            if (fee < 10) {
                fee = 10;
            }

            else if (fee > 450) {
                fee = 450;
            }

            // if the sender has been registered for at least 5 years
            if (top_trans.exec_date - tsender.reg_time_num >= 50000000000) {
                fee = (fee * 3) / 4;
            }

            top_trans.fee_amount = fee;

            bool funds_available = true;

            uint64_t sender_fee = 0;
            uint64_t recip_fee = 0;

            if (top_trans.fee_cover == "o") {
                sender_fee = fee;
                recip_fee = 0;

                funds_available = (tsender.bal_num >= (top_trans.amount + sender_fee));

                if (!funds_available) {
                    if (verbose) {
                        cout << "Insufficient funds to process transaction " << top_trans.transaction_id << ".\n";
                    }
                    continue;
                }

                else {
                    tsender.bal_num -= (top_trans.amount + fee);
                    trecip.bal_num += top_trans.amount;

                    if (verbose) {
                        cout << "Transaction " << top_trans.transaction_id << " executed at " << top_trans.exec_date 
                        << ": $" << top_trans.amount << " from " << top_trans.sender << " to " << top_trans.recipient 
                        << ".\n";
                    }
                }
            } // if o

            else if (top_trans.fee_cover == "s") {
                sender_fee = fee / 2;
                recip_fee = fee / 2;
                if (fee % 2 != 0) { // is odd
                    sender_fee++;
                }

                funds_available = ((tsender.bal_num >= (top_trans.amount + sender_fee)) && (trecip.bal_num >= recip_fee));

                if (!funds_available) {
                    if (verbose) {
                        cout << "Insufficient funds to process transaction " << top_trans.transaction_id << ".\n";
                    }
                    continue;
                }

                else {
                    tsender.bal_num -= (top_trans.amount + sender_fee);
                    trecip.bal_num -= recip_fee;
                    trecip.bal_num += top_trans.amount;

                    if (verbose) {
                        cout << "Transaction " << top_trans.transaction_id << " executed at " << top_trans.exec_date 
                        << ": $" << top_trans.amount << " from " << top_trans.sender << " to " << top_trans.recipient 
                        << ".\n";
                    }
                }
            } // if s

            all_transactions.push_back(top_trans);

        } // while loop
    }

    void process_list() {
        string x_time;
        string y_time;
        uint64_t x_time_num = 0;
        uint64_t y_time_num = 0;
        int count = 0;

        cin >> x_time >> y_time;

        x_time.erase(remove(x_time.begin(), x_time.end(), ':'), x_time.end());
        y_time.erase(remove(y_time.begin(), y_time.end(), ':'), y_time.end());

        x_time_num = stoull(x_time);
        y_time_num = stoull(y_time);

        if (x_time_num == y_time_num) {
            cout << "List Transactions requires a non-empty time interval.\n";
            return;
        }

        for (const auto& transaction : all_transactions) {
            if (transaction.exec_date >= x_time_num && transaction.exec_date < y_time_num) {
                cout << transaction.transaction_id << ": " << transaction.sender << " sent " << transaction.amount 
                << " dollar" << (transaction.amount == 1 ? "" : "s") << " to " << transaction.recipient << " at " 
                << transaction.exec_date << ".\n";

                count++;
            }
        }

        if (count == 1) {
            cout << "There was 1 transaction that was placed between time " << x_time_num << " to " << y_time_num << ".\n";
        }

        else {
            cout << "There were " << count << " transactions that were placed between time " << x_time_num << " to " << y_time_num << ".\n";
        }

    } // process_list

    void process_revenue() {
        string x_time;
        string y_time;
        uint64_t x_time_num = 0;
        uint64_t y_time_num = 0;
        uint64_t total_fees = 0;

        cin >> x_time >> y_time;

        x_time.erase(remove(x_time.begin(), x_time.end(), ':'), x_time.end());
        y_time.erase(remove(y_time.begin(), y_time.end(), ':'), y_time.end());

        x_time_num = stoull(x_time);
        y_time_num = stoull(y_time);

        if (x_time_num == y_time_num) {
            cout << "Bank Revenue requires a non-empty time interval.\n";
            return;
        }

        else {
            for (const auto& transaction : all_transactions) {
                if (transaction.exec_date >= x_time_num && transaction.exec_date < y_time_num) {
                    total_fees += transaction.fee_amount;
                }
            }

            uint64_t time_diff = y_time_num - x_time_num;

            string time_diff_string = to_string(time_diff);

            int years = 0;
            int months = 0;
            int days = 0;
            int hours = 0;
            int minutes = 0;
            int seconds = 0;

            if (time_diff_string.size() == 12) { // years
                years = stoi(time_diff_string.substr(0, 2));
                months = stoi(time_diff_string.substr(2, 2));
                days = stoi(time_diff_string.substr(4, 2));
                hours = stoi(time_diff_string.substr(6, 2));
                minutes = stoi(time_diff_string.substr(8, 2));
                seconds = stoi(time_diff_string.substr(10, 2));
            }

            else if (time_diff_string.size() == 11) {
                years = stoi(time_diff_string.substr(0, 1));
                months = stoi(time_diff_string.substr(1, 2));
                days = stoi(time_diff_string.substr(3, 2));
                hours = stoi(time_diff_string.substr(5, 2));
                minutes = stoi(time_diff_string.substr(7, 2));
                seconds = stoi(time_diff_string.substr(9, 2));
            }

            else if (time_diff_string.size() == 10) { // months
                months = stoi(time_diff_string.substr(0, 2));
                days = stoi(time_diff_string.substr(2, 2));
                hours = stoi(time_diff_string.substr(4, 2));
                minutes = stoi(time_diff_string.substr(6, 2));
                seconds = stoi(time_diff_string.substr(8, 2));
            }

            else if (time_diff_string.size() == 9) {
                months = stoi(time_diff_string.substr(0, 1));
                days = stoi(time_diff_string.substr(1, 2));
                hours = stoi(time_diff_string.substr(3, 2));
                minutes = stoi(time_diff_string.substr(5, 2));
                seconds = stoi(time_diff_string.substr(7, 2));
            }

            else if (time_diff_string.size() == 8) { // days
                days = stoi(time_diff_string.substr(0, 2));
                hours = stoi(time_diff_string.substr(2, 2));
                minutes = stoi(time_diff_string.substr(4, 2));
                seconds = stoi(time_diff_string.substr(6, 2));
            }

            else if (time_diff_string.size() == 7) {
                days = stoi(time_diff_string.substr(0, 1));
                hours = stoi(time_diff_string.substr(1, 2));
                minutes = stoi(time_diff_string.substr(3, 2));
                seconds = stoi(time_diff_string.substr(5, 2));
            }

            else if (time_diff_string.size() == 6) { // hours
                hours = stoi(time_diff_string.substr(0, 2));
                minutes = stoi(time_diff_string.substr(2, 2));
                seconds = stoi(time_diff_string.substr(4, 2));
            }

            else if (time_diff_string.size() == 5) {
                hours = stoi(time_diff_string.substr(0, 1));
                minutes = stoi(time_diff_string.substr(1, 2));
                seconds = stoi(time_diff_string.substr(3, 2));
            }

            else if (time_diff_string.size() == 4) { // minutes
                minutes = stoi(time_diff_string.substr(0, 2));
                seconds = stoi(time_diff_string.substr(2, 2));
            }

            else if (time_diff_string.size() == 3) {
                minutes = stoi(time_diff_string.substr(0, 1));
                seconds = stoi(time_diff_string.substr(1, 2));
            }

            else if (time_diff_string.size() == 2) { // seconds
                seconds = stoi(time_diff_string.substr(0, 2));
            }

            else if (time_diff_string.size() == 1) {
                seconds = stoi(time_diff_string.substr(0, 1));
            }

            vector<string> components;
            if (years > 0) components.push_back(to_string(years) + " year" + (years == 1 ? "" : "s"));
            if (months > 0) components.push_back(to_string(months) + " month" + (months == 1 ? "" : "s"));
            if (days > 0) components.push_back(to_string(days) + " day" + (days == 1 ? "" : "s"));
            if (hours > 0) components.push_back(to_string(hours) + " hour" + (hours == 1 ? "" : "s"));
            if (minutes > 0) components.push_back(to_string(minutes) + " minute" + (minutes == 1 ? "" : "s"));
            if (seconds > 0) components.push_back(to_string(seconds) + " second" + (seconds == 1 ? "" : "s"));


            cout << "281Bank has collected " << total_fees << " dollars in fees over ";
    
            for (size_t i = 0; i < components.size(); ++i) {
                cout << components[i];
                if (i < components.size() - 1) {
                    cout << " ";
                }
            }
            cout << ".\n";
        }
    } // process_revenue

    void process_history() {
        string huser_id;

        cin >> huser_id;

        if (users.find(huser_id) == users.end()) {
            cout << "User " << huser_id << " does not exist.\n";
            return; 
        }

        vector<Transaction> incoming;
        vector<Transaction> outgoing;

        const User& history_user = users[huser_id];

        // iterate through transactions to categorize them as incoming or outgoing
        for (const Transaction& htrans : all_transactions) {
            if (htrans.sender == huser_id) {
                outgoing.push_back(htrans);
            }

            else if (htrans.recipient == huser_id) {
                incoming.push_back(htrans);
            }
        }

        // sorts transactions by execution date in ascending order
        auto sort_by_date = [](vector<Transaction>& transactions) {
            sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
                return a.exec_date < b.exec_date;
            });
        };

        sort_by_date(incoming);
        sort_by_date(outgoing);

        cout << "Customer " << huser_id << " account summary:\n";
        cout << "Balance: $" << history_user.bal_num << "\n";
        cout << "Total # of transactions: " << (incoming.size() + outgoing.size()) << "\n";

        cout << "Incoming " << incoming.size() << ":\n";

        if (incoming.size() > 10) {
            incoming = vector<Transaction>(incoming.end() - 10, incoming.end());
        }

        for (const auto& incoming_trans : incoming) {
            cout << incoming_trans.transaction_id << ": " << incoming_trans.sender << " sent " 
            << incoming_trans.amount << " dollar" << (incoming_trans.amount == 1 ? "" : "s") << " to " 
            << incoming_trans.recipient << " at " << incoming_trans.exec_date << ".\n";
        }

        cout << "Outgoing " << outgoing.size() << ":\n";

        if (outgoing.size() > 10) {
            outgoing = vector<Transaction>(outgoing.end() - 10, outgoing.end());
        }

        for (const auto& outgoing_trans : outgoing) {
            cout << outgoing_trans.transaction_id << ": " << outgoing_trans.sender << " sent " 
            << outgoing_trans.amount << " dollar" << (outgoing_trans.amount == 1 ? "" : "s") << " to "
            << outgoing_trans.recipient << " at " << outgoing_trans.exec_date << ".\n";
        }
    } // process_history

    void process_summary() {
        string sumtime;
        uint64_t sumtime_num;

        cin >> sumtime;

        sumtime.erase(remove(sumtime.begin(), sumtime.end(), ':'), sumtime.end());

        sumtime_num = stoull(sumtime);

        uint64_t start_time = sumtime_num / 1000000 * 1000000; // sets hours, minutes, seconds to 0
        uint64_t end_time = start_time + 1000000;

        cout << "Summary of [" << start_time << ", " << end_time << "):\n";

        int transaction_count = 0;
        uint64_t total_fees = 0;

        // loop through all transactions within time range
        for (const auto& transaction : all_transactions) {
            if (transaction.exec_date >= start_time && transaction.exec_date < end_time) {
                cout << transaction.transaction_id << ": " << transaction.sender << " sent " << transaction.amount 
                << " dollar" << (transaction.amount == 1 ? "" : "s") << " to " << transaction.recipient << " at " 
                << transaction.exec_date << ".\n";

                transaction_count++;
                total_fees += transaction.fee_amount;
            }
        }

        if (transaction_count == 1) {
            cout << "There was a total of 1 transaction, 281Bank has collected " << total_fees << " dollars in fees.\n";
        }

        else {
            cout << "There were a total of " << transaction_count << " transactions, 281Bank has collected " << total_fees << " dollars in fees.\n";
        }

    }

    private:

    bool verbose = false;
    uint64_t rtime = 0;
    uint64_t ptime = 0;
    uint64_t pexec_num = 0;
    uint64_t sbalance = 0;
    uint64_t prev_ptime = 0;
    uint64_t current_timestamp = 0;
    uint32_t trans_id = 0;
    string filename;
    User user_info;
    unordered_map<string, User> users; // key type will be user_id
    priority_queue<Transaction, vector<Transaction>, TransactionComparator> trans_pq;
    vector<Transaction> all_transactions; // stores all executed transactions

};

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "C");
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);
    Bank bank;
    bank.get_mode(argc, argv);
    bank.read_registration();
    bank.read_input();
    return 0;
}