#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <cstdlib>

void createAccount() {
    int accountNumber;
    std::cout << "Enter Account Number: ";
    std::cin >> accountNumber;

    std::ifstream file;
    file.open("accounts.txt");

    int tempAccountNumber;
    std::string firstName;
    std::string lastName;
    std::string address;
    double balance;
    std::string accountType;

    while (file >> tempAccountNumber >> firstName >> lastName >> address >> balance >> accountType) {
        if (tempAccountNumber == accountNumber) {
            std::cout << "Account Number Already Exists" << std::endl;
            return;
        }
    }
    file.close();

    std::cout << "Enter First Name: ";
    std::cin >> firstName;
    std::cout << "Enter Last Name: ";
    std::cin >> lastName;
    std::cout << "Enter Address: ";
    std::cin >> address;
    std::cout << "Enter Balance: ";
    std::cin >> balance;
    std::cout << "Enter Account Type: ";
    std::cin >> accountType;

    std::ofstream outFile;
    outFile.open("accounts.txt", std::ios::app);
    outFile << accountNumber << " " << firstName << " " << lastName << " " << address << " " << balance << " " << accountType << std::endl;
    outFile.close();
}

void depositWithdraw(std::string transactionType) {
    int accountNumber;
    double amount;
    std::cout << "Enter Account Number: ";
    std::cin >> accountNumber;
    // Check if account exists
    std::ifstream file;
    file.open("accounts.txt");

    int tempAccountNumber;
    std::string firstName;
    std::string lastName;
    std::string address;
    double balance;
    std::string accountType;

    while (file >> tempAccountNumber >> firstName >> lastName >> address >> balance >> accountType) {
        if (tempAccountNumber == accountNumber) {
            break;
        }
    }
    if (tempAccountNumber != accountNumber) {
        std::cout << "Account Not Found" << std::endl;
        return;
    }
    file.close();

    std::cout << "Enter Amount: ";
    std::cin >> amount;

    file.open("accounts.txt");
    std::ofstream temp;
    temp.open("temp.txt");

    while (file >> tempAccountNumber >> firstName >> lastName >> address >> balance >> accountType) {
        if (tempAccountNumber == accountNumber) {
            if (transactionType == "deposit") {
                balance += amount;
            } else if (transactionType == "withdraw") {
                if (balance - amount < 0) {
                    std::cout << "Insufficient Funds" << std::endl;
                    return;
                } else {
                    balance -= amount;
                }
            }
        }
        temp << tempAccountNumber << " " << firstName << " " << lastName << " " << address << " " << balance << " " << accountType << std::endl;
    }
    file.close();
    temp.close();
    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");
}

void balanceInquiry() {
    int accountNumber;
    std::cout << "Enter Account Number: ";
    std::cin >> accountNumber;

    std::ifstream file;
    file.open("accounts.txt");

    int tempAccountNumber;
    std::string firstName;
    std::string lastName;
    std::string address;
    double balance;
    std::string accountType;

    while (file >> tempAccountNumber >> firstName >> lastName >> address >> balance >> accountType) {
        if (tempAccountNumber == accountNumber) {
            std::cout << "Account Number: " << tempAccountNumber << std::endl;
            std::cout << "First Name: " << firstName << std::endl;
            std::cout << "Last Name: " << lastName << std::endl;
            std::cout << "Address: " << address << std::endl;
            std::cout << "Balance: " << balance << std::endl;
            std::cout << "Account Type: " << accountType << std::endl;
            return;
        }
    }
    std::cout << "Account Not Found" << std::endl;
    file.close();
}

int main() {
    int choice;
    std::cout << "1. Create Account" << std::endl;
    std::cout << "2. Deposit Money" << std::endl;
    std::cout << "3. Withdraw Money" << std::endl;
    std::cout << "4. Balance Inquiry" << std::endl;
    std::cout << "Enter Choice: ";
    std::cin >> choice;

    switch (choice) {
        case 1:
            createAccount();
            break;
        case 2:
            depositWithdraw("deposit");
            break;
        case 3:
            depositWithdraw("withdraw");
            break;
        case 4:
            balanceInquiry();
            break;
        default:
            std::cout << "Invalid Choice" << std::endl;
            break;
    }
    return 0;
}